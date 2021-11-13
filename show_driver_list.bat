@echo off

@echo #################################################################
@echo ## Grabbing driver_install.exe from your DecaHub Driver folder ##
@echo #################################################################
@xcopy "C:\Program Files\Megadodo Games\DecaHub\driver\bin\win64\show_driver_list.exe" "bin\win64\" /Y /F
@echo #################################################################
bin\win64\show_driver_list.exe
pause
