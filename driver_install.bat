@echo off
@echo 
if not exist "C:\Program Files\Megadodo Games\DecaHub\driver\bin\win64\driver_install.exe" echo "C:\Program Files\Megadodo Games\DecaHub\driver\bin\win64\driver_install.exe missing. Please install Decahub"
@echo #################################################################
@echo ## Grabbing driver_install.exe from your DecaHub Driver folder ##
@echo #################################################################
@xcopy "C:\Program Files\Megadodo Games\DecaHub\driver\bin\win64\driver_install.exe" "bin\win64\" /Y
@echo #################################################################
@echo ## Installing virtual tracker driver                           ##
@echo #################################################################
"bin\win64\driver_install.exe"
@echo ...
@echo #################################################################
pause