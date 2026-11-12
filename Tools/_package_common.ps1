# 패키지 검증 도구(package/smoke/perf/pso_bundle_win64.ps1)가 dot-source로 함께 쓰는 함수.

# 프로젝트·출력 경로를 절대 경로로 정한다(RunUAT는 작업 폴더를 엔진 폴더로 바꾸므로 상대 경로를 넘기면 엉뚱한 곳에 쓴다).
function Resolve-PackagePaths([string]$Project, [string]$Out) {
    $projectPath = (Resolve-Path -LiteralPath $Project).Path
    $projectDir = Split-Path $projectPath
    if (-not $Out) { $Out = Join-Path $projectDir 'Saved\Package' }
    $Out = [IO.Path]::GetFullPath($Out)
    $name = [IO.Path]::GetFileNameWithoutExtension($projectPath)
    $buildRoot = Join-Path $Out 'Windows'
    [pscustomobject]@{
        Project = $projectPath; ProjectDir = $projectDir; Name = $name; Out = $Out; BuildRoot = $buildRoot
        Exe = Join-Path $buildRoot "$name.exe"
        # Development·DebugGame 패키지는 Saved를 실행 파일 옆 <프로젝트>\Saved에 쓴다.
        Saved = Join-Path $buildRoot "$name\Saved"
    }
}

# 패키지를 숨김 창으로 실행하고 끝날 때까지 기다린다. 제한 시간을 넘으면 종료시키고 예외.
function Invoke-PackagedGame($Paths, [string[]]$Arguments, [int]$TimeoutSeconds, [string]$Label) {
    if (-not (Test-Path -LiteralPath $Paths.Exe)) { throw "패키지 실행 파일 없음: $($Paths.Exe) (먼저 package_win64.ps1)" }
    $process = Start-Process -FilePath $Paths.Exe -ArgumentList $Arguments -WorkingDirectory $Paths.BuildRoot -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit($TimeoutSeconds * 1000)) { Stop-Process -Id $process.Id -Force; throw "$Label 제한 시간 초과" }
}

# 결과 JSON 판정. 패키지 종료 코드는 엔진 종료 경로에 따라 0일 수 있어 결과 파일로만 판정한다.
function Read-SmokeResult($Paths) {
    $file = Join-Path $Paths.Saved 'Smoke\result.json'
    if (-not (Test-Path -LiteralPath $file)) { throw "스모크 결과 파일 없음: $file" }
    $report = Get-Content -LiteralPath $file -Raw | ConvertFrom-Json
    $failed = @($report.steps.PSObject.Properties | Where-Object { -not $_.Value } | ForEach-Object { $_.Name })
    $steps = @($report.steps.PSObject.Properties).Count
    [pscustomobject]@{ Report = $report; Passed = ($report.passed -and $steps -gt 0 -and -not $failed.Count); Failed = $failed }
}

function Read-PerfResult($Paths) {
    $file = Join-Path $Paths.Saved 'Perf\result.json'
    if (-not (Test-Path -LiteralPath $file)) { throw "성능 결과 파일 없음: $file" }
    $report = Get-Content -LiteralPath $file -Raw | ConvertFrom-Json
    [pscustomobject]@{ Report = $report; Passed = [bool]$report.completed }
}

function Clear-Results($Paths) {
    foreach ($f in @('Smoke\result.json', 'Perf\result.json')) {
        $p = Join-Path $Paths.Saved $f
        if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p }
    }
}
