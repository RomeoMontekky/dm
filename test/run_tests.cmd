@echo off

set COMPILER=mingw
set CONFIGURATION=debug
set TEST_COMMAND=..\build_%COMPILER%\%CONFIGURATION%\bin\console.exe

call :run_one_test normalization
call :run_one_test function_test
call :run_one_test function_table
goto :eof

:run_one_test
%TEST_COMMAND% %1.in > %1.out 2> NUL
echo n | comp %1.out %1.exp > NUL 2> NUL
if ERRORLEVEL 1 (echo %1...Fail) else (echo %1...OK & del %1.out)
goto :eof

