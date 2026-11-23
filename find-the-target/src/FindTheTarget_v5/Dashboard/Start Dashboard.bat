@echo off
rem ===================================================================
rem  Find the Target v5 - double-click this file to start the dashboard.
rem  Leave the black window open while you play. Close it to stop.
rem ===================================================================
title Find the Target - Station Dashboard
cd /d "%~dp0"

rem ---- Find Python ("py" comes with the python.org installer) ----
set "PY="
where py >nul 2>nul
if not errorlevel 1 set "PY=py -3"
if not defined PY (
  python -c "import sys" >nul 2>nul
  if not errorlevel 1 set "PY=python"
)
if not defined PY goto nopython

rem ---- pyserial lets Python talk to the USB radio (installed once) ----
%PY% -c "import serial" >nul 2>nul
if errorlevel 1 (
  echo Installing pyserial. This needs the internet, but only the first time...
  %PY% -m pip install --user pyserial
  if errorlevel 1 goto nopip
)

%PY% start_dashboard.py --open %*
echo.
echo The dashboard has stopped.
pause
exit /b

:nopython
echo.
echo  Python is not installed on this laptop.
echo  1. Go to https://www.python.org/downloads/ and install Python 3.
echo  2. On the first installer screen, tick "Add python.exe to PATH".
echo  3. Double-click "Start Dashboard.bat" again.
echo.
pause
exit /b

:nopip
echo.
echo  Could not install pyserial. Connect this laptop to the internet once,
echo  then double-click "Start Dashboard.bat" again.
echo.
pause
