@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET DIRECTORY=%~dp0
SET ROOT=%cd%
CALL :ParseArgs %*
IF "!CONFIG!"=="clean" (
  CALL :CleanBuild "clean"
  EXIT /B 0
)
IF "!CONFIG!"=="reset" (
  CALL :CleanBuild "reset"
  EXIT /B 0
)
CALL :Configure
CALL :RunBuild
ENDLOCAL
EXIT /B !ERRORLEVEL!

:ParseArgs
SET DEPENDENCIES=
SET IS_DEPENDENCY=
SET CONFIG=
:ParseArgsLoop
SET ARG=%~1
IF "!IS_DEPENDENCY!"=="1" (
  SET DEPENDENCIES=!ARG!
  SET IS_DEPENDENCY=
  SHIFT
  GOTO ParseArgsLoop
) ELSE IF NOT "!ARG!"=="" (
  IF "!ARG:~0,3!"=="-DD" (
    SET IS_DEPENDENCY=1
  ) ELSE (
    SET CONFIG=!ARG!
  )
  SHIFT
  GOTO ParseArgsLoop
)
EXIT /B 0

:CleanBuild
IF "%~1"=="reset" (
  git clean -ffxd
) ELSE (
  git clean -ffxd -e "*Dependencies*"
)
IF EXIST "Dependencies\cache_files\aspen.txt" (
  DEL "Dependencies\cache_files\aspen.txt"
)
EXIT /B 0

:Configure
IF "!CONFIG!"=="" (
  IF EXIST "CMakeFiles\config.txt" (
    FOR /F %%i IN ('TYPE "CMakeFiles\config.txt"') DO (
      SET CONFIG=%%i
    )
  ) ELSE (
    SET CONFIG=Release
  )
)
IF /I "!CONFIG!"=="release" SET CONFIG=Release
IF /I "!CONFIG!"=="debug" SET CONFIG=Debug
IF /I "!CONFIG!"=="relwithdebinfo" SET CONFIG=RelWithDebInfo
IF /I "!CONFIG!"=="minsizerel" SET CONFIG=MinSizeRel
IF NOT "!DEPENDENCIES!"=="" (
  CALL "!DIRECTORY!configure.bat" -DD="!DEPENDENCIES!"
) ELSE (
  CALL "!DIRECTORY!configure.bat"
)
EXIT /B !ERRORLEVEL!

:RunBuild
cmake --build "!ROOT!" --target INSTALL --config "!CONFIG!" --parallel
IF ERRORLEVEL 1 EXIT /B 1
>"CMakeFiles\config.txt" ECHO !CONFIG!
EXIT /B 0
