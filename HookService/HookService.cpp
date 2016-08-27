
#include "stdAfx.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsvc.h>
#include <process.h>
#include <Shellapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Wtsapi32.h>



#define TRUNCATE_SIZE	1024		//kb

#define SERVICENAME        _T("NTservice")
#define LONGSERVICENAME    _T("NTservice")

SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   ServiceStatusHandle; 



 
HANDLE hStopServiceEvent = NULL;

VOID  WINAPI xCmdStart (DWORD argc, LPTSTR *argv); // prototype for the starting point of the service
VOID  WINAPI xCmdCtrlHandler (DWORD opcode); // prototype for the control handler callback function of the service
DWORD IsService( BOOL& );
void _ServiceMain( void* );
void ShowMessage(const TCHAR *tszMsg);
bool TruncateFile(const TCHAR* filepath, int size);
BOOL CtrlHandler( DWORD fdwCtrlType ) ;



int _tmain( int, LPTSTR* )
{
	ShowMessage("_tmain");

   SERVICE_TABLE_ENTRY DispatchTable[] = { 
        { SERVICENAME,	xCmdStart }, 
        { NULL, NULL } }; 

    
   // install log off event
    if( SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) ) 
	{
		ShowMessage("Log off event installation successful.");
	}
	else
	{
		ShowMessage("Log off event installation failed.");
	}

   // Start service
   return StartServiceCtrlDispatcher( DispatchTable);
}

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{
	switch( fdwCtrlType ) 
	{ 
		case CTRL_LOGOFF_EVENT:
			ShowMessage("System logging off, stopping the service.");
			xCmdCtrlHandler(SERVICE_CONTROL_STOP);
			return FALSE;

		default: 
			return FALSE; 
	}

}

// Deletes service
void DeleteSvc()
{
	ShowMessage("Deletevc");

   // Open service manager
   SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);

   if (hSCM == NULL)
   {
	   ShowMessage("Failed to open SC Manager.");
	   return;
   }


   // OPen service
   SC_HANDLE hService = ::OpenService( hSCM, SERVICENAME, SERVICE_ALL_ACCESS );

   if (hService == NULL)
   {
	   ShowMessage("Failed to open service.");
      ::CloseServiceHandle(hSCM);
      return;
   }

   //int retval = ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);

	
   // Stop the service
   ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
   ServiceStatus.dwCheckPoint         = 0; 
   ServiceStatus.dwWaitHint           = 0; 
   ServiceStatus.dwWin32ExitCode      = 0; 
   ServiceStatus.dwServiceSpecificExitCode = 0; 
   SetServiceStatus (ServiceStatusHandle, &ServiceStatus); 


   // Deletes service from service database
   DeleteService( hService );
   
   
   ::CloseServiceHandle(hService);
   ::CloseServiceHandle(hSCM);

   HWND hXplorer = FindWindow("XPLORER_CCLASS","Xplorer Client");
   if(hXplorer != NULL)
   {
		SendMessage(hXplorer, WM_CLOSE, 0,0);
   }
   else
   {
	   ShowMessage("Failed to stop the xplorer client.");
   }

}

