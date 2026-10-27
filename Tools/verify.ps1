# DrFramework를 다른 플러그인·게임 모듈 없이 빌드하고 기반 자동화 테스트(DrFramework.*)를 실행한다.
# 사용: .\Tools\verify.ps1 [-EngineRoot 'C:\Program Files\Epic Games\UE_5.8']
param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Continue'
$Root = Split-Path $PSScriptRoot -Parent
$Project = Join-Path $Root 'Host\DrFrameworkHost.uproject'
$Saved = Join-Path $Root 'Host\Saved'
New-Item -ItemType Directory -Force $Saved | Out-Null
& "$EngineRoot\Engine\Build\BatchFiles\Build.bat" UnrealEditor Win64 Development "-Project=$Project" -WaitMutex -NoHotReload *> "$Saved\verify-build.txt"
"build=$LASTEXITCODE"
if ($LASTEXITCODE -ne 0) { Get-Content "$Saved\verify-build.txt" | Select-String 'error ' | Select -First 20; exit 1 }
$Log = "$Saved\verify-tests.txt"
& "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $Project '-ExecCmds=Automation RunTests DrFramework' '-TestExit=Automation Test Queue Empty' -nullrhi -unattended -nosplash -stdout *> $Log
$Lines = Get-Content $Log
$Ok = ($Lines | Select-String 'Result=\{Success\}').Count
$Fail = ($Lines | Select-String 'Result=\{Fail').Count
"success=$Ok fail=$Fail"
$Lines | Select-String 'Result=\{Fail' | ForEach-Object { $_.Line }
if ($Fail -gt 0 -or $Ok -eq 0) { exit 1 }
