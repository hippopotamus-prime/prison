/****************************************************************************
** The Prison
** Copyright 2004 Aaron Curtis
** 
** This file is part of The Prison.
** 
** The Prison is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** The Prison is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with The Prison; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****************************************************************************/

#include <PalmOS.h>
#include <PalmNavigator.h>
#include <SonyCLIE.h>
#include <HsCommon.h>
#include "rc.h"
#include "guys.h"
#include "boni.h"
#include "sound.h"
#include "bricks.h"
#include "game.h"
#include "highscores.h"
#include "handlers.h"
#include "prefs.h"

#define ROMVERSION2 sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0)
#define ROMVERSION3 sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0)
#define ROMVERSION35 sysMakeROMVersion(3, 5, 0, sysROMStageRelease, 0)

#define FPS 30
#define GAMEUPDATE 1
#define SOUNDUPDATE 2

#define GRAFRIGHTBUTTON vchrFind	//vchrKeyboard... was better, but present on fewer devices
#define GRAFLEFTBUTTON vchrMenu

//functions
static UInt16 startapp(void);
static void stopapp(void);
static void eventhandler(void);
static void postprocess(EventPtr eventptr);
static Boolean preprocess(EventPtr eventptr);
static Boolean appdoevent(EventPtr eventptr);
static UInt16 loadbmps(void);
static void freebmps(void);
static Int32 timeleft(UInt8* flags);
static UInt16 checkdevicestate(void);
static void initgraf(void);
static Boolean dopen(Coord x, Coord y);

//globals
Int32 gametimefornext;
UInt32 timeinterval;
static UInt8 gamestatebak=NOPLAY;


UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	UInt16 err;

	if(cmd==sysAppLaunchCmdNormalLaunch)
	{
		err=startapp();
		if(!err)
		{
			eventhandler();
		}
		stopapp();
		if((err))
		{
			FrmAlert(alert_error);
		}
	}

	return 0;
}


UInt16 startapp(void)
{
	UInt16 err;
	UInt16 gvsize=0;
	FormPtr loadformptr = FrmInitForm(form_loading);
	FrmSetActiveForm(loadformptr);
	FrmSetEventHandler(loadformptr, nohandler);
	FrmDrawForm(loadformptr);

	err=checkdevicestate();
	if((err)) return err;

	timeinterval=SysTicksPerSecond()/FPS;
	gv.gamestate=NOPLAY;
	setsound(NOSOUND);

	getprefs();

	initgraf();

	setkeymask();

	screenbuffer=WinCreateOffscreenWindow(SCREENWIDTH,
		TOPBUFFER+SCREENHEIGHT+BOTTOMBUFFER, screenFormat, &err);
	if((err)) return err;

	if(deviceflags&df_color)
	{
		bgwin=WinCreateOffscreenWindow(BGWIDTH, BGHEIGHT,
			screenFormat, &err);
		if(err) return err;
	}

	err = loadbmps();
	if((err)) return err;

	loadsounds();

	switch(PrefGetAppPreferences(CR_ID, PREFGAMEVARS, NULL, &gvsize, false))
	{
		default:					//unsupported versions...
		case noPreferenceFound:
			FrmGotoForm(form_title);
			break;
		case 0:
		case 1:
		case UPREFVERSION:
			FrmGotoForm(form_restoring);
			break;
	}

	FrmEraseForm(loadformptr);
	return 0;
}


UInt16 loadbmps(void)
{
	UInt8 i;
	MemHandle mh;
	BitmapPtr bmpptr;
	UInt16 err = 0;
	WinHandle oldwin = WinGetDrawWindow();
	IndexedColorType oldForeColor = 0;
    IndexedColorType oldBackColor = 0;

	//mask frames are stored in 1-bit format, this is necessary
	//to get them into 8-bit mode and still have bitwise AND, OR, etc work
    if(deviceflags & df_color)
    {
    	oldForeColor = WinSetForeColor(255);
	    oldBackColor = WinSetBackColor(0);
    }

	for(i = 0; i < GUYFRAMES; ++i)
	{
		mh = DmGetResource(bitmapRsc, guy_start+i);
		bmpptr = MemHandleLock(mh);
		if(!bmpptr) return -1;

		guyhandles[i] = WinCreateOffscreenWindow(GUYWIDTH, GUYHEIGHT, screenFormat, &err);
		if(err) return err;
		WinSetDrawWindow(guyhandles[i]);
		WinDrawBitmap(bmpptr, 0, 0);

		MemHandleUnlock(mh);
		DmReleaseResource(mh);
	}

	err = loadbrickpics();
	if(err) return err;

	err = loadbonipics();
	if(err) return err;

	gv.paddle.pic = NULL;
	gv.paddle.savebehind = NULL;

	WinSetDrawWindow(screenbuffer);
	WinEraseWindow();

	WinSetDrawWindow(oldwin);

    if(deviceflags & df_color)
    {
    	WinSetForeColor(oldForeColor);
	    WinSetBackColor(oldBackColor);
    }

	return 0;
}


