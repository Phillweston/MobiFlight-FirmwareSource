param(
    [string]$Port = 'COM37',
    [switch]$CompileOnly
)

$ErrorActionPreference = 'Stop'
$config = Join-Path $PSScriptRoot 'esphome\a320-ovhd.yaml'
$secrets = Join-Path $PSScriptRoot 'esphome\secrets.yaml'

if (-not (Get-Command uvx -ErrorAction SilentlyContinue)) {
    throw 'uvx was not found. Install uv from https://docs.astral.sh/uv/ first.'
}
if (-not (Test-Path -LiteralPath $secrets)) {
    throw "Missing $secrets. Create it from secrets.yaml.example and enter the API/OTA credentials."
}

Write-Host "[CoreS3] ESPHome config: $config"
& uvx --python 3.13 esphome config $config
if ($LASTEXITCODE -ne 0) { throw 'ESPHome configuration validation failed.' }

if ($CompileOnly) {
    & uvx --python 3.13 esphome compile $config
} else {
    if ($Port -notin [System.IO.Ports.SerialPort]::GetPortNames()) {
        throw "Serial port $Port was not found."
    }
    & uvx --python 3.13 esphome run $config --device $Port
}
if ($LASTEXITCODE -ne 0) { throw "ESPHome exited with code $LASTEXITCODE." }
