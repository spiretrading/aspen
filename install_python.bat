@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "DIRECTORY=%~dp0"
SET "CONFIG=%~1"
IF "!CONFIG!"=="" SET "CONFIG=Release"
IF /I "!CONFIG!"=="release" (
  SET "CONFIG=Release"
) ELSE IF /I "!CONFIG!"=="debug" (
  SET "CONFIG=Debug"
) ELSE IF /I "!CONFIG!"=="relwithdebinfo" (
  SET "CONFIG=RelWithDebInfo"
) ELSE IF /I "!CONFIG!"=="minsizerel" (
  SET "CONFIG=MinSizeRel"
) ELSE (
  ECHO Error: Invalid configuration "!CONFIG!".
  EXIT /B 1
)
SET "PYTHON_PATH="
FOR /F "delims=" %%i IN ('python -m site --user-site 2^>NUL') DO (
  SET "PYTHON_PATH=%%i"
)
IF "!PYTHON_PATH!"=="" (
  ECHO Error: Unable to retrieve Python user-site path.
  EXIT /B 1
)
IF NOT EXIST "!PYTHON_PATH!" (
  MD "!PYTHON_PATH!"
  IF ERRORLEVEL 1 (
    ECHO Error: Unable to create directory "!PYTHON_PATH!".
    EXIT /B 1
  )
)
IF NOT EXIST "!DIRECTORY!Libraries\!CONFIG!\aspen.pyd" (
  ECHO Error: Source file "!DIRECTORY!Libraries\!CONFIG!\aspen.pyd" not found.
  EXIT /B 1
)
COPY /Y "!DIRECTORY!Libraries\!CONFIG!\aspen.pyd" "!PYTHON_PATH!\" >NUL
IF ERRORLEVEL 1 (
  ECHO Error: Failed to copy "aspen.pyd" to "!PYTHON_PATH!".
  EXIT /B 1
)
EXIT /B 0
ENDLOCAL
