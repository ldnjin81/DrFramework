# Win64 패키지(pak + iostore)를 만든다. 결과: <Out>\Windows\<프로젝트>.exe
# 사용: .\Tools\package_win64.ps1 -Project C:\Game\Game.uproject [-Out C:\Game_Build] [-Maps /Game/Maps/L_Title+/Game/Maps/L_Main]
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Out,
    [string]$Maps,
    [ValidateSet('Development', 'Shipping', 'DebugGame', 'Test')][string]$Config = 'Development',
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)
$ErrorActionPreference = 'Stop'
$Project = (Resolve-Path $Project).Path
$Name = [IO.Path]::GetFileNameWithoutExtension($Project)
$ProjectDir = Split-Path $Project
if (-not $Out) { $Out = Join-Path $ProjectDir 'Saved\Package' }
$Log = Join-Path $ProjectDir 'Saved\Logs\package_win64.txt'
New-Item -ItemType Directory -Force (Split-Path $Log), $Out | Out-Null
$arguments = @('BuildCookRun', "-project=$Project", '-noP4', '-platform=Win64', "-clientconfig=$Config", '-build', '-cook', '-stage',
    '-pak', '-iostore', '-archive', "-archivedirectory=$Out", '-unattended', '-utf8output')
# 맵을 지정하지 않으면 프로젝트 설정(맵 쿠킹 목록·기본 맵)을 따른다.
if ($Maps) { $arguments += "-map=$Maps" }
& "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" @arguments *> $Log
$code = $LASTEXITCODE
$exe = Join-Path $Out "Windows\$Name.exe"
if ($code -ne 0) { Get-Content $Log | Select-String 'Error:|error ' | Select-Object -Last 15 | ForEach-Object { $_.Line }; "package exit $code (로그: $Log)"; exit $code }
if (-not (Test-Path -LiteralPath $exe)) { "패키지 실행 파일 없음: $exe"; exit 1 }
"package ok: $exe"
