@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
IF EXIST cache_files\aspen.txt (
  SET CACHE_COMMAND=powershell -Command "& { " ^
    "$setupTimestamp = (Get-Item '%~dp0setup.bat').LastWriteTime; " ^
    "$aspenTimestamp = (Get-Item 'cache_files\\aspen.txt').LastWriteTime; " ^
    "if($setupTimestamp -lt $aspenTimestamp) {" ^
    "  Write-Output '0';" ^
    "} else {" ^
    "  Write-Output '1';" ^
    "}" ^
  "}"
  FOR /F "delims=" %%A IN ('CALL !CACHE_COMMAND!') DO SET IS_CACHED=%%A
  IF "!IS_CACHED!"=="0" (
    EXIT /B 0
  )
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq delims=" %%i IN (` ^
    !VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
CALL :DownloadAndExtract "doctest-2.4.11" ^
  "https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.zip"
CALL :DownloadAndExtract "pybind11-2.13.6" ^
  "https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.zip"
CALL :DownloadAndExtract "Python-3.13.0" ^
  "https://www.python.org/ftp/python/3.13.0/Python-3.13.0.tgz"
IF %BUILD_NEEDED%==1 (
  PUSHD Python-3.13.0
  PUSHD PCbuild
  CALL build.bat -c Debug
  CALL build.bat -c Release
  POPD
  COPY PCbuild\amd64\pyconfig.h Include
  POPD
)
IF NOT EXIST cache_files (
  MD cache_files
)
ECHO timestamp > cache_files\aspen.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!

:DownloadAndExtract
SET FOLDER=%~1
SET URL=%~2
SET BUILD_NEEDED=0
FOR /F "tokens=* delims=/" %%A IN ("%URL%") DO (
  SET ARCHIVE=%%~nxA
)
SET EXTENSION=%ARCHIVE:~-4%
IF EXIST !FOLDER! (
  EXIT /B 0
)
powershell -Command "$ProgressPreference = 'SilentlyContinue'; "^
  "Invoke-WebRequest -Uri '%URL%' -OutFile '%ARCHIVE%'"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to download !ARCHIVE!.
  SET EXIT_STATUS=1
  EXIT /B
)
SET EXTRACT_PATH=_extract_tmp
RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
MD "!EXTRACT_PATH!"
IF /I "!EXTENSION!"==".zip" (
  powershell -Command "$ProgressPreference = 'SilentlyContinue'; "^
    "Expand-Archive -Path '%ARCHIVE%' -DestinationPath '%EXTRACT_PATH%'"
) ELSE IF /I "!EXTENSION!"==".tgz" (
  powershell -Command "$ProgressPreference = 'SilentlyContinue'; "^
    "tar -xf '%ARCHIVE%' -C '%EXTRACT_PATH%'"
) ELSE IF /I "%ARCHIVE:~-7%"==".tar.gz" (
  powershell -Command "$ProgressPreference = 'SilentlyContinue'; "^
    "tar -xf '%ARCHIVE%' -C '%EXTRACT_PATH%'"
) ELSE (
  ECHO Error: Unknown archive format for %ARCHIVE%.
  SET EXIT_STATUS=1
  EXIT /B 1
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
RD /S /Q "!EXTRACT_PATH!"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to extract !ARCHIVE!.
  SET EXIT_STATUS=1
  EXIT /B 0
)
SET BUILD_NEEDED=1
DEL /F /Q !ARCHIVE!
EXIT /B 0
