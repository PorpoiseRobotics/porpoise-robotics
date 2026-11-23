@echo off
rem Try the dashboard with NO hardware: 4 pretend stations driving around.
rem This laptop pretends to be station 1. Close the window to stop.
title Find the Target - Practice Mode
cd /d "%~dp0"
set "PY="
where py >nul 2>nul
if not errorlevel 1 set "PY=py -3"
if not defined PY set "PY=python"
%PY% start_dashboard.py --simulate 4 --open
pause
