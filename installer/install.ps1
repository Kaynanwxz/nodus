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

Title "NODUS 0.4.0 - INSTALADOR"
Write-Host "Instalacao local e sem permissao de administrador." -ForegroundColor Gray

if (-not [Environment]::Is64BitOperatingSystem) {
    throw "Esta versao requer Windows 64-bit."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir

$candidates = @(
    (Join-Path $scriptDir 'Nodus.exe'),
    (Join-Path $repoRoot 'Nodus.exe'),
    (Join-Path $repoRoot 'bin\Nodus.exe')
)

$exeSource = $null
foreach ($candidate in $candidates) {
    if (Test-Path $candidate) {
        $exeSource = $candidate
        break
    }
}

if (-not $exeSource) {
    $buildFile = Join-Path $repoRoot 'build.bat'
    if (Test-Path $buildFile) {
        throw "Nodus.exe ainda nao foi compilado. Voce esta usando o codigo-fonte. Execute build.bat primeiro. Se nao tiver compilador, baixe o ZIP gerado pelo GitHub Actions, que ja inclui Nodus.exe."
    }

    throw "Nodus.exe nao foi encontrado no pacote. Baixe e extraia o ZIP completo gerado pelo GitHub Actions."
}

$installDir = Join-Path $env:LOCALAPPDATA 'Nodus'
$target = Join-Path $installDir 'Nodus.exe'

try {
    Step "Executavel encontrado: $exeSource"
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
        $shortcut.Description = 'Nodus 0.4.0'
        $shortcut.Save()
    }

    Title "INSTALACAO CONCLUIDA"
    Write-Host "Nodus 0.4.0 instalado com sucesso." -ForegroundColor Green
    Write-Host "Abrindo Nodus..." -ForegroundColor Green
    Start-Process $target
    Start-Sleep -Seconds 1
}
catch {
    Fail $_.Exception.Message
    exit 1
}
