@echo off
echo ==========================================
echo   Building Fish Story 2
echo ==========================================

REM Create build directory
if not exist build mkdir build
cd build

REM Run CMake
echo Running CMake...
cmake .. -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo CMake failed!
    pause
    exit /b %errorlevel%
)

REM Build the project
echo Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ==========================================
echo   Build successful!
echo ==========================================
echo.
echo To run the game, execute:
echo   cd build
echo   FishStory2.exe
echo.
pause
