@echo off
rem set your C++ enviromental variables here if needed
rem call cenv2.bat
cl /nologo /c /MT /EHsc /Ox /GA /W3 /D TRAE main.cpp
link /NOLOGO /RELEASE /OUT:TRAmod.exe main.obj TRAres.RES kernel32.lib user32.lib gdi32.lib advapi32.lib
erase *.obj
pause