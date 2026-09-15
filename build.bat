@echo off
setlocal
cd /d "%~dp0"

where cl >nul 2>nul
if %errorlevel%==0 goto msvc

where gcc >nul 2>nul
if %errorlevel%==0 goto mingw

echo.
echo Nenhum compilador C encontrado.
echo.
echo Opcoes:
echo 1. Abra este build.bat dentro do "Developer Command Prompt for Visual Studio"
echo 2. Ou instale MinGW-w64 e deixe gcc no PATH.
echo.
pause
exit /b 1

:msvc
echo Compilando com MSVC...
if not exist bin mkdir bin
cl /nologo /O2 /W4 /D_CRT_SECURE_NO_WARNINGS src\main.c user32.lib gdi32.lib /link /SUBSYSTEM:WINDOWS /OUT:bin\Nodus.exe
if %errorlevel% neq 0 goto fail
goto done

:mingw
echo Compilando com MinGW-w64...
if not exist bin mkdir bin
gcc src\main.c -O2 -s -mwindows -o bin\Nodus.exe -lgdi32 -luser32 -lkernel32
if %errorlevel% neq 0 goto fail
goto done

:done
echo.
echo OK: bin\Nodus.exe
echo.
start "" bin\Nodus.exe
exit /b 0

:fail
echo.
echo Falha na compilacao.
pause
exit /b 1
