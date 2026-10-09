@echo off
cd /d "%~dp0"
title Pricilla Mart - Java Full-Stack Server
echo ============================================================
echo                     PRICILLA MART
echo               JAVA FULL-STACK BACKEND SERVER
echo ============================================================
echo.

:: 1. Check system PATH for Java
where java >nul 2>nul
if %errorlevel% equ 0 (
    echo [*] Starting Pricilla Mart using system Java...
    echo [*] Server URL: http://localhost:8080
    echo.
    start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
    java PricillaMartServer.java
    goto end
)

:: 2. Check standard Java install paths
for /d %%D in ("C:\Program Files\Java\jdk*" "C:\Program Files\Eclipse Adoptium\jdk*" "C:\Program Files\Microsoft\jdk*") do (
    if exist "%%D\bin\java.exe" (
        echo [*] Starting Pricilla Mart using Java in %%D...
        echo [*] Server URL: http://localhost:8080
        echo.
        start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
        "%%D\bin\java.exe" PricillaMartServer.java
        goto end
    )
)

echo [!] Java (JDK) was not detected on this computer.
echo.
echo [*] To run the Java backend directly:
echo     1. Download and install OpenJDK 17 or 21 from:
echo        https://adoptium.net/temurin/releases/
echo     2. After installing, re-run this script: run_java.bat
echo.
pause
:end
