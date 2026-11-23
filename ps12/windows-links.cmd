@echo off
rem PS12 Lab 1 only. Run from an Administrator Command Prompt.
powershell.exe -NoProfile -File "%~dp0run-windows.ps1" -SecondDrive "%~1"
exit /b %errorlevel%
