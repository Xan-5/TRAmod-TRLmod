@echo off
rem set your C++ enviromental variables here if need
rem call cenv2.bat
cl /nologo /c /MT /EHsc /Ox /GA /W3 /D TR7 main.cpp
link /NOLOGO /RELEASE /OUT:TRLmod.exe main.obj TRLres.RES kernel32.lib user32.lib gdi32.lib advapi32.lib
erase *.obj
pause