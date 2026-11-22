@echo off
setlocal enabledelayedexpansion
title JEEVAN MARG - Standalone Demo Runner
echo ========================================================================
echo  JEEVAN MARG -- SMART EMERGENCY RESPONSE & HEALTHCARE NAVIGATION SYSTEM
echo  Running Phase-2 Standalone Demonstrations & Interactive Routing Engine
echo ========================================================================
echo.

if exist "C:\msys64\ucrt64\bin\g++.exe" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
if exist "C:\msys64\mingw64\bin\g++.exe" set "PATH=C:\msys64\mingw64\bin;%PATH%"

where g++ >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] g++ compiler was not found on your system PATH!
    echo.
    echo Please install MSYS2 UCRT64:
    echo   1. Download and install MSYS2 from https://www.msys2.org/
    echo   2. Open MSYS2 UCRT64 shell and run:
    echo        pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
    echo   3. Add C:\msys64\ucrt64\bin to your Windows PATH environment variable.
    echo.
    pause
    exit /b 1
)

if not exist "m3_navigation\build\libwinpthread-1.dll" if exist "C:\msys64\ucrt64\bin\libwinpthread-1.dll" copy /y "C:\msys64\ucrt64\bin\libwinpthread-1.dll" "m3_navigation\build\libwinpthread-1.dll" >nul
if not exist "m3_navigation\libwinpthread-1.dll" if exist "C:\msys64\ucrt64\bin\libwinpthread-1.dll" copy /y "C:\msys64\ucrt64\bin\libwinpthread-1.dll" "m3_navigation\libwinpthread-1.dll" >nul

echo [INFO] Building/Verifying demo executable...
set "MAKE_BIN="
where make >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "MAKE_BIN=make"
) else (
    where mingw32-make >nul 2>&1
    if %ERRORLEVEL% equ 0 (
        set "MAKE_BIN=mingw32-make"
    )
)

if defined MAKE_BIN (
    echo [INFO] Rebuilding via %MAKE_BIN% in m3_navigation...
    %MAKE_BIN% -C m3_navigation server
    if %ERRORLEVEL% neq 0 (
        echo [ERROR] Build failed! Check compiler errors above.
        pause
        exit /b 1
    )
) else (
    echo [INFO] Neither make nor mingw32-make found. Falling back to direct g++ compilation...
    cd m3_navigation
    if not exist build mkdir build
    if not exist build\sqlite3.o gcc -O2 -DSQLITE_THREADSAFE=1 -DSQLITE_ENABLE_FTS5 -c third_party/sqlite/sqlite3.c -o build/sqlite3.o
    g++ -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I. -I../m1_dispatch main.cpp model/Location.cpp structure/LocationNode.cpp structure/RoadEdge.cpp structure/RoadGraph.cpp repository/LocationRepository.cpp datasource/LocationCsvLoader.cpp datasource/RoadNetworkCsvLoader.cpp dto/RouteResponse.cpp service/RoadGraphBuilder.cpp service/ShortestPathFinder.cpp service/NearbyServicesFinder.cpp service/TransportEstimator.cpp service/RestApiServer.cpp service/EmergencyDispatcher.cpp ../m1_dispatch/service/TriageRuleEngine.cpp util/GeoUtil.cpp util/CryptoUtil.cpp service/AuthService.cpp persistence/Database.cpp persistence/SqliteLocationRepository.cpp persistence/SqliteRoadRepository.cpp persistence/SqliteHospitalRepository.cpp persistence/SqliteAmbulanceRepository.cpp persistence/SqliteEmergencyRepository.cpp persistence/SqlitePatientLocationRepository.cpp persistence/SqliteUserRepository.cpp persistence/SqliteSessionRepository.cpp persistence/DatabaseBootstrap.cpp build/sqlite3.o -o build/m3_nav.exe -lws2_32 -lwsock32 -lbcrypt -static-libgcc -static-libstdc++
    if %ERRORLEVEL% neq 0 (
        cd ..
        echo [ERROR] Direct g++ build failed!
        pause
        exit /b 1
    )
    cd ..
)

echo [INFO] Launching Demo Engine...
echo.
cd m3_navigation
build\m3_nav.exe
cd ..
