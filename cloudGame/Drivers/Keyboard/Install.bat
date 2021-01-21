@setlocal enableextensions
@cd /d "%~dp0"

echo Uninstalling driver first...
call devcon.exe remove "HID\ttcvcontrkb"
call devcon.exe remove "root\ttcvcontrkb"

echo Installing the driver...
call devcon.exe install ttcvcontrkb.inf root\ttcvcontrkb -runhidden

