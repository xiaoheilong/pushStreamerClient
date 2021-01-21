@setlocal enableextensions
@cd /d "%~dp0"

echo Uninstalling driver first...
call devcon.exe remove root\ttcvcontr -runhidden

echo Installing the driver...
call devcon.exe install ttcvcontr.inf root\ttcvcontr -runhidden



