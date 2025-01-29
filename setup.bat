@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
IF EXIST cache_files\aspen.txt (
  powershell -Command "& { "^
    "$setupTimestamp = (Get-Item '%~dp0setup.bat').LastWriteTime; "^
    "$aspenTimestamp = (Get-Item 'cache_files\\aspen.txt').LastWriteTime; "^
    "if ($setupTimestamp -lt $aspenTimestamp) {"^
    "  Exit 0;"^
    "} else {"^
    "  Exit 1;"^
    "}"^
  "}"
  IF ERRORLEVEL 0 (
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
IF EXIST %FOLDER% (
  EXIT /B 0
)
powershell -Command "Invoke-WebRequest -Uri '%URL%' -OutFile '%ARCHIVE%'"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to download %ARCHIVE%.
  SET EXIT_STATUS=1
  EXIT /B
)
IF /I "%EXTENSION%"==".zip" (
  powershell -Command "Expand-Archive -Path '%ARCHIVE%' -DestinationPath ."
) ELSE IF /I "%EXTENSION%"==".tgz" (
  powershell -Command "tar -xf '%ARCHIVE%'"
) ELSE IF /I "%ARCHIVE:~-7%"==".tar.gz" (
  powershell -Command "tar -xf '%ARCHIVE%'"
) ELSE (
  ECHO Error: Unknown archive format for %ARCHIVE%.
  SET EXIT_STATUS=1
  EXIT /B 1
)
IF ERRORLEVEL 1 (
  ECHO Error: Failed to extract %ARCHIVE%.
  SET EXIT_STATUS=1
  EXIT /B 0
)
SET BUILD_NEEDED=1
DEL /F /Q %ARCHIVE%
EXIT /B 0
