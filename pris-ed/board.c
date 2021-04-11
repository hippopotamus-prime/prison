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
#include <stdlib.h>
#include "board.h"
#include "main.h"
#include "resource.h"

boardtype* curboard=NULL;

const short brickvals[] =
{
	brick_none,
	brick_plain,
	brick_2hit,
	brick_3hit,
	brick_4hit,
	brick_bonus0,
	brick_jewel,
	brick_accel,
	brick_tophit,
	brick_regen,
	brick_unbreakable0,
	brick_bomb,
	brick_attract0,
	brick_attract2,
	brick_teleport0
};


boardtype* NewBoard(boardtype* reference_board)
{
	boardtype* newboard = GlobalAlloc(GPTR, sizeof(boardtype));
	if(!newboard)
	{
		MessageBox(NULL, "Error creating the board, chief.", NULL, MB_OK);
	}
    else
    {
    	if(reference_board)
    	{
    		newboard->highbonusflags = reference_board->highbonusflags;
    		newboard->lowbonusflags = reference_board->lowbonusflags;
    	}
    	else
    	{
    		newboard->highbonusflags = 0xff;
    		newboard->lowbonusflags = 0xff;
    	}

    	FillMemory(&newboard->boardarray, BOARDWIDTH*BOARDHEIGHT, 0xff);
    }
    
	return newboard;
}


void deleteboard(void)
{
	if((curboard))
	{
		boardtype* tempboard;

		if((!curboard->next) && (!curboard->prev))
		{
			tempboard = NewBoard(NULL);
		}
		else
		{
			if((curboard->prev))
			{
				curboard->prev->next=curboard->next;
				tempboard=curboard->prev;
			}
			if((curboard->next))
			{
				curboard->next->prev=curboard->prev;
				tempboard=curboard->next;
			}
		}
		GlobalFree(curboard);
		curboard=tempboard;
	}
}

/**********************************************************
** Import a board set from a file into the current set,
** placing it directly after the given board.
**********************************************************/
BOOL AppendFile(boardtype** prev_board)
{
    BOOL success = FALSE;
    boardtype* new_set = NULL;

    OpenBoardFile(&new_set, FALSE);

    if(new_set != NULL)
    {
        boardtype* first = FindFirstBoard(new_set);
        boardtype* last = FindLastBoard(new_set);

        first->prev = *prev_board;

        if(*prev_board)
        {
            last->next = (*prev_board)->next;

            if((*prev_board)->next)
            {
                (*prev_board)->next->prev = last;
            }

    		(*prev_board)->next = first;
        }

        *prev_board = new_set;
        success = TRUE;
    }

    return success;
}


/**********************************************************
** Import a board set from a file into the current set,
** placing it directly before the given board.
**********************************************************/
BOOL InsertFile(boardtype** next_board)
{
    BOOL success = FALSE;
    boardtype* new_set = NULL;

    OpenBoardFile(&new_set, FALSE);

    if(new_set != NULL)
    {
        boardtype* first = FindFirstBoard(new_set);
        boardtype* last = FindLastBoard(new_set);

        last->next = *next_board;

        if(*next_board)
        {
            first->prev = (*next_board)->prev;

            if((*next_board)->prev)
            {
                (*next_board)->prev->next = first;
            }

    		(*next_board)->prev = last;
        }

        *next_board = new_set;
        success = TRUE;
    }

    return success;
}



/**********************************************************
** Add a new board to the current set, placing it
** directly after the given one.
**********************************************************/
BOOL AppendBoard(boardtype** prev_board)
{
	boardtype* board = NewBoard(*prev_board);

	if(!board)
	{
		return FALSE;
	}

	board->prev = *prev_board;

	if(*prev_board)
	{
		board->next = (*prev_board)->next;

		if((*prev_board)->next)
		{
			(*prev_board)->next->prev = board;
		}

		(*prev_board)->next = board;
	}

    *prev_board = board;

	return TRUE;
}


/**********************************************************
** Add a new board to the current set, placing it
** directly before the given one.
**********************************************************/
BOOL InsertBoard(boardtype** next_board)
{
	boardtype* board = NewBoard(*next_board);

	if(!board)
	{
		return FALSE;
	}

	board->next = *next_board;

	if(*next_board)
	{
		board->prev = (*next_board)->prev;

		if((*next_board)->prev)
		{
			(*next_board)->prev->next = board;
		}

		(*next_board)->prev = board;
	}

	*next_board = board;

	return TRUE;
}

/*********************************************************
** Count the number of boards in the current set
** preceeding the one given.  If NULL is given,
** count all the boards in the current set.
*********************************************************/
unsigned int CountBoards(const boardtype* end)
{
	unsigned int nboards = 0;
	boardtype* tempboard = FindFirstBoard(curboard);

	while((tempboard)
	&&(tempboard != end))
	{
		++nboards;
		tempboard = tempboard->next;
	}
	return nboards;
}

/**********************************************************
** Return the last board in the set containing
** the given board.
**********************************************************/
boardtype* FindLastBoard(boardtype* board)
{
	boardtype* last = board;

	if(last)
	{
		while(last->next)
		{
			last = last->next;
		}
	}

	return last;
}


