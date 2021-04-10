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

#ifndef __BOARD__
#define __BOARD__

#include <windows.h>

#define BOARDWIDTH 9
#define BOARDHEIGHT 12

struct _boardtype
{
	BYTE lowbonusflags;
	BYTE highbonusflags;
	BYTE boardarray[BOARDWIDTH][BOARDHEIGHT];
	struct _boardtype* next;
	struct _boardtype* prev;
};
typedef struct _boardtype boardtype;

extern boardtype* curboard;

boardtype* NewBoard(boardtype* reference_board);
void redrawboard(void);
boardtype* FindFirstBoard(boardtype* board);
boardtype* FindLastBoard(boardtype* board);
void DestroyBoardSet(boardtype** board_set);
unsigned int CountBoards(const boardtype* end);
void deleteboard(void);
BOOL AppendFile(boardtype** prev_board);
BOOL InsertFile(boardtype** next_board);
BOOL AppendBoard(boardtype** prev_board);
BOOL InsertBoard(boardtype** next_board);
BOOL setbrick(const short i, const short j, const short type);
void drawbrick(const short i, const short j);

#endif
