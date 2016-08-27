
#include "stdafx.h"
#include <process.h>
#include "..\\common\\hashdefines.h"
#include "..\\common\\global.h"
#include "xplorer_csock.h"
#include "CatchBitmap.h"
#include "resource.h"
#include <stdio.h>
#include <Shellapi.h>
#include <sys/types.h>
#include <sys/stat.h>



//global-----
extern HINSTANCE hInst;
extern TCHAR szTitle[MAX_LOADSTRING];
extern TCHAR szWindowClass[MAX_LOADSTRING];
extern HWND g_hApp;
//extern TCHAR szTempPath[MAX_PATH];

extern BOOL g_bServerRunning;
SOCKET		g_Socket;			//socket opened by the app

BOOL InitSockLib()
{
	WSADATA wsaData;
	WORD wVersionRequested;
	int nRet;
	
	wVersionRequested = MAKEWORD( 2, 2 );
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if(wVersionRequested != wsaData.wVersion)
	{
		ShowMessage(_T("Socket initalization failed."));
		return FALSE;
	}

	return TRUE;
}

int ConnectToServer()
{
	SOCKET				ClientSocket;
	DWORD				dwSize = 256;
	struct				sockaddr_in Server_Addr;
	uintptr_t			hwndThread;
	TCHAR				IP[32], host[256];

	ClientSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	g_Socket = ClientSocket;
	if (ClientSocket == INVALID_SOCKET)
	{
		ShowMessage(_T("Error Creating Socket."));
		return FALSE;
	}

	Server_Addr.sin_family = AF_INET;
	Server_Addr.sin_port = htons(4001);
	//Server_Addr.sin_addr = *((LPIN_ADDR)*host->h_addr_list);
	//GetPrivateProfileString("Config", "IP", "NULL", IP, 32, ".\\Xplorer_c.ini");
	GetPrivateProfileString("Config", "HOST", "NULL", host, 32, ".\\Xplorer_c.ini");
	if(!strcmp(host, "NULL"))
	{
		GetPrivateProfileString("Config", "IP", "NULL", IP, 32, ".\\Xplorer_c.ini");
		if(!strcmp(IP, "NULL"))
		{
			ShowMessage(_T("Xplorer_c.ini file not found. Can't connect to server."));
			return INI_ERR;
		}
	}
	else
	{
		const hostent* host_info = NULL ;
		host_info = gethostbyname(host);
		if (host_info)
		{
			const in_addr* address = (in_addr*)host_info->h_addr_list[0] ;
			memset(IP,NULL,sizeof(IP)) ;
			strcpy(IP,inet_ntoa(*address));
		}
		else
		{
			ShowMessage(_T("Host cannot be resolved. Can't connect to server."));
			return HOST_RESOLVE_ERR;
		}
	}


	Server_Addr.sin_addr.S_un.S_addr = inet_addr (IP) ;
	if(connect(ClientSocket,(LPSOCKADDR)&Server_Addr,sizeof(struct sockaddr)) < 0)
	{	
		ShowMessage(_T("Connection Error."));
		return FALSE;
	}
	
	SendRootNodes(ClientSocket);

	//Now start recieving in a thread
	//anytime a request can come to update the server
	hwndThread = _beginthread(RecvThread,0,(void *)ClientSocket);
	
	
	return TRUE;
}

