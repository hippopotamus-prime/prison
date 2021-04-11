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

/****************************************************************************
** Updates
**
**      March 12, 2006 - Removed CtlNewControl calls from settingshandler
** as they often failed on devices with SD cards.  The controls in question
** are now defined in the resource file, but enabled/disabled/moved at
** run-time.
****************************************************************************/

#include <PalmOS.h>
#include <DLServer.h>
#include "handlers.h"
#include "game.h"
#include "rc.h"
#include "sound.h"
#include "prefs.h"
#include "guys.h"

#define MAXVISBOARDS 5

static UInt8 tempkeys[8];

static void updatestartboard(FormPtr formptr, const UInt16 value, const Boolean random);
static void updatekeysettings(FormPtr formptr, ListPtr keylistptr);
static void wipeboardlist(char** listitems, ListPtr boardlistptr);
static void updatenboards(const Boolean external, const char* dbname, const UInt16 card);


Boolean nohandler(EventPtr eventptr)
{
	return false;
}


void updatekeysettings(FormPtr formptr, ListPtr keylistptr)
{
	ControlType* keypopupptr;
	UInt8 i;

	for(i = 0; i < 6; i++)
	{
		keypopupptr = FrmGetObjectPtr(formptr,
			FrmGetObjectIndex(formptr, popup_rightkey+i));

		CtlSetLabel(keypopupptr, LstGetSelectionText(keylistptr,
			tempkeys[i]));
	}
}


Boolean keyshandler(EventPtr eventptr)
{
	Boolean handled = false;
	FormPtr formptr = FrmGetActiveForm();
	ListPtr keylistptr;
	ControlType* keventsptr = FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, checkbox_kevents));

	if(deviceflags & df_5way)
	{
		keylistptr = FrmGetObjectPtr(formptr,
			FrmGetObjectIndex(formptr, list_altkeys));
	}
	else
	{
		keylistptr = FrmGetObjectPtr(formptr,
			FrmGetObjectIndex(formptr, list_keys));
	}

	if(eventptr->eType == frmOpenEvent)
	{
		MemMove(tempkeys, prefs.settings.keys, 8);
		FrmDrawForm(formptr);

		updatekeysettings(formptr, keylistptr);

		//the allow key events checkbox
		CtlSetValue(keventsptr, (prefs.settings.moreflags>>mset_kevents_bit)&1);
		CtlShowControl(keventsptr);

		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		if((eventptr->data.ctlSelect.controlID >= popup_rightkey)
		&& (eventptr->data.ctlSelect.controlID <= popup_pausekey))
		{
			Int16 selection;
			UInt8 keyoffset = eventptr->data.ctlSelect.controlID - popup_rightkey;

			LstSetSelection(keylistptr, tempkeys[keyoffset]);
			selection = LstPopupList(keylistptr);
			if(selection != -1)
			{
				tempkeys[keyoffset] = selection;
			}

			updatekeysettings(formptr, keylistptr);
			handled = true;
		}
		else
		{
			switch(eventptr->data.ctlSelect.controlID)
			{
				case button_keysdefaults:
					tempkeys[key_fastleft] 	= 1;
					tempkeys[key_left] 		= 2;
					tempkeys[key_right] 	= 3;
					tempkeys[key_fastright] = 4;
					tempkeys[key_action] 	= 5;
					tempkeys[key_pause]		= 6;
					updatekeysettings(formptr, keylistptr);
					CtlSetValue(keventsptr, 0);
					handled = true;
					break;

				case button_keysok:
					prefs.settings.moreflags = 0;

					prefs.settings.moreflags |= CtlGetValue(keventsptr)<<mset_kevents_bit;

					MemMove(prefs.settings.keys, tempkeys, 8);
					setkeymask();

				case button_keyscancel:
					handled = true;
					FrmReturnToForm(0);
			}
		}
	}

	return handled;
}


