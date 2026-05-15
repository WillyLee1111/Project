@echo off

gcc src/main.c ^
src/dictionary.c ^
src/file.c ^
src/flashcard.c ^
src/game.c ^
src/utils.c ^
-o build/app.exe

if %errorlevel% neq 0 (
    echo Compile failed!
    pause
    exit /b
)

build\app.exe