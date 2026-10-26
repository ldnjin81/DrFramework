param(
    [string]$DrTable = 'C:\Project\ProjectDT\Tools\DrTable\drtable.exe'
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
$Output = Join-Path $env:TEMP 'DrFramework-DrTable'
New-Item -ItemType Directory -Force "$Output\client", "$Output\server" | Out-Null
& $DrTable build --input "$Root\Design\Tables" --schema "$Root\Plugins\DrFramework\Tables\Schemas" --out-cpp "$Root\Plugins\DrFramework\Source\DrFramework\TableData\Generated" --out-client "$Output\client" --out-server "$Output\server" --prefix Dr --ue-plugin
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $DrTable check --client "$Output\client" --server "$Output\server"
exit $LASTEXITCODE
