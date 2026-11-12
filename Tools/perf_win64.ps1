# 패키지를 -DrPerfCapture로 실행해 게임이 정한 카메라 경로(UDrPerfSubsystem)를 엔진 기본 품질로 측정한다.
# 결과: <Out>\Windows\<프로젝트>\Saved\Perf\result.json. 예산 초과 구간은 경고만 하고, 표본 누락·해상도 불일치는 실패다.
# 사용: .\Tools\perf_win64.ps1 -Project C:\Game\Game.uproject [-Out ...] [-Offscreen] [-Width 1920 -Height 1080]
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Out,
    [int]$TimeoutSeconds = 300,
    [int]$Width = 1920,
    [int]$Height = 1080,
    [switch]$Offscreen,
    [string[]]$ExtraArgs = @()
)
$ErrorActionPreference = 'Stop'
$Name = [IO.Path]::GetFileNameWithoutExtension($Project)
if (-not $Out) { $Out = Join-Path (Split-Path (Resolve-Path $Project).Path) 'Saved\Package' }
$BuildRoot = Join-Path $Out 'Windows'
$exe = Join-Path $BuildRoot "$Name.exe"
$result = Join-Path $BuildRoot "$Name\Saved\Perf\result.json"
if (-not (Test-Path -LiteralPath $exe)) { throw "패키지 실행 파일 없음: $exe (먼저 package_win64.ps1)" }
if (Test-Path -LiteralPath $result) { Remove-Item -LiteralPath $result }
$arguments = @('-DrPerfCapture', '-unattended', "-ResX=$Width", "-ResY=$Height", '-ForceRes', '-nosplash', '-log') + $ExtraArgs
$arguments += $(if ($Offscreen) { '-RenderOffscreen' } else { '-windowed' })
$process = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $BuildRoot -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) { Stop-Process -Id $process.Id -Force; throw '성능 측정 제한 시간 초과' }
if (-not (Test-Path -LiteralPath $result)) { throw "성능 결과 파일 없음: $result (게임 종료 코드 $($process.ExitCode))" }
$report = Get-Content -LiteralPath $result -Raw | ConvertFrom-Json
$report | ConvertTo-Json -Depth 8
if ($process.ExitCode -ne 0 -or -not $report.completed) { throw "성능 측정 실패 또는 표본 누락: $($report.error)" }
if ($report.width -ne $Width -or $report.height -ne $Height) { throw "해상도 불일치: $($report.width)x$($report.height)" }
foreach ($section in $report.sections) {
    if ($section.over_budget) { Write-Warning "$($section.name): p95 $($section.frame.p95_ms)ms > $($report.frame_budget_ms)ms. $($section.candidate_causes)" }
}
"perf ok"
