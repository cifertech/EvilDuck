@echo off
title EvilDuck Hacker Terminal Launcher
set "LAUNCHER=%~dp0launch_hacker_terminals.ps1"

if not exist "%LAUNCHER%" (
  echo Missing launcher:
  echo %LAUNCHER%
  pause
  exit /b 1
)

powershell -NoProfile -NoExit -ExecutionPolicy Bypass -File "%LAUNCHER%"
