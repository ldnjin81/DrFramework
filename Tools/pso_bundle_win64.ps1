# 묶음 PSO 캐시(<프로젝트>\Build\Windows\PipelineCaches\PSO_<프로젝트>_<형식>.spc)를 만든다.
# 순서: package_win64.ps1(쿡이 셰이더 고정 키 .shk를 남김) → 이 스크립트 → package_win64.ps1(캐시를 넣어 다시 패키징).
# 전제: Config/DefaultEngine.ini [DevOptions.Shaders] NeedsShaderStableKeys=true (없으면 .shk가 생기지 않는다).
# 패키지를 -logPSO로 스모크·성능 경로대로 돌려 실제로 쓰인 PSO를 모은다. 기록 실행이 실패하면 캐시를 만들지 않는다.
# 셰이더 형식은 쿡 결과에서 찾는다(프로젝트가 SM5/SM6 중 무엇을 쓰는지에 따라). 셰이더·머티리얼·FX가 크게 바뀌면 다시 돌린다.
# 사용: .\Tools\pso_bundle_win64.ps1 -Project C:\Game\Game.uproject [-Out ...] [-Runs '-DrSmokeTest','-DrPerfCapture']
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Out,
    [string[]]$Runs = @('-DrSmokeTest', '-DrPerfCapture'),
    [string]$ShaderFormat,
    [int]$TimeoutSeconds = 300,
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_package_common.ps1')
$p = Resolve-PackagePaths $Project $Out
$meta = Join-Path $p.ProjectDir "Saved\Cooked\Windows\$($p.Name)\Metadata\PipelineCaches"
if (-not $ShaderFormat) {
    $found = @(Get-ChildItem $meta -Filter "ShaderStableInfo-$($p.Name)-*.shk" -ErrorAction SilentlyContinue |
        ForEach-Object { $_.BaseName.Substring("ShaderStableInfo-$($p.Name)-".Length) })
    if ($found.Count -ne 1) { throw "셰이더 고정 키를 하나로 정할 수 없음(${meta}: $($found -join ', ')). NeedsShaderStableKeys=true로 패키징했는지 확인하거나 -ShaderFormat을 준다." }
    $ShaderFormat = $found[0]
}
$keys = @("$meta\ShaderStableInfo-$($p.Name)-$ShaderFormat.shk", "$meta\ShaderStableInfo-Global-$ShaderFormat.shk")
foreach ($k in $keys) { if (-not (Test-Path -LiteralPath $k)) { throw "셰이더 고정 키 없음: $k (NeedsShaderStableKeys=true로 다시 패키징)" } }
$output = Join-Path $p.ProjectDir "Build\Windows\PipelineCaches\PSO_$($p.Name)_$ShaderFormat.spc"
$collected = Join-Path $p.Saved 'CollectedPSOs'
$work = Join-Path $p.ProjectDir 'Saved\PSO'
New-Item -ItemType Directory -Force $work, (Split-Path $output) | Out-Null
if (Test-Path $collected) { Remove-Item "$collected\*" -Force }

foreach ($run in $Runs) {
    $label = $run.TrimStart('-')
    Clear-Results $p
    Invoke-PackagedGame $p @($run, '-logPSO', '-unattended', '-ResX=1920', '-ResY=1080', '-ForceRes', '-nosplash', '-RenderOffscreen', '-log') $TimeoutSeconds $label
    Copy-Item (Join-Path $p.Saved "Logs\$($p.Name).log") (Join-Path $work "record-$label.log") -Force
    # 경로 일부만 돈 기록으로 캐시를 만들면 빠진 PSO 때문에 패키지에서 멈칫한다.
    if ($run -eq '-DrSmokeTest') { $r = Read-SmokeResult $p; if (-not $r.Passed) { throw "$label 기록 실행 실패: $($r.Failed -join ', ')" } }
    elseif ($run -eq '-DrPerfCapture') { $r = Read-PerfResult $p; if (-not $r.Passed) { throw "$label 기록 실행 실패: $($r.Report.error)" } }
    "$label 기록 완료"
}
$records = @(Get-ChildItem $collected -Filter '*.rec.upipelinecache' -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName })
if (-not $records.Count) { throw "PSO 기록 파일이 없음: $collected" }

$log = Join-Path $work 'expand.log'
$arguments = @($p.Project, '-run=ShaderPipelineCacheTools', 'expand') + $records + $keys + @($output, '-unattended', '-nullrhi', '-stdout')
$ErrorActionPreference = 'Continue'
& "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" @arguments *> $log
$code = $LASTEXITCODE
Select-String $log -Pattern 'Loaded \d+ PSOs total|Generated \d+ stable PSOs' | ForEach-Object { $_.Line -replace '^\[[^\]]*\]\[[^\]]*\]', '' }
if ($code -ne 0 -or -not (Test-Path -LiteralPath $output)) { throw "캐시 생성 실패(종료 코드 $code, 로그: $log)" }
"생성: $output ($((Get-Item $output).Length) bytes) — package_win64.ps1을 다시 돌려 패키지에 넣는다."