Boolean settingshandler(EventPtr eventptr)
{
    Boolean handled = false;
    FormPtr formptr = FrmGetActiveForm();

    ControlType* autosaveptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, checkbox_autosave));
    ControlType* rconfptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, checkbox_rconf));
    ControlType* datehsptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, checkbox_datehs));
    ControlType* backgroundptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, checkbox_background));
    ControlType* compatmodeptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, checkbox_compat));
    ControlType* grafptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, checkbox_graf));

    ControlType* volumeptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, popup_volume));
    ControlType* diffsptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, popup_diffs));
    ControlType* penptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, popup_pen));

    ListPtr volumelistptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, list_volume));
    ListPtr diffslistptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, list_diffs));
    ListPtr penlistptr = FrmGetObjectPtr(formptr,
        FrmGetObjectIndex(formptr, list_pen));

	if(eventptr->eType == frmOpenEvent)
	{
        //get base coordinates for dynamically placed controls
        Coord dyn_cb_x;
        Coord dyn_cb_y;
        FrmGetObjectPosition(formptr,
            FrmGetObjectIndex(formptr, checkbox_background),
            &dyn_cb_x, &dyn_cb_y);

		//initialize the volume popup
		LstSetSelection(volumelistptr, prefs.settings.volume);
		CtlSetLabel(volumeptr, LstGetSelectionText(volumelistptr,
			prefs.settings.volume));

		//and the difficulty popup
		LstSetSelection(diffslistptr, prefs.settings.difficulty);
		CtlSetLabel(diffsptr, LstGetSelectionText(diffslistptr,
			prefs.settings.difficulty));

		//the pen control popup...
		LstSetSelection(penlistptr, prefs.settings.pencontrol);
		CtlSetLabel(penptr, LstGetSelectionText(penlistptr,
			prefs.settings.pencontrol));

		//the autosave checkbox...
		CtlSetValue(autosaveptr, (prefs.settings.flags >> set_autosave_bit) & 1);

		//the confirm retire checkbox...
		CtlSetValue(rconfptr, (prefs.settings.flags >> set_rconf_bit) & 1);

		//the date high scores checkbox...
		CtlSetValue(datehsptr, (prefs.settings.flags >> set_datehs_bit) & 1);

		//the background checkbox...
		if(deviceflags & df_color)
		{
            CtlSetUsable(backgroundptr, true);
			CtlSetValue(backgroundptr, (prefs.settings.flags >> set_background_bit) & 1);
            dyn_cb_y += 12;
		}

		//the compatibility mode checkbox...
		if(!(deviceflags & df_apionly))
		{
            CtlSetUsable(compatmodeptr, true);
			CtlSetValue(compatmodeptr, (prefs.settings.flags >> set_api_bit) & 1);
            FrmSetObjectPosition(formptr,
                FrmGetObjectIndex(formptr, checkbox_compat),
                dyn_cb_x, dyn_cb_y);
            dyn_cb_y += 12;
        }

		//and the grafitti control checkbox
		if((grinfo.right != NOGRAF)
		&& (grinfo.left != NOGRAF))
		{
            CtlSetUsable(grafptr, true);
			CtlSetValue(grafptr, (prefs.settings.flags>>set_graf_bit)&1);
            FrmSetObjectPosition(formptr,
                FrmGetObjectIndex(formptr, checkbox_graf),
                dyn_cb_x, dyn_cb_y);
		}

		FrmDrawForm(formptr);	
		handled = true;
	}
	else if(eventptr->eType == popSelectEvent)
	{
		//set the popup labels when something is selected...
		switch(eventptr->data.ctlSelect.controlID)
		{
			case popup_volume:
				CtlSetLabel(volumeptr, LstGetSelectionText(volumelistptr,
					LstGetSelection(volumelistptr)));
				handled = true;
				break;
			case popup_diffs:
				CtlSetLabel(diffsptr, LstGetSelectionText(diffslistptr,
					LstGetSelection(diffslistptr)));
				handled = true;
				break;
			case popup_pen:
				CtlSetLabel(penptr, LstGetSelectionText(penlistptr,
					LstGetSelection(penlistptr)));
				handled = true;
		}
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case button_settingskeys:
				FrmPopupForm(form_keys);
				handled = true;
				break;
			case button_settingsok:
			{
				//need to preserve the settings from boards form, clear all others
				prefs.settings.flags &= (set_extboards | set_random);

				if(deviceflags&df_color)
				{
					prefs.settings.flags |= CtlGetValue(backgroundptr) << set_background_bit;

					if(gv.gamestate != NOPLAY)
					{
						if(prefs.settings.flags & set_background)
						{
							drawbgtiles();
						}
						else
						{
							WinHandle oldwin = WinGetDrawWindow();
							WinSetDrawWindow(screenbuffer);
							WinEraseWindow();
							WinSetDrawWindow(oldwin);
						}
						drawbrickarray();
						drawborder();
					}
				}

				if(!(deviceflags&df_apionly))
				{
					prefs.settings.flags |= CtlGetValue(compatmodeptr) << set_api_bit;
				}

				if((grinfo.right!=NOGRAF)
				&& (grinfo.left!=NOGRAF))
				{
					prefs.settings.flags |= CtlGetValue(grafptr) << set_graf_bit;
				}

				prefs.settings.flags |= CtlGetValue(autosaveptr) << set_autosave_bit;
				prefs.settings.flags |= CtlGetValue(rconfptr) << set_rconf_bit;
				prefs.settings.flags |= CtlGetValue(datehsptr) << set_datehs_bit;
				prefs.settings.volume = LstGetSelection(volumelistptr);
				prefs.settings.difficulty = LstGetSelection(diffslistptr);
				prefs.settings.pencontrol = LstGetSelection(penlistptr);
				setvolume();
			}
			case button_settingscancel:
				handled = true;
				FrmReturnToForm(0);
				gv.gamestate &= ~POPUP;
		}
	}

	return handled;
}


