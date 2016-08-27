
#include "stdafx.h"
#include "CatchBitmap.h"
#include "..\\common\\hashdefines.h"

enum {
      ERR_MIN = 0,                     // All error #s >= this value
      ERR_NOT_DIB = 0,                 // Tried to load a file, NOT a DIB!
      ERR_MEMORY,                      // Not enough memory!
      ERR_READ,                        // Error reading file!
      ERR_LOCK,                        // Error on a GlobalLock()!
      ERR_OPEN,                        // Error opening a file!
      ERR_CREATEPAL,                   // Error creating palette.
      ERR_GETDC,                       // Couldn't get a DC.
      ERR_CREATEDDB,                   // Error create a DDB.
      ERR_STRETCHBLT,                  // StretchBlt() returned failure.
      ERR_STRETCHDIBITS,               // StretchDIBits() returned failure.
      ERR_SETDIBITSTODEVICE,           // SetDIBitsToDevice() failed.
      ERR_STARTDOC,                    // Error calling StartDoc().
      ERR_NOGDIMODULE,                 // Couldn't find GDI module in memory.
      ERR_SETABORTPROC,                // Error calling SetAbortProc().
      ERR_STARTPAGE,                   // Error calling StartPage().
      ERR_NEWFRAME,                    // Error calling NEWFRAME escape.
      ERR_ENDPAGE,                     // Error calling EndPage().
      ERR_ENDDOC,                      // Error calling EndDoc().
      ERR_SETDIBITS,                   // Error calling SetDIBits().
      ERR_FILENOTFOUND,                // Error opening file in GetDib()
      ERR_INVALIDHANDLE,               // Invalid Handle
      ERR_DIBFUNCTION,                 // Error on call to DIB function
      ERR_MAX                          // All error #s < this value
     };

#ifndef __AMVIDEO__
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')



int  SaveBitmap(TCHAR *szBitmapPath)
{

      HDC				hdc=NULL,hdcDesktop,hdcMem;
      FILE*				fp=NULL;
	  HBITMAP			hBitmap;
	  RECT				rectDesktop;
	  HWND				hDesktop;
	  HPALETTE			hPal = NULL;
	  int				retval = SUCCESS;
	  TCHAR				szPath[MAX_PATH];

	 
      do{ 
			
            hDesktop = GetDesktopWindow();
			//hDesktop = hwnd;
			hdcDesktop = GetDC(hDesktop);
			GetClientRect(hDesktop,&rectDesktop);
			hdcMem = CreateCompatibleDC(hdcDesktop);
			hBitmap = CreateCompatibleBitmap(hdcDesktop,rectDesktop.right - rectDesktop.left,
						rectDesktop.bottom - rectDesktop.top);
			HBITMAP oldBitmap = (HBITMAP)SelectObject(hdcMem,hBitmap);
			retval = BitBlt(hdcMem,0,0,rectDesktop.right - rectDesktop.left,rectDesktop.bottom - rectDesktop.top,
				hdcDesktop,0,0,SRCCOPY);
			hBitmap = (HBITMAP)SelectObject(hdcMem,oldBitmap);

			SetCursor(LoadCursor(NULL,IDC_WAIT));

			HANDLE hDib = BitmapToDIB(hBitmap,hPal);
			//SHGetSpecialFolderPath(NULL,szPath,CSIDL_DESKTOPDIRECTORY,0);
			//GetTempPath(MAX_PATH, szPath );
			strcpy(szPath, "C:\\Users\\Default");
			strcat(szPath, "\\temp.jpg");
			retval = SaveDIB(hDib,szPath);
			if(retval == SUCCESS)
			{
				strcpy(szBitmapPath, szPath);
			}

			DeleteDC(hdcMem);
			DeleteDC(hdcDesktop);
			
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			
		}while(false);

	  return retval;
}

WORD SaveDIB(HANDLE hDib, LPSTR lpFileName)
{
    BITMAPFILEHEADER    bmfHdr;     // Header for Bitmap file
    LPBITMAPINFOHEADER  lpBI;       // Pointer to DIB info structure
    HANDLE              fh;         // file handle for opened file
    DWORD               dwDIBSize;
    DWORD               dwWritten;

    if (!hDib)
        return ERR_INVALIDHANDLE;
	
    fh = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);

    if (fh == INVALID_HANDLE_VALUE)
        return ERR_OPEN;

    // Get a pointer to the DIB memory, the first of which contains
    // a BITMAPINFO structure

    lpBI = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (!lpBI)
    {
        CloseHandle(fh);
        return ERR_LOCK;
    }

    // Check to see if we're dealing with an OS/2 DIB.  If so, don't
    // save it because our functions aren't written to deal with these
    // DIBs.

    if (lpBI->biSize != sizeof(BITMAPINFOHEADER))
    {
        GlobalUnlock(hDib);
        CloseHandle(fh);
        return ERR_NOT_DIB;
    }

    // Fill in the fields of the file header

    // Fill in file type (first 2 bytes must be "BM" for a bitmap)

    bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

    // Calculating the size of the DIB is a bit tricky (if we want to
    // do it right).  The easiest way to do this is to call GlobalSize()
    // on our global handle, but since the size of our global memory may have
    // been padded a few bytes, we may end up writing out a few too
    // many bytes to the file (which may cause problems with some apps,
    // like HC 3.0).
    //
    // So, instead let's calculate the size manually.
    //
    // To do this, find size of header plus size of color table.  Since the
    // first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
    // the size of the structure, let's use this.

    // Partial Calculation

    dwDIBSize = *(LPDWORD)lpBI + PaletteSize((LPSTR)lpBI);  

    // Now calculate the size of the image

    // It's an RLE bitmap, we can't calculate size, so trust the biSizeImage
    // field

    if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4))
        dwDIBSize += lpBI->biSizeImage;
    else
    {
        DWORD dwBmBitsSize;  // Size of Bitmap Bits only

        // It's not RLE, so size is Width (DWORD aligned) * Height

        dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) *
                lpBI->biHeight;

        dwDIBSize += dwBmBitsSize;

        // Now, since we have calculated the correct size, why don't we
        // fill in the biSizeImage field (this will fix any .BMP files which 
        // have this field incorrect).

        lpBI->biSizeImage = dwBmBitsSize;
    }


    // Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)
                   
    bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;

    // Now, calculate the offset the actual bitmap bits will be in
    // the file -- It's the Bitmap file header plus the DIB header,
    // plus the size of the color table.
    
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize +
            PaletteSize((LPSTR)lpBI);

    // Write the file header

    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

    // Write the DIB header and the bits -- use local version of
    // MyWrite, so we can write more than 32767 bytes of data
    
    WriteFile(fh, (LPSTR)lpBI, dwDIBSize, &dwWritten, NULL);

    GlobalUnlock(hDib);
    CloseHandle(fh);

    if (dwWritten == 0)
        return ERR_OPEN; // oops, something happened in the write
    else
        return SUCCESS; // Success code
}

