@echo off
setlocal
cd /d "%~dp0"
title Nodus 0.3.0 - Instalador

if exist "%~dp0install.ps1" (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1"
) else (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0installer\install.ps1"
)

if errorlevel 1 (
  echo.
  echo A instalacao nao foi concluida.
  pause
)
