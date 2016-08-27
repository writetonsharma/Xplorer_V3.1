/****************************************************************************
 *   Copyright (C) 2007 by naveen sharma									*
 *   writetonsharma@gmail.com												*
 *																			*
 *	 File: TreeView.cpp, Jan 2008											*
 *   Description: Handles all the messages for the Tree view created		*
 *				  for each client windows.									*
 *				  Tree view created using win32, No MFC.					*
 ***************************************************************************/




#include "stdafx.h"
#include "xplorer_s.h"
#include <stdio.h>
#include <process.h>

#include <Shlobj.h>


extern TREECLIENT		g_treeclient[MAX_CLIENTS];
extern int				g_iMenuClicked;
extern TCHAR			szTitle[MAX_LOADSTRING];			// The title bar text
extern HINSTANCE		hInst;								// current instance
extern HIMAGELIST		g_hImageList;


//handles the popup menu on the tree control
LRESULT CALLBACK TreeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC			lpOldProc;
	TCHAR			szPath[MAX_PATH];
	HINTINFO		hi;
	HTREEITEM		hSelected = 0;
	NODEINFO		ni;
	SOCKET			ClientSocket;
	int				nSend;
	FILE			*fp;
	uintptr_t		hThread;


	
	lpOldProc = (WNDPROC)GetProp( hWnd, "oldproc" );
		

	switch(message)
	{
		case WM_COMMAND:
		{
			g_iMenuClicked = LOWORD(wParam);	//save the last menu clicked.

			switch(LOWORD(wParam))
			{

			//copy the file on local system and open it
			case MNU_OPENLOCAL:

				//copy it locally ***************
				SHGetSpecialFolderPath(hWnd,szPath,CSIDL_DESKTOPDIRECTORY,0);
			
				//get selected node and its path
				hSelected = (HTREEITEM)SendMessage(hWnd,TVM_GETNEXTITEM,
							TVGN_CARET,(LPARAM)hSelected);
				GetNodeInfo(hWnd,hSelected,&ni);

				ClientSocket = GetSocketInfoFromHwnd(hWnd);

				//Tell client to send this file
				hi.hint = COPY_FILE;
				_tcscpy(hi.szPath,ni.szFolderPath);

				//before sending, create an empty file
				_tcscat(szPath,_T("\\"));
				_tcscat(szPath,ni.szFileName);
				fp = fopen(szPath,_T("w"));
				if(fp == NULL)
				{
					MessageBox(hWnd,_T("Error creating file."),szTitle,0);
					break;
				}
				fclose(fp);

				//create a thread to show the progress dialog
				//_beginthread(ProgressDlgThread,0,(void *)NULL);				
				hThread = _beginthreadex(NULL,0,ProgressDlgThread,(void*)NULL,
					CREATE_SUSPENDED,NULL);
				ResumeThread((HANDLE)hThread);
				nSend = 0;
				nSend = send(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
				if(nSend == 0)
				{
					//error
				}				

				break;

			//send request to client to open file on client system.
			case MNU_OPENREMOTE:

				//get selected node and its path
				hSelected = (HTREEITEM)SendMessage(hWnd,TVM_GETNEXTITEM,
							TVGN_CARET,(LPARAM)hSelected);
				GetNodeInfo(hWnd,hSelected,&ni);

				ClientSocket = GetSocketInfoFromHwnd(hWnd);

				//Tell client to send this file
				hi.hint = OPEN_REMOTEFILE;
				_tcscpy(hi.szPath,ni.szFolderPath);
				nSend = send(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
				if(nSend == 0)
				{
					//error
				}

				break;

				//Files will be copied to desktop
				case MNU_COPY:
					SHGetSpecialFolderPath(hWnd,szPath,CSIDL_DESKTOPDIRECTORY,0);
					if(MessageBox(hWnd,_T("The file will be copied to desktop."),
						szTitle,MB_OKCANCEL) == IDCANCEL)
						break;
					//get selected node and its path
					hSelected = (HTREEITEM)SendMessage(hWnd,TVM_GETNEXTITEM,
								TVGN_CARET,(LPARAM)hSelected);
					GetNodeInfo(hWnd,hSelected,&ni);

					ClientSocket = GetSocketInfoFromHwnd(hWnd);

					if(ni.filetype == _DIRECTORY)
					{
						hi.hint = COPY_DIRECTORY;
					}
					else if(ni.filetype = _FILE)
					{
						//Tell client to send this file
						hi.hint = COPY_FILE;
					}


					_tcscpy(hi.szPath,ni.szFolderPath);
					//before sending, create an empty file
					_tcscat(szPath,_T("\\"));
					_tcscat(szPath,ni.szFileName);

					//Make a file else a directory
					if(hi.hint == COPY_FILE)
					{
						fp = fopen(szPath,_T("w"));
						if(fp == NULL)
						{
							MessageBox(hWnd,_T("Error creating file."),szTitle,0);
							break;
						}
						fclose(fp);
					}
					else if(hi.hint == COPY_DIRECTORY)
					{
						CreateDirectory(szPath, NULL);

					}

					//create a thread to show the progress dialog
					//_beginthread(ProgressDlgThread,0,(void *)NULL);				
					hThread = _beginthreadex(NULL,0,ProgressDlgThread,(void*)NULL,
						CREATE_SUSPENDED,NULL);
					ResumeThread((HANDLE)hThread);
					nSend = 0;
					nSend = send(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
					if(nSend == 0)
					{
						//error
					}					
					break;
					
				//remove the selected file from client system.
				case MNU_REMOVE:
					break;

					//show the properties of the file
				case MNU_PROP:
					break;

				case MNU_CATCH:
					//get selected node and its path
					hSelected = (HTREEITEM)SendMessage(hWnd,TVM_GETNEXTITEM,
								TVGN_CARET,(LPARAM)hSelected);
					GetNodeInfo(hWnd,hSelected,&ni);
					ClientSocket = GetSocketInfoFromHwnd(hWnd);

					memset(&hi,0, sizeof(hi));
					hi.hint = CATCH_DESKTOP;
					nSend = send(ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);

					//create a thread to show the progress dialog
					//_beginthread(ProgressDlgThread,0,(void *)NULL);				
					hThread = _beginthreadex(NULL,0,ProgressDlgThread,(void*)NULL,
						CREATE_SUSPENDED,NULL);
					ResumeThread((HANDLE)hThread);
					break;			
			}
			break;

			case WM_CLOSE:
				//V3.0, disconnets from the client
				//CleanUp(hWnd);

				//V4.0, minimizes only.


				break;
			
		}
	}

	return(CallWindowProc( lpOldProc, hWnd,message,wParam,lParam ));	
}


//it checks if roots are to be added
//or child nodes
short MakeTree(XINFO *pxi,HWND hParent)
{

	switch(pxi->infotype)
	{
	case ROOT:
		AddRootNode(pxi,hParent);
		break;

	case CHILD:
		AddChildNode(pxi);
		break;

	}
	
	return SUCCESS;
}

//adds the top most nodes to tree window having HWND = hParent
short AddRootNode(XINFO *pxi,HWND hParent)
{
	int					i = 0;
	TVINSERTSTRUCT		tvis;
	BOOL				bCreated = FALSE;
	static HWND			hwnd;
	HTREEITEM			hTree;
	NODEINFO			*ni;
	struct sockaddr_in	client_addr;
	TCHAR				szTemp[128];


	//check if this tree already there
	while(g_treeclient[i].hTree != 0)
	{
		if(g_treeclient[i].ClientSocket == pxi->SockInfo)
		{
			//this tree is already there, just add node
			//dont break, i will fetch us the length of array
			bCreated = TRUE;				
		}
		i++;
	}

	if(!bCreated)
	{
		//get the client ip
		int addrlen = sizeof(client_addr);
		getpeername(g_treeclient[i].ClientSocket,(struct sockaddr *)&client_addr,&addrlen);
		TCHAR *temp = inet_ntoa(client_addr.sin_addr);
		HOSTENT *host = gethostbyaddr((const char*)&client_addr.sin_addr,
			sizeof(client_addr.sin_addr),AF_INET);

		if(!(temp == NULL || host == NULL))
		{
			strcpy(szTemp,host->h_name);
			strcat(szTemp,_T(": "));
			strcat(szTemp,temp);
		}
		else
		{
			strcpy(szTemp,_T("No Info."));
		}
				
		hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, _T("SysTreeView32"),szTemp,
				WS_BORDER | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT,
				0,0,200,200,hParent, NULL, hInst,NULL);
		
		//update global list, rest two parameters are already updated
		//when a new a connection request arrives.
		g_treeclient[i].hTree = hwnd;
		SendMessage(hwnd,TVM_SETIMAGELIST,0,(LPARAM)g_hImageList);

		//Subclass tree control procedure
		SetProp(hwnd, "oldproc",(HANDLE)GetWindowLongPtr(hwnd,GWLP_WNDPROC)); 
		SetWindowLong(hwnd,GWLP_WNDPROC,(DWORD)TreeWndProc);

		//g_treeclient[i].ClientSocket = pxi->SockInfo;

		//add the entery in the users list
		SendMessage(hParent, WM_ADD_TO_USER_LIST, 0, (LPARAM)szTemp);
	}

	//node's private data , 
	//this memory will be freed when the tree collapse
	ni = (NODEINFO*)malloc(sizeof(NODEINFO));
	_tcscpy(ni->szFolderPath,pxi->szFolderPath);
	_tcscpy(ni->szFileName,pxi->szFileName);
	ni->filetype = pxi->filetype;

	tvis.hParent = TVI_ROOT ;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.item.pszText = pxi->szFolderPath;
	tvis.item.lParam = (LPARAM)ni;
	switch(ni->filetype)
	{
	case FIXED_DRIVE:
		tvis.item.iImage=DRIVE;
		tvis.item.iSelectedImage=DRIVE;
		break;
	case CDROM_DRIVE:
		tvis.item.iImage=CD_ROM;
		tvis.item.iSelectedImage=CD_ROM;
		break;
	case REMOVABLE_DRIVE:
		tvis.item.iImage=FLOPPY;
		tvis.item.iSelectedImage=FLOPPY;
		break;
	}
	
	hTree = TreeView_InsertItem(hwnd,&tvis);
	
	//add a dummy to show + sign with the node
	tvis.hParent = hTree ;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT;
	tvis.item.pszText = _T("dummy");
	TreeView_InsertItem(hwnd,&tvis);

	//select this node
	SendMessage(hwnd,TVM_SELECTITEM,(WPARAM)TVGN_CARET,(LPARAM)hTree);

	return SUCCESS;
}

//adds child nodes to the selected node
//hParent is the handle to the tree window
short AddChildNode(XINFO *pxi)
{
//	NM_TREEVIEW 			*pnmtv;
	TVINSERTSTRUCT			tvis;
	NODEINFO				*ni;
	HTREEITEM				hSelected = 0;
	HTREEITEM				hTree;
	HWND					hTreeWnd;
	

	//MessageBox(GetParent(pxi->hTree),pxi->szFileName,"",0);

	//check for "." or ".." directory
	//dont add them to tree
	if(!(_tcscmp(pxi->szFileName,_T(".")) && _tcscmp(pxi->szFileName,_T(".."))))
	{
		send(pxi->SockInfo,_T("ok"),(int)_tcslen(_T("ok")),0);
		return SUCCESS;
	}

	hTreeWnd = GetMatchingHwndFromSocket(pxi->SockInfo);
	if((int)hTreeWnd == HWND_NOT_FOUND)
	{
		//error
	}

	//Get the handle of the selected node
	hSelected = (HTREEITEM)SendMessage(hTreeWnd,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hSelected);

	ni = (NODEINFO*)malloc(sizeof(NODEINFO));
	_tcscpy(ni->szFolderPath,pxi->szFolderPath);
	_tcscpy(ni->szFileName,pxi->szFileName);
	ni->filetype = pxi->filetype;
	
	tvis.hParent = hSelected ;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.item.pszText = pxi->szFileName;
	tvis.item.lParam = (LPARAM)ni;
	switch(ni->filetype)
	{
	case _FILE:
		tvis.item.iImage=FILE1;
		tvis.item.iSelectedImage=FILE1;
		break;
	case _DIRECTORY:
		tvis.item.iImage=FOLDER_PLUS;
		tvis.item.iSelectedImage=FOLDER_PLUS;
		break;
	}	
	hTree = TreeView_InsertItem(hTreeWnd,&tvis);

	//if its a directory add a dummy to show + sign with the node
	//else dont
	if(pxi->filetype == _DIRECTORY)
	{
		tvis.hParent = hTree ;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT;
		tvis.item.pszText = _T("dummy");
		TreeView_InsertItem(hTreeWnd,&tvis);
	}

	//just to tell we recieved something and send me next item
	send(pxi->SockInfo,_T("ok"),(int)_tcslen(_T("ok")),0);

	return SUCCESS;
}

//Gets the first child node of hNode
//hTree is the tree window
HTREEITEM GetFirstChildNode(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hChild = 0;

	if(hTree == NULL)
		return (HTREEITEM)WRONG_PARAMS_PASSED;

	//if hNode == NULL, we return the child node of seleted node
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);

	hChild = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM ,
				TVGN_CHILD,(LPARAM)hNode);

	if(hChild != NULL)
		return  hChild;
	else
		return (HTREEITEM)NULL;
}

