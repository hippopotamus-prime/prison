/****************************************************************************
** Pris-Ed, The Prison Editor
** Copyright 2004 Aaron Curtis
** 
** This file is part of The Prison.
** 
** Pris-Ed is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Pris-Ed is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with Pris-Ed; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****************************************************************************/

#include <windows.h>
#include "handlers.h"
#include "main.h"
#include "board.h"
#include "resource.h"

#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_MINUS      0xBD   // '-' any country

LRESULT CALLBACK
mainhandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	int command=0;
	BOOL returnval=FALSE;

    switch(message)
    {
		case WM_CREATE:
			loadbmps();
			createchilluns(hwnd);
			loadconfig();
			setzoom(zoom);
			updateboardmenu();
			updatestatusbar();
			setpalette(colorvals, 255);
			redrawboard();
			break;
		case WM_KEYDOWN:
			switch(wparam)
			{
				case VK_RIGHT:
					command = id_board_next;
					break;
				case VK_LEFT:
					command = id_board_prev;
					break;
				case VK_ADD:
				case VK_OEM_PLUS:
					command = id_zoom_more;
					break;
				case VK_SUBTRACT:
				case VK_OEM_MINUS:
					command = id_zoom_less;
					break;
				case VK_F1:
					command = id_board_insert;
					break;
				case VK_F2:
					command = id_board_append;
					break;
				case VK_F3:
					command = id_board_delete;
					break;
                case VK_F5:
                    command = id_board_insert_file;
                    break;
                case VK_F6:
                    command = id_board_append_file;
                    break;
				case VK_F12:
					command = id_board_flags;
			}
			break;
		case WM_COMMAND:
			command=LOWORD(wparam);
			break;
		case WM_CLOSE:
			if(asktosave()!=IDCANCEL)
			{
				DestroyWindow(hwnd);
			}
			break;
        case WM_DESTROY:
			storeconfig();
			DestroyBoardSet(&curboard);
			freebmps();
            PostQuitMessage(0);
            break;
		case WM_SIZE:
			SendMessage(statuswin, WM_SIZE, wparam, lparam);
        default:
            return DefWindowProc(hwnd, message, wparam, lparam);
    }

	switch(command)
	{
		case id_file_quit:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case id_file_saveas:
			savefileas();
			break;
		case id_file_save:
			savefile();
			break;
		case id_file_open:
			if(asktosave()!=IDCANCEL)
			{
                OpenBoardFile(&curboard, TRUE);
				updatestatusbar();
				updateboardmenu();
				redrawboard();
			}
			break;
		case id_file_reopen:
			ReopenFile();
			updatestatusbar();
			updateboardmenu();
			redrawboard();
			break;
		case id_file_export:
			exportfile();
			break;
		case id_file_new:
			if(asktosave()!=IDCANCEL)
			{
				DestroyBoardSet(&curboard);
				curboard = NewBoard(NULL);
				setcurfile(NULL);
				updatestatusbar();
				updateboardmenu();
				redrawboard();
			}
			break;
		case id_board_insert:
			InsertBoard(&curboard);
			seteditflag();
			updatestatusbar();
			updateboardmenu();
			redrawboard();
			break;
		case id_board_append:
			AppendBoard(&curboard);
			seteditflag();
			updatestatusbar();
			updateboardmenu();
			redrawboard();
			break;

		case id_board_delete:
			deleteboard();
			seteditflag();
			updatestatusbar();
			updateboardmenu();
			redrawboard();
			break;

        case id_board_insert_file:
            InsertFile(&curboard);
			seteditflag();
			updatestatusbar();
			updateboardmenu();
			redrawboard();
            break;

        case id_board_append_file:
            AppendFile(&curboard);
			seteditflag();
			updatestatusbar();
			updateboardmenu();
			redrawboard();
            break;

		case id_board_next:
			if(((curboard)) && ((curboard->next)))
			{
				curboard=curboard->next;
				updatestatusbar();
				updateboardmenu();
				redrawboard();
			}
			break;
		case id_board_prev:
			if(((curboard)) && ((curboard->prev)))
			{
				curboard=curboard->prev;
				updatestatusbar();
				updateboardmenu();
				redrawboard();
			}
			break;
		case id_board_flags:
			dialogwin=CreateDialog(GetModuleHandle(NULL),
				MAKEINTRESOURCE(idd_flags),
				GetParent(boardwin), flagshandler);
			break;
		case id_zoom_100:
		case id_zoom_125:
		case id_zoom_150:
		case id_zoom_175:
		case id_zoom_200:
		case id_zoom_225:
		case id_zoom_250:
		case id_zoom_275:
		case id_zoom_300:
			setzoom(MINZOOM+command-id_zoom_start);
			break;
		case id_zoom_more:
			setzoom(zoom+1);
			break;
		case id_zoom_less:
			setzoom(zoom-1);
			break;
		case id_help_about:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(idd_about),
				hwnd, abouthandler);
			break;
	}

    return returnval;
}