Boolean bonusinfohandler(EventPtr eventptr)
{
	if(eventptr->eType==frmOpenEvent)
	{
		UInt8 i;
		RectangleType lamerect;
		FormPtr formptr=FrmGetActiveForm();

		FrmDrawForm(formptr);

		RctSetRectangle(&lamerect, 0, 0, BONUSWIDTH-2, BONUSHEIGHT-2);
		for(i = 0; i < NBONITYPES; i++)
		{
			WinCopyRectangle(bonuspics[NBONIFRAMES+i],
				FrmGetWindowHandle(formptr),
				&lamerect, (i&1)?134:6, 18+i*12, winPaint);
		}
		return true;
	}
	else if(eventptr->eType==ctlSelectEvent)
	{
		if(eventptr->data.ctlSelect.controlID == button_bonibricks)
		{
			FrmGotoForm(form_brickinfo);
		}
		else
		{
			FrmEraseForm(FrmGetActiveForm());
			FrmReturnToForm(0);
			gv.gamestate &= ~POPUP;
			return true;
		}
	}
	return false;
}


Boolean infohandler(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		if(eventptr->data.ctlSelect.controlID == button_brickmore)
		{
			FrmGotoForm(form_brickinfo2);
		}
		else if(eventptr->data.ctlSelect.controlID == button_brickmore2)
		{
			FrmGotoForm(form_brickinfo);
		}
		else if(eventptr->data.ctlSelect.controlID == button_brickboni)
		{
			FrmGotoForm(form_bonusinfo);
		}
		else
		{
			FrmEraseForm(FrmGetActiveForm());
			FrmReturnToForm(0);
			gv.gamestate &= ~POPUP;
			handled = true;
		}
	}

	return handled;
}


Boolean pausehandler(EventPtr eventptr)
{
	FormPtr formptr = FrmGetActiveForm();
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		char boardtext[12] = "Board ";
		RectangleType textrect;
		const UInt16 textindex = FrmGetObjectIndex(formptr, label_pauseboard);

		boardtostr(&boardtext[6]);

		FrmCopyLabel(formptr, label_pauseboard, boardtext);
		FrmGetObjectBounds(formptr, textindex, &textrect);
		FrmSetObjectPosition(formptr, textindex, (100 - textrect.extent.x)>>1,
			textrect.topLeft.y);
		FrmShowObject(formptr, textindex);

		FrmDrawForm(formptr);
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		FrmEraseForm(formptr);
		FrmReturnToForm(0);
		gv.gamestate &= ~PAUSE;
		handled = true;
	}
	else if(eventptr->eType == menuEvent)
	{
		switch(eventptr->data.menu.itemID)
		{
			case menuitem_convert:
				convert_life();
				break;
			case menuitem_suicide:
				if((!(prefs.settings.flags & set_rconf))
				|| (!FrmAlert(alert_suiconf)))
				{
					killallguys(STATUS_GO);
				}
				break;
			case menuitem_retire:
				if((!(prefs.settings.flags & set_rconf))
				|| (!FrmAlert(alert_rconf)))
				{
					killallguys(STATUS_ALL);
					killallboni();
					removeallbrickanims();
					FrmReturnToForm(0);
					gv.gamestate = RETURN;
				}
				break;
			case menuitem_resume:
				FrmReturnToForm(0);
				gv.gamestate &= ~PAUSE;
				break;
			case menuitem_settings:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_settings);
				break;
			case menuitem_bonusinfo:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_bonusinfo);
				break;
			case menuitem_brickinfo:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_brickinfo);
				break;
			case menuitem_about:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_about);
				break;
			case menuitem_quit:
			{
				EventType quitevent;

				quitevent.eType = appStopEvent;
				EvtAddEventToQueue(&quitevent);
			}
		}

		handled = true;
	}

	return handled;
}


