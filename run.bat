
@echo off

set PATH=C:\msys64\ucrt64\bin;%PATH%

gcc src/*.c -o build/app.exe

if %errorlevel% neq 0 (
    echo Compile failed!
    pause
    exit
)

build\app.exe

pause
