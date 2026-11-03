@echo off
:: Double-click to install SuperTune (asks for admin permission).
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)
cd /d "%~dp0"
echo Installing SuperTune...
if exist "%CommonProgramFiles%\VST3\SuperTune.vst3" rmdir /s /q "%CommonProgramFiles%\VST3\SuperTune.vst3"
xcopy "SuperTune.vst3" "%CommonProgramFiles%\VST3\SuperTune.vst3\" /E /I /Y >nul
echo.
echo Done! Open FL Studio -^> Options -^> Manage plugins -^> Find more plugins.
pause
