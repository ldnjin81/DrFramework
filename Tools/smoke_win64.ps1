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
. (Join-Path $PSScriptRoot '_package_common.ps1')
$p = Resolve-PackagePaths $Project $Out
Clear-Results $p
$arguments = @('-DrSmokeTest', '-unattended', '-ResX=1920', '-ResY=1080', '-nosplash') + $ExtraArgs
# 원격(SSH) 세션처럼 창을 만들 수 없을 때는 스왑체인 없이 렌더링한다.
$arguments += $(if ($Offscreen) { '-RenderOffscreen' } else { '-windowed' })
Invoke-PackagedGame $p $arguments $TimeoutSeconds '스모크 테스트'
$r = Read-SmokeResult $p
$r.Report | ConvertTo-Json -Depth 5
if (-not $r.Passed) { throw "스모크 테스트 실패: $($r.Failed -join ', ')" }
"smoke ok"