Boolean restorehandler(EventPtr eventptr)
{
	if(eventptr->eType==frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		switch(loadgamevars())
		{
			//UPREFVERSION (i.e. the current version) doesn't need anything done
			case 0:
				gv.difficulty = diff_normal;
			case 1:
			{
				UInt8 i;
		
				for(i = 0; i < gv.nguys; i++)
				{
					//status codes were changed from 0,1,2 to 1,2,4
					gv.guylist[i]->status = 1 << gv.guylist[i]->status;
				}
				break;
			}
		}
		FrmGotoForm(form_main);
		return true;
	}
	return false;
}


Boolean gameoverhighhandler(EventPtr eventptr)
{
	FormPtr formptr=FrmGetActiveForm();
	UInt16 fieldindex=FrmGetObjectIndex(formptr, field_highscore);
	FieldType* fieldptr=FrmGetObjectPtr(formptr, fieldindex);

	if(eventptr->eType == frmOpenEvent)
	{
		#if (dlkUserNameBufSize > MAXNAMELEN+1)
			MemHandle namehand = MemHandleNew(dlkUserNameBufSize);
		#else
			MemHandle namehand = MemHandleNew(MAXNAMELEN+1);
		#endif

		char* nameptr = MemHandleLock(namehand);

		if(DlkGetSyncInfo(NULL, NULL, NULL, nameptr, NULL, NULL) != errNone)
		{
			StrCopy(nameptr, "");
		}
		else
		{
			char* spaceptr = StrChr(nameptr, ' ');
			if(spaceptr)
			{
				*spaceptr = '\0';
			}
			nameptr[MAXNAMELEN] = '\0';	

			if(prefs.settings.flags & set_datehs)
			{
				DateType date;
				Char datestring[dateStringLength];

				DateSecondsToDate(TimGetSeconds(), &date);
				DateToAscii(date.month, date.day, (date.year+4)%100,
					PrefGetPreference(prefDateFormat), datestring);

				if(StrLen(datestring)+StrLen(nameptr) <= MAXNAMELEN-2)
				{
					StrCat(nameptr, ", ");
					StrCat(nameptr, datestring);
				}
			}
		}

		MemPtrUnlock(nameptr);
		FldSetTextHandle(fieldptr, namehand);

		FrmSetFocus(formptr, fieldindex);
		FrmDrawForm(formptr);
		return true;
	}
	else if(eventptr->eType==ctlSelectEvent)
	{
		if(eventptr->data.ctlSelect.controlID==button_highscoreok)
		{
			char* nameptr=FldGetTextPtr(fieldptr);
			if((nameptr))
			{
				highscore newhighscore;

				StrCopy(newhighscore.name, nameptr);
				newhighscore.score=gv.score;
				newhighscore.finished=gv.finishflag;
				inserthighscore(checkhighscore(), &newhighscore);
			}
		}
		FrmReturnToForm(0);
		gv.gamestate=RETURN;
		return true;
	}

	return false;
}


Boolean gameoverhandler(EventPtr eventptr)
{
	if(eventptr->eType==frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		return true;
	}
	else if(eventptr->eType==penDownEvent)
	{
		FrmReturnToForm(0);
		gv.gamestate=RETURN;
		return true;
	}
	return false;
}