void RecvThread(void *pParam)
{
	SOCKET		ClientSocket;
	int			nRecv;
	HINTINFO	hi;
	TCHAR		szBitmapPath[MAX_PATH];


	ClientSocket = (SOCKET)pParam;

	//run untill the server closed the respective tree
	//at that time it send a empty string
	while(1)
	{
		//According to hint we do what we need to do
		memset(&hi,0,sizeof(HINTINFO));
		nRecv = recv(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
		switch(hi.hint)
		{
		case COPY_FILE:
		case COPY_DIRECTORY:		// V3.0
			HandleCopy(hi.szPath,ClientSocket, hi.hint);
			break;

		case REMOVE_FILE:
			break;

		case PROP_INFO:
			break;

		case SUB_FOLDERS:
			//we get a path to a folder here
			//need to send all folders and files under this folder
			//send all subfolders under tszPath to server
			SendSubfolders(ClientSocket,hi.szPath);			
			break;

		case CLEAN_UP:	//server closing
		{
			//do something, server closing
			//tell the main window to close down
		//	HWND hWnd = FindWindow(szWindowClass,szTitle);
			SendMessage(g_hApp,WM_DESTROY,0,0);
			shutdown(ClientSocket, 2);
			//hi.hint = CLOSED;
			//send(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
			_endthread();
			break;
		}

		//file should be opened at client
		case OPEN_REMOTEFILE:
			ShellExecute(GetDesktopWindow(),"open",hi.szPath,hi.szPath,NULL,SW_SHOW );
			break;

		case CATCH_DESKTOP:
			strcpy(szBitmapPath,"");

			if(SaveBitmap(szBitmapPath) == SUCCESS)
			{
				HandleCopy(szBitmapPath,ClientSocket, COPY_FILE);
			}
			break;
		}

		
		
		//connection is broken
		if(hi.hint == 0 && hi.nRecv == 0)
		{
			g_bServerRunning = FALSE;
			closesocket(g_Socket);
			g_Socket = 0;
			break;
		}
	}
}

void SendRootNodes(SOCKET ClientSocket)
{
	XINFO			xi;
	TCHAR			szDrives[MAX_PATH];
	TCHAR			szTemp[MAX_PATH];
	int				i = 0,j;
	int				nSend;
	UINT			uiDrive;
	

	GetLogicalDriveStrings(MAX_PATH,szDrives);
	while(1)
	{
		j = 0;
		while(szTemp[j++] = szDrives[i++]);
		
		//make xinfo and send
		//a new tree will be created at server,
		//so htree and filename not need when creating ROOT
		memset(&xi,0,sizeof(XINFO));
		xi.infotype = ROOT;
		_tcscpy(xi.szFolderPath,szTemp);
		_tcscpy(xi.szFileName,szTemp);

		//get drive type
		uiDrive = GetDriveType(xi.szFolderPath);
		switch(uiDrive)
		{
		case DRIVE_UNKNOWN:
			xi.filetype = UNKNOWN_DRIVE;
			break;
		case DRIVE_REMOVABLE:
			xi.filetype = REMOVABLE_DRIVE;
			break;
		case DRIVE_FIXED:
			xi.filetype = FIXED_DRIVE;
			break;
		case DRIVE_REMOTE:
			xi.filetype = REMOTE_DRIVE;
			break;
		case DRIVE_CDROM:
			xi.filetype = CDROM_DRIVE;
			break;
		case DRIVE_RAMDISK:
			xi.filetype = RAMDISK_DRIVE;
			break;
		}

		nSend = send(ClientSocket,(TCHAR*)&xi,sizeof(XINFO),0);
		if(nSend <= 0)
		{
			ShowMessage(_T("Error Sending root node info."));			
			return;
		}

		//check if server got the data-----

		//---------------------------------
		
		//check if we extracted all drives
		//if yes, send a null to server to ask it to stop
		if(szDrives[i] == '\0')
		{
			xi.infotype = STOP;
			nSend = send(ClientSocket,(TCHAR*)&xi,sizeof(XINFO),0);
			break;
		}
	}
	
}


short SendSubfolders(SOCKET ServerSocket,TCHAR *ptszPath)
{

	WIN32_FIND_DATA		FindData;
	HANDLE				hFind;
	int					nSend;
	TCHAR				tszTemp[MAX_PATH],tszTempFull[MAX_PATH];
	XINFO				xi;
	char				sztemp[10];


	_tcscpy(tszTemp,ptszPath);
	if(*(tszTemp + _tcslen(tszTemp) - 1) == '\\')
		_tcscat(tszTemp,_T("*"));
	else
		_tcscat(tszTemp,_T("\\*"));


	//keep on sending subfolder names to serversocket
	hFind = FindFirstFile(tszTemp,&FindData);
	
	//make full path to the file/folder and make XINFO to be send to server
	memset(&xi,0,sizeof(XINFO));
	tszTempFull[0] = '\0';
	_tcsncpy(tszTempFull,tszTemp,_tcslen(tszTemp) - 1);
	tszTempFull[_tcslen(tszTemp)-1] = '\0';
	_tcscat(tszTempFull,FindData.cFileName);
	_tcscpy(xi.szFolderPath,tszTempFull);
	_tcscpy(xi.szFileName,FindData.cFileName);
	xi.SockInfo = ServerSocket;
	xi.infotype = CHILD;
	//check for directory or file
	if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		xi.filetype = _DIRECTORY;
	else
		xi.filetype = _FILE;

	
	if (hFind != INVALID_HANDLE_VALUE)
		nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);

	recv(ServerSocket,sztemp,10,0);
	while(1)
	{
		if(!FindNextFile(hFind,&FindData))
			break;

		//make full path to the file/folder and make XINFO to be send to server
		memset(&xi,0,sizeof(XINFO));
		tszTempFull[0] = '\0';
		_tcsncpy(tszTempFull,tszTemp,_tcslen(tszTemp) - 1);
		tszTempFull[_tcslen(tszTemp)-1] = '\0';
		_tcscat(tszTempFull,FindData.cFileName);
		_tcscpy(xi.szFolderPath,tszTempFull);
		_tcscpy(xi.szFileName,FindData.cFileName);
		xi.SockInfo = ServerSocket;
		xi.infotype = CHILD;
		//check for directory or file
		if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			xi.filetype = _DIRECTORY;
		else
			xi.filetype = _FILE;

		nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);
		recv(ServerSocket,sztemp,10,0);
	}

	//all child folders sent, send a stop