/**********************************************************
** Return the fisrt board in the set containing
** the given board.
**********************************************************/
boardtype* FindFirstBoard(boardtype* board)
{
	boardtype* firstboard = board;

	if(firstboard)
	{
		while(firstboard->prev)
		{
			firstboard = firstboard->prev;
		}
	}

	return firstboard;
}


void DestroyBoardSet(boardtype** board_set)
{
    boardtype* board = FindFirstBoard(*board_set);

	if(board)
	{
		boardtype* next_board;

		while(board)
		{
			next_board = board->next;
			GlobalFree(board);
			board = next_board;
		}

        *board_set = NULL;
	}
}


void redrawboard(void)
{
	short i, j;

	for(j=0; j<BOARDHEIGHT; j++)
	{
		for(i=0; i<BOARDWIDTH; i++)
		{
			drawbrick(i, j);
		}
	}

	InvalidateRect(boardwin, NULL, FALSE);
	PostMessage(boardwin, WM_PAINT, 0, 0);
}


void drawbrick(const short i, const short j)
{
	const BYTE brickinverses[] =
	{
		0,			//brick_explode0
		0,			//brick_explode1
		0,			//brick_explode2
		0,			//brick_explode3
		10,			//brick_unbreakable0
		0,			//brick_unbreakable1
		0,			//brick_unbreakable2
		0,			//brick_unbreakable3
		11,			//brick_bomb
		12,			//brick_attract0
		0,			//brick_attract1
		13,			//brick_attract2
		14,			//brick_teleport0
		0,			//brick_teleport1
		0,			//brick_teleport2
		0,			//brick_PLACEHOLDER3
		1,			//brick_plain
		2,			//brick_2hit
		3,			//brick_3hit
		4,			//brick_4hit
		5,			//brick_bonus0
		0,			//brick_bonus1
		0,			//brick_bonus2
		0,			//brick_bonus3
		6,			//brick_jewel
		7,			//brick_accel
		8,			//brick_tophit
		9,			//brick_regen
		0,			//brick_PLACEHOLDER5
		0,			//brick_PLACEHOLDER6
		0,			//brick_PLACEHOLDER7
		0,			//brick_disabled
	};

	BYTE type=0;

	if((curboard))
	{
		if(curboard->boardarray[i][j] < 16)
		{
			type=brickinverses[curboard->boardarray[i][j]];
		}
		else if(curboard->boardarray[i][j] != 0xff)
		{
			type=brickinverses[((curboard->boardarray[i][j])&15)+16];
			if((curboard->boardarray[i][j]-16)>>4 != curcolor)
			{
				setpalette(&colorvals[(curboard->boardarray[i][j]-16)>>4],
					type-1);
			}
		}
	}

	if((type))
	{
		HDC bdc=CreateCompatibleDC(boarddc);
		HBITMAP defbmp=SelectObject(bdc, brickbmps[type-1]);

		if(zoom==4)
		{
			BitBlt(boarddc, i*BRICKWIDTH, j*BRICKHEIGHT, BRICKWIDTH,
				BRICKHEIGHT, bdc, 0, 0, SRCCOPY);
		}
		else if((zoom))
		{
			StretchBlt(boarddc, (i*BRICKWIDTH*zoom)>>2,
				(j*BRICKHEIGHT*zoom)>>2, (BRICKWIDTH*zoom)>>2,
				(BRICKHEIGHT*zoom)>>2, bdc, 0, 0,
				BRICKWIDTH, BRICKHEIGHT, SRCCOPY);
		}

		SelectObject(bdc, defbmp);
		DeleteDC(bdc);
		
		if((type<=NREGBRICKS)
		&& ((curboard->boardarray[i][j]-16)>>4 != curcolor))
		{
			setpalette(&colorvals[curcolor], type-1);
		}
	}
	else
	{
		PatBlt(boarddc, (i*BRICKWIDTH*zoom)>>2,
			(j*BRICKHEIGHT*zoom)>>2,
			(BRICKWIDTH*zoom)>>2, (BRICKHEIGHT*zoom)>>2,
			WHITENESS);
	}
}


BOOL setbrick(const short i, const short j, const short type)
{
	if(((curboard))
	&& (i<BOARDWIDTH) && (j<BOARDHEIGHT))
	{
		BOOL updateflag=FALSE;

		if(((type>NREGBRICKS) || (!type))
		&& (curboard->boardarray[i][j]!=brickvals[type]))
		{
			curboard->boardarray[i][j]=brickvals[type];
			updateflag=TRUE;
		}
		else if((type<=NREGBRICKS) && (type)
		&& (curboard->boardarray[i][j]!=brickvals[type]+(curcolor<<4)))
		{
			curboard->boardarray[i][j]=brickvals[type]+(curcolor<<4);
			updateflag=TRUE;
		}

		if((updateflag))
		{
			seteditflag();
			drawbrick(i, j);
			return TRUE;
		}
	}
	return FALSE;
}
