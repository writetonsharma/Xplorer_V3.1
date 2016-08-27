
/************************************************************************************
 *   Copyright (C) 2007 by naveen sharma											*
 *   writetonsharma@gmail.com														*
 *																					*
 *	 File: Xplorer_sSock.cpp, Jan 2008												*
 *   Description: Base server engine for communication between the server and 		*
 *				  client. No interface is created here. Messages are send from 		*
 *				  client here, they are handled here and then send to the interface	*
 *				  accordingly. The server can handle multiple clients at one time	*
 *				  using threads and hence will not block on client.					*
 ************************************************************************************/



#include "stdafx.h"
#include <process.h>
#include <stdio.h>
#include "xplorer_ssock.h"
#include "resource.h"
#include "..\\common\\hashdefines.h"
#include "..\\common\\global.h"
#include "xplorer_s.h"
#include <Shlobj.h>


//global-----
extern HINSTANCE hInst;
extern TCHAR szTitle[MAX_LOADSTRING];
extern TCHAR szWindowClass[MAX_LOADSTRING];
//-----------


TREECLIENT		g_treeclient[MAX_CLIENTS];//global list, array of all the client-tree relationship
static BOOL		g_bRecvThreadWorking = FALSE;
int				g_iMenuClicked;


BOOL InitSockLib()
{
	WSADATA wsaData;
	WORD wVersionRequested;
	int nRet;
	
	wVersionRequested = MAKEWORD( 2, 2 );
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if(wVersionRequested != wsaData.wVersion)
		return FALSE;

	return TRUE;
}

short StartServer(SOCKET *ServerSocket,BOOL bStart)
{
	
	LPHOSTENT		host;
	TCHAR			szCompName[256];
	DWORD			dwSize = 256;
	SOCKADDR_IN		Server_Addr;
	int				nRet;
	static unsigned long hThread;

	if(bStart)
	{
		*ServerSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if (*ServerSocket == INVALID_SOCKET)
		{
			ShowMessage(_T("Socket failed."));
			return FALSE;
		}

		GetComputerName(szCompName,&dwSize);
		host = gethostbyname(szCompName); 
		TCHAR *szIP = (TCHAR*)inet_ntoa(*(LPIN_ADDR)*(host->h_addr_list));
		Server_Addr.sin_family = AF_INET;
		Server_Addr.sin_addr.S_un.S_addr = INADDR_ANY;
		Server_Addr.sin_port = htons(4001);

		if(bind(*ServerSocket,(LPSOCKADDR)&Server_Addr,sizeof(struct sockaddr)) < 0)
		{
			ShowMessage(_T("Binding failed."));
			return FALSE;
		}
		nRet = listen(*ServerSocket,SOMAXCONN);
		if(nRet < 0)
		{
			ShowMessage(_T("Listen failed."));
			return FALSE;
		}
		
		hThread = _beginthread(AcceptThread,0,(void *)*ServerSocket);
		
	}
	else
	{		
		//bStopThread = TRUE;
		//WaitForSingleObject((void*)hThread,INFINITE);
		TerminateThread((void*)hThread,0);
		closesocket(*ServerSocket);
		
	}	

	
	return SUCCESS;
}


void AcceptThread(void *pParam)
{
	
	SOCKET				ClientSocket,ServerSocket;
	unsigned long		hThread;
	unsigned			ThreadID;
	int					i;
	


	ServerSocket = (SOCKET)pParam;
	if(ServerSocket == INVALID_SOCKET)
	{
		ShowMessage(_T("Invalid Socket"));
		return;
	}

	//keep accepting connections
	while(1)
	{
		ClientSocket = accept(ServerSocket,NULL,NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			ShowMessage(_T("Invalid Socket"));
			return;
		}		
		
		//hThread = _beginthread(RecvThread,0,(void *)ClientSocket);
		hThread = _beginthreadex(NULL,0,RecvThread,(void *)ClientSocket,
					CREATE_SUSPENDED,&ThreadID);
		i = 0;
		while(g_treeclient[i].hTree != 0)
		{
			i++;
		}

		//for each thread started we save the client with which it is connected
		//the third parameter hTree will be updated when the a new tree is created.
		g_treeclient[i].ClientSocket = ClientSocket;
		ResumeThread((void*)hThread);

	}
}

TCHAR		szDlgTitle[MAX_LOADSTRING];
TCHAR		szDlgClass[MAX_LOADSTRING];

