@echo off
cd /d "%~dp0"
title Pricilla Mart - Full-Stack Java Application (Port 8080)

echo ============================================================
echo                     PRICILLA MART
echo            FULL-STACK JAVA CAPSTONE APPLICATION
echo ============================================================
echo.

:: 1. Prioritize Java (System PATH)
where java >nul 2>nul
if %errorlevel% equ 0 (
    echo [*] Starting Pricilla Mart Server using Java...
    echo [*] Server URL: http://localhost:8080
    echo.
    start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
    java PricillaMartServer.java
    goto end
)

:: 2. Check standard Java installation directories
for /d %%D in ("C:\Program Files\Java\jdk*" "C:\Program Files\Eclipse Adoptium\jdk*" "C:\Program Files\Microsoft\jdk*") do (
    if exist "%%D\bin\java.exe" (
        echo [*] Starting Pricilla Mart Server using Java in %%D...
        echo [*] Server URL: http://localhost:8080
        echo.
        start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
        "%%D\bin\java.exe" PricillaMartServer.java
        goto end
    )
)

:: 3. Fallback to Local Runner (if Java is not yet installed on this machine)
echo [!] Java is not detected in your system PATH.
echo [*] Launching local preview runner so you can view your store immediately...
echo [*] Server URL: http://localhost:8080
echo.

if exist "C:\Program Files\Python314\python.exe" (
    start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
    "C:\Program Files\Python314\python.exe" dev_server.py
    goto end
)

where py >nul 2>nul
if %errorlevel% equ 0 (
    start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
    py dev_server.py
    goto end
)

where python >nul 2>nul
if %errorlevel% equ 0 (
    start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
    python dev_server.py
    goto end
)

:: 4. Direct Browser View if no runtime
echo [*] Opening store directly in your default browser...
start "" "frontend\index.html"

:end
echo.
echo Server stopped.
pause
