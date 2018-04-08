cd source
for /f %%f in ('dir /b') do ..\RWObfuscator.exe %%~nxf ..\output\%%~nxf
cd ..