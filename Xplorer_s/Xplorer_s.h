

/****************************************************************************
 *   Copyright (C) 2007 by naveen sharma									*
 *   writetonsharma@gmail.com												*
 *																			*
 *	 File: Xplorer_s.h, Jan 2008											*
 *   Description: All the declaration respective to Xplorer server app		*
 *																			*
 ***************************************************************************/


#if !defined(AFX_XPLORER_S_H__FA5CFC98_5E00_4992_9A35_E1912DC251BB__INCLUDED_)
#define AFX_XPLORER_S_H__FA5CFC98_5E00_4992_9A35_E1912DC251BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "xplorer_ssock.h"
#include "..\\common\\hashdefines.h"
#include "..\\common\\global.h"

#include <commctrl.h>


//all modules used in Xplorer_s.cpp-----------------
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	TreeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


//used in making the tree
short				MakeTree(XINFO *pxi,HWND hParent);
short				AddRootNode(XINFO *pxi,HWND hParent);
short				AddChildNode(XINFO *pxi);
SOCKET				GetSocketInfoFromHwnd(HWND hTree);
HWND				GetMatchingHwndFromSocket(SOCKET sock);
HWND				GetMatchingHwndFromID(int ID);
HTREEITEM			GetFirstChildNode(HWND hTree,HTREEITEM hNode = NULL);
HTREEITEM			GetNextNode(HWND hTree,HTREEITEM hNode = NULL);
HTREEITEM			GetParentNode(HWND hTree,HTREEITEM hNode = NULL);
HTREEITEM			GetRoot(HWND hTree);
int					FreeAllChildNodesData(HWND hTree,HTREEITEM hNode = NULL);
BOOL				EnumerateAllSubNodes(HWND hTree,HTREEITEM hNode = NULL);
int					FreeNodeData(HWND hTree,HTREEITEM hNode);
int					GetNodeInfo(HWND hTree,HTREEITEM hNode,NODEINFO *pni);
int					DeleteAllChildNodes(HWND hTree,HTREEITEM hNode);
HTREEITEM			GetFirstRootNode(HWND hTree);
HTREEITEM			GetPrevNode(HWND hTree,HTREEITEM hNode);
int					CleanUp();
int					CleanUp(HWND hTree);
void				HandleRClickOnNode(LPARAM	lParam,HMENU hPopupMenu);
void				GetDesktopPath(TCHAR *szPath);
//void				ProgressDlgThread(void *pParam);
unsigned int __stdcall ProgressDlgThread(void *pParam);
void __stdcall CopyComplete(TCHAR *);

//about dialog
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	Progress(HWND, UINT, WPARAM, LPARAM);

#endif // !defined(AFX_XPLORER_S_H__FA5CFC98_5E00_4992_9A35_E1912DC251BB__INCLUDED_)
