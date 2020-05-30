@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
IF EXIST cache_files\aspen.txt (
  FOR /F %%i IN (
      'ls -l --time-style=full-iso "%~dp0\setup.bat" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
        'ls -l --time-style=full-iso cache_files\aspen.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" LSS "%%j" (
        EXIT /B 0
      )
    )
  )
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq delims=" %%i IN (`!VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
IF NOT EXIST doctest-2.3.6 (
  wget https://github.com/onqtam/doctest/archive/2.3.6.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    unzip 2.3.6.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q 2.3.6.zip
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
  IF !ERRORLEVEL! LEQ 0 (
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
IF NOT EXIST cache_files (
  MD cache_files
)
ECHO timestamp > cache_files\aspen.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!
