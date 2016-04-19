@echo off

set TEST_COMMAND=..\bin\console.exe

set SUCCESSFUL_TESTS=0
set FAILED_TESTS=0

call :run_one_test expression_base
call :run_one_test expression_normalization
call :run_one_test expression_simplification
call :run_one_test function_compare
call :run_one_test function_copy
call :run_one_test function_display
call :run_one_test function_display_all
call :run_one_test function_eval
call :run_one_test function_print
call :run_one_test function_remove
call :run_one_test function_remove_all
call :run_one_test function_table
call :run_one_test operation_base
call :run_one_test operation_priority

echo ---------------------------------------
echo Successful tests - %SUCCESSFUL_TESTS%, Failed tests - %FAILED_TESTS%
goto :eof

:run_one_test
%TEST_COMMAND% %1.in > %1.out
echo n | comp %1.out %1.exp > NUL 2> NUL
if ERRORLEVEL 1 (echo %1...Fail & set /a FAILED_TESTS=%FAILED_TESTS%+1) else (echo %1...OK & del %1.out & set /a SUCCESSFUL_TESTS=%SUCCESSFUL_TESTS%+1)
goto :eof

