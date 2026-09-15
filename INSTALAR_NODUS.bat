@echo off
setlocal
cd /d "%~dp0"
title Nodus - Instalador
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0installer\install.ps1"
if errorlevel 1 (
  echo.
  echo A instalacao nao foi concluida.
  pause
)
