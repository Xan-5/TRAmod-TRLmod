# include <windows.h>
# include "resource.h"

#if TRAE
	#define MUTEX_NAME "TRAmod_"
	#define DLL_NAME "TRAhook.dll"
	#define REGISTRY_KEY "SOFTWARE\\Crystal Dynamics\\Tomb Raider: Anniversary"
	#define EXE_NAME "\\tra.exe"
	
#elif TR7
	#define MUTEX_NAME "TRLmod_"
	#define DLL_NAME "TRLhook.dll"
	#define REGISTRY_KEY "SOFTWARE\\Crystal Dynamics\\Tomb Raider: Legend"
	#define EXE_NAME "\\trl.exe"
#else
	#error "No game specified, set TRAE for Anniversary or TR7 for Legend"
#endif


char pathname[_MAX_PATH];
//HINSTANCE hDLL;

bool FileExists(const char* filename)
{
	if(GetFileAttributes(filename)==0xFFFFFFFF){
		if(GetLastError()==ERROR_FILE_NOT_FOUND)
			return false;
	}
	return true;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	HANDLE mutex = CreateMutex(NULL,TRUE,MUTEX_NAME);
	if(GetLastError()==ERROR_ALREADY_EXISTS)
		return 1;
	
	GetModuleFileName(NULL,pathname,_MAX_PATH);
	char* p = strrchr(pathname,'\\');
	if(p)
		p[1]=0;
	strcat_s(pathname,DLL_NAME);
	
	if(!FileExists(pathname)){
		MessageBox(NULL,DLL_NAME,"Error: Not found",MB_OK);
		return 2;
	}
	
	memset(&si,0,sizeof(si));
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	{
		char pathname[_MAX_PATH];
		char directory[_MAX_PATH];
		char key[_MAX_PATH];		
		HKEY hKey;
		DWORD size=_MAX_PATH;
		
		strcpy_s(key,REGISTRY_KEY);
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,key,0,KEY_READ|KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS){
			if(RegQueryValueEx(hKey,"InstallPath",NULL,NULL,LPBYTE(directory),&size)==ERROR_SUCCESS){
				strcpy_s(pathname,directory);
				strcat_s(pathname,EXE_NAME);
				if(!FileExists(pathname)){
					MessageBox(NULL,pathname,"Error: Not found!",MB_OK);
					return 5;
				}
			}
			else{
				MessageBox(NULL,"Can't query \"InstallPath\".","Error",MB_OK);
				RegCloseKey(hKey);
				return 4;
			}
			RegCloseKey(hKey);
		}
		else{
			MessageBox(NULL,REGISTRY_KEY,"Error: Can't open for readng",MB_OK);
			return 3;
		}
		CreateProcess(pathname,NULL,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS|CREATE_SUSPENDED,NULL,directory,&si,&pi);
	}
	
	//inject via CreateRemoteThread
	//We have the handle from CreateProcess, so if it has sufficient priviledges use it rather that calling OpenHandle again.
	//HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE|PROCESS_VM_OPERATION|PROCESS_CREATE_THREAD,NULL,pi.dwProcessId);
	//if(hProcess)
	{
		LPVOID lpDllPath = VirtualAllocEx(pi.hProcess,NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
		if(lpDllPath){
			if(WriteProcessMemory(pi.hProcess,lpDllPath,::pathname,strlen(::pathname)+1,NULL)){
				HINSTANCE hKernel32 = GetModuleHandle("Kernel32.dll");
				if(hKernel32){
					FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");
					CreateRemoteThread(pi.hProcess,NULL,0,LPTHREAD_START_ROUTINE(pLoadLibrary),lpDllPath,0,NULL);					
				}
			}
			else
				MessageBox(NULL,"WriteProcessMemory() failed!","Error!",MB_OK);
			VirtualFreeEx(pi.hProcess,NULL,NULL,MEM_RELEASE);
		}
		else{
			MessageBox(NULL,"VirtualAllocEx() failed!","Error!",MB_OK); //should call GetLastError()
		}
	}
	//else call GetLastError and display error no
	
	ResumeThread(pi.hThread);
			
	ReleaseMutex(mutex);

	return 0;	
}
