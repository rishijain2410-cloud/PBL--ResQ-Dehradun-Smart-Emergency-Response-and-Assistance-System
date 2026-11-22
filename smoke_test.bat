@echo off
setlocal

echo ========================================================================
echo  JEEVAN MARG - SHOWCASE SMOKE TEST LAUNCHER (Windows cmd.exe)
echo ========================================================================

where powershell >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] PowerShell is required to execute smoke_test.ps1
    exit /b 1
)

set "SCRIPT_PATH=%~dp0scripts\smoke_test.ps1"
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_PATH%" %*
exit /b %ERRORLEVEL%
