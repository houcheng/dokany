if "%VisualStudioVersion%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
@rem msbuild dokan.sln /t:clean /p:Configuration="Win7 Release" /p:Platform="x64" && call update.bat
@rem msbuild dokan.sln /t:clean /p:Configuration="Win7 Release" /p:Platform="x64" && call update.bat
msbuild dokan.sln /t:build /p:Configuration="Win7 Release" /p:Platform="x64"  || goto :failure
msbuild dokan.sln /t:build /p:Configuration="Release" /p:Platform="x64"  || goto :failure
call update.bat
pause
goto :EOF

:failure
echo At least one build-command failed. The last command that failed returned with error %errorlevel% 1>&2
exit /b %errorlevel%
