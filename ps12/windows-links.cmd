@echo off
rem PS12 Lab 1 on Windows: hard links, symbolic links, junctions, and loops.
rem Run from an elevated Command Prompt (or with Developer Mode on), inside ps12\:
rem     windows-links.cmd D: > results\05_windows_links.txt 2>&1
rem The optional argument is a second drive letter for the cross-drive test.

setlocal
set "SECOND=%~1"
if "%SECOND%"=="" set "SECOND=D:"
set "WORK=%TEMP%\ps12-links"
if exist "%WORK%" rmdir /s /q "%WORK%"
mkdir "%WORK%"
cd /d "%WORK%"
prompt $G$S
echo on

ver

@echo.
@echo ### 1.1 Hard and soft link to a source, then delete the source
echo original data> source.txt
mklink /H hard.txt source.txt
mklink soft.txt source.txt
dir
fsutil hardlink list hard.txt
type hard.txt
type soft.txt
del source.txt
dir
type hard.txt
type soft.txt
echo new source> source.txt
type soft.txt
type hard.txt

@echo.
@echo ### 1.2a Symbolic link chain: delete the source
mkdir sym-a
cd sym-a
echo level 0 data> source.txt
mklink level1 source.txt
mklink level2 level1
type level2
del source.txt
type level1
type level2
cd ..

@echo.
@echo ### 1.2b Symbolic link chain: delete level 1
mkdir sym-b
cd sym-b
echo level 0 data> source.txt
mklink level1 source.txt
mklink level2 level1
del level1
dir
type source.txt
type level2
cd ..

@echo.
@echo ### 1.2c Hard link chain: delete the source
mkdir hard-a
cd hard-a
echo level 0 data> source.txt
mklink /H level1 source.txt
mklink /H level2 level1
del source.txt
fsutil hardlink list level2
type level1
type level2
cd ..

@echo.
@echo ### 1.2d Hard link chain: delete level 1
mkdir hard-b
cd hard-b
echo level 0 data> source.txt
mklink /H level1 source.txt
mklink /H level2 level1
del level1
fsutil hardlink list level2
type level2
cd ..

@echo.
@echo ### 1.3 Link to a folder and a loop back to an upper level
mkdir top\sub
echo file in sub> top\sub\file.txt
mklink /H top-hard top
mklink /D top-soft top
mklink /J top-junction "%WORK%\top"
dir top-soft\sub
dir top-junction\sub
mklink /J top\sub\up "%WORK%\top"
dir top\sub
dir /s /b top > loop-list.txt
find /c /v "" loop-list.txt
powershell -NoProfile -Command "Get-Content loop-list.txt -Tail 3; (Get-Content loop-list.txt | Measure-Object -Property Length -Maximum).Maximum"

@echo.
@echo ### 1.4 Link on the second drive to a source on C:
if not exist %SECOND%\ goto nodrive
mkdir %SECOND%\ps12-links
mklink /H %SECOND%\ps12-links\hard.txt "%WORK%\hard.txt"
mklink %SECOND%\ps12-links\soft.txt "%WORK%\hard.txt"
mklink /J %SECOND%\ps12-links\junction "%WORK%\top"
type %SECOND%\ps12-links\soft.txt
dir %SECOND%\ps12-links\junction\sub
rmdir /s /q %SECOND%\ps12-links
goto done
:nodrive
@echo Drive %SECOND% not found. Pass another drive letter, for example: windows-links.cmd E:
:done
@echo off
cd /d "%TEMP%"
rmdir /s /q "%WORK%"
endlocal
