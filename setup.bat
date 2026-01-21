@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET SETUP_HASH=%%H
)
IF EXIST cache_files\aspen.txt (
  SET /P CACHED_HASH=<cache_files\aspen.txt
  IF "!SETUP_HASH!"=="!CACHED_HASH!" EXIT /B 0
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq delims=" %%i IN (` ^
    !VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
CALL :DownloadAndExtract "doctest-2.4.12" ^
  "https://github.com/doctest/doctest/archive/refs/tags/v2.4.12.zip" ^
  "7a7afb5f70d0b749d49ddfcb8a454299a8fcd53e9db9c131abe99b456e88a1fe"
CALL :DownloadAndExtract "pybind11-3.0.1" ^
  "https://github.com/pybind/pybind11/archive/refs/tags/v3.0.1.zip" ^
  "20fb420fe163d0657a262a8decb619b7c3101ea91db35f1a7227e67c426d4c7e"
CALL :DownloadAndExtract "Python-3.14.2" ^
  "https://www.python.org/ftp/python/3.14.2/Python-3.14.2.tgz" ^
  "c609e078adab90e2c6bacb6afafacd5eaf60cd94cf670f1e159565725fcd448d"
IF !BUILD_NEEDED!==1 (
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
)
IF !EXIT_STATUS! NEQ 0 (
  EXIT /B !EXIT_STATUS!
)
IF NOT EXIST cache_files (
  MD cache_files
)
>cache_files\aspen.txt ECHO !SETUP_HASH!
EXIT /B 0
ENDLOCAL

:DownloadAndExtract
SET FOLDER=%~1
SET URL=%~2
SET EXPECTED_HASH=%~3
SET BUILD_NEEDED=0
FOR /F "tokens=* delims=/" %%A IN ("%URL%") DO (
  SET ARCHIVE=%%~nxA
)
IF EXIST !FOLDER! (
  EXIT /B 0
)
IF NOT EXIST !ARCHIVE! (
  curl -fsL -o "!ARCHIVE!" "!URL!"
  IF ERRORLEVEL 1 (
    ECHO Error: Failed to download !ARCHIVE!.
    SET EXIT_STATUS=1
    EXIT /B
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
  SET EXIT_STATUS=1
  SET ACTUAL_HASH=
  EXIT /B
)
SET ACTUAL_HASH=
SET EXTRACT_PATH=_extract_tmp
RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
MD "!EXTRACT_PATH!"
tar -xf "!ARCHIVE!" -C "!EXTRACT_PATH!"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to extract !ARCHIVE!.
  SET EXIT_STATUS=1
  EXIT /B 0
)
SET DETECTED_FOLDER=
FOR %%F IN ("!EXTRACT_PATH!\*") DO (
  IF "!DETECTED_FOLDER!"=="" (
    SET DETECTED_FOLDER=%%F
  ) ELSE (
    SET DETECTED_FOLDER=MULTIPLE
  )
)
FOR /D %%F IN ("!EXTRACT_PATH!\*") DO (
  IF "!DETECTED_FOLDER!"=="" (
    SET DETECTED_FOLDER=%%F
  ) ELSE (
    SET DETECTED_FOLDER=MULTIPLE
  )
)
IF "!DETECTED_FOLDER!"=="MULTIPLE" (
  REN "!EXTRACT_PATH!" "!FOLDER!"
) ELSE IF NOT "!DETECTED_FOLDER!"=="!EXTRACT_PATH!\!FOLDER!" (
  MOVE /Y "!DETECTED_FOLDER!" "!FOLDER!" >NUL
) ELSE (
  MOVE /Y "!EXTRACT_PATH!\!FOLDER!" "!ROOT!" >NUL
)
RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
SET BUILD_NEEDED=1
DEL /F /Q !ARCHIVE!
EXIT /B 0
