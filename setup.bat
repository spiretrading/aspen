@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT="%cd%"
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq delims=" %%i IN (`%VSWHERE% -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
IF NOT EXIST Catch2-2.6.1 (
  git clone --branch v2.6.1 https://github.com/catchorg/Catch2.git Catch2-2.6.1
  IF !ERRORLEVEL! NEQ 0 (
    RD /S /Q Catch2-2.6.1
    SET EXIT_STATUS=1
  )
)
IF NOT EXIST pybind11-2.4.3 (
  git clone --branch v2.4.3 https://github.com/pybind/pybind11.git pybind11-2.4.3
  IF !ERRORLEVEL! NEQ 0 (
    RD /S /Q pybind11-2.4.3
    SET EXIT_STATUS=1
  )
)
IF NOT EXIST Python-3.8.1 (
  wget https://www.python.org/ftp/python/3.8.1/Python-3.8.1.tgz --no-check-certificate
  IF !ERRORLEVEL! EQU 0 (
    gzip -d -c Python-3.8.1.tgz | tar -xf -
    PUSHD Python-3.8.1
    PUSHD PCbuild
    CALL build.bat -E -c Debug -p Win32
    CALL build.bat -E -c Release -p Win32
    POPD
    COPY PC\pyconfig.h Include
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q Python-3.8.1.tgz
)
ENDLOCAL
EXIT /B %EXIT_STATUS%
