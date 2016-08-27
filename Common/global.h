

/****************************************************************************
 *   Copyright (C) 2007 by naveen sharma									*
 *   writetonsharma@gmail.com												*
 *																			*
 *	 File: global.h, Jan 2008												*
 *   Description: global data structure declarations here.					*
 ***************************************************************************/


#ifndef _GLOBAL_H__
#define _GLOBAL_H__

//client sends this info to server
typedef struct xinfo
{
	HWND			hTree;						//which tree this structure point to
	SOCKET			SockInfo;					//which socket sending data
	INFOTYPE		infotype;					//what info, like adding root, child etc.
	FILETYPE		filetype;					//what kind of node is this
											//directory,file,remote,cdrom etc.
	DWORD			nRead;					//How much data send
	double			dwSize;					//File size
	
	TCHAR			szFolderPath[MAX_PATH];		//path of remote folder
	TCHAR			szFileName[MAX_PATH];		//name of remote file/folder

	//TCHAR			szBuffer[READ_BUFFER];
	TCHAR			*szBuffer;
	

}XINFO;



//Hint struct, telling client what he should do
//like should he send sub-folders
//or should he remove files or send file properties or file data..etc
typedef struct hintinfo
{
	HINT		hint;
	DWORD		nRecv;
	TCHAR		szPath[MAX_PATH];

}HINTINFO;

//all nodes of the tree will save this info
//Nodes private data
typedef struct nodeinfo
{
	TCHAR		szFolderPath[MAX_PATH];
	TCHAR		szFileName[MAX_PATH];
	FILETYPE	filetype;					//what kind of node is this
											//directory,file,remote,cdrom etc.

}NODEINFO;


//which tree is connected to which client
//as the client sends data, 
//we can know which tree its refering from its SOCKET info
//or by know the hTree, we can know which client we want to communicate..simple!!
typedef struct treeclient
{
	HWND				hTree;				//which client this window is showing
	SOCKET				ClientSocket;		//which client socket is this										

}TREECLIENT;


#endif