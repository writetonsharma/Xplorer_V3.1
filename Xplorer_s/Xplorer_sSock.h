
/****************************************************************************
 *   Copyright (C) 2007 by naveen sharma									*
 *   writetonsharma@gmail.com												*
 *																			*
 *	 File: Xplorer_s.h, Jan 2008											*
 *   Description: All the declaration respective to Xplorer server socket	*
 *				  engine here.												*
 ***************************************************************************/


#ifndef __Xplorer_sSock_h__
#define __Xplorer_sSock_h__

BOOL InitSockLib();
short StartServer(SOCKET *,BOOL bStart);
void ShowMessage(const TCHAR *tszMsg);
void CloseSocket(SOCKET sock);


//threads
void AcceptThread(void *pParam);
unsigned int __stdcall RecvThread(void *pParam);


#endif


