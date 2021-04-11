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

#ifndef __MAIN__
#define __MAIN__

#include <windows.h>

#define BRICKWIDTH 16
#define BRICKHEIGHT 8
#define NBRICKS 14
#define NREGBRICKS 9
#define NCOLORS 13
#define MINZOOM 4
#define MAXZOOM 12
#define NBONI 8
#define BOARDVERSION 1

typedef struct
{
	COLORREF entry[4];
}palentry;

extern HWND boardwin;
extern HWND brickwin;
extern HWND palettewin;
extern HWND statuswin;
extern HWND dialogwin;
extern HBITMAP brickbmps[NBRICKS];
extern HDC boarddc;
extern BYTE curbrick;
extern BYTE altbrick;
extern BYTE curcolor;
extern BOOL editflag;
extern BYTE zoom;
extern const palentry colorvals[];

void createchilluns(HWND);
void loadbmps(void);
void freebmps(void);
void drawframe(const HWND win, const short i, COLORREF color);
void seteditflag(void);
void reseteditflag(void);
void updateboardmenu(void);
void updatestatusbar(void);
short getexepath(char* pathname);
void storeconfig(void);
void loadconfig(void);
void setzoom(const BYTE zvalue);
void setpalette(const palentry* newcolors, const short index);

#endif