unsigned int __stdcall RecvThread(void *pParam)
{
	XINFO				xi;
	int					nRecv;
	SOCKET				ClientSocket;
	HWND				hParent;
	TCHAR				szPath[MAX_PATH];
	TCHAR				szTemp[MAX_PATH];
	
	HWND				hDlg;
	


	LoadString(hInst, IDS_PRGS_DLG_TITLE, szDlgTitle, MAX_LOADSTRING);
	LoadString(hInst, IDS_PRGS_DLG_CLASS, szDlgClass, MAX_LOADSTRING);

	ClientSocket = (SOCKET)pParam;

	hParent = FindWindow(szWindowClass,szTitle);
	if(hParent == NULL)
	{
		g_bRecvThreadWorking = FALSE;
		_endthread();
	}

	SHGetSpecialFolderPath(GetFocus(),szPath,CSIDL_DESKTOPDIRECTORY,0);

	xi.szBuffer = (TCHAR*)malloc(READ_BUFFER * sizeof(TCHAR));
	if(xi.szBuffer == NULL)
	{
		//handle error
		g_bRecvThreadWorking = FALSE;
		_endthread();
	}


	while(1)
	{	
		memset(&xi, 0, sizeof(xi));
		xi.infotype = -1;
		nRecv = recv(ClientSocket,(TCHAR*)&xi,sizeof(XINFO),0);
		if(nRecv <= 0)
		{
			free(xi.szBuffer);
			_endthreadex(0);			
		}
		
		switch(xi.infotype)
		{
			case ROOT:
			case CHILD:
				//update client socket info
				xi.SockInfo = ClientSocket;
				//ask parent to add node
				SendMessage(hParent,WM_MAKETREE,(WPARAM)&xi,0);
				break;

				//No interface interaction needed to make the file
				//so we can do everything here only.
			case FILEDATA_FINISH:
			case FILEDATA:
			{
				HINTINFO		hi;


				_tcscpy(szTemp,szPath);
				_tcscat(szTemp,_T("\\"));
				if(strcmp(xi.szFolderPath, ""))
				{
					_tcscat(szTemp,xi.szFolderPath);
					_tcscat(szTemp,_T("\\"));
				}
				
				_tcscat(szTemp,xi.szFileName);

				_tcscpy(xi.szFolderPath,szTemp);
				
				hDlg = FindWindow(NULL,szDlgTitle);
				DWORD dw = GetLastError();
				SendMessage(hDlg,WM_MAKEFILE,(WPARAM)&xi,0);

				//Tell client how much data u have got
				if(xi.infotype != FILEDATA_FINISH)
				{
					hi.hint = RECV_SIZE;
					hi.nRecv = xi.nRead;
					send(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
				}
				
				break;
			}

			case FILEDATA_WITH_PATH:
			
				break;

			case MAKE_DIRECTORY:
			{
				//HINTINFO		hi;

				_tcscpy(szTemp,szPath);
				_tcscat(szTemp,_T("\\"));
				_tcscat(szTemp,xi.szFileName);

				CreateDirectory(szTemp, NULL);

				/**
				hDlg = FindWindow(NULL,szDlgTitle);
				DWORD dw = GetLastError();
				SendMessage(hDlg,WM_MAKEFILE,(WPARAM)&xi,0);

				hi.hint = RECV_SIZE;
				hi.nRecv = xi.nRead;
				send(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
				*/

				break;
			}

			case FILESIZE:
			{
				Sleep(500);
				hDlg = FindWindow(NULL,szDlgTitle);
				DWORD dw = GetLastError();
				SendMessage(hDlg,WM_SETFILESIZE,(WPARAM)&xi,0);	
				break;
			}

			case TOTAL_FILESIZE:
			{
				Sleep(500);
				hDlg = FindWindow(NULL,szDlgTitle);
				if(hDlg == NULL)
				{
					free(xi.szBuffer);
					return GEN_ERROR;
				}
				DWORD dw = GetLastError();
				SendMessage(hDlg,WM_SET_TOTAL_FILESIZE,(WPARAM)&xi,0);	
				break;
			}

			case ERROR_SENDING_DATA:
			{
				_tcscpy(szTemp,szPath);
				_tcscat(szTemp,_T("\\"));
				_tcscat(szTemp,xi.szFileName);

				_tcscpy(xi.szFolderPath,szTemp);

				hDlg = FindWindow(NULL,szDlgTitle);
				SendMessage(hDlg,WM_NETWORK_ERR,(WPARAM)&xi,0);	
				break;
			}

			case STOP:
					//dont do anything..
				break;

			default:
				//handle error
				//MessageBox(GetFocus(), "Communication error with client", "error",0);
				break;
		}				

		//tell client to send next node
		//only when sending subfolders
	//	if(xi.filetype == _FILE || xi.filetype == _DIRECTORY)
	//		send(ClientSocket,"ok",strlen("ok"),0);
	}
		
//	g_bRecvThreadWorking = FALSE;
	free(xi.szBuffer);
	_endthreadex(0);
}


void ShowMessage(const TCHAR *tszMsg)
{
	LoadString(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInst, IDC_XPLORER_S, szWindowClass, MAX_LOADSTRING);

	MessageBox(FindWindow(szWindowClass,szTitle),tszMsg,_T("Xplorer Server"),0);

}



