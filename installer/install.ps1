$ErrorActionPreference = 'Stop'

function Title([string]$text) {
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor DarkYellow
    Write-Host "  $text" -ForegroundColor Yellow
    Write-Host "============================================================" -ForegroundColor DarkYellow
}

function Step([string]$text) {
    Write-Host "[+] $text" -ForegroundColor Cyan
}

function Fail([string]$text) {
    Write-Host "[!] $text" -ForegroundColor Red
}

Title "NODUS 0.3.0 - INSTALADOR"
Write-Host "Instalacao local, sem compilador e sem permissao de administrador." -ForegroundColor Gray

if (-not [Environment]::Is64BitOperatingSystem) {
    throw "Esta versao requer Windows 64-bit."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$packageRoot = $scriptDir
$exeSource = Join-Path $packageRoot 'Nodus.exe'
if (-not (Test-Path $exeSource)) {
    $candidate = Join-Path (Split-Path -Parent $scriptDir) 'Nodus.exe'
    if (Test-Path $candidate) { $exeSource = $candidate }
}
if (-not (Test-Path $exeSource)) {
    throw "Nodus.exe nao foi encontrado ao lado do instalador. Extraia o ZIP completo antes de instalar."
}

$installDir = Join-Path $env:LOCALAPPDATA 'Nodus'
$target = Join-Path $installDir 'Nodus.exe'

try {
    Step "Fechando versoes antigas do Nodus..."
    Get-Process -Name 'Nodus' -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    Start-Sleep -Milliseconds 300

    Step "Instalando Nodus em $installDir"
    New-Item -ItemType Directory -Force -Path $installDir | Out-Null
    Copy-Item $exeSource $target -Force

    $uninstallPs1 = @'
$ErrorActionPreference = "SilentlyContinue"
Get-Process -Name "Nodus" -ErrorAction SilentlyContinue | Stop-Process -Force
$installDir = Join-Path $env:LOCALAPPDATA "Nodus"
$desktop = [Environment]::GetFolderPath("Desktop")
$startMenu = Join-Path $env:APPDATA "Microsoft\Windows\Start Menu\Programs\Nodus"
Remove-Item (Join-Path $desktop "Nodus.lnk") -Force
Remove-Item $startMenu -Force -Recurse
Start-Sleep -Milliseconds 300
Remove-Item $installDir -Force -Recurse
'@
    Set-Content -Path (Join-Path $installDir 'uninstall.ps1') -Value $uninstallPs1 -Encoding UTF8

    $uninstallBat = "@echo off`r`npowershell.exe -NoProfile -ExecutionPolicy Bypass -File `"%~dp0uninstall.ps1`"`r`n"
    Set-Content -Path (Join-Path $installDir 'DESINSTALAR_NODUS.bat') -Value $uninstallBat -Encoding ASCII

    Step "Criando atalhos..."
    $shell = New-Object -ComObject WScript.Shell
    $desktop = [Environment]::GetFolderPath('Desktop')
    $startMenu = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Nodus'
    New-Item -ItemType Directory -Force -Path $startMenu | Out-Null

    foreach ($shortcutPath in @(
        (Join-Path $desktop 'Nodus.lnk'),
        (Join-Path $startMenu 'Nodus.lnk')
    )) {
        $shortcut = $shell.CreateShortcut($shortcutPath)
        $shortcut.TargetPath = $target
        $shortcut.WorkingDirectory = $installDir
        $shortcut.Description = 'Nodus 0.3.0'
        $shortcut.Save()
    }

    Title "INSTALACAO CONCLUIDA"
    Write-Host "Nodus 0.3.0 instalado com sucesso." -ForegroundColor Green
    Write-Host "Abrindo Nodus..." -ForegroundColor Green
    Start-Process $target
    Start-Sleep -Seconds 1
}
catch {
    Fail $_.Exception.Message
    exit 1
}
