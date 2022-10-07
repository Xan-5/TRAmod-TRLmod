#include <windows.h>
#include <psapi.h>
#include "include/MinHook.h"
#include "game/filesystem.hpp"

typedef unsigned char byte;

static cdc::FileSystem** pDiskFS;
static cdc::FileSystem** pArchiveFS;

byte __declspec(noinline)* GetAddress(byte* ptr, byte offset, byte instr_size)
{
    return (ptr + *(__int32*)(ptr + offset) + instr_size);
}

bool __declspec(noinline) bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for(;*szMask; ++szMask, ++pData, ++bMask){
		if(*szMask == 'x' && *pData != *bMask)
			return FALSE;
	}
	return (*szMask) == NULL;
}

byte __declspec(noinline)* FindPattern(BYTE* bMask, char* szMask)
{
    MODULEINFO moduleInfo = { 0 };
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(MODULEINFO));

    byte* dwBaseAddress = (byte*)moduleInfo.lpBaseOfDll + 0x1000; // start from first section
    int dwModuleSize = (int)moduleInfo.SizeOfImage - 0x1000; // scan size - 0x1000, due to the above

    for (int i = 0; i < dwModuleSize; i++)
    {
        if (bDataCompare((BYTE*)(dwBaseAddress + i), bMask, szMask))
            return (byte*)(dwBaseAddress + i);
    }
    return 0;
}


cdc::FileSystem* GetFS()
{	
	static cdc::FileSystem* pFS = CreateHookFileSystem(*(cdc::FileSystem**)pArchiveFS, *(cdc::FileSystem**)pDiskFS);

	return pFS;
}

static bool hit = false;

HMODULE hHookDll;

BOOL (WINAPI* dGetVersionExA)(LPSTARTUPINFOA lpStartupInfo);

BOOL  WINAPI hGetVersionExA(LPSTARTUPINFOA lpStartupInfo)
{
	if(!hit){ // insert this hook early
		byte* match;
	
		// 8D 44 24 18 68 ? ? ? ? 50 E8 ? ? ? ? E8
		match = FindPattern((PBYTE)"\x8D\x44\x24\x18\x68\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\xE8", "xxxxx????xx????x");
		if(!match)
			MessageBox(0,"\x8D\x44\x24\x18\x68\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\xE8","Error: Pattern not found!",MB_OK);
		match += 15;

		byte* getFS = GetAddress(match, 1, 5);

		// E8 ? ? ? ? EB 02 33 C0 68 ? ? ? ? 8B C8 A3
		match = FindPattern((PBYTE)"\xE8\x00\x00\x00\x00\xEB\x02\x33\xC0\x68\x00\x00\x00\x00\x8B\xC8\xA3", "x????xxxxx????xxx");
		if(!match)
				MessageBox(0,"\xE8\x00\x00\x00\x00\xEB\x02\x33\xC0\x68\x00\x00\x00\x00\x8B\xC8\xA3","Error: Pattern not found!",MB_OK);

		match -= 7;
		pDiskFS = *(cdc::FileSystem***)match;

		match += 24;
		pArchiveFS = *(cdc::FileSystem***)match;
	
		MH_CreateHook((void*)getFS, GetFS, NULL);
		MH_EnableHook(GetFS);

		MH_EnableHook(MH_ALL_HOOKS);
		hit = true;
	}

	return dGetVersionExA(lpStartupInfo);
}

DWORD WINAPI Hook(LPVOID lpParam)
{
    MH_Initialize();
    // we cannot insert our hooks now since game is not done yet unpacking
    // hook one of the first functions called from unpacked code and insert our hooks then
    MH_CreateHookApi(L"Kernel32", "GetStartupInfoA", hGetVersionExA, reinterpret_cast<void**>(&dGetVersionExA));
    MH_EnableHook(MH_ALL_HOOKS);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    // since there could be retail and debug asi in same folder
    // unload early if exe timestamp is wrong one
#if TR7
    if (!CheckVersion())
    {
        return FALSE;
    }
#endif

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            hHookDll = hModule;
            DisableThreadLibraryCalls(hModule);
            Hook(NULL);
            break;
        case DLL_PROCESS_DETACH:
            MH_Uninitialize();
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}
