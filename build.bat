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

if not exist node_modules (
  echo [+] Instalando dependencias...
  call npm install
  if errorlevel 1 goto fail
)

echo [+] Gerando assets e compilando Nodus...
call npm run desktop:build
if errorlevel 1 goto fail

echo.
echo ============================================================
echo   BUILD CONCLUIDO
echo ============================================================
echo Executavel:
echo   src-tauri\target\release\nodus.exe
echo.
echo Instalador NSIS:
echo   src-tauri\target\release\bundle\nsis\
echo.
exit /b 0

:fail
echo.
echo A compilacao do Nodus nao foi concluida.
pause
exit /b 1
