@echo off
setlocal enabledelayedexpansion
title JEEVAN MARG - Automated Test Runner
echo ========================================================================
echo  JEEVAN MARG -- AUTOMATED UNIT & INTEGRATION TEST RUNNER
echo  Running Comprehensive Test Suites across All System Modules (M1, M2, M3)
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
    echo [INFO] Running test suites via %MAKE_BIN% test...
    %MAKE_BIN% test
    if %ERRORLEVEL% neq 0 (
        echo [ERROR] Test suite failed!
        pause
        exit /b 1
    )
) else (
    echo [1/8] Testing Module 1 (Emergency Dispatch)...
    cd m1_dispatch
    g++ -std=c++17 -Wall -Wextra -O2 -I. test/test_m1.cpp model/EmergencyContact.cpp model/EmergencyRequest.cpp service/TriageRuleEngine.cpp service/EmergencyManager.cpp -o m1_test.exe -static-libgcc -static-libstdc++
    m1_test.exe
    cd ..

    echo.
    echo [2/8] Testing Module 2 (Healthcare Discovery)...
    cd m2_healthcare
    g++ -std=c++17 -Wall -Wextra -O2 -I. test/test_m2.cpp model/BedAvailability.cpp model/Doctor.cpp model/Department.cpp model/Hospital.cpp repository/HospitalRepository.cpp service/HealthcareService.cpp -o m2_test.exe -static-libgcc -static-libstdc++
    m2_test.exe
    cd ..

    echo.
    echo [3/8] Testing Module 3 Core (Dijkstra vs Bellman-Ford 380 Pairs)...
    cd m3_navigation
    if not exist build mkdir build
    if not exist build\sqlite3.o gcc -O2 -DSQLITE_THREADSAFE=1 -DSQLITE_ENABLE_FTS5 -c third_party/sqlite/sqlite3.c -o build/sqlite3.o
    g++ -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I. test/test_main.cpp model/Location.cpp structure/LocationNode.cpp structure/RoadEdge.cpp structure/RoadGraph.cpp repository/LocationRepository.cpp datasource/LocationCsvLoader.cpp datasource/RoadNetworkCsvLoader.cpp dto/RouteResponse.cpp service/RoadGraphBuilder.cpp service/ShortestPathFinder.cpp service/NearbyServicesFinder.cpp service/TransportEstimator.cpp util/GeoUtil.cpp -o build/m3_test.exe -static-libgcc -static-libstdc++
    build\m3_test.exe

    echo.
    echo [4/8] Running Module 3 Database Persistence Tests...
    g++ -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I. test/test_db.cpp model/Location.cpp structure/LocationNode.cpp structure/RoadEdge.cpp structure/RoadGraph.cpp repository/LocationRepository.cpp datasource/LocationCsvLoader.cpp datasource/RoadNetworkCsvLoader.cpp dto/RouteResponse.cpp service/RoadGraphBuilder.cpp service/ShortestPathFinder.cpp util/GeoUtil.cpp util/CryptoUtil.cpp persistence/Database.cpp persistence/SqliteLocationRepository.cpp persistence/SqliteRoadRepository.cpp persistence/SqliteHospitalRepository.cpp persistence/SqliteAmbulanceRepository.cpp persistence/SqliteEmergencyRepository.cpp persistence/SqlitePatientLocationRepository.cpp persistence/SqliteUserRepository.cpp persistence/SqliteSessionRepository.cpp persistence/DatabaseBootstrap.cpp build/sqlite3.o -o build/m3_db_test.exe -lws2_32 -lwsock32 -lbcrypt -static-libgcc -static-libstdc++
    build\m3_db_test.exe

    echo.
    echo [5/8] Running Module 3 Phase 2 Dispatch Integration Tests...
    g++ -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I. -I../m1_dispatch test/test_dispatch.cpp model/Location.cpp structure/LocationNode.cpp structure/RoadEdge.cpp structure/RoadGraph.cpp repository/LocationRepository.cpp datasource/LocationCsvLoader.cpp datasource/RoadNetworkCsvLoader.cpp dto/RouteResponse.cpp service/RoadGraphBuilder.cpp service/ShortestPathFinder.cpp service/NearbyServicesFinder.cpp service/TransportEstimator.cpp service/EmergencyDispatcher.cpp ../m1_dispatch/service/TriageRuleEngine.cpp util/GeoUtil.cpp util/CryptoUtil.cpp service/AuthService.cpp persistence/Database.cpp persistence/SqliteLocationRepository.cpp persistence/SqliteRoadRepository.cpp persistence/SqliteHospitalRepository.cpp persistence/SqliteAmbulanceRepository.cpp persistence/SqliteEmergencyRepository.cpp persistence/SqlitePatientLocationRepository.cpp persistence/SqliteUserRepository.cpp persistence/SqliteSessionRepository.cpp persistence/DatabaseBootstrap.cpp build/sqlite3.o -o build/m3_dispatch_test.exe -lws2_32 -lwsock32 -lbcrypt -static-libgcc -static-libstdc++
    build\m3_dispatch_test.exe

    echo.
    echo [6/8] Running Module 3 Phase 3 Authentication & Security Tests...
    g++ -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I. -I../m1_dispatch test/test_auth.cpp model/Location.cpp structure/LocationNode.cpp structure/RoadEdge.cpp structure/RoadGraph.cpp repository/LocationRepository.cpp datasource/LocationCsvLoader.cpp datasource/RoadNetworkCsvLoader.cpp dto/RouteResponse.cpp service/RoadGraphBuilder.cpp service/ShortestPathFinder.cpp service/NearbyServicesFinder.cpp service/TransportEstimator.cpp service/RestApiServer.cpp service/EmergencyDispatcher.cpp ../m1_dispatch/service/TriageRuleEngine.cpp util/GeoUtil.cpp util/CryptoUtil.cpp service/AuthService.cpp persistence/Database.cpp persistence/SqliteLocationRepository.cpp persistence/SqliteRoadRepository.cpp persistence/SqliteHospitalRepository.cpp persistence/SqliteAmbulanceRepository.cpp persistence/SqliteEmergencyRepository.cpp persistence/SqlitePatientLocationRepository.cpp persistence/SqliteUserRepository.cpp persistence/SqliteSessionRepository.cpp persistence/DatabaseBootstrap.cpp build/sqlite3.o -o build/m3_auth_test.exe -lws2_32 -lwsock32 -lbcrypt -static-libgcc -static-libstdc++
    build\m3_auth_test.exe

    echo.
    echo [7/8] Running Module 3 Phase 4 Multi-User Portals Integration Tests...
    g++ -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I. -I../m1_dispatch test/test_portals.cpp model/Location.cpp structure/LocationNode.cpp structure/RoadEdge.cpp structure/RoadGraph.cpp repository/LocationRepository.cpp datasource/LocationCsvLoader.cpp datasource/RoadNetworkCsvLoader.cpp dto/RouteResponse.cpp service/RoadGraphBuilder.cpp service/ShortestPathFinder.cpp service/NearbyServicesFinder.cpp service/TransportEstimator.cpp util/GeoUtil.cpp util/CryptoUtil.cpp service/AuthService.cpp service/EmergencyDispatcher.cpp ../m1_dispatch/service/TriageRuleEngine.cpp persistence/Database.cpp persistence/SqliteLocationRepository.cpp persistence/SqliteRoadRepository.cpp persistence/SqliteHospitalRepository.cpp persistence/SqliteAmbulanceRepository.cpp persistence/SqliteEmergencyRepository.cpp persistence/SqlitePatientLocationRepository.cpp persistence/SqliteUserRepository.cpp persistence/SqliteSessionRepository.cpp persistence/DatabaseBootstrap.cpp build/sqlite3.o -o build/m3_portals_test.exe -lws2_32 -lwsock32 -lbcrypt -static-libgcc -static-libstdc++
    build\m3_portals_test.exe

    echo.
    echo [8/8] Running Module 3 Phase 5 Reliability, GPS & Live Tracking Tests...
    g++ -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I. -I../m1_dispatch test/test_reliability.cpp model/Location.cpp structure/LocationNode.cpp structure/RoadEdge.cpp structure/RoadGraph.cpp repository/LocationRepository.cpp datasource/LocationCsvLoader.cpp datasource/RoadNetworkCsvLoader.cpp dto/RouteResponse.cpp service/RoadGraphBuilder.cpp service/ShortestPathFinder.cpp service/NearbyServicesFinder.cpp service/TransportEstimator.cpp util/GeoUtil.cpp util/CryptoUtil.cpp service/AuthService.cpp service/EmergencyDispatcher.cpp ../m1_dispatch/service/TriageRuleEngine.cpp persistence/Database.cpp persistence/SqliteLocationRepository.cpp persistence/SqliteRoadRepository.cpp persistence/SqliteHospitalRepository.cpp persistence/SqliteAmbulanceRepository.cpp persistence/SqliteEmergencyRepository.cpp persistence/SqlitePatientLocationRepository.cpp persistence/SqliteUserRepository.cpp persistence/SqliteSessionRepository.cpp persistence/DatabaseBootstrap.cpp build/sqlite3.o -o build/m3_reliability_test.exe -lws2_32 -lwsock32 -lbcrypt -static-libgcc -static-libstdc++
    build\m3_reliability_test.exe
    cd ..
)

echo.
echo ========================================================================
echo  ALL TEST SUITES EXECUTED SUCCESSFULLY!
echo ========================================================================
