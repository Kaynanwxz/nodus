@echo off
setlocal
cd /d "%~dp0"
title Nodus 0.4.0 - Instalador

if exist "%~dp0install.ps1" (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1"
) else (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0installer\install.ps1"
)

if errorlevel 1 (
  echo.
  echo A instalacao nao foi concluida.
  echo.
  echo Se voce clonou o codigo-fonte, rode build.bat primeiro.
  echo Se nao tiver compilador, use o ZIP gerado pelo GitHub Actions.
  echo.
  pause
)
