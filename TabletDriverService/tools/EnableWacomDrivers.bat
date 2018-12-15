
REM  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\System32\cacls.exe" "%SYSTEMROOT%\System32\config\system"
  
REM --> If error flag set, we do not have admin.
if '%ERRORLEVEL%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto HasAdminPrivilegesAlready )
  
:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%TEMP%\reqadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%TEMP%\reqadmin.vbs"
    "%TEMP%\reqadmin.vbs"
    exit /B
  
:HasAdminPrivilegesAlready
    if exist "%TEMP%\reqadmin.vbs" ( del "%TEMP%\reqadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"


net stop WTabletServicePro
net start WTabletServicePro
net stop WTabletServiceCon
net start WTabletServiceCon
taskkill /F /IM WacomDesktopCenter.exe

timeout /t 5

