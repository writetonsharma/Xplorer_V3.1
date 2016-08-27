

#ifndef __Xplorer_cSock_h__
#define __Xplorer_cSock_h__

//#include "..\\common\\global.h"



BOOL InitSockLib();
int ConnectToServer();
void ShowMessage(const TCHAR *tszMsg);
void SendRootNodes(SOCKET);
short SendSubfolders(SOCKET ServerSocket,TCHAR *tszPath);

//threads
void RecvThread(void *pParam);

//Helper functions
int HandleCopy(const TCHAR *pszPath,const SOCKET ServerSocket, const HINT);
int SendFile(const TCHAR *pszPath, const SOCKET ServerSocket, 
					const BOOL bSendFileWithPath, const TCHAR *szRelPath);
int SendDirectorySize(const TCHAR *pszPath,const SOCKET ServerSocket);
void GetDirectorySize(const TCHAR*, double* );
int SendDirectoryAndData(const TCHAR *szBasePath, const TCHAR *pszCurrFolder, 
							TCHAR *szCurrRelPath,const SOCKET ServerSocket);
bool TruncateFile(const TCHAR* filepath, int size);
#endif
