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

#ifndef __HIGHSCORES__
#define __HIGHSCORES__

#define MAXNAMELEN 16
#define NHIGHSCORES 10

typedef struct
{
	UInt32 score;
	Boolean finished;
	char name[MAXNAMELEN+1];
}highscore;

typedef struct
{
	UInt16 topboard;
	UInt16 startboard;
	UInt16 reserved1;
	UInt16 reserved2;
	UInt16 reserved3;
	highscore highscores[NHIGHSCORES];
}boarddata;

extern void openboarddata(DmOpenRef* dbpp, boarddata** datapp,
	const Boolean external, const char* dbname, const UInt16 card);
extern void closeboarddata(DmOpenRef dbp, boarddata* bdp, Boolean dirty);
extern UInt8 checkhighscore(void);
extern void inserthighscore(UInt8 index, highscore* newscore);
extern UInt16 getstartboard(const Boolean external, const char* dbname, const UInt16 card);
extern void setstartboard(const Boolean external, const char* dbname,
	const UInt16 card, const UInt16 startboard);
extern void settopboard(const Boolean external, const char* dbname,
	const UInt16 card, const UInt16 topboard);
extern UInt16 gettopboard(const Boolean external, const char* dbname, const UInt16 card);

#define openboarddataprefs(dbpp, datapp) openboarddata(dbpp, datapp, prefs.settings.flags&set_extboards, prefs.settings.boardsetname, prefs.settings.boardsetcard)
#define getstartboardprefs() getstartboard(prefs.settings.flags&set_extboards, prefs.settings.boardsetname, prefs.settings.boardsetcard)
#define setstartboardprefs(value) setstartboard(prefs.settings.flags&set_extboards, prefs.settings.boardsetname, prefs.settings.boardsetcard, value)
#define gettopboardprefs() gettopboard(prefs.settings.flags&set_extboards, prefs.settings.boardsetname, prefs.settings.boardsetcard)
#define settopboardprefs(value) settopboard(prefs.settings.flags&set_extboards, prefs.settings.boardsetname, prefs.settings.boardsetcard, value)

#endif