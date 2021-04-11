/****************************************************************************
** Pris-Ed, The Prison Editor
** Copyright 2004 Aaron Curtis
** 
** This file is part of The Prison.
** 
** Pris-Ed is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** Pris-Ed is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with The Prison; if not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "main.h"
#include "resource.h"
#include "handlers.h"
#include "file.h"
#include "board.h"

char szClassName[]="mainwc";
char boardwcname[]="boardwc";
char brickwcname[]="brickwc";
char palettewcname[]="palettewc";

const palentry colorvals[] =
{
	{0x000000, 0x848484, 0xc4c4c4, 0xffffff},
	{0x333366, 0x666699, 0x9999cc, 0xccccff},
	{0x000033, 0x000066, 0x333399, 0x6666cc},
	{0x336633, 0x669966, 0x99cc99, 0xccffcc},
	{0x003300, 0x006600, 0x339933, 0x66cc66},
	{0x663333, 0x996666, 0xcc9999, 0xffcccc},
	{0x330000, 0x660000, 0x993333, 0xcc6666},
	{0x666633, 0x999966, 0xcccc99, 0xffffcc},
	{0x333300, 0x666600, 0x999933, 0xcccc66},
	{0x663366, 0x996699, 0xcc99cc, 0xffccff},
	{0x330033, 0x660066, 0x993399, 0xcc66cc},
	{0x336666, 0x669999, 0x99cccc, 0xccffff},
	{0x003333, 0x006666, 0x339999, 0x66cccc}
};

HWND boardwin;
HWND brickwin;
HWND palettewin;
HWND statuswin;
HWND dialogwin=NULL;
HBITMAP brickbmps[NBRICKS];
HDC boarddc;
BYTE curbrick=1;
BYTE altbrick=0;
BYTE curcolor=0;
BOOL editflag=FALSE;
BYTE zoom=4;

int WINAPI
WinMain (HINSTANCE hThisInstance,
         HINSTANCE hPrevInstance,
         LPSTR lpszArgument,
         int nFunsterStil)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance=hThisInstance;
    wincl.lpszClassName=szClassName;
    wincl.lpfnWndProc=mainhandler;
    wincl.style=0;
    wincl.cbSize=sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(idi_main));
    wincl.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(idi_main));
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = MAKEINTRESOURCE(idr_mainmenu);
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

	InitCommonControls();

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "Pris-Ed",	        /* Title Text */
           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
				WS_MINIMIZEBOX | WS_MAXIMIZEBOX, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           200,                 /* The programs width */
           100,     /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nFunsterStil);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&messages, NULL, 0, 0) > 0)
    {
		if(!IsDialogMessage(dialogwin, &messages))
		{
	        /* Translate virtual-key messages into character messages */
	        TranslateMessage(&messages);
	        /* Send message to WindowProcedure */
	        DispatchMessage(&messages);
		}
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


void setzoom(const BYTE zvalue)
{
	if((zvalue>=MINZOOM) && (zvalue<=MAXZOOM))
	{
		HBITMAP boardbmp=CreateCompatibleBitmap(boarddc,
			(BOARDWIDTH*BRICKWIDTH*zvalue)>>2,
			(BOARDHEIGHT*BRICKHEIGHT*zvalue)>>2);

		if((boardbmp))
		{
			HBITMAP oldbmp;
			RECT boardrect={0, 0, 0, 0};
			RECT brickrect={0, 0, 0, 0};
			RECT paletterect={0, 0, 0, 0};
			RECT mainrect={0, 0, 0, 0};
			RECT statusrect;
			HWND mainwin=GetParent(boardwin);
			HMENU zoommenu=GetSubMenu(GetMenu(mainwin), 2);

			oldbmp=SelectObject(boarddc, boardbmp);
			DeleteObject(oldbmp);
			PatBlt(boarddc, 0, 0,
				(BOARDWIDTH*BRICKWIDTH*zvalue)>>2,
				(BOARDHEIGHT*BRICKHEIGHT*zvalue)>>2,
				WHITENESS);

			//check the right menu item...
			CheckMenuItem(zoommenu, (zoom-MINZOOM)+3,
				MF_BYPOSITION | MF_UNCHECKED);
			CheckMenuItem(zoommenu, (zvalue-MINZOOM)+3,
				MF_BYPOSITION | MF_CHECKED);

			zoom=zvalue;

			//resize board window...
			boardrect.right=(BRICKWIDTH*BOARDWIDTH*zoom)>>2;
			boardrect.bottom=(((BRICKHEIGHT*zoom)>>2)+2)*(NBRICKS+1);
			AdjustWindowRectEx(&boardrect,
				GetWindowLong(boardwin, GWL_STYLE),
				FALSE,
				GetWindowLong(boardwin, GWL_EXSTYLE));
			SetWindowPos(boardwin, NULL,
				boardrect.left,
				boardrect.top,
				boardrect.right-boardrect.left,
				boardrect.bottom-boardrect.top,
				SWP_NOZORDER | SWP_SHOWWINDOW);

			//resize brick window...
			brickrect.right=((BRICKWIDTH*zoom)>>2)+2;
			brickrect.bottom=(((BRICKHEIGHT*zoom)>>2)+2)*(NBRICKS+1);
			AdjustWindowRectEx(&brickrect,
				GetWindowLong(brickwin, GWL_STYLE),
				FALSE,
				GetWindowLong(brickwin, GWL_EXSTYLE));
			SetWindowPos(brickwin, NULL,
				boardrect.right,
				brickrect.top,
				brickrect.right-brickrect.left,
				brickrect.bottom-brickrect.top,
				SWP_NOZORDER | SWP_SHOWWINDOW);

			//resize the palette window...
			paletterect.right=((BRICKWIDTH*zoom)>>2)+2;
			paletterect.bottom=(((BRICKHEIGHT*zoom)>>2)+2)*(NBRICKS+1);
			AdjustWindowRectEx(&paletterect,
				GetWindowLong(palettewin, GWL_STYLE),
				FALSE,
				GetWindowLong(palettewin, GWL_EXSTYLE));
			SetWindowPos(palettewin, NULL,
				boardrect.right+brickrect.right,
				paletterect.top,
				paletterect.right-paletterect.left,
				paletterect.bottom-paletterect.top,
				SWP_NOZORDER | SWP_SHOWWINDOW);

			//and the main window...
			GetWindowRect(statuswin, &statusrect);
			mainrect.right=boardrect.right +
				brickrect.right +
				paletterect.right;
			mainrect.bottom=boardrect.bottom +
				(statusrect.bottom-statusrect.top);
			AdjustWindowRectEx(&mainrect,
				GetWindowLong(mainwin, GWL_STYLE),
				TRUE,
				GetWindowLong(mainwin, GWL_EXSTYLE));
			SetWindowPos(mainwin, NULL, 0, 0,
				mainrect.right-mainrect.left,
				mainrect.bottom-mainrect.top,
				SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);

			redrawboard();
			InvalidateRect(brickwin, NULL, FALSE);
			InvalidateRect(palettewin, NULL, FALSE);
		}
	}
}


void storeconfig(void)
{
	char inifile[MAX_PATH];

	if(getexepath(inifile))
	{
		char zoomtext[4];

		lstrcat(inifile, "pris-ed.ini");

		WritePrivateProfileString("general", "curfile", curfile, inifile);

		itoa(zoom, zoomtext, 10);
		WritePrivateProfileString("general", "zoom", zoomtext, inifile);
	}
}


short getexepath(char* pathname)
{
	DWORD pathlen=GetModuleFileName(GetModuleHandle(NULL), pathname, MAX_PATH);
	if((pathlen))
	{
		DWORD strpos=pathlen-1;

		while(pathname[strpos]!='\\')
		{
			strpos--;
		}
		pathname[strpos+1]='\0';
		return 1;
	}
	return 0;
}


void loadconfig(void)
{
	char inifile[MAX_PATH];

	if(getexepath(inifile))
	{
		lstrcat(inifile, "pris-ed.ini");

		if(GetPrivateProfileString("general", "curfile", "",
		curfile, MAX_PATH, inifile))
		{
			ReopenFile();
		}
		else
		{
			curboard = NewBoard(NULL);
		}

		zoom=GetPrivateProfileInt("general", "zoom",
			4, inifile);
	}
}


void updateboardmenu(void)
{
	if((curboard))
	{
		HMENU boardmenu=GetSubMenu(GetMenu(GetParent(boardwin)), 1);

		if((curboard->next))
		{
			EnableMenuItem(boardmenu, 0, MF_BYPOSITION | MF_ENABLED);
		}
		else
		{
			EnableMenuItem(boardmenu, 0, MF_BYPOSITION | MF_GRAYED);
		}

		if((curboard->prev))
		{
			EnableMenuItem(boardmenu, 1, MF_BYPOSITION | MF_ENABLED);
		}
		else
		{
			EnableMenuItem(boardmenu, 1, MF_BYPOSITION | MF_GRAYED);
		}
	}
}


void updatestatusbar(void)
{
	char foo[30];
	short nboards = CountBoards(NULL);

	itoa(nboards, foo, 10);
	if(nboards==1)
	{
		lstrcat(foo, " Board Total");		
	}
	else
	{
		lstrcat(foo, " Boards Total");
	}
	SendMessage(statuswin, SB_SETTEXT, 1, (LPARAM)foo);

	lstrcpy(foo, "Board ");
	itoa(CountBoards(curboard)+1, &foo[6], 10);
	SendMessage(statuswin, SB_SETTEXT, 0, (LPARAM)foo);
}


void reseteditflag(void)
{
	if((editflag))
	{
		char titletext[MAX_PATH+14];
		int titlelen;

		GetWindowText(GetParent(boardwin), titletext, MAX_PATH+11);
		titlelen=lstrlen(titletext);
		titletext[titlelen-2]='\0';
		SetWindowText(GetParent(boardwin), titletext);
		editflag=FALSE;
	}
}


void seteditflag(void)
{
	if(!editflag)
	{
		char titletext[MAX_PATH+14];

		GetWindowText(GetParent(boardwin), titletext, MAX_PATH+11);
		lstrcat(titletext, " *");
		SetWindowText(GetParent(boardwin), titletext);
		editflag=TRUE;
	}
}


void loadbmps(void)
{
	short i;
	HINSTANCE hinstance=GetModuleHandle(NULL);

	for(i=0; i<NREGBRICKS; i++)
	{
		brickbmps[i]=LoadImage(hinstance,
			MAKEINTRESOURCE(idb_brick_start+i),
			IMAGE_BITMAP,
			0, 0,
			LR_CREATEDIBSECTION);
		if(!brickbmps[i])
		{
			MessageBox(NULL, "Error loading bitmaps [1], chief.", NULL, MB_OK);
			break;
		}
	}

	for(i=NREGBRICKS; i<NBRICKS; i++)
	{
		brickbmps[i]=LoadImage(hinstance,
			MAKEINTRESOURCE(idb_brick_start+i),
			IMAGE_BITMAP,
			0, 0, 0);
		if(!brickbmps[i])
		{
			MessageBox(NULL, "Error loading bitmaps [2], chief.", NULL, MB_OK);
			break;
		}
	}
}


void freebmps(void)
{
	short i;

	for(i=0; i<NBRICKS; i++)
	{
		DeleteObject(brickbmps[i]);
	}
}


/***************************************************
Create the main sub-windows...
****************************************************/
void createchilluns(HWND mainwin)
{
	WNDCLASSEX boardwc;
	WNDCLASSEX brickwc;
	WNDCLASSEX palettewc;
	int statuswidths[]={60, -1};

	boardwc.cbSize=sizeof(WNDCLASSEX);
	boardwc.style=0;
	boardwc.lpfnWndProc=boardhandler;
	boardwc.cbClsExtra=0;
	boardwc.cbWndExtra=0;
	boardwc.hInstance=GetModuleHandle(NULL);
	boardwc.hIcon=LoadIcon(NULL, IDI_APPLICATION);
	boardwc.hCursor=LoadCursor(NULL, IDC_ARROW);
	boardwc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	boardwc.lpszMenuName=NULL;
	boardwc.lpszClassName=boardwcname;
	boardwc.hIconSm=LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&boardwc);

	brickwc.cbSize=sizeof(WNDCLASSEX);
	brickwc.style=0;
	brickwc.lpfnWndProc=brickhandler;
	brickwc.cbClsExtra=0;
	brickwc.cbWndExtra=0;
	brickwc.hInstance=GetModuleHandle(NULL);
	brickwc.hIcon=LoadIcon(NULL, IDI_APPLICATION);
	brickwc.hCursor=LoadCursor(NULL, IDC_ARROW);
	brickwc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	brickwc.lpszMenuName=NULL;
	brickwc.lpszClassName=brickwcname;
	brickwc.hIconSm=LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&brickwc);

	palettewc.cbSize=sizeof(WNDCLASSEX);
	palettewc.style=0;
	palettewc.lpfnWndProc=palettehandler;
	palettewc.cbClsExtra=0;
	palettewc.cbWndExtra=0;
	palettewc.hInstance=GetModuleHandle(NULL);
	palettewc.hIcon=LoadIcon(NULL, IDI_APPLICATION);
	palettewc.hCursor=LoadCursor(NULL, IDC_ARROW);
	palettewc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	palettewc.lpszMenuName=NULL;
	palettewc.lpszClassName=palettewcname;
	palettewc.hIconSm=LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&palettewc);

	boardwin=CreateWindowEx(WS_EX_CLIENTEDGE, boardwcname, "foo",
		WS_CHILD,
		0, 0, BOARDWIDTH*BRICKWIDTH+4, 40,
		mainwin, NULL,
		GetModuleHandle(NULL), NULL);

	brickwin=CreateWindowEx(WS_EX_CLIENTEDGE, brickwcname, "bar",
		WS_CHILD | WS_CLIPSIBLINGS,
		BOARDWIDTH*BRICKWIDTH+4, 0,
		BRICKWIDTH+2+4, 40,
		mainwin, NULL,
		GetModuleHandle(NULL), NULL);

	palettewin=CreateWindowEx(WS_EX_CLIENTEDGE, palettewcname, "meh",
		WS_CHILD,
		(BOARDWIDTH+1)*BRICKWIDTH+10, 0,
		BRICKWIDTH+2+4, 40,
		mainwin, NULL,
		GetModuleHandle(NULL), NULL);

	statuswin=CreateWindowEx(0, STATUSCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		mainwin, (HMENU)idc_status,
		GetModuleHandle(NULL), NULL);

	SendMessage(statuswin, SB_SETPARTS,
		sizeof(statuswidths)/sizeof(int), (LPARAM)statuswidths);

	ShowWindow(boardwin, SW_SHOWNORMAL);
	ShowWindow(brickwin, SW_SHOWNORMAL);
	ShowWindow(palettewin, SW_SHOWNORMAL);
}


