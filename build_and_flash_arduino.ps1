param(
    [string]$Port = 'COM22',
    [ValidateSet('mobiflight_mega', 'mobiflight_micro', 'mobiflight_uno', 'mobiflight_nano')]
    [string]$Environment = 'mobiflight_mega'
)

$ErrorActionPreference = 'Stop'
$projectDirectory = $PSScriptRoot

function Find-PlatformIo {
    foreach ($commandName in @('platformio', 'pio')) {
        $command = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($null -ne $command) {
            return $command.Source
        }
    }

    foreach ($candidate in @(
        (Join-Path $env:USERPROFILE '.platformio\penv\Scripts\platformio.exe'),
        (Join-Path $env:USERPROFILE '.platformio\penv\Scripts\pio.exe')
    )) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    throw 'PlatformIO was not found. Install PlatformIO Core before running this script.'
}

$availablePorts = [System.IO.Ports.SerialPort]::GetPortNames()
if ($Port -notin $availablePorts) {
    $portList = if ($availablePorts.Count) { $availablePorts -join ', ' } else { 'none' }
    throw "Serial port $Port was not found. Available ports: $portList"
}

$platformIo = Find-PlatformIo
Write-Host "[Arduino] Project:     $projectDirectory"
Write-Host "[Arduino] Environment: $Environment"
Write-Host "[Arduino] Port:        $Port"
Write-Host "[Arduino] Tool:        $platformIo"
Write-Host '[Arduino] Close MobiFlight Connector and serial monitors before continuing.' -ForegroundColor Yellow
Write-Host '[Arduino] Building and flashing firmware...'

& $platformIo run --project-dir $projectDirectory --environment $Environment --target upload --upload-port $Port
if ($LASTEXITCODE -ne 0) {
    throw "PlatformIO failed with exit code $LASTEXITCODE."
}

Write-Host "[Arduino] Success. $Environment firmware was flashed to $Port." -ForegroundColor Green