void freebmps(void)
{
	UInt16 i;

	for(i=0; i<GUYFRAMES; i++)
	{
		if((guyhandles[i]))
		{
			WinDeleteWindow(guyhandles[i], false);
		}
	}
	freebrickpics();
	freebonipics();

	if((gv.paddle.pic)) WinDeleteWindow(gv.paddle.pic, false);
	if((gv.paddle.savebehind)) WinDeleteWindow(gv.paddle.savebehind, false);
	if((gv.paddle.mask)) WinDeleteWindow(gv.paddle.mask, false);

	return;
}


void stopapp(void)
{
	UInt32 prefchecksum=0;
	UInt8* prefptr=(UInt8*)(&prefs);
	UInt16 i;

	//	see if prefs were successfully loaded before storing them
	for(i=0; i<sizeof(prisprefs); i++)
	{
		prefchecksum+=prefptr[i];
	}
	if((prefchecksum))
	{
		PrefSetAppPreferences(CR_ID, 0, SPREFVERSION, &prefs, sizeof(prisprefs), true);
	}

	storegamevars();
	KeySetMask(keyBitsAll);
	EvtEnableGraffiti(true);
	killallguys(STATUS_ALL);
	removeallbrickanims();
	killallboni();
	freesounds();
	freebmps();
	if((screenbuffer)) WinDeleteWindow(screenbuffer, false);
	if((bgwin)) WinDeleteWindow(bgwin, false);
	FrmCloseAllForms();
	return;
}


void eventhandler(void)
{
	EventType event;
	UInt16 err;

	do
	{
	EvtGetEvent(&event, timeleft(NULL));

	if(!preprocess(&event))
	{
		if(!SysHandleEvent(&event))
		{
			if(!MenuHandleEvent(NULL, &event, &err))
			{
				if(!appdoevent(&event))
				{
					FrmDispatchEvent(&event);
				}
			}
		}
	}
	postprocess(&event);

	}while(event.eType!=appStopEvent);
}