//Get the next node to hNode
//hTree is the tree window
HTREEITEM GetNextNode(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hNext = 0;

	if(hTree == NULL)
		return (HTREEITEM)WRONG_PARAMS_PASSED;

	//if hNode == NULL, we return the next node of seleted node
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);

	hNext = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM ,
				TVGN_NEXT,(LPARAM)hNode);

	if(hNext != NULL)
		return  hNext;
	else
		return (HTREEITEM)NULL;

}

//Gets the parent of hNode
//hTree is the tree window
HTREEITEM GetParentNode(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hParent = 0;

	if(hTree == NULL)
		return (HTREEITEM)WRONG_PARAMS_PASSED;

	//if hNode == NULL, we return the parent node of seleted node
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);

	hParent = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM ,
				TVGN_PARENT,(LPARAM)hNode);

	if(hParent != NULL)
		return  hParent;
	else
		return (HTREEITEM)NULL;

}

//Enumerates all folder under hNode recurrsively
//and for each node, if it has got some child frees the node data
//hNode should be the first child folder of the node which is collapsing
/*
	[-]Songs
	 |--[+]Beatles		//a directory
	 |-----Lyrics		//a file
	 |--[+]Metallica
	 |--[+]Nirvana
	 |--[-]RollingStones
		 |----Song1.mp3
		 |----Song2.mp3

 if Songs is collapsing then hNode should point to Beatles and not to Songs
*/
BOOL EnumerateAllSubNodes(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hChild;
	//TCHAR			szBuffer[MAX_PATH];
	NODEINFO		ni;
	BOOL			bRetval;
	int				nNodeInfo;


	//if hNode is NULL, get the selected node.
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);

	while(TRUE)
	{
		hChild = GetFirstChildNode(hTree,hNode);
		if(hChild == NULL)
			return FALSE;	

		bRetval = EnumerateAllSubNodes(hTree,hChild);
	
		hNode = GetParentNode(hTree,hChild);
		hChild = hNode;
		
		FreeNodeData(hTree,hChild);			//free it boy, or else it gonna leak

		//do it till we're finding file
		while(1)
		{
			hNode = GetNextNode(hTree,hChild);
			if(hNode == NULL)
				return FALSE;
			nNodeInfo = GetNodeInfo(hTree,hNode,&ni);
			//if its a file, free memory
			if(nNodeInfo == _FILE)
			{
				FreeNodeData(hTree,hNode);
				hChild = hNode;
			}
			else
				break;
		}	
	}
}

