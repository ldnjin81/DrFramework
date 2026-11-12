# 묶음 PSO 캐시(<프로젝트>\Build\Windows\PipelineCaches\PSO_<프로젝트>_<형식>.spc)를 만든다.
# 순서: package_win64.ps1(쿡이 셰이더 고정 키 .shk를 남김) → 이 스크립트 → package_win64.ps1(캐시를 넣어 다시 패키징).
# 전제: Config/DefaultEngine.ini [DevOptions.Shaders] NeedsShaderStableKeys=true (없으면 .shk가 생기지 않는다).
# 패키지를 -logPSO로 스모크·성능 경로대로 돌려 실제로 쓰인 PSO를 모은다. 셰이더·머티리얼·FX가 크게 바뀌면 다시 돌린다.
# 사용: .\Tools\pso_bundle_win64.ps1 -Project C:\Game\Game.uproject [-Out ...] [-Runs '-DrSmokeTest','-DrPerfCapture']
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Out,
    [string[]]$Runs = @('-DrSmokeTest', '-DrPerfCapture'),
    [string]$ShaderFormat = 'PCD3D_SM6',
    [int]$TimeoutSeconds = 300,
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)
$ErrorActionPreference = 'Stop'
$Project = (Resolve-Path $Project).Path
$Name = [IO.Path]::GetFileNameWithoutExtension($Project)
$ProjectDir = Split-Path $Project
if (-not $Out) { $Out = Join-Path $ProjectDir 'Saved\Package' }
$BuildRoot = Join-Path $Out 'Windows'
$exe = Join-Path $BuildRoot "$Name.exe"
$collected = Join-Path $BuildRoot "$Name\Saved\CollectedPSOs"
$meta = Join-Path $ProjectDir "Saved\Cooked\Windows\$Name\Metadata\PipelineCaches"
$keys = @("$meta\ShaderStableInfo-$Name-$ShaderFormat.shk", "$meta\ShaderStableInfo-Global-$ShaderFormat.shk")
$output = Join-Path $ProjectDir "Build\Windows\PipelineCaches\PSO_${Name}_$ShaderFormat.spc"
$work = Join-Path $ProjectDir 'Saved\PSO'
if (-not (Test-Path -LiteralPath $exe)) { throw "패키지 실행 파일 없음: $exe (먼저 package_win64.ps1)" }
foreach ($k in $keys) { if (-not (Test-Path -LiteralPath $k)) { throw "셰이더 고정 키 없음: $k (NeedsShaderStableKeys=true로 다시 패키징)" } }
New-Item -ItemType Directory -Force $work, (Split-Path $output) | Out-Null
if (Test-Path $collected) { Remove-Item "$collected\*" -Force }

foreach ($run in $Runs) {
    $label = $run.TrimStart('-')
    $arguments = @($run, '-logPSO', '-unattended', '-ResX=1920', '-ResY=1080', '-ForceRes', '-nosplash', '-RenderOffscreen', '-log')
    $process = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $BuildRoot -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit($TimeoutSeconds * 1000)) { Stop-Process -Id $process.Id -Force; throw "$label 제한 시간 초과" }
    Copy-Item (Join-Path $BuildRoot "$Name\Saved\Logs\$Name.log") (Join-Path $work "record-$label.log") -Force
    "$label 종료 코드 $($process.ExitCode)"
}
$records = @(Get-ChildItem $collected -Filter '*.rec.upipelinecache' -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName })
if (-not $records.Count) { throw "PSO 기록 파일이 없음: $collected" }

$log = Join-Path $work 'expand.log'
$arguments = @($Project, '-run=ShaderPipelineCacheTools', 'expand') + $records + $keys + @($output, '-unattended', '-nullrhi', '-stdout')
& "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" @arguments *> $log
Select-String $log -Pattern 'Loaded \d+ PSOs total|Generated \d+ stable PSOs' | ForEach-Object { $_.Line -replace '^\[[^\]]*\]\[[^\]]*\]', '' }
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $output)) { throw "캐시 생성 실패(로그: $log)" }
"생성: $output ($((Get-Item $output).Length) bytes) — package_win64.ps1을 다시 돌려 패키지에 넣는다."