Boolean preprocess(EventPtr eventptr)
{
	UInt8 flags;

	gamestatebak=gv.gamestate;

	if(eventptr->eType==winExitEvent)
	{
		if(eventptr->data.winExit.exitWindow==(WinHandle)FrmGetFirstForm())
		{
			// this means some window has come up over top the main one
			gv.gamestate|=INTERRUPT;
		}
	}

	else if(eventptr->eType==winEnterEvent)
	{
		if(eventptr->data.winEnter.enterWindow==(WinHandle)FrmGetFirstForm())
		{
			// this means everything is returning to normal...
			gv.gamestate&=~INTERRUPT;
		}
	}

	if(eventptr->eType == keyDownEvent)
	{
		if(!(eventptr->data.keyDown.modifiers & poweredOnKeyMask))
		{
			//I had hoped this would solve the Treo's power-on problem,
			//but there doesn't seem to be any way to reliably set
			//the mask again after the thing is turned back on...
			/***********************************************************
			if((eventptr->data.keyDown.chr == vchrHardPower)
			|| (eventptr->data.keyDown.chr == vchrAutoOff)
			|| (eventptr->data.keyDown.chr == vchrResumeSleep))
			{
				//normally if the device turns off, keys will generate
				//events with the poweredOnKeyMask to turn it back on,
				//even if they've been masked off with KeySetMask()....
				//but this doesn't happen with the Treo's REM Sleep!
				//so we need to turn the keys on manually...

				KeySetMask(keyBitsAll);
			}
			*************************************************************/

			if((eventptr->data.keyDown.chr == vcharlist[prefs.settings.keys[key_pause]])
			&& (!(eventptr->data.keyDown.modifiers & autoRepeatKeyMask))
			&& ((!codelist[prefs.settings.keys[key_pause]])
			|| (eventptr->data.keyDown.keyCode & codelist[prefs.settings.keys[key_pause]])))
			{
				FormPtr formptr = FrmGetActiveForm();

				if(FrmGetFormId(formptr) == form_pause)
				{
					CtlHitControl(FrmGetObjectPtr(formptr,
						FrmGetObjectIndex(formptr, button_pauseresume)));
				}
				else if((gv.gamestate != NOPLAY)
				&& (!(gv.gamestate & ~INPLAYMASK)))
				{
					gv.gamestate |= PAUSE;
					FrmPopupForm(form_pause);
					pentarget = NOTARGET;
				}		
	
				return true;
			}
			else if((TxtCharIsHardKey(eventptr->data.keyDown.modifiers, eventptr->data.keyDown.chr))
			&& (eventptr->data.keyDown.chr >= vchrHard1)
			&& (eventptr->data.keyDown.chr <= vchrHard4))
			{
				return true;
			}
			else if((eventptr->data.keyDown.chr >= vchrJogUp)
			&& (eventptr->data.keyDown.chr <= vchrJogRelease)
			&& (gv.gamestate == PLAY))
			{
				switch(eventptr->data.keyDown.chr)
				{
					case vchrJogPush:
						pentarget = PENACTION;
						break;

					case vchrJogPushedDown:
					case vchrJogDown:
						if(gv.paddle.x < SCREENWIDTH-gv.paddle.width-7)
						{
							pentarget = gv.paddle.x + 8;
						}
						else if(gv.paddle.x < SCREENWIDTH-gv.paddle.width)
						{
							pentarget = SCREENWIDTH - gv.paddle.width;
						}
						break;
	
					case vchrJogPushedUp:
					case vchrJogUp:
						if(gv.paddle.x > 7)
						{
							pentarget = gv.paddle.x - 8;
						}
						else if(gv.paddle.x > 0)
						{
							pentarget = 0;	
						}
						break;
				}
				return true;
			}
		}
	}

	else if(((eventptr->eType==penDownEvent) || (eventptr->eType==penMoveEvent))
	&& (gv.gamestate==PLAY))
	{
		return dopen(eventptr->screenX, eventptr->screenY);
	}
	else if((eventptr->eType == nilEvent)
	&& (gv.gamestate == PLAY))
	{
		//All this was added because OS 5 can (reportedly...) sometimes
		//send nilEvents in place of penMoveEvents....

		Int16 x, y;
		Boolean pendown;

		EvtGetPen(&x, &y, &pendown);

		if(pendown)
		{
			dopen(x, y);
		}
		else if(prefs.settings.pencontrol == pc_action)
		{
			pentarget = NOTARGET;
		}
	}

	if(!timeleft(&flags))
	{
		if(flags&GAMEUPDATE)
		{
			elapsestate();
			if(timeinterval==evtWaitForever)
			{
				gametimefornext=evtWaitForever;
			}
			else
			{
				gametimefornext=TimGetTicks()+timeinterval;
			}
		}

		if(flags&SOUNDUPDATE)
		{
			dosound();
			soundtimefornext=getnextbeeptime();
			gv.curbeep++;
		}
	}

	return false;
}


void postprocess(EventPtr eventptr)
{
	// check if the game state has changed
	if(gamestatebak!=gv.gamestate)
	{
		if((!(gv.gamestate&~INPLAYMASK))
		&& (gv.gamestate!=NOPLAY))
		{
			EvtEnableGraffiti(!(prefs.settings.flags&set_graf));
			timeinterval=SysTicksPerSecond()/FPS;
			gametimefornext=TimGetTicks()+timeinterval;
		}
		else if(gv.gamestate&POPUP)
		{
			//info screens need to be animated
			EvtEnableGraffiti(true);
			timeinterval=SysTicksPerSecond()/FPS;
			gametimefornext=TimGetTicks()+timeinterval;
		}
		else
		{
			EvtEnableGraffiti(true);
			timeinterval=evtWaitForever;
		}
	}
}


