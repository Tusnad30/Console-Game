@echo off

call vcvars32.bat
echo:
cl /O1 main.c /link /NOLOGO /SUBSYSTEM:CONSOLE /ENTRY:main /ALIGN:16 /NODEFAULTLIB ucrt.lib vcruntime.lib msvcrt.lib user32.lib

start main.exe