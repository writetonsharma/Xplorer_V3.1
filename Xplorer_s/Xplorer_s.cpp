
/****************************************************************************
 *   Copyright (C) 2007 by naveen sharma									*
 *   writetonsharma@gmail.com												*
 *																			*
 *	 File: Xplorer_s.cpp, Jan 2008											*
 *   Description: The Xplorer Server application							*
 *				  Handles the basic messages which are then deligated to	*
 *				  other windows or children windows.						*
 ***************************************************************************/


#include "stdafx.h"
#include "xplorer_s.h"
#include <stdio.h>
#include <Shlobj.h>
#include <process.h>
#include <Shellapi.h>




// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text
HIMAGELIST g_hImageList;


extern TREECLIENT		g_treeclient[MAX_CLIENTS];
extern int				g_iMenuClicked;



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	HWND hPrevWindow;

	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_XPLORER_S, szWindowClass, MAX_LOADSTRING);
	
	if(hPrevWindow = FindWindow(szWindowClass, szTitle))
	{
		ShowMessage("Another instance of the application already running!!");
		return 0;
	}

	
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_XPLORER_S);




	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_XPLORER_S);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_XPLORER_S;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 100, 100, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//TCHAR			szHello[MAX_LOADSTRING];
	static SOCKET	ServerSocket;
	static HMENU	hPopupMenu;
	static HWND		hListConnUsers;


	//LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);


	switch (message) 
	{
		case WM_CREATE:
		{
			
			//CoInitializeEx(NULL,COINIT_APARTMENTTHREADED );
			CoInitialize (NULL);

			if(!InitSockLib())
			{
				MessageBox(hWnd,"failed to initalized sockets.",0,0);				
			}

			if(!StartServer(&ServerSocket,TRUE))
			{
			}

			InitCommonControls();
		
			POINT point;
			point.x = 0;
			point.y = 0;
			ClientToScreen(hWnd,&point);

			HBITMAP	hBitMap;
			g_hImageList=ImageList_Create(16,16,ILC_COLOR16,11,15);
			hBitMap=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_IMAGELIST));
			ImageList_Add(g_hImageList,hBitMap,NULL);
			DeleteObject(hBitMap);

			hPopupMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPUP));
			hPopupMenu = GetSubMenu (hPopupMenu, 0) ;

			//V4.0, list box for connected users, just create, adjustment will be done in wm_size
			hListConnUsers = CreateWindow("LISTBOX", "Users Connected",
				WS_CHILD | WS_VISIBLE | WS_BORDER | WS_THICKFRAME  | LBS_HASSTRINGS | LBS_SORT | LBS_DISABLENOSCROLL | WS_VSCROLL | WS_HSCROLL,
				0, 0, 0, 0, hWnd, NULL, hInst, NULL);

			//set the window size
			RECT rect;
			GetWindowRect(GetDesktopWindow(), &rect);

			MoveWindow(hWnd, ((rect.right - rect.left) / 2) - (rect.right - rect.left) / 4,
				((rect.bottom - rect.top) / 2) - (rect.bottom - rect.top) / 4,
				(rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2, TRUE);


			break;
		}

		case WM_COMMAND:
		{
			int	wmId, wmEvent;

			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;				
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;	
		}

		case WM_NOTIFY:
		{
			HWND		hTreeExpanding = NULL;
			SOCKET		ClientSock = 0;
			HTREEITEM	hTemp;
			HINTINFO	hi;
			NM_TREEVIEW 	*pnmtv;
							

			//some tree is expanding
			if(((LPNMHDR)lParam)->code == TVN_ITEMEXPANDED)
			{	
				pnmtv = (NM_TREEVIEW FAR *)lParam;
				//Get tree window handle
				hTreeExpanding = pnmtv->hdr.hwndFrom;
				if((int)hTreeExpanding == HWND_NOT_FOUND)
				{
					//error
				}

				if(pnmtv->action & TVE_EXPAND)
				{						
					ClientSock = GetSocketInfoFromHwnd(hTreeExpanding);
					if((int)ClientSock == SOCKET_NOT_FOUND)
					{
						//error
					}					

					//select this node before doing anything
					SendMessage(hTreeExpanding,TVM_SELECTITEM,
								(WPARAM)TVGN_CARET,(LPARAM)pnmtv->itemNew.hItem);

					//we know the tree now, get the expanding node
					HTREEITEM		hSelected = 0;
					hSelected = (HTREEITEM)SendMessage(hTreeExpanding,TVM_GETNEXTITEM,
								TVGN_CARET,(LPARAM)hSelected);

					//before adding  child node, remove the dummy node
					hTemp = GetFirstChildNode(hTreeExpanding,hSelected);
					if(hTemp == NULL)
					{
						//error
					}
					SendMessage(hTreeExpanding,TVM_DELETEITEM,0,(WPARAM)hTemp);
					
					TCHAR tszBuffer[MAX_PATH];

					pnmtv->itemNew.hItem = hSelected;
					pnmtv->itemNew.mask = TVIF_TEXT | TVIF_PARAM;
					pnmtv->itemNew.pszText  = tszBuffer;
					pnmtv->itemNew.cchTextMax  = MAX_PATH;

					
					SendMessage(hTreeExpanding,TVM_GETITEM,0,
						(LPARAM)(TV_ITEM FAR**)&pnmtv->itemNew);

					NODEINFO *pnitemp = (NODEINFO*)pnmtv->itemNew.lParam;
				
					//send the full path to client, with hint
					//which will start sending the child folder of the specified path
					hi.hint = SUB_FOLDERS;
					_tcscpy(hi.szPath,pnitemp->szFolderPath);
					int	nSend = send(ClientSock,(TCHAR*)&hi,sizeof(HINTINFO),0);		
				}

				//tree collapsing
				//remove all nodes under collapsing node,
				//free memory assosiated with node data
				//add a dummy under the folder
				if(pnmtv->action & TVE_COLLAPSE)
				{
					TVINSERTSTRUCT		tvis;

					//select this node before doing anything
					SendMessage(hTreeExpanding,TVM_SELECTITEM,
								(WPARAM)TVGN_CARET,(LPARAM)pnmtv->itemNew.hItem);
					
					//remove node data from all the nodes
					//Why we are sending the child of the node which is collapsing?
					EnumerateAllSubNodes(hTreeExpanding,
						GetFirstChildNode(hTreeExpanding,pnmtv->itemNew.hItem));
					
					//Delete all nodes under the selected node
					DeleteAllChildNodes(hTreeExpanding,pnmtv->itemNew.hItem);

					//Add dummy
					tvis.hParent = pnmtv->itemNew.hItem ;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT;
					tvis.item.pszText = _T("dummy");
					TreeView_InsertItem(hTreeExpanding,&tvis);
				}
			}
			
			if(((LPNMHDR)lParam)->code == NM_RCLICK)
			{
				HandleRClickOnNode(lParam,hPopupMenu);
			}
		}
		break;
		
		//adds one node according to info it gets
		case WM_MAKETREE:
		{
			XINFO	*xi = (XINFO*)wParam;
			MakeTree(xi,hWnd);
			break;
		}
		
		//V4.0
		case WM_SIZE:
		{	
			int x = LOWORD(lParam);
			MoveWindow(hListConnUsers, (LOWORD(lParam) * 2) / 3, 0, LOWORD(lParam) / 3, HIWORD(lParam), TRUE);
			break;
		}

		case WM_ADD_TO_USER_LIST:
			SendMessage(hListConnUsers, LB_ADDSTRING, wParam, lParam);
			break;

		case WM_CLOSE:
			CleanUp();
			DestroyWindow(hWnd);
			return 0;

		case WM_DESTROY:
			StartServer(&ServerSocket,FALSE);
			CoUninitialize();
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}



// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			ShowWindow(hDlg,SW_SHOW);
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}


//whenever copy file completes this notification will be fired
//do whatever you want
void __stdcall CopyComplete(TCHAR *pszFilePath)
{
	//if menu clicked is open->locally then only open the file
	if(g_iMenuClicked == MNU_OPENLOCAL)
		ShellExecute(GetDesktopWindow(),"open",pszFilePath,pszFilePath,NULL,SW_SHOW );
}


