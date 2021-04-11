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
#include "prefs.h"
#include "game.h"
#include "sound.h"


#define PREFGUYSTART (PREFGAMEVARS+1)
#define PREFBONISTART (PREFGUYSTART+MAXGUYS)
#define PREFBRICKANIMSTART (PREFBONISTART+MAXBONI)

prisprefs prefs;


void getprefs(void)
{
	UInt16 prefsize = sizeof(prisprefs);
	UInt16 prefversion;
	tpref temp;

	prefversion = PrefGetAppPreferences(CR_ID, 0, &temp, &prefsize, true);

	if(prefversion == SPREFVERSION)
	{
		MemMove(&prefs, &temp.prefs, sizeof(prisprefs));
	}
	else if(prefversion <= 3)
	{
		UInt8 i;

		if(prefversion <= 1)
		{
			MemMove(&prefs.settings, &temp.prefsV12.settings,
				sizeof(UInt16) + sizeof(char)*32);
			MemMove(prefs.settings.keys, &temp.prefsV12.settings.keys,
				sizeof(UInt8)*5);
			prefs.settings.keys[key_pause] = 5;
			MemMove(&prefs.settings.flags, &temp.prefsV12.settings.flags,
				sizeof(UInt8)*3);
			prefs.settings.pencontrol = pc_movement;
			MemMove(&prefs.data, &temp.prefsV12.data,
				sizeof(boarddata));

			if(!prefversion)
			{
				prefs.settings.difficulty = diff_normal;
			}
		}
		else
		{
			MemMove(&prefs, &temp.prefs, sizeof(prisprefs));
		}

		if(prefversion <= 2)
		{
			for(i = 0; i < 6; i++)
			{
				prefs.settings.keys[i] = (prefs.settings.keys[i]+1)%7;
			}
		}

		prefs.settings.moreflags = 0;
	}
	else
	{
		UInt8 i;

		for(i = 0; i < NHIGHSCORES; i++)
		{
			prefs.data.highscores[i].score = 100*(20-i);
			prefs.data.highscores[i].finished = false;
			StrCopy(prefs.data.highscores[i].name, "----------------");
		}

#ifdef CHEAT
		prefs.data.topboard		= 59;
#else
		prefs.data.topboard 	= 0;
#endif
		prefs.data.startboard 	= 0;

		prefs.settings.flags				= set_autosave | set_background;
		prefs.settings.volume				= vol_sys;
		prefs.settings.difficulty			= diff_normal;
		prefs.settings.pencontrol			= pc_movement;
		prefs.settings.keys[key_fastleft]	= 1;
		prefs.settings.keys[key_left]		= 2;
		prefs.settings.keys[key_right]		= 3;
		prefs.settings.keys[key_fastright]	= 4;
		prefs.settings.keys[key_action]		= 5;
		prefs.settings.keys[key_pause] 		= 6;
		prefs.settings.moreflags			= 0;
	}

	//if using an external board set, make sure it still exists
	if(prefs.settings.flags & set_extboards)
	{
		if(!DmFindDatabase(prefs.settings.boardsetcard,
		prefs.settings.boardsetname))
		{
			prefs.settings.flags &= ~set_extboards;
		}
	}

	return;
}


void killgamevars(void)
{
	UInt8 i;
	brickanim* baptr=gv.bahead;
	brickanim tempbrickanim;
	UInt16 basize=sizeof(brickanim);

	//gv contains recently loaded vars...
	PrefSetAppPreferences(CR_ID, PREFGAMEVARS,
		UPREFVERSION, NULL, 0, false);
	for(i=0; i<gv.nguys; i++)
	{
		PrefSetAppPreferences(CR_ID, i+PREFGUYSTART,
			UPREFVERSION, NULL, 0, false);
	}
	for(i=0; i<gv.nboni; i++)
	{
		PrefSetAppPreferences(CR_ID, i+PREFBONISTART,
			UPREFVERSION, NULL, 0, false);
	}
	i=PREFBRICKANIMSTART;
	while(baptr!=NULL)
	{
		PrefGetAppPreferences(CR_ID, i, &tempbrickanim, &basize, false);
		baptr=tempbrickanim.next;
		PrefSetAppPreferences(CR_ID, i++, UPREFVERSION, NULL, 0, false);
	}

	return;
}