Boolean appdoevent(EventPtr eventptr)
{
	if(eventptr->eType==frmLoadEvent)
	{
		FormPtr formptr=FrmInitForm(eventptr->data.frmLoad.formID);
		FrmSetActiveForm(formptr);
		switch(eventptr->data.frmLoad.formID)
		{
			case form_title:
				FrmSetEventHandler(formptr, titlehandler);
				break;
			case form_main:
				FrmSetEventHandler(formptr, mainhandler);
				break;
			case form_highscores:
				FrmSetEventHandler(formptr, highscoreshandler);
				break;
			case form_gameover:
			case form_wingame:
				FrmSetEventHandler(formptr, gameoverhandler);
				break;
			case form_gameoverhigh:
			case form_wingamehigh:
				FrmSetEventHandler(formptr, gameoverhighhandler);
				break;
			case form_restoring:
				FrmSetEventHandler(formptr, restorehandler);
				break;
			case form_bonusinfo:
				FrmSetEventHandler(formptr, bonusinfohandler);
				break;
			case form_pause:
				FrmSetEventHandler(formptr, pausehandler);
				break;
			case form_brickinfo2:
			case form_brickinfo:
			case form_about:
				FrmSetEventHandler(formptr, infohandler);
				break;
			case form_settings:
				FrmSetEventHandler(formptr, settingshandler);
				break;
			case form_keys:
				FrmSetEventHandler(formptr, keyshandler);
				break;
			case form_boards:
				FrmSetEventHandler(formptr, boardshandler);
		}
		return true;
	}
	return false;
}


Int32 timeleft(UInt8* flagsptr)
{
	Int32 timeleft=evtWaitForever;

	if((flagsptr))
	{
		*flagsptr=0;
	}

	if(gametimefornext!=evtWaitForever)
	{
		timeleft=gametimefornext-TimGetTicks();
		if(timeleft<=0)
		{
			if((flagsptr))
			{
				*flagsptr|=GAMEUPDATE;
			}
			timeleft=0;
		}
	}

	if(soundtimefornext!=evtWaitForever)
	{
		Int32 soundtime=soundtimefornext-TimGetTicks();
		if(soundtime<=0)
		{
			if((flagsptr))
			{
				*flagsptr|=SOUNDUPDATE;
			}
			soundtime=0;
		}
		if(soundtime<(unsigned)timeleft)
		{
			timeleft=soundtime;
		}
	}

	return timeleft;
}


/****************************************************
Set up (or check) device-specific things...
screen size, bit depth, ROM version, etc.
*****************************************************/
UInt16 checkdevicestate(void)
{
	UInt32 featureval;
	UInt32 width;
	UInt32 height;
	UInt32 depth;
	Err err;
	UInt16 refnum;

	//check rom version
	err = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &featureval);
	if(err) return err;

	if(featureval < ROMVERSION35)
	{
		if(featureval < ROMVERSION3)
		{
			if(featureval < ROMVERSION2)
			{
				//apparently rom 1.0 was pretty lame...
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
			}
			return sysErrRomIncompatible;
		}
		deviceflags |= df_oldrom;
	}

	//check for the 5-way navigator deal
	err = FtrGet(navFtrCreator, navFtrVersion, &featureval);
	if(!err)
	{
		deviceflags |= df_5way;
	}
	else if((!FtrGet(sysFileCSystem, sysFtrNumUIHardwareFlags, &featureval))
	&& (featureval & sysFtrNumUIHardwareHas5Way))
	{
		//Handspring's lame alternate 5-way API.
		deviceflags |= df_5way;

		//up/down should use pageup/pagedown vchars since the main form is non-modal
		//hopefully the keyBit- values work similarly...?
		keylist[7] = keyBitRockerRight;
		keylist[8] = keyBitRockerLeft;
		keylist[9] = keyBitRockerCenter;
		vcharlist[7] = vchrRockerRight;
		vcharlist[8] = vchrRockerLeft;
		vcharlist[9] = vchrRockerCenter; 
		codelist[7] = 0;
		codelist[8] = 0;
		codelist[9] = 0;
	}

	//check and set screen depth
	WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, NULL);
	if((depth&128) && !(deviceflags&df_oldrom))
	{
		depth = 8;
		WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
		deviceflags |= df_color;
	}
	else if(depth&2)
	{
		depth = 2;
		WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	}
	else
	{
		return -1;
	}

	//check display resolution
	if((!FtrGet(sysFtrCreator, sysFtrNumWinVersion, &featureval))
	&& (featureval >= 4))
	{
		//Palm hi-res API is present
		WinScreenGetAttribute(winScreenWidth, &width);
		WinScreenGetAttribute(winScreenHeight, &height);
	}
	else if((!FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, &featureval))
	&& (((SonySysFtrSysInfoP)featureval)->libr & sonySysFtrSysInfoLibrHR)
	&& ((!SysLibFind(sonySysLibNameHR, &refnum)) || (!SysLibLoad('libr', sonySysFileCHRLib, &refnum)))
	&& (!HROpen(refnum)))
	{
		//Sony's hi-res library

		//this will disable hi-res assist
		width = 160;
		height = 160;
		HRWinScreenMode(refnum, winScreenModeSet, &width, &height, NULL, NULL);
		//check to see if it worked...
		HRWinScreenMode(refnum, winScreenModeGet, &width, &height, NULL, NULL);

		HRClose(refnum);
	}
	else
	{
		WinScreenMode(winScreenModeGetDefaults, &width, &height, NULL, NULL);
	}

	if((width != 160) || (height < 160))
	{
		//if((width == 320) && (height >= 320)
		//&& (deviceflags & df_color))
		//{
		//	deviceflags |= df_hires;
		//}
		//else
		//{
			deviceflags |= df_apionly;
		//}
	}

	//check processor
	FtrGet(sysFtrCreator, sysFtrNumProcessorID, &featureval);
	if((featureval != sysFtrNumProcessor328)
	&& (featureval != sysFtrNumProcessorEZ)
	&& (featureval != sysFtrNumProcessorVZ)
	&& (featureval != sysFtrNumProcessorSuperVZ))
	{
		deviceflags |= df_apionly;
	}

	return 0;
}