Boolean highscoreshandler(EventPtr eventptr)
{
	if(eventptr->eType==frmOpenEvent)
	{
		UInt8 i;
		char scoretext[11];
		DmOpenRef dbref;
		boarddata* dataptr;

		FrmDrawForm(FrmGetActiveForm());

		openboarddataprefs(&dbref, &dataptr);

		for(i=0; i<NHIGHSCORES; i++)
		{
			WinDrawChars(dataptr->highscores[i].name,
				StrLen(dataptr->highscores[i].name), 4, 16+i*12);
			StrIToA(scoretext, dataptr->highscores[i].score);
			WinDrawChars(scoretext, StrLen(scoretext), 112, 16+i*12);
			if((dataptr->highscores[i].finished))
			{
				WinDrawChars("*", 1, 150, 16+i*12);
			}
		}

		closeboarddata(dbref, dataptr, false);
		return true;
	}
	else if(eventptr->eType==penDownEvent)
	{
		FrmGotoForm(form_title);
		return true;
	}
	return false;
}


Boolean titlehandler(EventPtr eventptr)
{
	if(eventptr->eType==frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		return true;
	}
	else if(eventptr->eType==ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case button_titlestart:
				FrmGotoForm(form_main);
				break;
			case button_titlehs:
				FrmEraseForm(FrmGetActiveForm());
				FrmGotoForm(form_highscores);
				break;
			case button_titlesettings:
				FrmPopupForm(form_settings);
				break;
			case button_titleboards:
				FrmGotoForm(form_boards);
		}
		return true;
	}
	else if(eventptr->eType==menuEvent)
	{
		switch(eventptr->data.menu.itemID)
		{
			case menuitem_start:
				FrmGotoForm(form_main);
				break;
			case menuitem_scores:
				FrmEraseForm(FrmGetActiveForm());
				FrmGotoForm(form_highscores);
				break;
			case menuitem_settings:
				FrmPopupForm(form_settings);
				break;
			case menuitem_boards:
				FrmGotoForm(form_boards);
				break;
			case menuitem_bonusinfo:
				gv.gamestate |= POPUP;				//signals the animation
				FrmPopupForm(form_bonusinfo);
				break;
			case menuitem_brickinfo:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_brickinfo);
				break;
			case menuitem_about:
				FrmPopupForm(form_about);
				break;
			case menuitem_quit:
			{
				EventType quitevent;

				quitevent.eType = appStopEvent;
				EvtAddEventToQueue(&quitevent);
			}
		}
	}
	return false;
}


Boolean mainhandler(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());

		if(gv.gamestate&RESTORE)
		{
			gv.gamestate &= ~RESTORE;
			gv.gamestate |= PAUSE;
			FrmPopupForm(form_pause);
		}
		else
		{
			resetgamevars();
			loadboard();
			gv.gamestate = ENTER;
		}

		drawboard();		//calls drawsides
		updategamegfx();
		gametimefornext = TimGetTicks()+timeinterval;
		handled = true;
	}

	else if(eventptr->eType == frmUpdateEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		drawsides();
		handled = true;
	}

	else if((eventptr->eType == penUpEvent)
	&& (pentarget == PENACTION))
	{
		pentarget = NOTARGET;
		handled = true;
	}

	else if(eventptr->eType == menuEvent)
	{
		switch(eventptr->data.menu.itemID)
		{
			case menuitem_convert:
				convert_life();
				break;
			case menuitem_suicide:
				if((!(prefs.settings.flags & set_rconf))
				|| (!FrmAlert(alert_suiconf)))
				{
					killallguys(STATUS_GO);
				}
				break;
			case menuitem_retire:
				if((!(prefs.settings.flags & set_rconf))
				|| (!FrmAlert(alert_rconf)))
				{
					killallguys(STATUS_ALL);
					killallboni();
					removeallbrickanims();
					gv.gamestate = RETURN;
				}
				break;
			case menuitem_pause:
				gv.gamestate |= PAUSE;
				FrmPopupForm(form_pause);
				break;
			case menuitem_settings:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_settings);
				break;
			case menuitem_bonusinfo:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_bonusinfo);
				break;
			case menuitem_brickinfo:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_brickinfo);
				break;
			case menuitem_about:
				gv.gamestate |= POPUP;
				FrmPopupForm(form_about);
				break;
			case menuitem_quit:
			{
				EventType quitevent;

				quitevent.eType = appStopEvent;
				EvtAddEventToQueue(&quitevent);
			}
		}
		handled = true;
	}

	return handled;
}


