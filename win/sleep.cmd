@echo off
set _x=%1
set tick=0
set startTime=%time%
set myMili=%startTime:~9,2%

:newTick
set /A tick=%tick%+1
set myTime=%time%
set mySec=%myTime:~7,1%

:newtime
set newTime=%time%
set twoSec=%newTime:~7,1%
set twoMili=%newTime:~9,2%

if %mySec% EQU %twoSec% (GOTO newtime) else GOTO nextTest

:nextTest
if %myMili% LEQ %twoMili%+1 (goto tick) else goto newtime
:tick 

if %tick% NEQ %_x% goto newTick else goto end
:end