/********************************************************
Initialize the graffiti controls; affects the global
grinfo
*********************************************************/
void initgraf(void)
{
	UInt16 i;

	grinfo.right=NOGRAF;
	grinfo.left=NOGRAF;

	grinfo.penbtnlist=EvtGetPenBtnList(&grinfo.npenbtns);

	for(i=0; i<grinfo.npenbtns; i++)
	{
		if(grinfo.penbtnlist[i].asciiCode==GRAFLEFTBUTTON)
		{
			grinfo.left=grinfo.penbtnlist[i].boundsR.topLeft.x +
				grinfo.penbtnlist[i].boundsR.extent.x;
		}
		else if(grinfo.penbtnlist[i].asciiCode==GRAFRIGHTBUTTON)
		{
			grinfo.right=grinfo.penbtnlist[i].boundsR.topLeft.x;
		}
	}

	if((grinfo.right==NOGRAF)
	|| (grinfo.left==NOGRAF))
	{
		grinfo.right=NOGRAF;
		grinfo.left=NOGRAF;
		prefs.settings.flags&=~set_graf;
	}

	EvtEnableGraffiti(!(prefs.settings.flags&set_graf));
}


/*******************************************************
Pre-process a pen tap.....
********************************************************/
Boolean dopen(Coord x, Coord y)
{
	Coord extent_x, extent_y;
	Boolean eatpen 		= false;
	Boolean flushpen	= false;

	WinGetDisplayExtent(&extent_x, &extent_y);

	if((y >= extent_y)
	&& (prefs.settings.flags & set_graf)
	&& (grinfo.right != NOGRAF)
	&& (grinfo.left != NOGRAF))
	{
		UInt16 i;

		eatpen		= true;
		flushpen	= true;

		for(i = 0; i < grinfo.npenbtns; i++)
		{
			if(RctPtInRectangle(x, y,
			&grinfo.penbtnlist[i].boundsR))
			{
				flushpen = false;
			}
		}

		if(flushpen)
		{
			Coord newx = (x - grinfo.left) *
				(extent_x) /
				(grinfo.right - grinfo.left);

			if(newx > (gv.paddle.width>>1)+SCREENSTARTX)
			{
				pentarget = newx-(gv.paddle.width>>1)-SCREENSTARTX;
			}
			else
			{
				pentarget = 0;
			}
		}
	}
	else if((y > SCREENSTARTY)
	&& (y < extent_y))
	{
		flushpen = true;

		switch(prefs.settings.pencontrol)
		{
			case pc_movement:
				if(x > (gv.paddle.width>>1)+SCREENSTARTX)
				{
					pentarget = x-(gv.paddle.width>>1)-SCREENSTARTX;
				}
				else
				{
					pentarget = 0;
				}
				break;

			case pc_action:
				pentarget = PENACTION;
				break;

			case pc_pause:
				gv.gamestate |= PAUSE;
				FrmPopupForm(form_pause);
				pentarget = NOTARGET;
				break;
		}
	}

	//thought this might solve some pen issues on the T|T1 and T|C - nope.
	//if((flushpen)
	//&& (!(prefs.settings.flags & set_api))
	//&& (!(deviceflags & df_apionly)))
	//update - problems might have been a conflict with NewPen?

	if(flushpen)
	{
		EvtFlushPenQueue();
	}

	return eatpen;
}