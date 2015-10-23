@echo off
REM This script installs etjump to the test mod directory

REM -----------------CONFIGURATION------------------------

set PATH_TO_ET_ROOT="D:\Modding\ET\"
set PATH_TO_7ZIP="C:\Program Files\7-Zip\7z.exe"
set MODNAME="etjump"
set BATCH_PATH=%~dp0
set ADDITIONAL_MOD_FILES="%BATCH_PATH%..\..\etjump\"

REM -----------------END OF CONFIGURATION-----------------

REM ------------------------------------------------------
REM You probably don't have to change anything below this 
REM line
REM ------------------------------------------------------

REM Change directory to the batch file directory to 
REM generate pk3 correctly

cd %BATCH_PATH%

REM Next we need to copy the latest additional files from
REM the etjump directory

xcopy %ADDITIONAL_MOD_FILES%animations %BATCH_PATH%animations\ /s /e /y
xcopy %ADDITIONAL_MOD_FILES%gfx %BATCH_PATH%gfx\ /s /e /y
xcopy %ADDITIONAL_MOD_FILES%icons %BATCH_PATH%icons\ /s /e /y
xcopy %ADDITIONAL_MOD_FILES%models %BATCH_PATH%models\ /s /e /y
xcopy %ADDITIONAL_MOD_FILES%scripts %BATCH_PATH%scripts\ /s /e /y
xcopy %ADDITIONAL_MOD_FILES%sound %BATCH_PATH%sound\ /s /e /y
xcopy %ADDITIONAL_MOD_FILES%ui %BATCH_PATH%ui\ /s /e /y
xcopy %ADDITIONAL_MOD_FILES%weapons %BATCH_PATH%weapons\ /s /e /y

REM Now that the files are there, all we need to do is 
REM create the pk3

%PATH_TO_7ZIP% u -tzip etjump.zip animations gfx icons models scripts sound ui weapons cgame.mp.i386.so ui.mp.i386.so cgame_mp_x86.dll ui_mp_x86.dll -r

rename etjump.zip unversioned_etjump.pk3

xcopy unversioned_etjump.pk3 %PATH_TO_ET_ROOT%\%MODNAME%\ /y
xcopy cgame_mp_x86.dll %PATH_TO_ET_ROOT%\%MODNAME%\ /y
xcopy ui_mp_x86.dll %PATH_TO_ET_ROOT%\%MODNAME%\ /y
xcopy qagame_mp_x86.dll %PATH_TO_ET_ROOT%\%MODNAME%\ /y

del unversioned_etjump.pk3

@echo on
