@echo off
@echo 
if not exist "C:\Program Files\Megadodo Games\DecaHub\driver\bin\win64\driver_uninstall.exe" echo "C:\Program Files\Megadodo Games\DecaHub\driver\bin\win64\driver_uninstall.exe missing"
@echo #################################################################
@echo ## Grabbing driver_install.exe from your DecaHub Driver folder ##
@echo #################################################################
@xcopy "C:\Program Files\Megadodo Games\DecaHub\driver\bin\win64\driver_uninstall.exe" "bin\win64\" /Y
@echo #################################################################
@echo ## Uninstalling virtual tracker driver                           ##
@echo #################################################################
"bin\win64\driver_uninstall.exe"
pause