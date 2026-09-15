$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

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

Title "NODUS - INSTALADOR"
Write-Host "Este instalador cria a versao nativa em C e instala no seu usuario." -ForegroundColor Gray
Write-Host "Nao precisa Visual Studio, MinGW ou permissao de administrador." -ForegroundColor Gray
Write-Host ""

if (-not [Environment]::Is64BitOperatingSystem) {
    throw "Esta versao requer Windows 64-bit."
}

$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$source = Join-Path $root 'src\main.c'
if (-not (Test-Path $source)) { throw "Arquivo src\main.c nao encontrado." }

$installDir = Join-Path $env:LOCALAPPDATA 'Nodus'
$tempRoot = Join-Path $env:TEMP ('nodus-' + [guid]::NewGuid().ToString('N'))
$toolZip = Join-Path $tempRoot 'llvm-mingw.zip'
$toolDir = Join-Path $tempRoot 'toolchain'
$buildDir = Join-Path $tempRoot 'build'
$exeOut = Join-Path $buildDir 'Nodus.exe'
$release = '20260908'
$url = "https://github.com/mstorsjo/llvm-mingw/releases/download/$release/llvm-mingw-$release-ucrt-x86_64.zip"

try {
    New-Item -ItemType Directory -Force -Path $tempRoot, $toolDir, $buildDir, $installDir | Out-Null

    Step "Baixando compilador C portatil (uso apenas durante a instalacao)..."
    Invoke-WebRequest -Uri $url -OutFile $toolZip -UseBasicParsing

    Step "Preparando compilador..."
    Expand-Archive -Path $toolZip -DestinationPath $toolDir -Force
    $clang = Get-ChildItem -Path $toolDir -Filter 'x86_64-w64-mingw32-clang.exe' -Recurse -File | Select-Object -First 1
    if (-not $clang) { throw "Compilador C nao encontrado no pacote baixado." }

    Step "Compilando Nodus em C nativo..."
    $compileArgs = @(
        $source,
        '-O2',
        '-s',
        '-mwindows',
        '-municode',
        '-o', $exeOut,
        '-lgdi32',
        '-luser32',
        '-lkernel32'
    )

    # O codigo usa WinMain ANSI, portanto nao precisamos forcar Unicode no entry point.
    $frameOverride = Join-Path $root 'src\frame_override.h'
    if (-not (Test-Path $frameOverride)) { throw "Arquivo src\frame_override.h nao encontrado." }

    $compileArgs = @(
        '-include', $frameOverride,
        $source,
        '-O2',
        '-s',
        '-mwindows',
        '-o', $exeOut,
        '-lgdi32',
        '-luser32',
        '-lkernel32'
    )

    & $clang.FullName @compileArgs
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exeOut)) {
        throw "Falha ao compilar o aplicativo."
    }

    Step "Instalando arquivos..."
    Copy-Item $exeOut (Join-Path $installDir 'Nodus.exe') -Force
    Copy-Item $source (Join-Path $installDir 'main.c') -Force

    $uninstallPs1 = @'
$ErrorActionPreference = "SilentlyContinue"
$installDir = Join-Path $env:LOCALAPPDATA "Nodus"
$desktop = [Environment]::GetFolderPath("Desktop")
$startMenu = Join-Path $env:APPDATA "Microsoft\Windows\Start Menu\Programs\Nodus"
Remove-Item (Join-Path $desktop "Nodus.lnk") -Force
Remove-Item (Join-Path $startMenu "Nodus.lnk") -Force
Remove-Item $startMenu -Force -Recurse
Start-Sleep -Milliseconds 250
Remove-Item $installDir -Force -Recurse
Write-Host "Nodus foi removido."
Start-Sleep -Seconds 2
'@
    Set-Content -Path (Join-Path $installDir 'uninstall.ps1') -Value $uninstallPs1 -Encoding UTF8

    $uninstallBat = '@echo off`r`npowershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0uninstall.ps1"`r`n'
    Set-Content -Path (Join-Path $installDir 'DESINSTALAR.bat') -Value $uninstallBat -Encoding ASCII

    Step "Criando atalhos..."
    $shell = New-Object -ComObject WScript.Shell
    $target = Join-Path $installDir 'Nodus.exe'
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
        $shortcut.Description = 'Nodus'
        $shortcut.Save()
    }

    Step "Limpando arquivos temporarios..."
    Remove-Item $tempRoot -Recurse -Force -ErrorAction SilentlyContinue

    Title "INSTALACAO CONCLUIDA"
    Write-Host "Atalho criado na Area de Trabalho e no Menu Iniciar." -ForegroundColor Green
    Write-Host "Abrindo Nodus..." -ForegroundColor Green
    Start-Process $target
    Start-Sleep -Seconds 2
}
catch {
    Fail $_.Exception.Message
    Write-Host ""
    Write-Host "Se o Windows bloqueou o download, verifique sua conexao e tente novamente." -ForegroundColor Gray
    Remove-Item $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
    exit 1
}
