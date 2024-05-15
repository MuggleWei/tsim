@echo off

setlocal ENABLEDELAYEDEXPANSION

:: handle argv
set count=0
for %%x in (%*) do (
    set /a count+=1
)

if %count% LSS 1 (
    echo "[ERROR] build.bat without build type"
	echo "[ERROR] Usage: build.bat <Debug|Release> [shared|static]"
    exit /b
)

set BUILD_TYPE=%~1

if %count% GEQ 2 (
	set LIB_TYPE=%~2
) else (
	set LIB_TYPE=shared
)


set "CMAKE_SHARED_LIBS=ON"
if "%LIB_TYPE%"=="shared" (
	set "CMAKE_SHARED_LIBS=ON"
) else (
	set "CMAKE_SHARED_LIBS=OFF"
)

echo build type: %BUILD_TYPE%
echo lib type: %LIB_TYPE%
echo CMAKE_SHARED_LIBS: %CMAKE_SHARED_LIBS%

set origin_dir=%~dp0
set build_dir=%origin_dir%build
set dist_dir=%origin_dir%dist

if exist %build_dir% (
	echo remove build dir
	rd /s /q %build_dir%
)

echo make build dir
md build

if exist %dist_dir% (
	echo remove dist dir
	rd /s /q %dist_dir%
)

cmake ^
	-S %origin_dir% -B %build_dir% ^
	-G "Visual Studio 16 2019" -A x64 ^
	-DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
	-DBUILD_SHARED_LIBS=%CMAKE_SHARED_LIBS% ^
	-DCMAKE_INSTALL_PREFIX=%dist_dir% ^
	-DBUILD_TESTING=OFF ^
	-DLIBXML2_WITH_PROGRAMS=OFF ^
	-DLIBXML2_WITH_TESTS=OFF ^
	-DLIBXML2_WITH_PYTHON=OFF ^
	-DLIBXML2_WITH_LZMA=OFF ^
	-DLIBXML2_WITH_ZLIB=OFF ^
	-DLIBXML2_WITH_ICONV=OFF
cmake --build %build_dir% --config %BUILD_TYPE%
cmake --build %build_dir% --config %BUILD_TYPE% --target install