//	xi.infotype = STOP;
//	nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);	
	
	return SUCCESS;

}

int HandleCopy(const TCHAR *pszPath,const SOCKET ServerSocket, const HINT hint)
{
	int retval;

	switch(hint)
	{
	case COPY_FILE:
		retval = SendFile(pszPath, ServerSocket, FALSE, NULL);
		break;

	case COPY_DIRECTORY:
	{
		TCHAR	szCurrent[MAX_PATH];
		SendDirectorySize(pszPath, ServerSocket);
		
		//take out the last folder name from path
		TCHAR *ret = (TCHAR*)_tcsrchr(pszPath, '\\');
		strcpy(szCurrent, (const char*)(ret + 1));

		//take full path but the last folder
		TCHAR temp[MAX_PATH];
		strcpy(temp, (const char *)pszPath);
		size_t len = strlen(temp);
		while(*(temp + len--) != '\\');
		*(temp + ++len) = '\0';

		retval = SendDirectoryAndData(temp, pszPath,szCurrent, ServerSocket);
		break;
	}

	default:
		retval = GEN_ERROR;
		break;
	}

	return retval;
}

/**
	@Relative path helps server in making the file
	check the server code when a folder is copied
	@bSendFileWithPath helps in knowing if @pszPath is coming from copy of folder or a single file
*/
int SendFile(const TCHAR *pszPath, const SOCKET ServerSocket,
			 const BOOL bSendFileWithPath, const TCHAR *szRelPath)
{
	FILE		*fp;
	int			nRead,nSend;
	XINFO		xi;
	TCHAR		szFileName[MAX_PATH];
	int			i,j;
	DWORD		dwSize = 0;
	HINTINFO	hi;



	//Take out the file name
	if(*(pszPath + 0) == '\0')
		return GEN_ERROR;
	i = (int)_tcslen(pszPath);

	for(;*(pszPath + i) != '\\';i--);
	i++;
	j = 0;
	while(szFileName[j++] = *(pszPath + i++));
	

	//First send the file size
	HANDLE hFile = CreateFile(pszPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile != NULL)
	{
		dwSize = GetFileSize(hFile,NULL);
		CloseHandle(hFile);		
	}

	xi.szBuffer = (TCHAR*)malloc(READ_BUFFER * sizeof(TCHAR));
	if(xi.szBuffer == NULL)
	{
		//handle error
		return GEN_ERROR;
	}


	if(dwSize != 0)
	{
		xi.infotype = FILESIZE;
		xi.dwSize = dwSize;
		strcpy(xi.szFileName, pszPath);
		nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);
	}

	//recv(ServerSocket,"",1,0);

	fp = fopen(pszPath,_T("rb"));
	if(fp == NULL)
	{
		free(xi.szBuffer);
		return FILE_NOT_FOUND;
	}

	while(TRUE)
	{
		memset(xi.szBuffer,0,READ_BUFFER);
		nRead = (int)fread(xi.szBuffer,sizeof(TCHAR),READ_BUFFER,fp);
		xi.infotype = FILEDATA;
		_tcscpy(xi.szFileName,szFileName);
		xi.nRead = nRead;
		if(bSendFileWithPath)
		{
			strcpy(xi.szFolderPath,szRelPath);
		}
		else
		{
			strcpy(xi.szFolderPath, "");
		}

	
		nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);
		recv(ServerSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
		//see if server received all the data
		if(hi.nRecv != xi.nRead)
		{
			//error occurred, stop sending data
			memset(&xi,0,sizeof(XINFO));
			xi.infotype = ERROR_SENDING_DATA;
			_tcscpy(xi.szFileName,szFileName);
			send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);
			fclose(fp);

			//free(xi.szBuffer);
			return ERROR;
		}

		if(feof(fp))
			break;
	}

	fclose(fp);

	//send empty string to tell, we
	xi.infotype = FILEDATA_FINISH;
	nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);
	

	//free(xi.szBuffer);
	return SUCCESS;
}


