# 패키지를 -DrSmokeTest로 실행해 게임이 등록한 스모크 단계(UDrSmokeSubsystem::RegisterSmokeSteps)를 검사한다.
# 결과: <Out>\Windows\<프로젝트>\Saved\Smoke\result.json (실행 전에 지워 이전 결과를 재사용하지 않는다)
# 사용: .\Tools\smoke_win64.ps1 -Project C:\Game\Game.uproject [-Out ...] [-Offscreen] [-ExtraArgs '-MyFlag']
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Out,
    [int]$TimeoutSeconds = 180,
    [switch]$Offscreen,
    [string[]]$ExtraArgs = @()
)
$ErrorActionPreference = 'Stop'
$Name = [IO.Path]::GetFileNameWithoutExtension($Project)
if (-not $Out) { $Out = Join-Path (Split-Path (Resolve-Path $Project).Path) 'Saved\Package' }
$BuildRoot = Join-Path $Out 'Windows'
$exe = Join-Path $BuildRoot "$Name.exe"
$result = Join-Path $BuildRoot "$Name\Saved\Smoke\result.json"
if (-not (Test-Path -LiteralPath $exe)) { throw "패키지 실행 파일 없음: $exe (먼저 package_win64.ps1)" }
if (Test-Path -LiteralPath $result) { Remove-Item -LiteralPath $result }
$arguments = @('-DrSmokeTest', '-unattended', '-ResX=1920', '-ResY=1080', '-nosplash') + $ExtraArgs
# 원격(SSH) 세션처럼 창을 만들 수 없을 때는 스왑체인 없이 렌더링한다.
$arguments += $(if ($Offscreen) { '-RenderOffscreen' } else { '-windowed' })
$process = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $BuildRoot -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) { Stop-Process -Id $process.Id -Force; throw '스모크 테스트 제한 시간 초과' }
if (-not (Test-Path -LiteralPath $result)) { throw "결과 파일 없음: $result (게임 종료 코드 $($process.ExitCode))" }
$report = Get-Content -LiteralPath $result -Raw | ConvertFrom-Json
$report | ConvertTo-Json -Depth 5
$failed = @($report.steps.PSObject.Properties | Where-Object { -not $_.Value } | ForEach-Object { $_.Name })
if ($process.ExitCode -ne 0 -or -not $report.passed -or $failed.Count) { throw "스모크 테스트 실패(종료 코드 $($process.ExitCode)) $($failed -join ', ')" }
"smoke ok"
