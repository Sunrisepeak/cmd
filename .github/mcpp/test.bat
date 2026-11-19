@echo off
rem The batch script the mcpp workflow runs on every target that can run.
setlocal enabledelayedexpansion
set N=0
:loop
set /a N+=1
if !N! lss 3 goto loop
echo N=!N!
call :sub hi there
echo errorlevel=%errorlevel%
for %%f in (a b c) do echo item %%f
rem files: a writable directory (a run on a read-only disk leaves this part out)
md okt
echo x> okt\f1.txt
copy okt\f1.txt okt\f2.txt >nul
ren okt\f2.txt f3.txt
type okt\f3.txt
dir /b okt
del okt\f1.txt okt\f3.txt
rd okt
if not exist okt echo okt removed
rem end of files
set /a X=6*7
echo X=%X%
rem process: a program the interpreter starts, itself (a run where no program can be started leaves this part out)
"%COMSPEC%" /c exit 3
echo child=%errorlevel%
rem end of process
echo done
endlocal
goto :eof
:sub
echo sub args %1 %2
exit /b 5