// Start service
VOID WINAPI xCmdStart (DWORD, LPTSTR* ) 
{
   DWORD status = 0; 
   DWORD specificError = 0;

   // Prepare the ServiceStatus structure that will be used for the
   // comunication with SCM(Service Control Manager).
   // If you fully under stand the members of this structure, feel
   // free to change these values :o)
   ServiceStatus.dwServiceType        = SERVICE_WIN32; 
   ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
   ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP; 
   ServiceStatus.dwWin32ExitCode      = 0; 
   ServiceStatus.dwServiceSpecificExitCode = 0; 
   ServiceStatus.dwCheckPoint         = 0; 
   ServiceStatus.dwWaitHint           = 0; 

   // Here we register the control handler for our service.
   // We tell the SCM about a call back function that SCM will
   // call when user tries to Start, Stop or Pause your service.
   ServiceStatusHandle = RegisterServiceCtrlHandler( 
         TEXT("Service"), xCmdCtrlHandler ); 

   if (ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
      return; 
   
   // Handle error condition 
   if (status != NO_ERROR) 
   { 
      ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
      ServiceStatus.dwCheckPoint         = 0; 
      ServiceStatus.dwWaitHint           = 0; 
      ServiceStatus.dwWin32ExitCode      = status; 
      ServiceStatus.dwServiceSpecificExitCode = specificError; 

      SetServiceStatus (ServiceStatusHandle, &ServiceStatus); 
      return; 
   } 

   // Initialization complete - report running status. 
   ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
   ServiceStatus.dwCheckPoint         = 0; 
   ServiceStatus.dwWaitHint           = 0; 

   if (!SetServiceStatus (ServiceStatusHandle, &ServiceStatus)) 
      status = GetLastError(); 
   else
   {
      // Start the main thread
      hStopServiceEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
      _beginthread( _ServiceMain, 0, NULL );
   }

   return; 
} 

// Service "main" function
void _ServiceMain( void* )
{

	ShowMessage("ServiceMain");

	bool bWindows;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char TempStr[255];

	strcpy(TempStr, "c:");
	strcat(TempStr, "\\windows\\SysWOW64");

	hFind = FindFirstFile(TempStr, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		//x32
		bWindows = FALSE;

	}
	else
	{
		//x64
		bWindows = TRUE;
	}

	BOOL bError = true;
 	while(1)
 	{
// 		HWND hXplorer = FindWindow("XPLORER_CCLASS","Xplorer Client");
 		if(bError)
 		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(STARTUPINFO));
			si.cb= sizeof(STARTUPINFO);
			si.lpDesktop = TEXT("winsta0\\default");
			ZeroMemory( &pi, sizeof(pi) );

			HANDLE hToken;
			DWORD dw;
			if(!WTSQueryUserToken(WTSGetActiveConsoleSessionId (), &hToken))
			{
				dw = GetLastError();
				ShowMessage("WTSQueryUserToken failed.");
			}
			else
			{
				//not running, execute it
				if(bWindows)
				{
					//	ShellExecute(NULL, NULL, "c:\\windows\\SysWOW64\\ntsnmp.exe", 
					//							NULL, NULL, SW_HIDE);
					//CreateProcess("c:\\windows\\SysWOW64\\ntsnmp.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
					if(!CreateProcessAsUser(hToken, "c:\\windows\\SysWOW64\\ntsnmp.exe", NULL, NULL, NULL, 
										TRUE, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
					{
						dw = GetLastError();
						ShowMessage("CreateProcessAsUser failed.");
					}
					else
					{
						bError = FALSE;
					}
				}
				else
				{
					//ShellExecute(NULL, NULL, "c:\\windows\\system32\\ntsnmp.exe", 
					//						NULL, NULL, SW_HIDE);
					//CreateProcess("c:\\windows\\system32\\ntsnmp.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
					if(!CreateProcessAsUser(hToken, "c:\\windows\\system32\\ntsnmp.exe", NULL, NULL, NULL, 
						TRUE, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
					{
						dw = GetLastError();
						ShowMessage("CreateProcessAsUser failed.");
					}
					else
					{
						bError = FALSE;
					}
				}
				CloseHandle(hToken);
				Sleep(10000);
			}			
		}    
	}
}
 
// Service Ctrl handler
VOID WINAPI xCmdCtrlHandler (DWORD Opcode) 
{ 
    DWORD status; 
	TCHAR szbuff[100];
	sprintf(szbuff, "code=%d", Opcode);
	ShowMessage(szbuff);

    switch(Opcode) 
    { 
        case SERVICE_CONTROL_STOP: 
            // Signal the event to stop the main thread
            SetEvent( hStopServiceEvent );

            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            ServiceStatus.dwCheckPoint    = 0; 
            ServiceStatus.dwWaitHint      = 0; 
 
            if (!SetServiceStatus (ServiceStatusHandle, 
                &ServiceStatus))
            { 
                status = GetLastError(); 
            } 
			return; 
 
        case SERVICE_CONTROL_INTERROGATE: 
        // Fall through to send current status. 
            break; 
    } 
 
    // Send current status. 
    if (!SetServiceStatus (ServiceStatusHandle,  &ServiceStatus)) 
    { 
        status = GetLastError(); 
    } 
    return; 
}

//if file size is more then @size kb, returns true
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


//logs to temp file.
void ShowMessage(const TCHAR *tszMsg)
{
	//LoadString(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInst, IDC_XPLORER_C, szWindowClass, MAX_LOADSTRING);
	TCHAR szTempPath[MAX_PATH];
	//GetTempPath(MAX_PATH, szTempPath);
	strcpy(szTempPath, "C:\\Users\\Default");
	char logpath[MAX_PATH];
	strcpy(logpath, szTempPath);
	strcat(logpath, "\\ntservice.log");
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