int SendDirectorySize(const TCHAR *pszPath,const SOCKET ServerSocket)
{
	int			nSend;
	XINFO		xi;
	double		size = 0.0;


	GetDirectorySize(pszPath, &size);

	if(size != 0)
	{
		xi.infotype = TOTAL_FILESIZE;
		xi.dwSize = size;
		nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);
	}

	return SUCCESS;
}

int SendDirectoryAndData(const TCHAR *szBasePath, const TCHAR *pszCurrFolder, 
										TCHAR *szCurrRelPath,const SOCKET ServerSocket)
{
	WIN32_FIND_DATA		fd;
	TCHAR				temp[MAX_PATH];
	XINFO				xi;
	int					nSend;

	
	SetCurrentDirectory(pszCurrFolder);

	HANDLE hFile = ::FindFirstFile(_T("*.*"),&fd);
	if(hFile == INVALID_HANDLE_VALUE)
		return GEN_ERROR;

	while(1)
	{
		//dont show "." & ".."
		_tcscpy(temp, fd.cFileName);

		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{			
			if(_tcscmp(temp, ".") && _tcscmp(temp, "..") 
				&& !(fd.dwFileAttributes & FILE_SHARE_WRITE))
			{
				strcat(szCurrRelPath, "\\");
				strcat(szCurrRelPath, temp);
				xi.infotype = MAKE_DIRECTORY;
				strcpy(xi.szFileName, szCurrRelPath);
				nSend = send(ServerSocket,(TCHAR*)&xi,sizeof(XINFO),0);

				if(SendDirectoryAndData(szBasePath, temp, szCurrRelPath, ServerSocket) == GEN_ERROR)
					return GEN_ERROR;

				::SetCurrentDirectory("..");

				//remove the last folder as we have done a "cd .."
				size_t len = strlen(szCurrRelPath);
				while(*(szCurrRelPath + len--) != '\\');
				len++;
				*(szCurrRelPath + len) = '\0';
			}
		}
		else
		{
			//its a file, send it.
			TCHAR	szFilePath[MAX_PATH];
			strcpy(szFilePath, szBasePath);
			strcat(szFilePath, "\\");
			strcat(szFilePath, szCurrRelPath);
			strcat(szFilePath, "\\");
			strcat(szFilePath, fd.cFileName);

			SendFile(szFilePath, ServerSocket, TRUE, szCurrRelPath);
			//printf("");
			
		}
		

		if(!::FindNextFile(hFile,&fd))
			break;
	}

	FindClose(hFile);

	return SUCCESS;

}


void GetDirectorySize(const TCHAR *pszPath, double *pSize)
{

	WIN32_FIND_DATA		fd;
	TCHAR				temp[MAX_PATH];


	SetCurrentDirectory(pszPath);

	HANDLE hFile = ::FindFirstFile(_T("*.*"),&fd);
	if(hFile == INVALID_HANDLE_VALUE)
		return;

	while(1)
	{
		//dont show "." & ".."
		_tcscpy(temp, fd.cFileName);

/*		
		FILE *fp = fopen("C:\\code\\Xplorer_V3.0\\Xplorer_c\\temp.txt", "a+");
		if(fp != NULL)
		{
			fwrite(temp, strlen(temp), sizeof(char), fp);
			fwrite("\r\n", strlen("\r\n"), sizeof(char), fp);
			fclose(fp);
		}
*/		

		
		//check if directory, and 
		//not previous or current directory	

		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{			
			if(_tcscmp(temp, ".") && _tcscmp(temp, "..") 
				&& !(fd.dwFileAttributes & FILE_SHARE_WRITE))
			{				
				GetDirectorySize(temp, pSize);
				::SetCurrentDirectory("..");
			}
		}
		else
		{
			//its a file, add the size in kb
			
			//*pSize += ((double)fd.nFileSizeLow / 1048576);		//converts to MB
			*pSize += fd.nFileSizeLow;
		}
		

		if(!::FindNextFile(hFile,&fd))
			break;
	}

	FindClose(hFile);
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
