@ECHO OFF
SETLOCAL EnableDelayedExpansion
CALL :CheckCache "aspen"
IF ERRORLEVEL 1 EXIT /B 0
CALL :SetupVSEnvironment
CALL :AddDependency "doctest-2.4.12" ^
  "https://github.com/doctest/doctest/archive/refs/tags/v2.4.12.zip" ^
  "7a7afb5f70d0b749d49ddfcb8a454299a8fcd53e9db9c131abe99b456e88a1fe"
CALL :AddDependency "pybind11-3.0.1" ^
  "https://github.com/pybind/pybind11/archive/refs/tags/v3.0.1.zip" ^
  "20fb420fe163d0657a262a8decb619b7c3101ea91db35f1a7227e67c426d4c7e"
CALL :AddDependency "Python-3.14.2" ^
  "https://www.python.org/ftp/python/3.14.2/Python-3.14.2.tgz" ^
  "c609e078adab90e2c6bacb6afafacd5eaf60cd94cf670f1e159565725fcd448d" ^
  ":BuildPython"
CALL :InstallDependencies
IF ERRORLEVEL 1 EXIT /B 1
CALL :Commit
ENDLOCAL
EXIT /B !ERRORLEVEL!

:BuildPython
PUSHD Python-3.14.2
PUSHD PCbuild
CALL build.bat -c Debug
CALL build.bat -c Release
POPD
IF EXIST "PCbuild\amd64\pyconfig.h" (
  COPY /Y "PCbuild\amd64\pyconfig.h" "Include\pyconfig.h"
) ELSE (
  COPY /Y "PC\pyconfig.h" "Include\pyconfig.h"
)
POPD
EXIT /B 0

:CheckCache
SET CACHE_NAME=%~1
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET SETUP_HASH=%%H
)
IF EXIST "cache_files\!CACHE_NAME!.txt" (
  SET /P CACHED_HASH=<"cache_files\!CACHE_NAME!.txt"
  IF "!SETUP_HASH!"=="!CACHED_HASH!" EXIT /B 1
)
EXIT /B 0

:Commit
IF NOT EXIST cache_files (
  MD cache_files
)
>"cache_files\!CACHE_NAME!.txt" ECHO !SETUP_HASH!
EXIT /B 0

:SetupVSEnvironment
SET VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
FOR /F "usebackq delims=" %%i IN (` ^
    "!VSWHERE!" -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
EXIT /B 0

:AddDependency
IF NOT DEFINED NEXT_DEPENDENCY_INDEX SET NEXT_DEPENDENCY_INDEX=0
SET DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].NAME=%~1
SET DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].URL=%~2
SET DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].HASH=%~3
SET DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].BUILD=%~4
SET /A NEXT_DEPENDENCY_INDEX+=1
EXIT /B 0

:InstallDependencies
SET I=0
:InstallDependenciesLoop
IF NOT DEFINED DEPENDENCIES[%I%].NAME EXIT /B 0
CALL :DownloadAndExtract "!DEPENDENCIES[%I%].NAME!" "!DEPENDENCIES[%I%].URL!" ^
  "!DEPENDENCIES[%I%].HASH!" "!DEPENDENCIES[%I%].BUILD!"
IF ERRORLEVEL 1 EXIT /B 1
SET /A I+=1
GOTO InstallDependenciesLoop

:DownloadAndExtract
SET FOLDER=%~1
SET URL=%~2
SET EXPECTED_HASH=%~3
SET BUILD_LABEL=%~4
FOR /F "tokens=* delims=/" %%A IN ("!URL!") DO (
  SET ARCHIVE=%%~nxA
)
IF EXIST "!FOLDER!" (
  EXIT /B 0
)
IF NOT EXIST "!ARCHIVE!" (
  curl -fsL -o "!ARCHIVE!" "!URL!"
  IF ERRORLEVEL 1 (
    ECHO Error: Failed to download !ARCHIVE!.
    EXIT /B 1
  )
)
FOR /F "skip=1 tokens=*" %%H IN ('certutil -hashfile "!ARCHIVE!" SHA256') DO (
  IF NOT DEFINED ACTUAL_HASH SET ACTUAL_HASH=%%H
)
SET ACTUAL_HASH=!ACTUAL_HASH: =!
IF /I NOT "!ACTUAL_HASH!"=="!EXPECTED_HASH!" (
  ECHO Error: SHA256 mismatch for !ARCHIVE!.
  ECHO   Expected: !EXPECTED_HASH!
  ECHO   Actual:   !ACTUAL_HASH!
  DEL /F /Q "!ARCHIVE!"
  SET ACTUAL_HASH=
  EXIT /B 1
)
SET ACTUAL_HASH=
MD "!FOLDER!"
tar -xf "!ARCHIVE!" -C "!FOLDER!" --strip-components=1
IF ERRORLEVEL 1 (
  ECHO Error: Failed to extract !ARCHIVE!.
  RD /S /Q "!FOLDER!" >NUL 2>NUL
  EXIT /B 1
)
DEL /F /Q "!ARCHIVE!"
IF DEFINED BUILD_LABEL CALL %BUILD_LABEL%
EXIT /B 0
