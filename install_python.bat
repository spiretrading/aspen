IF "%1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%1
)
FOR /f "delims=" %%i IN ('python -m site --user-site') DO SET PYTHON_PATH="%%i"
mkdir "%PYTHON_PATH%" /p
COPY "Libraries\%CONFIG%\aspen.pyd" "%PYTHON_PATH%"
