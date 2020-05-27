@ECHO OFF
SETLOCAL
IF [%1] == [] (
  SET config=Release
) ELSE (
  SET config="%1"
)
IF "%1" == "clean" (
  git clean -fxd -e *Dependencies*
) ELSE IF "%1" == "reset" (
  IF EXIST Dependencies (
    RD /S /Q Dependencies
  )
  git clean -fxd
) ELSE (
  cmake --build . --target INSTALL --config %config%
)
ENDLOCAL
