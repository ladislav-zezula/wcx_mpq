:: Creation of the ZIP installation package for WCX_MPQ plugin
@echo off
set TOTALCMD_FOLDER=c:\Program Files\Totalcmd\plugins\wcx\mpq
set WWW_FOLDER=..\..\WWW

::Test the parameters
if not exist %1 echo Error: Missing 64-bit binary name && goto:eof
if not exist %2 echo Error: Missing 32-bit binary name && goto:eof

:: Create the 'publish' folder
echo [*] Preparing files for publishing ...
if not exist .\publish md .\publish
copy %1                  .\publish\mpq.wcx64 >nul
copy %2                  .\publish\mpq.wcx   >nul
copy .\res\pluginst.inf  .\publish\          >nul
copy .\res\readme.txt    .\publish\          >nul

:: Create the ZIP installation file
echo [*] Creating ZIP: wcx_mpq.zip
pushd .\publish
zip.exe -ju9 ..\wcx_mpq.zip *  >nul
popd
rd /s /q .\publish

:: Copy new binaries to Total Commander
if not exist "%TOTALCMD_FOLDER%" goto:eof
echo [*] Copying plugins to '%TOTALCMD_FOLDER%' ...
copy %1 "%TOTALCMD_FOLDER%\mpq.wcx64" >nul
copy %2 "%TOTALCMD_FOLDER%\mpq.wcx"   >nul

:: Copy the ZIP file to the publish folder
if not exist "%WWW_FOLDER%" goto:eof
echo [*] Publishing files ...
copy .\wcx_mpq.zip "%WWW_FOLDER%\web\download\" >nul

:: Update the web pages
echo [*] Updating web pages ...
pushd ..\..\WWW
MakeWeb.exe /nologo
popd
