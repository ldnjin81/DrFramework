# Win64 패키지(pak + iostore)를 만든다. 결과: <Out>\Windows\<프로젝트>.exe
# 스모크·성능·PSO 도구는 콘솔 명령과 실행 파일 옆 Saved를 쓰므로 Development·DebugGame만 지원한다.
# 사용: .\Tools\package_win64.ps1 -Project C:\Game\Game.uproject [-Out C:\Game_Build] [-Maps /Game/Maps/L_Title+/Game/Maps/L_Main]
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Out,
    [string]$Maps,
    [ValidateSet('Development', 'DebugGame')][string]$Config = 'Development',
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_package_common.ps1')
$p = Resolve-PackagePaths $Project $Out
$Log = Join-Path $p.ProjectDir 'Saved\Logs\package_win64.txt'
New-Item -ItemType Directory -Force (Split-Path $Log), $p.Out | Out-Null
$arguments = @('BuildCookRun', "-project=$($p.Project)", '-noP4', '-platform=Win64', "-clientconfig=$Config", '-build', '-cook', '-stage',
    '-pak', '-iostore', '-archive', "-archivedirectory=$($p.Out)", '-unattended', '-utf8output')
# 맵을 지정하지 않으면 프로젝트 설정(맵 쿠킹 목록·기본 맵)을 따른다.
if ($Maps) { $arguments += "-map=$Maps" }
$ErrorActionPreference = 'Continue'   # UAT의 stderr 안내로 멈추지 않게
& "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" @arguments *> $Log
$code = $LASTEXITCODE
if ($code -ne 0) { Get-Content $Log | Select-String 'Error:|error ' | Select-Object -Last 15 | ForEach-Object { $_.Line }; "package exit $code (로그: $Log)"; exit $code }
if (-not (Test-Path -LiteralPath $p.Exe)) { "패키지 실행 파일 없음: $($p.Exe)"; exit 1 }
"package ok: $($p.Exe)"
