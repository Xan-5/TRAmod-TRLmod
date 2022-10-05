@echo off
rem set your C++ enviromental variables here if need
rem call cenv2.bat
cl /nologo /c /MT /EHsc /Ox /W3 dllmain.cpp
cl /nologo /c /MT /EHsc /Ox /W3 game\hookfilesystem.cpp
cl /nologo  /c /MT /Ox /W3 minhook-src\hook.c minhook-src\trampoline.c minhook-src\buffer.c minhook-src\hde\hde32.c
link /NOLOGO /RELEASE /DLL /OUT:TRhook.dll dllmain.obj hookfilesystem.obj hook.obj buffer.obj trampoline.obj hde32.obj kernel32.lib user32.lib psapi.lib
erase *.obj
pause
