@echo off

call vcvars32.bat
echo:
cl /O1 /Os /GS- /W4 main.c /link /NOLOGO /SUBSYSTEM:CONSOLE /ENTRY:main /ALIGN:16 /NODEFAULTLIB ucrt.lib vcruntime.lib msvcrt.lib user32.lib kernel32.lib
pause