//frees memory for "hNode" only
int FreeNodeData(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hTemp = NULL;
	NODEINFO		*pni;
	TV_ITEM			tvi;

	if(hTree == NULL || hNode == NULL)
		return WRONG_PARAMS_PASSED;

	//get node info from this node
	memset(&tvi,0,sizeof(TV_ITEM));
	tvi.hItem = hNode;
	tvi.mask = TVIF_PARAM;
	SendMessage(hTree,TVM_GETITEM,0,(LPARAM)(TV_ITEM FAR*)&tvi);

	//Free lParam memory
	pni = NULL;
	pni = (NODEINFO*)tvi.lParam;
	if(pni != NULL)
		free(pni);
	pni = NULL;

	return SUCCESS;

}

//frees memory associated with all child of hNode
//hTree is the tree window
//it just keeps on searching all the child nodes one by one and freeing the memory
int FreeAllChildNodesData(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hTemp = NULL;
	NODEINFO		*pni;
	TV_ITEM			tvi;


	if(hTree == NULL)
		return WRONG_PARAMS_PASSED;

	//if hNode == NULL, we delete all child nodes under the selected node
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);

	hTemp = GetFirstChildNode(hTree,hNode);
	if(hTemp == NULL)
	{
		//error
	}

	//get node info from this node
	memset(&tvi,0,sizeof(TV_ITEM));
	tvi.hItem = hTemp;
	tvi.mask = TVIF_PARAM;
	SendMessage(hTree,TVM_GETITEM,0,
		(LPARAM)(TV_ITEM FAR*)&tvi);

	//got lParam memory and free it
	pni = NULL;
	pni = (NODEINFO*)tvi.lParam;
	if(pni != NULL)
		free(pni);
	pni = NULL;

	//get all same level nodes and free node data
	while(1)
	{
		hNode = hTemp;
		hTemp = GetNextNode(hTree,hNode);
		if(hTemp == NULL)
			break;

		memset(&tvi,0,sizeof(TV_ITEM));
		tvi.hItem = hTemp;
		tvi.mask = TVIF_PARAM;
		SendMessage(hTree,TVM_GETITEM,0,
			(LPARAM)(TV_ITEM FAR*)&tvi);

		//got lParam memory and free it
		pni = NULL;
		pni = (NODEINFO*)tvi.lParam;
		if(pni != NULL)
			free(pni);
		pni = NULL;

	}
	
	return SUCCESS;
}

