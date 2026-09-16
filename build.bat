@echo off
setlocal
cd /d "%~dp0"
title Nodus 0.5.0 - Tauri Build

echo ============================================================
echo   NODUS 0.5.0 - TAURI BUILD
echo ============================================================
echo.

where node >nul 2>nul
if errorlevel 1 (
  echo [ERRO] Node.js nao foi encontrado no PATH.
  echo Instale Node.js 20 ou superior e tente novamente.
  goto fail
)

where npm >nul 2>nul
if errorlevel 1 (
  echo [ERRO] npm nao foi encontrado no PATH.
  goto fail
)

where cargo >nul 2>nul
if errorlevel 1 (
  echo [ERRO] Rust/Cargo nao foi encontrado no PATH.
  echo Instale Rust pelo rustup e tente novamente.
  goto fail
)

echo [+] Atualizando dependencias do frontend...
call npm install --no-audit --no-fund
if errorlevel 1 goto fail

echo [+] Validando imagens e compilando Nodus...
call npm run desktop:build
if errorlevel 1 goto fail

if not exist "src-tauri\target\release\nodus.exe" (
  echo [ERRO] Executavel nao foi gerado.
  goto fail
)
if not exist "src-tauri\target\release\bundle\nsis\Nodus_0.5.0_x64-setup.exe" (
  echo [ERRO] Instalador NSIS nao foi gerado.
  goto fail
)

echo.
echo ============================================================
echo   BUILD CONCLUIDO
echo ============================================================
echo Executavel:
echo   src-tauri\target\release\nodus.exe
echo.
echo Instalador NSIS:
echo   src-tauri\target\release\bundle\nsis\Nodus_0.5.0_x64-setup.exe
echo.
exit /b 0

:fail
echo.
echo A compilacao do Nodus nao foi concluida.
exit /b 1
