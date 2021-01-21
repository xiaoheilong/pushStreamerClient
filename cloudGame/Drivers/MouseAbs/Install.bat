@setlocal enableextensions
@cd /d "%~dp0"

echo Uninstalling driver first...
call devcon.exe remove root\ttcvcontrms -runhidden

echo Installing the driver...
call devcon.exe install ttcvcontrms.inf root\ttcvcontrms -runhidden

