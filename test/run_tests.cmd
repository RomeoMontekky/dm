@echo off

set COMPILER=mingw
set CONFIGURATION=debug
set TEST_COMMAND=..\build_%COMPILER%\%CONFIGURATION%\bin\console.exe

rem TESTS LIST BEGIN
   call :run_one_test function_test
   call :run_one_test function_table
rem TESTS LIST END

goto :eof

:run_one_test
%TEST_COMMAND% < %1.in > %1.out 2> NUL
echo n | comp %1.out %1.exp > NUL 2> NUL
if ERRORLEVEL 1 (echo %1...Fail & exit 1) else (echo %1...OK & del %1.out)
goto :eof

