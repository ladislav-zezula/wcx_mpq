:: Creation of the ZIP installation package for a Total Commander packer plugin
@echo off
set PLUGIN_NAME=mpq
set PLUGIN_DLL_NAME=wcx_%PLUGIN_NAME%
set TOTALCMD_FOLDER=c:\Program Files\Totalcmd\plugins\wcx\%PLUGIN_NAME%
set WWW_FOLDER=..\..\WWW

::Test the parameters
if not exist %1 echo Error: Missing 64-bit binary name && goto:eof
if not exist %2 echo Error: Missing 32-bit binary name && goto:eof

:: Create the 'publish' folder
echo [*] Preparing files for publishing ...
if not exist .\publish md .\publish
copy %1                  .\publish\%PLUGIN_NAME%.wcx64 >nul
copy %2                  .\publish\%PLUGIN_NAME%.wcx   >nul
copy .\res\pluginst.inf  .\publish\                    >nul
copy .\res\readme.txt    .\publish\                    >nul

:: Create the ZIP installation file
echo [*] Creating ZIP: %PLUGIN_DLL_NAME%.zip
pushd .\publish
zip.exe -ju9 ..\%PLUGIN_DLL_NAME%.zip *  >nul
popd
rd /s /q .\publish

:: Copy new binaries to Total Commander
if not exist "%TOTALCMD_FOLDER%" goto:eof
echo [*] Copying plugins to '%TOTALCMD_FOLDER%' ...
copy %1 "%TOTALCMD_FOLDER%\%PLUGIN_NAME%.wcx64" >nul
copy %2 "%TOTALCMD_FOLDER%\%PLUGIN_NAME%.wcx"   >nul

:: Copy the ZIP file to the publish folder
if not exist "%WWW_FOLDER%" goto:eof
echo [*] Publishing files ...
copy .\%PLUGIN_DLL_NAME%.zip "%WWW_FOLDER%\web\download\" >nul

:: Update the web pages
echo [*] Updating web pages ...
pushd ..\..\WWW
MakeWeb.exe /nologo
popd
