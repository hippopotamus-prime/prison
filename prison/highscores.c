/****************************************************************************
** The Prison
** Copyright 2004 Aaron Curtis
** 
** This file is part of The Prison.
** 
** The Prison is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** The Prison is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with The Prison; if not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#include <PalmOS.h>
#include "highscores.h"
#include "game.h"

static void writehighscore(DmOpenRef dbp, boarddata* dataptr, UInt8 index, highscore* hssrc);


UInt16 gettopboard(const Boolean external, const char* dbname, const UInt16 card)
{
	DmOpenRef dbp;
	boarddata* bdp;
	UInt16 topboard;

	openboarddata(&dbp, &bdp, external, dbname, card);
	topboard=bdp->topboard;
	closeboarddata(dbp, bdp, false);
	return topboard;
}


void settopboard(const Boolean external, const char* dbname,
const UInt16 card, const UInt16 topboard)
{
	DmOpenRef dbp;
	boarddata* bdp;

	openboarddata(&dbp, &bdp, external, dbname, card);
	if((external))
	{
		DmWrite(bdp, 0, &topboard, sizeof(UInt16));
	}
	else
	{
		bdp->topboard=topboard;
	}
	closeboarddata(dbp, bdp, false);
}


UInt16 getstartboard(const Boolean external, const char* dbname, const UInt16 card)
{
	DmOpenRef dbp;
	boarddata* bdp;
	UInt16 startboard;

	openboarddata(&dbp, &bdp, external, dbname, card);
	startboard=bdp->startboard;
	closeboarddata(dbp, bdp, false);
	return startboard;
}


void setstartboard(const Boolean external, const char* dbname,
const UInt16 card, const UInt16 startboard)
{
	DmOpenRef dbp;
	boarddata* bdp;

	openboarddata(&dbp, &bdp, external, dbname, card);
	if((external))
	{
		DmWrite(bdp, sizeof(UInt16), &startboard, sizeof(UInt16));
	}
	else
	{
		bdp->startboard=startboard;
	}
	closeboarddata(dbp, bdp, false);
}


void openboarddata(DmOpenRef* dbpp, boarddata** datapp,
const Boolean external, const char* dbname, const UInt16 card)
{
	if((external))
	{
		MemHandle datahand;
		LocalID lid=DmFindDatabase(card, dbname);
		ErrFatalDisplayIf(!lid, "Error opening high score DB, chief.");
		*dbpp=DmOpenDatabase(card, lid, dmModeReadWrite);
		datahand=DmGetRecord(*dbpp, 0);
		*datapp=(boarddata*)MemHandleLock(datahand);
	}
	else
	{
		*dbpp=NULL;
		*datapp=&prefs.data;
	}
}


void closeboarddata(DmOpenRef dbp, boarddata* bdp, Boolean dirty)
{
	if((dbp))
	{
		MemPtrUnlock(bdp);
		DmReleaseRecord(dbp, 0, dirty);
		DmCloseDatabase(dbp);
	}
}


void writehighscore(DmOpenRef dbp, boarddata* dataptr, UInt8 index, highscore* hssrc)
{
	if((dbp))
	{
		DmWrite(dataptr, index*sizeof(highscore)+5*sizeof(UInt16),
			hssrc, sizeof(highscore));
	}
	else
	{
		MemMove(&dataptr->highscores[index], hssrc, sizeof(highscore));
	}
}


/************************************************************
Insert a high score / name combo into a high score list,
shifting the old scores as needed.
*************************************************************/
void inserthighscore(UInt8 index, highscore* newscore)
{
	Int8 i;
	DmOpenRef dbp;
	boarddata* dataptr;

	openboarddataprefs(&dbp, &dataptr);

	for(i=NHIGHSCORES-2; i>=index; i--)
	{
		writehighscore(dbp, dataptr, i+1, &dataptr->highscores[i]);
	}

	writehighscore(dbp, dataptr, index, newscore);
	closeboarddata(dbp, dataptr, true);
	return;
}


/**********************************************************
Check if the current score is greater than any of the
high scores, and return which one if any.
***********************************************************/
UInt8 checkhighscore(void)
{
	UInt8 i;
	UInt8 hsindex=NOHIGHSCORE;
	DmOpenRef dbp;
	boarddata* dataptr;

	openboarddataprefs(&dbp, &dataptr);

	for(i=0; i<NHIGHSCORES; i++)
	{
		if(gv.score > dataptr->highscores[i].score)
		{
			hsindex=i;
			break;
		}
	}

	closeboarddata(dbp, dataptr, false);
	return hsindex;
}