
@ECHO OFF

REM %1 == HAVOK_EXPORT_FILTERS_ROOT (taken from environment variable)
REM %2 == TARGET_PATH (full path to DLL)

setlocal
mkdir "%~1\filters" > nul
xcopy "%~2" "%~1\filters" /s /y >nul
if errorlevel 1 goto failure

echo Files copied to %1

REM We copy the output to a file so VisualStudio knows that the build step was successful

echo Files copied to %1 > "%~2.output"

exit /b 0

endlocal 


:failure

echo Couldn't copy files to %1... 
echo Make sure DLLs are not in use and that HAVOK_EXPORT_FILTERS_ROOT is defined

REM Return an ErrorLevel of 1
exit /b 1

