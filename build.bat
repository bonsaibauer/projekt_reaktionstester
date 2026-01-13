@echo off
setlocal

set FILE_DIR=%1
echo %FILE_DIR%

setlocal enabledelayedexpansion enableextensions
set list=
for %%a in (%FILE_DIR%/*.c) do set list=!list! %FILE_DIR%/%%a
set list=%list:~1%
echo %list%


set GCC_PREFIX=%~dp0..\App\msp430\
set LIBRARY_PATH=%GCC_PREFIX%include\
set INCLUDE_PATH=%GCC_PREFIX%include\

set as=%GCC_PREFIX%bin\msp430-elf-as.exe

set PATH=%GCC_PREFIX%bin\;%GCC_PREFIX%libexec\gcc\msp430-elf\9.3.1\;%PATH%

del "%FILE_DIR%\\main.elf"

msp430-elf-gcc -mmcu=msp430f5529 -funsigned-char -B%GCC_PREFIX%bin\ -L%LIBRARY_PATH% -I%INCLUDE_PATH% -o %FILE_DIR%/main.elf %list% -g3 -lm
IF %ERRORLEVEL% NEQ 0 (
  IF "%2" == "ErrorNone" exit /b 0 
  exit /b 1
)
exit /b 0
endlocal