void updatenboards(const Boolean external, const char* dbname, const UInt16 card)
{
	FormPtr formptr = FrmGetActiveForm();
	UInt16 index = FrmGetObjectIndex(formptr, label_nboards);
	char nbtext[6] = "60";		//ok, this is kind of cheating

	if(external)
	{
		LocalID		lid			= DmFindDatabase(card, dbname);
		DmOpenRef	dbref		= DmOpenDatabase(card, lid, dmModeReadOnly);
		MemHandle	boardhandle	= DmGetRecord(dbref, 1);
		boardset*	boardsetptr	= (boardset*)MemHandleLock(boardhandle);

		StrIToA(nbtext, boardsetptr->nboards);

		MemHandleUnlock(boardhandle);
		DmReleaseRecord(dbref, 1, false);
		DmCloseDatabase(dbref);
	}

	FrmHideObject(formptr, index);
	FrmCopyLabel(formptr, label_nboards, nbtext);
	FrmShowObject(formptr, index);
}


void updatestartboard(FormPtr formptr, const UInt16 value, const Boolean random)
{
	char startboardtext[6];

	if(random)
	{
		StrCopy(startboardtext, "???");
	}
	else
	{
		StrIToA(startboardtext, value);
	}

	FrmHideObject(formptr, FrmGetObjectIndex(formptr, label_startboard));
	FrmSetObjectPosition(formptr, FrmGetObjectIndex(formptr, label_startboard),
		STARTBOARDX - (FntCharsWidth(startboardtext, StrLen(startboardtext))>>1),
		STARTBOARDY);
	FrmCopyLabel(formptr, label_startboard, startboardtext);
	FrmShowObject(formptr, FrmGetObjectIndex(formptr, label_startboard));
}


void wipeboardlist(char** listitems, ListPtr boardlistptr)
{
	UInt8 i;
	Int16 nboardsets = LstGetNumberOfItems(boardlistptr);

	for(i = 1; i < nboardsets; i++)
	{
		if(listitems[i])
		{
			MemPtrFree(listitems[i]);
			listitems[i] = NULL;
		}
	}
}


