@setlocal enableextensions
@cd /d "%~dp0"

echo Uninstalling driver...
call devcon.exe remove root\ttcvcontrms -runhidden

