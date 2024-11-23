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
IF NOT EXIST doctest-2.4.9 (
  wget https://github.com/doctest/doctest/archive/refs/tags/v2.4.9.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf v2.4.9.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q v2.4.9.zip
)
IF NOT EXIST pybind11-2.13.6 (
  wget https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.zip -O pybind11-2.13.6.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf pybind11-2.13.6.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q pybind11-2.13.6.zip
)
IF NOT EXIST Python-3.13.0 (
  wget https://www.python.org/ftp/python/3.13.0/Python-3.13.0.tgz --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    gzip -d -c Python-3.13.0.tgz | tar -xf -
    PUSHD Python-3.13.0
    PUSHD PCbuild
    CALL build.bat -c Debug -p Win32
    CALL build.bat -c Release -p Win32
    POPD
    COPY PCbuild\win32\pyconfig.h Include
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
