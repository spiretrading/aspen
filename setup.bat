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
IF NOT EXIST doctest-2.4.11 (
  powershell -Command "Invoke-WebRequest "^
    "-Uri https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.zip "^
    "-OutFile v2.4.11.zip"
  IF !ERRORLEVEL! LEQ 0 (
    powershell -Command "Expand-Archive -Path v2.4.11.zip -DestinationPath ."
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q v2.4.11.zip
)
IF NOT EXIST pybind11-2.13.6 (
  powershell -Command "Invoke-WebRequest "^
    "-Uri https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.zip "^
    "-OutFile pybind11-2.13.6.zip"
  IF !ERRORLEVEL! LEQ 0 (
    powershell ^
      -Command "Expand-Archive -Path pybind11-2.13.6.zip -DestinationPath ."
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q pybind11-2.13.6.zip
)
IF NOT EXIST Python-3.13.0 (
  powershell -Command "Invoke-WebRequest "^
    "-Uri https://www.python.org/ftp/python/3.13.0/Python-3.13.0.tgz "^
    "-OutFile Python-3.13.0.tgz"
  IF !ERRORLEVEL! LEQ 0 (
    powershell -Command "tar -xf Python-3.13.0.tgz"
    PUSHD Python-3.13.0
    PUSHD PCbuild
    CALL build.bat -c Debug
    CALL build.bat -c Release
    POPD
    COPY PCbuild\amd64\pyconfig.h Include
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q Python-3.13.0.tgz
)
IF NOT EXIST cache_files (
  MD cache_files
)
ECHO timestamp > cache_files\aspen.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!
