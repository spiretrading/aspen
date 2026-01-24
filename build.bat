@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "DIRECTORY=%~dp0"
SET "ROOT=%cd%"
CALL :ParseArgs %*
IF /I "!CONFIG!"=="clean" (
  CALL :CleanBuild "clean"
  EXIT /B !ERRORLEVEL!
)
IF /I "!CONFIG!"=="reset" (
  CALL :CleanBuild "reset"
  EXIT /B !ERRORLEVEL!
)
CALL :Configure || EXIT /B 1
CALL :RunBuild
EXIT /B !ERRORLEVEL!
ENDLOCAL

:ParseArgs
SET "DEPENDENCIES="
SET "IS_DEPENDENCY="
SET "CONFIG="
:ParseArgsLoop
SET "ARG=%~1"
IF "!IS_DEPENDENCY!"=="1" (
  SET "DEPENDENCIES=!ARG!"
  SET "IS_DEPENDENCY="
  SHIFT
  GOTO ParseArgsLoop
) ELSE IF NOT "!ARG!"=="" (
  IF "!ARG:~0,4!"=="-DD=" (
    SET "DEPENDENCIES=!ARG:~4!"
  ) ELSE IF "!ARG!"=="-DD" (
    SET "IS_DEPENDENCY=1"
  ) ELSE (
    SET "CONFIG=!ARG!"
  )
  SHIFT
  GOTO ParseArgsLoop
)
EXIT /B 0

:CleanBuild
SET "CLEAN_ERROR=0"
IF "%~1"=="reset" (
  RD /S /Q Dependencies 2>NUL
  git clean -ffxd || SET "CLEAN_ERROR=1"
) ELSE (
  git clean -ffxd -e "*Dependencies*" || SET "CLEAN_ERROR=1"
  IF EXIST "Dependencies\cache_files\aspen.txt" (
    DEL "Dependencies\cache_files\aspen.txt" || SET "CLEAN_ERROR=1"
  )
)
EXIT /B !CLEAN_ERROR!

:Configure
IF "!CONFIG!"=="" (
  IF EXIST "CMakeFiles\config.txt" (
    SET /P CONFIG=<"CMakeFiles\config.txt"
  ) ELSE (
    SET "CONFIG=Release"
  )
)
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
IF NOT "!DEPENDENCIES!"=="" (
  CALL "!DIRECTORY!configure.bat" -DD="!DEPENDENCIES!"
) ELSE (
  CALL "!DIRECTORY!configure.bat"
)
EXIT /B !ERRORLEVEL!

:RunBuild
cmake --build "!ROOT!" --target INSTALL --config "!CONFIG!" --parallel ^
  || EXIT /B 1
>"CMakeFiles\config.txt" ECHO !CONFIG!
EXIT /B 0
