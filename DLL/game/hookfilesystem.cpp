#include <windows.h>
#include <psapi.h>
#include "filesystem.hpp"

#include <string.h>
#include <ctype.h>
#include <stdio.h>

//# define HOOKDLL_NAME "TRAhook.dll"
extern HMODULE hHookDll;

class HookFileSystem : public cdc::FileSystem
{
private:
	cdc::FileSystem* m_pFS;
	cdc::FileSystem* m_pDiskFS;

	unsigned int m_specMask;

public:
	HookFileSystem(cdc::FileSystem* pFS, cdc::FileSystem* pDiskFS)
	{
		m_pFS = pFS;
		m_pDiskFS = pDiskFS;
		m_specMask = 1;
	}

	cdc::FileSystem* GetBestFileSystem(const char* fileName, char** outFilename)
	{
		//HANDLE hCurrentProcess;
		//HMODULE hHookDll;
		int i;
		char pathname[_MAX_PATH];
		const char* p;
		
		p = strrchr(fileName,'\\');
		if(p)
			p++;
		else
			p=fileName;
				
		//hCurrentProcess=GetCurrentProcess();
		//hHookDll=GetModuleHandleA(HOOKDLL_NAME);
		GetModuleFileNameA(hHookDll,pathname,_MAX_PATH);
		for(i=strlen(pathname);pathname[i]!='\\';i--);
		pathname[i+1]=0;
		strcat_s(pathname,"mods\\");
		strcat_s(pathname,p);
		
		// check first for file on disk suffixed by our specialisation mask ??????
		char specPath[_MAX_PATH];
		sprintf_s(specPath, "%s_%03d", pathname, m_specMask);

		if (m_pDiskFS->FileExists(specPath))
		{
			*outFilename = specPath;
			return m_pDiskFS;
		}

		// check if file exists on disk, if so return the diskFS
		if (m_pDiskFS->FileExists(pathname))
		{
			*outFilename = pathname;
			return m_pDiskFS;
		}

		*outFilename = (char*)fileName;
		return m_pFS;
	}

	virtual void* RequestRead(void* receiver, const char* fileName, unsigned int startOffset)
	{
		char* path;
		cdc::FileSystem* pFS = GetBestFileSystem(fileName, &path);

		return pFS->RequestRead(receiver, path, startOffset);
	}

	virtual void* OpenFile(char const* fileName)
	{
		char* path;
		cdc::FileSystem* pFS = GetBestFileSystem(fileName, &path);

		return pFS->OpenFile(path);
	}

	virtual bool FileExists(char const* fileName)
	{
		char* path;
		cdc::FileSystem* pFS = GetBestFileSystem(fileName, &path);

		return pFS->FileExists(path);
	}

	virtual unsigned int GetFileSize(char const* fileName)
	{
		char* path;
		cdc::FileSystem* pFS = GetBestFileSystem(fileName, &path);

		return pFS->GetFileSize(path);
	}

	virtual void SetSpecialisationMask(unsigned int specMask)
	{
		m_pFS->SetSpecialisationMask(specMask);

		// unset next generation bit and set our spec mask
		m_specMask = specMask & ~0x80000000;
	}

	virtual unsigned int GetSpecialisationMask()
	{
		return m_pFS->GetSpecialisationMask();
	}

	// these below don't need to call diskFS, the archive filesystem will do so
	virtual int GetStatus()
	{
		return m_pFS->GetStatus();
	}

	virtual void Update()
	{
		m_pFS->Update();
	}

	virtual void Synchronize()
	{
		m_pFS->Synchronize();
	}

#if TR8
	virtual void Suspend()
	{
		m_pFS->Suspend();
	}

	virtual bool Resume()
	{
		return m_pFS->Resume();
	}

	virtual bool IsSuspended()
	{
		return m_pFS->IsSuspended();
	}
#endif
};

cdc::FileSystem* CreateHookFileSystem(cdc::FileSystem* pFS, cdc::FileSystem* pDiskFS)
{
	return new HookFileSystem(pFS, pDiskFS);
}