//Sets pni to node info
//returns node type, if its file,folder or drive
int GetNodeInfo(HWND hTree,HTREEITEM hNode,NODEINFO *pni)
{
	TV_ITEM		tvi;
	NODEINFO	*niTemp;
	TCHAR		szName[MAX_PATH];


	if(hTree == NULL)
		return WRONG_PARAMS_PASSED;

	//if hNode == NULL, return the name of selected node
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);
	
		tvi.hItem = hNode;
		
		//if pszName == NULL, it mean user dont need the node name
		if(pni != NULL)
		{
			tvi.mask = TVIF_TEXT | TVIF_PARAM;
			tvi.pszText  = szName;
			tvi.cchTextMax  = MAX_PATH;
		}
		else
			tvi.mask = TVIF_PARAM;

	
	SendMessage(hTree,TVM_GETITEM,0,
		(LPARAM)(TV_ITEM FAR**)&tvi);

	niTemp = (NODEINFO*)tvi.lParam;
	if(pni != NULL)
	{
		
		_tcscpy(pni->szFileName,niTemp->szFileName);
		_tcscpy(pni->szFolderPath,niTemp->szFolderPath);
		pni->filetype = niTemp->filetype;
	}
	

	if(niTemp == NULL)
		return 0;
	else
		return niTemp->filetype;
}

