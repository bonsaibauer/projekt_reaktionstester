@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
SET index=1

@echo off
FOR /D %%a IN (*) DO (
  if exist "%%a\main.c" (
        set Arr[!index!]=%%a
        SET /a index+=1
    )
)

:Auswahlmenu
set "x=1"
cls
echo.
echo Auswahlmenue
echo ===========
echo.
:SymLoop
if not defined Arr[%x%] goto :endLoop
call set VAL=%%Arr[%x%]%%
echo %x%:%VAL%
REM do your stuff VAL
SET /a "x+=1"
GOTO :SymLoop

:endLoop
set asw=0
set /p asw="Bitte eine Auswahl treffen: "
call set VAL=%%Arr[%asw%]%%
echo %VAL%

call build.bat %VAL%
if not exist "%VAL%\main.elf" (
  pause
  goto :Auswahlmenu
)
..\App\Java\bin\java -jar ..\App\mspsimHeintz\mspsim_heintz.jar -platform=mspexp430f5529 %VAL%\main.elf start
goto :Auswahlmenu