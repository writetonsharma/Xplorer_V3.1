
#ifndef __CATCH_BITMAP_H__
#define __CATCH_BITMAP_H__


#include <windows.h>
#include <stdio.h>
#include <Shlobj.h>

int  SaveBitmap(TCHAR *szBitmapPath);
WORD SaveDIB(HANDLE hDib, LPSTR lpFileName);
HANDLE BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
WORD PaletteSize(LPSTR lpDIB);
WORD DIBNumColors(LPSTR lpDIB);

#endif