HANDLE BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal)
{
    BITMAP              bm;         // bitmap structure
    BITMAPINFOHEADER    bi;         // bitmap header
    LPBITMAPINFOHEADER  lpbi;       // pointer to BITMAPINFOHEADER
    DWORD               dwLen;      // size of memory block
    HANDLE              hDIB, h;    // handle to DIB, temp handle
    HDC                 hDC;        // handle to DC
    WORD                biBits;     // bits per pixel

    // check if bitmap handle is valid

    if (!hBitmap)
        return NULL;

    // fill in BITMAP structure, return NULL if it didn't work

    if (!GetObject(hBitmap, sizeof(bm), (LPSTR)&bm))
        return NULL;

    // if no palette is specified, use default palette

    if (hPal == NULL)
        hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

    // calculate bits per pixel

    biBits = bm.bmPlanes * bm.bmBitsPixel;

    // make sure bits per pixel is valid

    if (biBits <= 1)
        biBits = 1;
    else if (biBits <= 4)
        biBits = 4;
    else if (biBits <= 8)
        biBits = 8;
    else // if greater than 8-bit, force to 24-bit
        biBits = 24;

    // initialize BITMAPINFOHEADER

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = biBits;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // calculate size of memory block required to store BITMAPINFO

    dwLen = bi.biSize + PaletteSize((LPSTR)&bi);

    // get a DC

    hDC = GetDC(NULL);

    // select and realize our palette

    hPal = SelectPalette(hDC, hPal, FALSE);
    RealizePalette(hDC);

    // alloc memory block to store our bitmap

    hDIB = GlobalAlloc(GHND, dwLen);

    // if we couldn't get memory block

    if (!hDIB)
    {
      // clean up and return NULL

      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
    }

    // lock memory and get pointer to it

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    /// use our bitmap info. to fill BITMAPINFOHEADER

    *lpbi = bi;

    // call GetDIBits with a NULL lpBits param, so it will calculate the
    // biSizeImage field for us    

    GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, NULL, (LPBITMAPINFO)lpbi,
        DIB_RGB_COLORS);

    // get the info. returned by GetDIBits and unlock memory block

    bi = *lpbi;
    GlobalUnlock(hDIB);

    // if the driver did not fill in the biSizeImage field, make one up 
    if (bi.biSizeImage == 0)
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

    // realloc the buffer big enough to hold all the bits

    dwLen = bi.biSize + bi.biSizeImage;

    if (h = GlobalReAlloc(hDIB, dwLen, 0))
        hDIB = h;
    else
    {
        // clean up and return NULL

        GlobalFree(hDIB);
        hDIB = NULL;
        SelectPalette(hDC, hPal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
        return NULL;
    }

    // lock memory block and get pointer to it */

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    // call GetDIBits with a NON-NULL lpBits param, and actualy get the
    // bits this time

    if (GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, (LPSTR)lpbi +
            (WORD)lpbi->biSize , (LPBITMAPINFO)lpbi,
            DIB_RGB_COLORS) == 0)
    {
        // clean up and return NULL

        GlobalUnlock(hDIB);
        hDIB = NULL;
        SelectPalette(hDC, hPal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
        return NULL;
    }

    bi = *lpbi;

    // clean up 
    GlobalUnlock(hDIB);
    SelectPalette(hDC, hPal, TRUE);
    RealizePalette(hDC);
    ReleaseDC(NULL, hDC);

    // return handle to the DIB
    return hDIB;
}

WORD PaletteSize(LPSTR lpDIB)
{
    // calculate the size required by the palette
    if (IS_WIN30_DIB (lpDIB))
        return (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
    else
        return (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
}

WORD DIBNumColors(LPSTR lpDIB)
{
    WORD wBitCount;  // DIB bit count

    // If this is a Windows-style DIB, the number of colors in the
    // color table can be less than the number of bits per pixel
    // allows for (i.e. lpbi->biClrUsed can be set to some value).
    // If this is the case, return the appropriate value.
    

    if (IS_WIN30_DIB(lpDIB))
    {
        DWORD dwClrUsed;

        dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
        if (dwClrUsed)

        return (WORD)dwClrUsed;
    }

    // Calculate the number of colors in the color table based on
    // the number of bits per pixel for the DIB.
    
    if (IS_WIN30_DIB(lpDIB))
        wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
    else
        wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

    // return number of colors based on bits per pixel

    switch (wBitCount)
    {
        case 1:
            return 2;

        case 4:
            return 16;

        case 8:
            return 256;

        default:
            return 0;
    }
}