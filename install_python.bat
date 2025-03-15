@ECHO OFF
IF "%~1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%~1
)
SET PYTHON_PATH=
FOR /f "delims=" %%i IN ('python -m site --user-site 2^>nul') ^
  DO SET PYTHON_PATH="%%i"
IF "%PYTHON_PATH%" == "" (
  ECHO Error: Unable to retrieve Python user-site path.
  EXIT /B 1
)
IF NOT EXIST %PYTHON_PATH% (
  MKDIR %PYTHON_PATH%
  IF ERRORLEVEL 1 (
    ECHO Error: Unable to create directory "%PYTHON_PATH%".
    EXIT /B 1
  )
)
IF NOT EXIST "Libraries\%CONFIG%\aspen.pyd" (
  ECHO Error: Source file "Libraries\%CONFIG%\aspen.pyd" not found.
  EXIT /B 1
)
COPY "Libraries\%CONFIG%\aspen.pyd" %PYTHON_PATH% >nul
IF ERRORLEVEL 1 (
  ECHO Error: Failed to copy "aspen.pyd" to "%PYTHON_PATH%".
  EXIT /B 1
)
ECHO File copied successfully to %PYTHON_PATH%.
EXIT /B 0
