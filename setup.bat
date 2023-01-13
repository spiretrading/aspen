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
    tar -xf 2.4.9.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q 2.4.9.zip
)
IF NOT EXIST pybind11-2.10.3 (
  wget https://github.com/pybind/pybind11/archive/refs/tags/v2.10.3.zip -O pybind11-2.10.3.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf pybind11-2.10.3.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q pybind11-2.10.3.zip
)
IF NOT EXIST Python-3.11.1 (
  wget https://www.python.org/ftp/python/3.11.1/Python-3.11.1.tgz --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    gzip -d -c Python-3.11.1.tgz | tar -xf -
    PUSHD Python-3.11.1
    PUSHD PCbuild
    CALL build.bat -E -c Debug -p Win32
    CALL build.bat -E -c Release -p Win32
    POPD
    COPY PC\pyconfig.h Include
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q Python-3.11.1.tgz
)
IF NOT EXIST cache_files (
  MD cache_files
)
ECHO timestamp > cache_files\aspen.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!
