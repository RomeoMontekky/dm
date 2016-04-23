@echo off

set TEST_COMMAND=..\bin\console.exe

set SUCCESSFUL_TESTS=0
set FAILED_TESTS=0

for %%f in (*.in) do ( call :run_one_test %%~nf )

echo ---------------------------------------
echo Successful tests - %SUCCESSFUL_TESTS%, Failed tests - %FAILED_TESTS%
goto :eof

:run_one_test
%TEST_COMMAND% %1.in > %1.out
echo n | comp %1.out %1.exp > NUL 2> NUL
if ERRORLEVEL 1 (
   echo %1 ... Fail & set /a FAILED_TESTS=%FAILED_TESTS%+1
) else (
   echo %1 ... OK & del %1.out & set /a SUCCESSFUL_TESTS=%SUCCESSFUL_TESTS%+1
)
goto :eof