Boolean boardshandler(EventPtr eventptr)
{
	FormPtr formptr = FrmGetActiveForm();
	ControlType* randomptr = FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, checkbox_random));
	ListPtr boardlistptr = FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, list_boards));
	static UInt16* dbcards = NULL;
	static char** listitems = NULL;
	Boolean handled = false;

	switch(eventptr->eType)
	{
	case frmOpenEvent:
		dbcards = (UInt16*)MemPtrNew(MAXBOARDSETS*sizeof(UInt16));
		listitems = (char**)MemPtrNew(MAXBOARDSETS*sizeof(char**));
	case frmUpdateEvent:
	{
		DmSearchStateType searchstate;
		UInt16 card;
		LocalID lid;
		Boolean newsearch=true;
		UInt8 i=1;
		Int16 selection=0;

		wipeboardlist(listitems, boardlistptr);

		listitems[0]=LstGetSelectionText(boardlistptr, 0);

		while((DmGetNextDatabaseByTypeCreator(newsearch, &searchstate,
		'Pbrd', CR_ID, false, &card, &lid) == errNone)
		&& (i < MAXBOARDSETS))
		{
			UInt16 version;

			DmDatabaseInfo(card, lid, NULL, NULL, &version, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL);

			if(version <= BOARDVERSION)
			{
				listitems[i] = (char*)MemPtrNew(32);
				ErrFatalDisplayIf(!listitems[i], "Error creating board list.");

				DmDatabaseInfo(card, lid, listitems[i], NULL, NULL, NULL,
					NULL, NULL, NULL, NULL, NULL, NULL, NULL);

				if((prefs.settings.flags & set_extboards)
				&& (!StrCompare(listitems[i], prefs.settings.boardsetname)))
				{
					selection = i;
				}

				dbcards[i++] = card;
			}

			newsearch = false;
		}

		LstSetListChoices(boardlistptr, listitems, i);
		if(i>MAXVISBOARDS)
		{
			LstSetHeight(boardlistptr, MAXVISBOARDS);
		}
		else
		{
			LstSetHeight(boardlistptr, i);
		}
		LstSetSelection(boardlistptr, selection);

		//the random order checkbox
		CtlSetValue(randomptr, (prefs.settings.flags>>set_random_bit)&1);
		CtlShowControl(randomptr);

		updatestartboard(formptr, getstartboard((Boolean)selection,
			listitems[selection], dbcards[selection]) + 1,
			CtlGetValue(randomptr));

		updatenboards((Boolean)selection, listitems[selection], dbcards[selection]);

		if((selection))
		{
			FrmShowObject(formptr,
				FrmGetObjectIndex(formptr, button_boardsdelete));
		}
		else
		{
			FrmHideObject(formptr,
				FrmGetObjectIndex(formptr, button_boardsdelete));
		}

		FrmDrawForm(formptr);
		handled = true;
		break;
	}
	case lstSelectEvent:
	{
		updatestartboard(formptr,
			getstartboard((Boolean)eventptr->data.lstSelect.selection,
			listitems[eventptr->data.lstSelect.selection],
			dbcards[eventptr->data.lstSelect.selection]) + 1,
			CtlGetValue(randomptr));

		updatenboards((Boolean)eventptr->data.lstSelect.selection,
			listitems[eventptr->data.lstSelect.selection],
			dbcards[eventptr->data.lstSelect.selection]);

		if((eventptr->data.lstSelect.selection))
		{
			FrmShowObject(formptr,
				FrmGetObjectIndex(formptr, button_boardsdelete));
		}
		else
		{
			FrmHideObject(formptr,
				FrmGetObjectIndex(formptr, button_boardsdelete));
		}
		break;
	}
	case ctlSelectEvent:
	{
		Int16 boardset = LstGetSelection(boardlistptr);

		switch(eventptr->data.ctlSelect.controlID)
		{
			case checkbox_random:
				updatestartboard(formptr, getstartboard((Boolean)boardset,
					listitems[boardset], dbcards[boardset]) + 1,
					CtlGetValue(randomptr));
				break;
			case button_boardsdelete:
				if((boardset)
				&& (!FrmAlert(alert_boardsdelete)))
				{
					if(!StrCompare(listitems[boardset], prefs.settings.boardsetname))
					{
						prefs.settings.flags &= ~set_extboards;
					}
					DmDeleteDatabase(dbcards[boardset],
						DmFindDatabase(dbcards[boardset],
						listitems[boardset]));
					wipeboardlist(listitems, boardlistptr);
					FrmEraseForm(formptr);
					FrmUpdateForm(form_boards, 0);
				}
				handled = true;
				break;
			case button_boardsplus:
			{
				if(!CtlGetValue(randomptr))
				{
					UInt16 startboard = StrAToI(FrmGetLabel(formptr, label_startboard));
					UInt16 topboard = gettopboard((Boolean)boardset,
						listitems[boardset], dbcards[boardset])+1;
					updatestartboard(formptr, startboard%topboard + 1, false);
				}
				handled = true;
				break;
			}
			case button_boardsminus:
			{
				if(!CtlGetValue(randomptr))
				{
					UInt16 startboard = StrAToI(FrmGetLabel(formptr, label_startboard))-1;
					if(!startboard)
					{
						startboard=gettopboard((Boolean)boardset,
							listitems[boardset], dbcards[boardset])+1;
					}
					updatestartboard(formptr, startboard, false);
				}
				handled = true;
				break;
			}
			case button_boardsok:
			{
				if(CtlGetValue(randomptr))
				{
					setstartboard((Boolean)boardset, listitems[boardset],
						dbcards[boardset], 0);
					prefs.settings.flags |= set_random;
				}
				else
				{
					setstartboard((Boolean)boardset, listitems[boardset],
						dbcards[boardset], StrAToI(FrmGetLabel(formptr, label_startboard))-1);
					prefs.settings.flags &= ~set_random;
				}

				if((boardset))
				{
					prefs.settings.boardsetcard = dbcards[boardset];
					prefs.settings.flags |= set_extboards;
					StrCopy(prefs.settings.boardsetname, listitems[boardset]);
				}
				else
				{
					prefs.settings.flags &= ~set_extboards;
				}
			}
			case button_boardscancel:
			{
				FrmGotoForm(form_title);
				handled = true;
			}
		}
		break;
	}
	case frmCloseEvent:
	{
		wipeboardlist(listitems, boardlistptr);
		MemPtrFree(dbcards);
		MemPtrFree(listitems);
	}
	default:
	}
	return handled;
}