void drawframe(const HWND win, const short i, COLORREF color)
{
	RECT brickframe;
	HDC windc=GetDC(win);
	HBRUSH brush;

	brickframe.left=0;
	brickframe.top=i*(((BRICKHEIGHT*zoom)>>2)+2);
	brickframe.right=((BRICKWIDTH*zoom)>>2)+2;
	brickframe.bottom=(i+1)*(((BRICKHEIGHT*zoom)>>2)+2);

	if(!color)
	{
		color=GetBkColor(windc);
	}

	brush=CreateSolidBrush(color);

	FrameRect(windc, &brickframe, brush);
	ReleaseDC(win, windc);
	DeleteObject(brush);
}




void setpalette(const palentry* newcolors, const short index)
{
	short i, j;
	RGBQUAD brickcolors[4];
	short end, start;
	HDC bdc=CreateCompatibleDC(boarddc);
	HBITMAP defbmp=SelectObject(bdc, brickbmps[0]);
	
	if(index<NREGBRICKS)
	{
		end=index+1;
		start=index;
	}
	else
	{
		end=NREGBRICKS;
		start=0;
	}

	for(i=start; i<end; i++)
	{
		SelectObject(bdc, brickbmps[i]);
		for(j=0; j<4; j++)
		{
			brickcolors[j].rgbRed=(newcolors->entry[j])&255;
			brickcolors[j].rgbGreen=(newcolors->entry[j]>>8)&255;
			brickcolors[j].rgbBlue=(newcolors->entry[j]>>16)&255;
		}
		SetDIBColorTable(bdc, 0, 4, brickcolors);
	}

	SelectObject(bdc, defbmp);
	DeleteDC(bdc);
	InvalidateRect(brickwin, NULL, FALSE);
}