void storegamevars(void)
{
	if(((gv.gamestate&INPLAYMASK) >= PLAY)
	&& ((prefs.settings.flags&set_autosave)))
	{
		UInt8 i;
		brickanim* baptr=gv.bahead;

		gv.gamestate&=INPLAYMASK;
		PrefSetAppPreferences(CR_ID, PREFGAMEVARS, UPREFVERSION,
			&gv, sizeof(gamevars), false);
		for(i=0; i<gv.nguys; i++)
		{
			PrefSetAppPreferences(CR_ID, i+PREFGUYSTART,
				UPREFVERSION, gv.guylist[i],
				sizeof(guytype), false);
		}
		for(i=0; i<gv.nboni; i++)
		{
			PrefSetAppPreferences(CR_ID, i+PREFBONISTART,
				UPREFVERSION, gv.bonuslist[i],
				sizeof(bonus), false);
		}
		i=PREFBRICKANIMSTART;
		while(baptr!=NULL)
		{
			PrefSetAppPreferences(CR_ID, i++, UPREFVERSION, baptr,
				sizeof(brickanim), false);
			baptr=baptr->next;
		}
	}
	return;
}


UInt16 loadgamevars(void)
{
	UInt16 err;
	UInt8 i;

	UInt16 size=sizeof(gamevars);
	UInt16 version=PrefGetAppPreferences(CR_ID, PREFGAMEVARS, &gv, &size, false);

	gv.paddle.pic=NULL;
	gv.paddle.savebehind=NULL;
	gv.paddle.mask=NULL;
	resizepaddle(gv.paddle.width);

	for(i=0; i<gv.nguys; i++)
	{
		gv.guylist[i]=MemPtrNew(sizeof(guytype));
		ErrFatalDisplayIf(!gv.guylist[i], "Error reconstructing guy list, chief.");
		size=sizeof(guytype);
		PrefGetAppPreferences(CR_ID, i+PREFGUYSTART, gv.guylist[i],
			&size, false);
		gv.guylist[i]->savebehind=WinCreateOffscreenWindow(GUYWIDTH,
			GUYHEIGHT, screenFormat, &err);
		ErrFatalDisplayIf(err, "Error reconstructing guy background window, chief.");
	}

	for(i=0; i<gv.nboni; i++)
	{
		gv.bonuslist[i]=MemPtrNew(sizeof(bonus));
		ErrFatalDisplayIf(!gv.bonuslist[i], "Error reconstructing bonus list, chief.");
		size=sizeof(bonus);
		PrefGetAppPreferences(CR_ID, i+PREFBONISTART, gv.bonuslist[i],
			&size, false);
		gv.bonuslist[i]->savebehind=WinCreateOffscreenWindow(BONUSWIDTH,
			BONUSHEIGHT, screenFormat, &err);
		ErrFatalDisplayIf(err, "Error reconstructing bonus background window, chief.");
	}

	if((gv.bahead))
	{
		brickanim* baptr;
		i=PREFBRICKANIMSTART;

		gv.bahead=MemPtrNew(sizeof(brickanim));
		ErrFatalDisplayIf(!gv.bahead, "Error reconstructing brick anim head, chief.");
		size=sizeof(brickanim);
		PrefGetAppPreferences(CR_ID, i, gv.bahead, &size, false);

		baptr=gv.bahead;
		while((baptr->next))
		{
			baptr->next=MemPtrNew(sizeof(brickanim));
			ErrFatalDisplayIf(!baptr->next,
				"Error reconstructing brick anim list, chief.");
			size=sizeof(brickanim);
			PrefGetAppPreferences(CR_ID, ++i, baptr->next, &size, false);
			baptr=baptr->next;
		}
	}

	killgamevars();
	gv.gamestate|=RESTORE;
	return version;
}