//Deletes all child nodes under hNode
int DeleteAllChildNodes(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hNode1,hNode2;


	if(hTree == NULL)
		return WRONG_PARAMS_PASSED;

	//if hNode == NULL, delete all child nodes under selected node
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);

	hNode1 = GetFirstChildNode(hTree,hNode);
	if(hNode1 == NULL)
		return SUCCESS;
	
	while(1)
	{
		hNode2 = GetNextNode(hTree,hNode1);

		SendMessage(hTree,TVM_DELETEITEM,0,(LPARAM)hNode1);
		
		hNode1 = hNode2;
		if(hNode1 == NULL)
			break;
	}

	return SUCCESS;
}

HTREEITEM GetPrevNode(HWND hTree,HTREEITEM hNode)
{
	HTREEITEM		hPrev = NULL;


	if(hTree == NULL)
		return (HTREEITEM)WRONG_PARAMS_PASSED;

	//if hNode == NULL, return the previous node of current selected node
	if(hNode == NULL)
		hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
				TVGN_CARET,(LPARAM)hNode);

	hPrev = TreeView_GetPrevSibling(hTree,hNode);

	return hPrev;
}

//Gets the root node to the tree hTree
HTREEITEM GetRoot(HWND hTree)
{
	HTREEITEM		hRoot = 0;

	if(hTree == NULL)
		return (HTREEITEM)WRONG_PARAMS_PASSED;

	hRoot = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM ,
				TVGN_ROOT,(LPARAM)hRoot);

	if(hRoot != NULL)
		return  hRoot;
	else
		return (HTREEITEM)GEN_ERROR;

}


HTREEITEM GetFirstRootNode(HWND hTree)
{
	HTREEITEM		hNode = NULL,hTemp;


	if(hTree == NULL)
		return (HTREEITEM)WRONG_PARAMS_PASSED;
	
	//get the selected node
	hNode = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
			TVGN_CARET,(LPARAM)hNode);

	//reach to level zero node
	while(1)
	{
		hTemp = GetParentNode(hTree,hNode);
		if(hTemp == NULL)
			break;
		hNode = hTemp;
	}

	//GetNodeInfo(hTree,hNode,szBuff);
	//keep on finding the prev node of hNode
	while(1)
	{
		hTemp = GetPrevNode(hTree,hNode);
		if(hTemp == NULL)
			break;
		hNode = hTemp;
	}

	return hNode;
}

