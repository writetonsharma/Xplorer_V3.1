

/****************************************************************************
 *   Copyright (C) 2007 by naveen sharma									*
 *   writetonsharma@gmail.com												*
 *																			*
 *	 File: hashdefines.h, Jan 2008											*
 *   Description: all the hashdefines like error's, messages etc etc..		*
 *				  Lots of stuff here.										*
 ***************************************************************************/


#ifndef __HASHDEFINES_H__
#define __HASHDEFINES_H__

#define INFOTYPE		short
#define HINT			short
#define FILETYPE		short
#define MAX_CLIENTS		10
#define READ_BUFFER		2048000		//used while reading files
#define TRUNCATE_SIZE	1024		//kb


//INFOTYPES-------
#define STOP				0
#define ROOT				1			//its a root
#define CHILD				2			//its a child
#define CLOSE				3		
#define FILEDATA			4			//file will be create on desktop
#define FILEPROP			5
#define FILESIZE			6			//file size has been send, help in updating the progress dialog
#define FILEDATA_FINISH		7			//data finished
#define ERROR_SENDING_DATA	8			//error occured in sending the file data
#define FILEDATA_WITH_PATH	9			//file will be created at a specified path
#define MAKE_DIRECTORY		10			//information will be send in wparam or lparam where to make directory
#define TOTAL_FILESIZE		11			//total size which we are sending
//----------------

#define MAX_LOADSTRING 100


//Messages----------
#define WM_MAKETREE				WM_USER + 1001
#define WM_MAKEFILE				WM_USER + 1002
#define WM_SETFILESIZE			WM_USER + 1003
#define WM_NETWORK_ERR			WM_USER + 1004
#define WM_MAKEFILE_WITH_PATH	WM_USER + 1005
#define WM_MAKE_DIRECTORY		WM_USER + 1006
#define WM_SET_TOTAL_FILESIZE	WM_USER + 1007
#define WM_ADD_TO_USER_LIST		WM_USER + 1008
//------------------

//Error handling-----------
#define SUCCESS				0
#define GEN_ERROR			1
#define FILE_NOT_FOUND		2
#define INI_ERR				3
#define HOST_RESOLVE_ERR	4
//-------------------------

//Drive/File Type--------------- FILETYPE
#define FIXED_DRIVE				11
#define REMOTE_DRIVE			12
#define CDROM_DRIVE				13
#define RAMDISK_DRIVE			14
#define UNKNOWN_DRIVE			15
#define REMOVABLE_DRIVE			16
#define _FILE					17
#define _DIRECTORY				18
//-------------------------

//Icons in image list-----------
#define FOLDER_PLUS		0
#define FOLDER_OPEN		1
#define CD_ROM			2
#define FOLDER			3
#define FILE1			4
#define FLOPPY			5
#define DRIVE			6
//------------------------------

//Client hint values-------------
//These values tell the client as to what it should do 
//and what the server is expecting
#define COPY_FILE		1		//send file data, a path to file will be send
#define REMOVE_FILE		2		//remove that file, a path will be send
#define PROP_INFO		3		//send properties for the given file
#define SUB_FOLDERS		4		//send all subfolders name in the specified file
#define CLEAN_UP		5		//Server closing
#define RECV_SIZE		6		//how much data server recieved
#define OPEN_REMOTEFILE	7		//open a file on the client system
#define COPY_DIRECTORY	8		//copy the whole directory and all the sub directries and files (V3.0)
#define CATCH_DESKTOP	9		//save an image of desktop for sending to server (V3.0)
#define CLOSED			10		//client closed
//-------------------------------

#endif