LRESULT CALLBACK
boardhandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	short refresh=0;

	switch(message)
	{
		case WM_CREATE:
		{
			HDC windc=GetDC(hwnd);
			short fail=1;

			if((windc))
			{
				boarddc=CreateCompatibleDC(windc);
				if((boarddc))
				{
					HBITMAP boardbmp=CreateCompatibleBitmap(windc,
						BOARDWIDTH*BRICKWIDTH, BOARDHEIGHT*BRICKHEIGHT);
					if((boardbmp))
					{
						HBITMAP oldbmp;
						oldbmp=SelectObject(boarddc, boardbmp);
						DeleteObject(oldbmp);
						PatBlt(boarddc, 0, 0, BOARDWIDTH*BRICKWIDTH,
							BOARDHEIGHT*BRICKHEIGHT, WHITENESS);
						fail=0;
					}
				}
				ReleaseDC(hwnd, windc);
			}
			if((fail))
			{
				MessageBox(NULL, "You're probably out of memory, chief.",
					NULL, MB_OK);
				PostQuitMessage(0);
			}
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			HDC vishdc=BeginPaint(hwnd, &ps);
			BitBlt(vishdc, 0, 0, (BOARDWIDTH*BRICKWIDTH*zoom)>>2,
				(BOARDHEIGHT*BRICKHEIGHT*zoom)>>2, boarddc, 0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_MOUSEMOVE:
			if(wparam&MK_LBUTTON)
			{
				refresh=setbrick(LOWORD(lparam)/((BRICKWIDTH*zoom)>>2),
					HIWORD(lparam)/((BRICKHEIGHT*zoom)>>2), curbrick);
			}
			else if(wparam&MK_RBUTTON)
			{
				refresh=setbrick(LOWORD(lparam)/((BRICKWIDTH*zoom)>>2),
					HIWORD(lparam)/((BRICKHEIGHT*zoom)>>2), altbrick);
			}
			break;
		case WM_RBUTTONDOWN:
			refresh=setbrick(LOWORD(lparam)/((BRICKWIDTH*zoom)>>2),
				HIWORD(lparam)/((BRICKHEIGHT*zoom)>>2), altbrick);
			break;
		case WM_LBUTTONDOWN:
			refresh=setbrick(LOWORD(lparam)/((BRICKWIDTH*zoom)>>2),
				HIWORD(lparam)/((BRICKHEIGHT*zoom)>>2), curbrick);
			break;
		case WM_CLOSE:
			DeleteDC(boarddc);
		default:
			return DefWindowProc(hwnd, message, wparam, lparam);
	}

	if((refresh))
	{
		HDC vishdc=GetDC(hwnd);
		BitBlt(vishdc, 0, 0, (BOARDWIDTH*BRICKWIDTH*zoom)>>2,
			(BOARDHEIGHT*BRICKHEIGHT*zoom)>>2, boarddc, 0, 0, SRCCOPY);
		ReleaseDC(hwnd, vishdc);
	}
	return 0;
}



LRESULT CALLBACK
brickhandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
		case WM_PAINT:
		{
			short i;
			PAINTSTRUCT ps;
			HDC visdc=BeginPaint(hwnd, &ps);
			HDC brickdc=CreateCompatibleDC(visdc);
			HBITMAP defbmp=SelectObject(brickdc, brickbmps[0]);

			for(i=0; i<NBRICKS; i++)
			{
				SelectObject(brickdc, brickbmps[i]);
				StretchBlt(visdc, 1, 1+(i+1)*(((BRICKHEIGHT*zoom)>>2)+2),
					(BRICKWIDTH*zoom)>>2,
					(BRICKHEIGHT*zoom)>>2,
					brickdc, 0, 0, BRICKWIDTH, BRICKHEIGHT, SRCCOPY);
			}
			drawframe(brickwin, curbrick, 0x0000ff00);
			drawframe(brickwin, altbrick, 0x000000ff);

			SelectObject(brickdc, defbmp);
			DeleteDC(brickdc);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_LBUTTONDOWN:
			drawframe(brickwin, curbrick, 0);
			curbrick=HIWORD(lparam)/(((BRICKHEIGHT*zoom)>>2)+2);
			drawframe(brickwin, curbrick, 0x0000ff00);
			break;
		case WM_RBUTTONDOWN:
			drawframe(brickwin, altbrick, 0);
			altbrick=HIWORD(lparam)/(((BRICKHEIGHT*zoom)>>2)+2);
			drawframe(brickwin, altbrick, 0x000000ff);
			break;
		default:
			return DefWindowProc(hwnd, message, wparam, lparam);
	}

	return 0;
}