SOCKET GetSocketInfoFromHwnd(HWND hTree)
{
	int i = 0;

	while(g_treeclient[i].hTree != 0)
	{
		if(g_treeclient[i].hTree == hTree)
			return g_treeclient[i].ClientSocket;
		i++;
	}

	return (SOCKET)SOCKET_NOT_FOUND;
}

HWND GetMatchingHwndFromSocket(SOCKET sock)
{
	int i = 0;

	while(g_treeclient[i].hTree != 0)
	{
		if(g_treeclient[i].ClientSocket == sock)
			return g_treeclient[i].hTree;
		i++;
	}

	return (HWND)HWND_NOT_FOUND;
}

HWND GetMatchingHwndFromID(int ID)
{
	int i = 0;

	while(g_treeclient[i].hTree != 0)
	{
		if(GetWindowLongPtr(g_treeclient[i].hTree,GWLP_ID) == ID)
			return g_treeclient[i].hTree;
		i++;
	}

	return (HWND)NULL;
}


//Clean up all the tree's
//Telling each client that server is closing
int	CleanUp()
{
	int				i = 0;
	HTREEITEM		hRoot;
	int				nSend;
	HINTINFO		hi;


	while(g_treeclient[i].hTree != 0)
	{
		//select root for g_treeclient[i].hTree
		hRoot = GetFirstRootNode(g_treeclient[i].hTree);
		
		//this will all memory for the this tree
		EnumerateAllSubNodes(g_treeclient[i].hTree,hRoot);

		//Destroy window
		DestroyWindow(g_treeclient[i].hTree);

		//tell client to close down
		hi.hint = CLEAN_UP;
		_tcscpy(hi.szPath,"");
		nSend = send(g_treeclient[i].ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
		//recv(g_treeclient[i].ClientSocket,(TCHAR*)&hi,sizeof(HINTINFO),0);
		g_treeclient[i].hTree = NULL;

		i++;			
	}

	return SUCCESS;
}

//clean a specific client, and close it
int CleanUp(HWND hTree)
{

	int				i = 0;
	HTREEITEM		hRoot;
	HINTINFO		hi;

	//select root for g_treeclient[i].hTree
	hRoot = GetFirstRootNode(hTree);
	
	//this will all memory for the this tree
	EnumerateAllSubNodes(hTree,hRoot);

	//Destroy window
	DestroyWindow(hTree);

	//tell client to close down
	hi.hint = CLEAN_UP;
	_tcscpy(hi.szPath,"");
	send(GetSocketInfoFromHwnd(hTree),(TCHAR*)&hi,sizeof(HINTINFO),0);

	hTree = NULL;

	return SUCCESS;

}

void HandleRClickOnNode(LPARAM	lParam,HMENU hPopupMenu)
{

	NMHDR				*ptemp;
	POINT				point;
	int					nInfo,nItem,i;
	HTREEITEM			hTemp;
	HWND				hTree;


	ptemp = (NMHDR *)lParam;
	hTree = ptemp->hwndFrom;

	hTemp=(HTREEITEM)SendMessage (hTree,TVM_GETNEXTITEM,TVGN_DROPHILITE,0);
	//dont break here, might be user clicked the already selected node
	//so get the selected node
	if(hTemp != NULL)
		SendMessage(hTree,TVM_SELECTITEM,TVGN_CARET,(LPARAM)hTemp);
	else
		hTemp = (HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,
					TVGN_CARET,(LPARAM)hTemp);
	
	//Show the popup menu, where the mouse is clicked					
	GetCursorPos(&point);

	//before showing, see if the node is file
	//coz rite now we only allow file to be copied.
	//if its not file, disable all menu's
	nInfo = GetNodeInfo(hTree,hTemp,NULL);
	nItem = GetMenuItemCount(hPopupMenu);
	switch(nInfo)
	{
	case _DIRECTORY:		
		for(i = 1;i<nItem;i++)
			EnableMenuItem(hPopupMenu,i,MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(hPopupMenu,0,MF_BYPOSITION | MF_GRAYED);
		break;

	case _FILE:		
		for(i = 0;i<nItem;i++)
			EnableMenuItem(hPopupMenu,i,MF_BYPOSITION | MF_ENABLED);
		break;

	default:		
		for(i = 0;i<nItem;i++)
			EnableMenuItem(hPopupMenu,i,MF_BYPOSITION | MF_GRAYED);
		break;

	}
	
	TrackPopupMenu (hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 
				  0, hTree, NULL) ;
}