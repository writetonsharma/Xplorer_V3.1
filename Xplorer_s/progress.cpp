
/****************************************************************************
 *   Copyright (C) 2007 by naveen sharma									*
 *   writetonsharma@gmail.com												*
 *																			*
 *	 File: progress.cpp, Jan 2008											*
 *   Description: Interface to handle the copy of file or folder from client*
 *				  Core functionality is handled by the socket server		*
 *				  This dialog is responsible for updating the UI.			*
 ***************************************************************************/



#include "stdafx.h"
#include "xplorer_s.h"
#include <stdio.h>
#include <process.h>


extern TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
extern TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text


LRESULT CALLBACK Progress(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static float 	nPos,nStep;
	static double	dwFileSize;		//set for each file which we want to copy
	static DWORD	dwRecvSize;
	static double	Completed;
	TCHAR	szBuff[MAX_PATH];
	TCHAR	szProp[MAX_PATH];
	
	switch (message)
	{
		case WM_INITDIALOG:
			ShowWindow(hDlg,SW_SHOW);
			SetWindowText(GetDlgItem(hDlg,STC_SIZE),_T("Waiting..."));
			SetWindowText(GetDlgItem(hDlg,STC_PERCENTAGE),_T("0.0%"));
			
			SendMessage(GetDlgItem(hDlg,IDC_PROGRESS),PBM_SETRANGE,0,MAKELPARAM(0,100));
			SendMessage(GetDlgItem(hDlg,IDC_PROGRESS),PBM_SETPOS,0,0);
			nPos = nStep = 0;
			dwFileSize = dwRecvSize = 0;
			Completed = 0;

			return TRUE;

		case WM_MAKEFILE:
		{
			FILE				*fp;
			XINFO				*xi = (XINFO*)wParam;
			//xi->szBuffer = (TCHAR*)malloc(READ_BUFFER * sizeof(TCHAR));
			//xi = (XINFO*)wParam;
			
			//in the end client send an empty string
			if(xi->infotype == FILEDATA_FINISH)
			{
				//EndDialog(hDlg,SUCCESS);
				//notify that copy is complete	
				void (_stdcall *fnptr)(TCHAR *) = CopyComplete;
				(*fnptr)(xi->szFolderPath);

				//setting the completed progress
				sprintf(szBuff,_T("Total completed: %.2fMB\r\n"),(float)Completed/(float)1048576);
				SetWindowText(GetDlgItem(hDlg,STC_COMPLETED),szBuff);
				InvalidateRect(GetDlgItem(hDlg,STC_COMPLETED),NULL,TRUE);
				Sleep(10);

				nPos = nStep = 0;
				dwFileSize = dwRecvSize = 0;
				break;
			}
			
			//update progress bar
			dwRecvSize += xi->nRead;
			Completed += xi->nRead;

			//szFolderPath contains the full file name
			fp = fopen(xi->szFolderPath,_T("ab"));
			fwrite(xi->szBuffer,sizeof(TCHAR),xi->nRead,fp);
			fclose(fp);

			nPos += nStep;
			SendMessage(GetDlgItem(hDlg,IDC_PROGRESS),PBM_SETPOS,nPos,0);
			InvalidateRect(GetDlgItem(hDlg,IDC_PROGRESS),NULL,FALSE);
			
			break;
		}

		case WM_SET_TOTAL_FILESIZE:
		{
					
			XINFO	*xi = (XINFO*)wParam;
			dwFileSize = xi->dwSize;

			sprintf(szBuff,_T("Total size: %.2fMB"),(float)dwFileSize/(float)1048576);

			SetWindowText(GetDlgItem(hDlg,STC_SIZE),szBuff);
			InvalidateRect(GetDlgItem(hDlg,STC_SIZE),NULL,TRUE);
			Sleep(10);

			//calculate the step size of progress bar			
			nStep = ((float)100/(float)dwFileSize) * (float)READ_BUFFER;
			break;
		}

		case WM_SETFILESIZE:
		{
						
			XINFO	*xi = (XINFO*)wParam;
			dwFileSize = xi->dwSize;

			sprintf(szProp, _T("Source Path: %s\r\n"), xi->szFileName);
			sprintf(szBuff,_T("File size: %.2fMB\r\n"),(float)dwFileSize/(float)1048576);
			strcat(szProp, szBuff);

			SetWindowText(GetDlgItem(hDlg,EDT_PROPERTIES),szProp);
			InvalidateRect(GetDlgItem(hDlg,EDT_PROPERTIES),NULL,TRUE);
			
			//calculate the step size of progress bar			
			nStep = ((float)100/(float)dwFileSize) * (float)READ_BUFFER;
			
			break;
		}
			
		//an error occured while recieving data
		case WM_NETWORK_ERR:
		{
			XINFO *xi = (XINFO*)wParam;

			DeleteFile(xi->szFolderPath);
			MessageBox(hDlg,
				_T("Data loss while recieving the file.\nPlease try again"),
				_T("Error"),MB_ICONERROR);
			EndDialog(hDlg,ERROR);
		//	nPos = nStep = dwFileSize = dwRecvSize = 0;
			break;
		}

		case WM_CLOSE:
			EndDialog(hDlg,SUCCESS);
		//	nPos = nStep = dwFileSize = dwRecvSize = 0;
			return TRUE;
	}

	return FALSE;
}

//void ProgressDlgThread(void *pParam)
unsigned int __stdcall ProgressDlgThread(void *pParam)
{
	HWND hParent = FindWindow(szWindowClass,szTitle);
	if(hParent == NULL)
		_endthreadex(0);
	else	
	//Show the progress dialog, 
	//so that we can send messages to the dialog directly
	DialogBox((HINSTANCE)GetWindowLongPtr(hParent,GWLP_HINSTANCE),
		(LPCTSTR)DLG_PROGRESS, hParent, (DLGPROC)Progress);

	_endthreadex(0);

	

	return 0;
}