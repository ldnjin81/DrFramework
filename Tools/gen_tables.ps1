param([string]$DrTable = 'C:\Project\ProjectDT\Tools\DrTable\drtable.exe')
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
$Output = Join-Path $env:TEMP 'DrFramework-DrTable'
foreach ($Scope in @('framework','sample')) {
    $Prefix = if ($Scope -eq 'framework') { 'Dr' } else { 'Ds' }
    $ExportApi = if ($Scope -eq 'framework') { 'DRFRAMEWORK_API' } else { 'DRSAMPLE_API' }
    $Schema = if ($Scope -eq 'framework') { "$Root\Plugins\DrFramework\Tables\Schemas" } else { "$Root\Source\DrSample\Tables\Schemas" }
    $InputData = if ($Scope -eq 'framework') { "$Root\Design\Tables" } else { "$Root\Source\DrSample\Tables\Data" }
    $Cpp = if ($Scope -eq 'framework') { "$Root\Plugins\DrFramework\Source\DrFramework\TableData\Generated" } else { "$Root\Source\DrSample\TableData\Generated" }
    New-Item -ItemType Directory -Force "$Output\$Scope\client", "$Output\$Scope\server" | Out-Null
    & $DrTable build --input $InputData --schema $Schema --out-cpp $Cpp --out-client "$Output\$Scope\client" --out-server "$Output\$Scope\server" --prefix $Prefix --ue-plugin
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    # drtable은 모듈 API 매크로를 붙이지 않는다. 다른 모듈(샘플·게임)이 행 정적 함수(Find/GetAll)를 링크할 수 있게 행 구조체를 내보낸다.
    Get-ChildItem $Cpp -Filter "$Prefix*Row.h" | ForEach-Object {
        $h = Get-Content $_.FullName -Raw
        $n = [regex]::Replace($h, "struct (F$Prefix\w+Row) : public FTableRowBase", "struct $ExportApi `$1 : public FTableRowBase")
        if ($n -ne $h) { [IO.File]::WriteAllText($_.FullName, $n, (New-Object Text.UTF8Encoding $false)) }
    }
    & $DrTable check --client "$Output\$Scope\client" --server "$Output\$Scope\server"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
