To build DLL file edit build.bat to work with your C++ compiler.

Remember to buld as 32bit DLL (we are injecting into 32-bit address space).

In order to succesfully build the DLL you will need to copy source code files from The Minimalistic x86/x64 API Hooking Library for Windows to include and minhook-src subsirectories.
( Copy to include/ file from https://github.com/TsudaKageyu/minhook/tree/master/include and to minhook-src/ files from: https://github.com/TsudaKageyu/minhook/tree/master/src/ )
 
The DLL code is based on https://github.com/TheIndra55/TRAE-menu-hook (pruned all code not needed for drm loading and added changes to read .drm files from local "mods" subdirectory.
Also offsets are being found dynamically rather than hardcoded, so DLL might work with older game builds (tested with latest Gog.com versions).