LRESULT CALLBACK
palettehandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
		case WM_PAINT:
		{
			short i;
			PAINTSTRUCT ps;
			HBRUSH brush;
			RECT foorect={1, 0, ((BRICKWIDTH*zoom)>>2)+1, 0};
			HDC visdc=BeginPaint(hwnd, &ps);

			for(i=0; i<NCOLORS; i++)
			{
				foorect.top=1+i*(((BRICKHEIGHT*zoom)>>2)+2);
				foorect.bottom=(i+1)*(((BRICKHEIGHT*zoom)>>2)+2)-1;
				brush=CreateSolidBrush(colorvals[i].entry[1]);
				FillRect(visdc, &foorect, brush);
				DeleteObject(brush);
			}
			drawframe(palettewin, curcolor, 0x00ff00);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_LBUTTONDOWN:
			if(HIWORD(lparam)/(((BRICKHEIGHT*zoom)>>2)+2) < NCOLORS)
			{
				drawframe(palettewin, curcolor, 0);
				curcolor=HIWORD(lparam)/(((BRICKHEIGHT*zoom)>>2)+2);
				drawframe(palettewin, curcolor, 0x00ff00);
				setpalette(&colorvals[curcolor], 255);
			}
			break;
		default:
			return DefWindowProc(hwnd, message, wparam, lparam);
	}

	return 0;
}



BOOL CALLBACK
flagshandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{	
			short i;
			
			for(i=0; i<NBONI; i++)
			{
				if((curboard->lowbonusflags>>i)&1)
				{
					CheckDlgButton(hwnd, idcb_flag_start+i, BST_CHECKED);
				}
				else
				{
					CheckDlgButton(hwnd, idcb_flag_start+i, BST_UNCHECKED);
				}
			}
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case IDOK:
				{
					short i;
					BYTE newflags=0;

					for(i=0; i<NBONI; i++)
					{
						if(IsDlgButtonChecked(hwnd, idcb_flag_start+i)==BST_CHECKED)
						{
							newflags|=(1<<i);
						}
					}
					if(newflags!=curboard->lowbonusflags)
					{
						curboard->lowbonusflags=newflags;
						seteditflag();
					}
				}
				case IDCANCEL:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			dialogwin=NULL;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK
pdbnamehandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case IDOK:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
			}
			break;
		case WM_CLOSE:
		{
			char* pdbname=(char*)GlobalAlloc(GMEM_FIXED, 32);
			if((pdbname))
			{
				GetDlgItemText(hwnd,  ide_pdbname, pdbname, 32);
			}
			else
			{
				MessageBox(NULL, "Joo is out of memory", NULL, MB_OK);
			}

			EndDialog(hwnd, (int)pdbname);
		}
		default:
			return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK
abouthandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case IDOK:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hwnd, 0);		
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
