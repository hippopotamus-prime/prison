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

#define ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS

#include <PalmOS.h>
#include <PalmNavigator.h>
#include "rc.h"
#include "boni.h"
#include "sound.h"
#include "bricks.h"
#include "guys.h"
#include "highscores.h"
#include "game.h"
#include "prefs.h"

gamevars gv;
WinHandle screenbuffer;
WinHandle bgwin;
UInt8 pentarget=NOTARGET;
UInt8 deviceflags=0;
grafinfo grinfo;

static void erasepaddle(void);
static void drawpaddle(void);
static void docontrols(void);
static void drawscore(void);
static void copybuffer(void);
static void drawboardtext(void);

UInt32 keylist[10] = {0, keyBitHard1, keyBitHard2, keyBitHard3,
	keyBitHard4, keyBitPageUp, keyBitPageDown, keyBitNavRight,
	keyBitNavLeft, keyBitNavSelect};

UInt16 vcharlist[10] = {0, vchrHard1, vchrHard2, vchrHard3,
	vchrHard4, vchrPageUp, vchrPageDown, vchrNavChange,
	vchrNavChange, vchrNavChange};

UInt16 codelist[10] = {0, 0, 0, 0, 0, 0, 0, navBitRight,
	navBitLeft, navBitSelect};


void updategamegfx(void)
{
	if((gv.counter >= gv.dispboardtime)
	&& (gv.redraw))
	{
		drawboard();
		gv.redraw = false;
	}

	drawguys();
	drawpaddle();
	drawboni();

	if(gv.counter < gv.dispboardtime)
	{
		drawboardtext();
	}

	copybuffer();
	eraseboni();
	erasepaddle();
	eraseguys();

	if(!((gv.counter)%ANIMINTERVAL))
	{
		animatebricks();
		animateguys();
	}
}

void updategameplay(void)
{
	moveguys();
	moveboni();
	docontrols();

	if(!(gv.counter%ACCINTERVAL))
	{
		accelerateall();
	}
}



/***********************************
Trade an extra life for a clone...
***********************************/
void convert_life(void)
{
	if((gv.lives)
	&& (addguy()))
	{
		gv.lives--;
		drawlives();
	}
	else
	{
		FrmAlert(alert_failconvert);
	}
}


/*********************************************
turn off events for keys that are in use
**********************************************/
void setkeymask(void)
{
	//this is needed to prevent VirtualCharHandling notifications
	//from being broadcast - the key events themselves aren't
	//that important because they can be discarded
	//(which in fact they are...)

	if(prefs.settings.moreflags & mset_kevents)
	{
		KeySetMask(keyBitsAll);
	}
	else
	{
		KeySetMask(~(keylist[prefs.settings.keys[key_left]]
			| keylist[prefs.settings.keys[key_fastleft]]
			| keylist[prefs.settings.keys[key_action]]
			| keylist[prefs.settings.keys[key_right]]
			| keylist[prefs.settings.keys[key_fastright]]
			/*| keylist[prefs.settings.keys[key_pause]]*/));
	}
}


/**************************************************
Move the paddle onto the visible part of the
screen....
***************************************************/
Boolean paddleup(void)
{
	Boolean finish=false;

	if(!(gv.counter%ANIMINTERVAL))
	{
		if((--gv.paddle.y)==TOPBUFFER+SCREENHEIGHT-8)
		{
			finish=true;
		}
	}
	return finish;
}


void drawsides(void)
{
	UInt16 err;
	UInt8 i;
	RectangleType lamerect;
	WinHandle oldwin = WinGetDrawWindow();
	MemHandle bmphand = DmGetResource(bitmapRsc, bmp_border_side);
	BitmapPtr bmpptr = MemHandleLock(bmphand);
	WinHandle borderhandle = WinCreateOffscreenWindow(BORDERSIDEWIDTH,
		BORDERSIDEHEIGHT, screenFormat, &err);
	ErrFatalDisplayIf(err, "Error drawing border, chief.");

	WinSetDrawWindow(borderhandle);
	WinDrawBitmap(bmpptr, 0, 0);

	MemHandleUnlock(bmphand);
	DmReleaseResource(bmphand);
	WinSetDrawWindow(oldwin);

	RctSetRectangle(&lamerect, 0, 0, BORDERSIDEWIDTH, BORDERSIDEHEIGHT);
	for(i=BORDERSIDESTARTY; i<160; i+=BORDERSIDEHEIGHT)
	{
		WinCopyRectangle(borderhandle, NULL, &lamerect, 0, i, winPaint);
		WinCopyRectangle(borderhandle, NULL, &lamerect,
			160-BORDERSIDEWIDTH, i, winPaint);
	}
	WinDeleteWindow(borderhandle, false);

	//the top counts as a side :)
	WinDrawChars("Score:", 6, 65, 2);
	drawscore();
	RctSetRectangle(&lamerect, 0, 0, GUYWIDTH, GUYHEIGHT);
	WinCopyRectangle(guyhandles[GUYMASKFRAME], NULL,
		&lamerect, 134, 3, winMask);
	WinCopyRectangle(guyhandles[GUYACTFRAMESTART], NULL,
		&lamerect, 134, 3, winOverlay);
	drawlives();
}


void resetgamevars(void)
{
	gv.score = 0;
	gv.lives = 3;
	gv.finishflag = false;
	gv.counter = 0;
	gv.actiontime = 0;
	gv.dispboardtime = DISPBOARDLEN;
	gv.redraw = true;
	gv.nguys = 0;
	gv.nboni = 0;
	gv.bahead = NULL;
	gv.difficulty = prefs.settings.difficulty;
	gv.board = getstartboardprefs();
}


void resetpaddle(Boolean deathflag)
{
	if((deathflag))
	{
		gv.lives--;
		drawlives();
	}
	resizepaddle(PADDLESTARTWIDTH);
	gv.paddle.x=PADDLESTARTX;
	gv.paddle.y=TOPBUFFER+SCREENHEIGHT;
	pentarget=NOTARGET;
	gv.brickval=4+gv.difficulty;		//maybe this should only be done on death?
}


Boolean tonextlevel(void)
{
	Boolean loaded = false;

	killallguys(STATUS_ALL);
	removeallbrickanims();
	killallboni();
	gv.board++;
	if(loadboard())
	{
		if((gv.board > gettopboardprefs())
		&& (!(prefs.settings.flags & set_random)))
		{
			settopboardprefs(gv.board);
		}
		drawboard();
		gv.dispboardtime = gv.counter+DISPBOARDLEN;
		gv.redraw = true;
		loaded = true;
	}

	return loaded;
}


void hitborder(void)
{
	RectangleType eraserect;

	if((deviceflags&df_color)
	&& (prefs.settings.flags&set_background))
	{
		UInt8 i;

		RctSetRectangle(&eraserect, 0, --gv.borderstrength, BGWIDTH, 1);

		for(i=0; i<(SCREENWIDTH-1)/BGWIDTH+1; i++)
		{
			WinCopyRectangle(bgwin, screenbuffer, &eraserect,
				i*BGWIDTH, TOPBUFFER+gv.borderstrength, winPaint);
		}
	}
	else
	{
		WinHandle oldwin=WinGetDrawWindow();
		RctSetRectangle(&eraserect, 0, TOPBUFFER+(--gv.borderstrength),
			SCREENWIDTH, 1);
		WinSetDrawWindow(screenbuffer);
		WinEraseRectangle(&eraserect, 0);
		WinSetDrawWindow(oldwin);
	}

	gv.score+=10;
	return;
}


void drawscore(void)
{
	char scoretext[11];
	StrIToA(scoretext, gv.score);
	WinDrawChars(scoretext, StrLen(scoretext), 92, 2);
	return;
}


void drawlives(void)
{
	RectangleType eraserect;
	char livestext[3];

	RctSetRectangle(&eraserect, 144, 2, 16, 8);
	WinEraseRectangle(&eraserect, 0);
	StrIToA(livestext, gv.lives);
	WinDrawChars(livestext, StrLen(livestext), 144, 2);
	return;
}


/*********************************************************
Move the paddle according to what keys are being pressed
and where the pen has been tapped.  And don't move off the
edge of the screen.
**********************************************************/
void docontrols(void)
{
	UInt8 i;
	Int8 moveamount = 0;
	UInt32 keystate = KeyCurrentState();

	if(((keystate & keylist[prefs.settings.keys[key_action]])
	|| (pentarget == PENACTION))
	&& (gv.actiontime <= gv.counter))
	{
		Boolean acted = false;

		acted |= releaseguy();
		//other stuff here...

		if((acted))
		{
			gv.actiontime = gv.counter+ACTIONINTERVAL;
		}
	}

	if(pentarget < PENACTION)
	{
		if(pentarget >= SCREENWIDTH-gv.paddle.width)
		{
			pentarget=SCREENWIDTH-gv.paddle.width;
		}

		if(pentarget > gv.paddle.x)
		{
			if(pentarget-gv.paddle.x > 12)
			{
				moveamount=12;
			}
			else
			{
				moveamount=pentarget-gv.paddle.x;
			}
		}
		else if(pentarget < gv.paddle.x)
		{
			if(gv.paddle.x-pentarget > 12)
			{
				moveamount=-12;
			}
			else
			{
				moveamount=pentarget-gv.paddle.x;
			}
		}
		else
		{
			pentarget=NOTARGET;
		}
	}
	else if(keystate & keylist[prefs.settings.keys[key_left]])
	{
		if(gv.paddle.x > 1)
		{
			moveamount=-2;
		}
		else if(gv.paddle.x > 0)
		{
			moveamount=-1;
		}
	}
	else if(keystate & keylist[prefs.settings.keys[key_right]])
	{
		if(gv.paddle.x < SCREENWIDTH-gv.paddle.width-1)
		{
			moveamount=2;
		}
		else if(gv.paddle.x < SCREENWIDTH-gv.paddle.width)
		{
			moveamount=1;
		}
	}
	else if(keystate & keylist[prefs.settings.keys[key_fastleft]])
	{
		if(gv.paddle.x > 4)
		{
			moveamount=-5;
		}
		else if(gv.paddle.x > 0)
		{
			moveamount=-1;
		}
	}
	else if(keystate & keylist[prefs.settings.keys[key_fastright]])
	{
		if(gv.paddle.x < SCREENWIDTH-gv.paddle.width-4)
		{
			moveamount=5;
		}
		else if(gv.paddle.x < SCREENWIDTH-gv.paddle.width)
		{
			moveamount=1;
		}
	}

	if(moveamount)
	{
		for(i=0; i<gv.nguys; i++)
		{
			if(gv.guylist[i]->status<STATUS_GO)
			{
				gv.guylist[i]->x+=moveamount<<8;
			}
		}
		gv.paddle.x+=moveamount;

		if(!(gv.counter & 3))
		{
			EvtResetAutoOffTimer();
		}
	}
}


/***********************************************
Find the square root of a (large) number...
************************************************/
UInt16 sqroot(const UInt32 c)
{
	UInt32 sqrt=0;
	UInt32 jump=0x8000;

	while((jump))
	{
		if(c > sqrt*sqrt)
		{
			sqrt+=jump;
		}
		else if(c < sqrt*sqrt)
		{
			sqrt-=jump;
		}
		else if(c == sqrt*sqrt)
		{
			break;
		}
		jump>>=1;
	}

	return (UInt16)sqrt;
}


/**********************************************************
Get the data portion of a window so we can write to it
directly
***********************************************************/
UInt8* getwinbits(WinHandle win)
{
	UInt8* winbits;

	if(deviceflags&df_oldrom)
	{
		winbits=(UInt8*)(win->displayAddrV20);
	}
	else
	{
		winbits=BmpGetBits(WinGetBitmap(win));
	}

	return winbits;
}


void copybuffer(void)
{
	if((deviceflags & df_apionly)
	|| (prefs.settings.flags & set_api))
	{
		RectangleType screenrect;
		RctSetRectangle(&screenrect, 0, TOPBUFFER, SCREENWIDTH, SCREENHEIGHT);
		WinCopyRectangle(screenbuffer, NULL, &screenrect, SCREENSTARTX,
			SCREENSTARTY, winPaint);
	}
	else
	{
		UInt8* bufferbits = getwinbits(screenbuffer);
		UInt8* visbits = getwinbits(WinGetDisplayWindow());

		if(deviceflags & df_color)
		//&& (!(deviceflags & df_hires))
		{
			bufferbits += TOPBUFFER*SCREENWIDTH;
			visbits += 16*160+8;

			asm("
				movem.l %%d0-%%d7/%%a0-%%a6, -(%%a7)

				movea.l %0, %%a0
				movea.l %1, %%a1
				move.l %2, %%d0

			color_loop:
				movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a6
				movem.l %%d1-%%d7/%%a2-%%a6, (%%a1)
				adda.l #48, %%a1
				movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a6
				movem.l %%d1-%%d7/%%a2-%%a6, (%%a1)
				adda.l #48, %%a1
				movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a6
				movem.l %%d1-%%d7/%%a2-%%a6, (%%a1)
				adda.l #64, %%a1
				dbra %%d0, color_loop

				movem.l (%%a7)+, %%d0-%%d7/%%a0-%%a6
			" : : "m" (bufferbits), "m" (visbits), "i" (SCREENHEIGHT-1));
		}

		//custom pixel-doubling routine...  never got it fast enough
		/************************************************************
		else if(deviceflags & df_color)
		{
			bufferbits += TOPBUFFER*SCREENWIDTH;
			visbits += 32*320+16;

			asm("
				movem.l %%d0-%%d3/%%a0-%%a1, -(%%a7)

				movea.l %0, %%a0
				movea.l %1, %%a1
				move.l %2, %%d1

			hi_vloop:
				move.l #71,%%d0
			hi_hloop:
					move.w (%%a0)+,%%d2

					move.b %%d2,%%d3
					lsl.w #8,%%d3
					move.b %%d2,%%d3

					lsr.w #8,%%d2

					swap %%d3
					move.b %%d2,%%d3
					lsl.w #8,%%d3
					move.b %%d2,%%d3
					swap %%d3

					move.l %%d3,320(%%a1)
					move.l %%d3,(%%a1)+
  					dbra %%d0,hi_hloop

				adda.l #32+320,%%a1
				dbra %%d1,hi_vloop

				movem.l (%%a7)+,%%d0-%%d3/%%a0-%%a1
			" : : "m" (bufferbits), "m" (visbits), "i" (SCREENHEIGHT-1));
		}
		********************************************************************/

		else
		{
			bufferbits+=TOPBUFFER*SCREENWIDTH/4;
			visbits+=(16*160+8)/4;

			asm("
				movem.l %%d0-%%d7/%%a0-%%a3, -(%%a7)

				movea.l %0, %%a0
				movea.l %1, %%a1
				move.l %2, %%d0

			altloop:
				movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a3
				movem.l %%d1-%%d7/%%a2-%%a3, (%%a1)
				adda.l #40, %%a1
				dbra %%d0, altloop

				movem.l (%%a7)+, %%d0-%%d7/%%a0-%%a3
			" : : "m" (bufferbits), "m" (visbits), "i" (SCREENHEIGHT-1));
		}
	}
}


void drawpaddle(void)
{
	RectangleType lamerect;

	RctSetRectangle(&lamerect, gv.paddle.x,
		gv.paddle.y, gv.paddle.width, 8);
	WinCopyRectangle(screenbuffer, gv.paddle.savebehind,
		&lamerect, 0, 0, winPaint);

	RctSetRectangle(&lamerect, 0, 0, gv.paddle.width, 8);
	WinCopyRectangle(gv.paddle.mask, screenbuffer, &lamerect,
		gv.paddle.x, gv.paddle.y, winMask);
	WinCopyRectangle(gv.paddle.pic, screenbuffer, &lamerect,
		gv.paddle.x, gv.paddle.y, winOverlay);
	return;
}


void erasepaddle(void)
{
	RectangleType lamerect;
	RctSetRectangle(&lamerect, 0, 0, gv.paddle.width, 8);
	WinCopyRectangle(gv.paddle.savebehind, screenbuffer, &lamerect,
		gv.paddle.x, gv.paddle.y, winPaint);
	return;
}


void resizepaddle(const UInt8 newsize)
{
	UInt16 err;
	MemHandle bmphand;
	BitmapPtr bmpptr;
	WinHandle oldwin = WinGetDrawWindow();
	IndexedColorType oldForeColor = 0;
    IndexedColorType oldBackColor = 0;
	WinHandle midwin;
	UInt8 i;
	RectangleType lamerect;

	if(gv.paddle.pic)
	{
		WinDeleteWindow(gv.paddle.pic, false);
	}
	if(gv.paddle.savebehind)
	{
		WinDeleteWindow(gv.paddle.savebehind, false);
	}
	if(gv.paddle.mask)
	{
		WinDeleteWindow(gv.paddle.mask, false);
	}

	gv.paddle.pic = WinCreateOffscreenWindow(newsize, 8, screenFormat, &err);
	ErrFatalDisplayIf(err, "Error creating paddle, chief.");
	gv.paddle.savebehind = WinCreateOffscreenWindow(newsize, 8, screenFormat, &err);
	ErrFatalDisplayIf(err, "Error creating paddle background window, chief.");
	gv.paddle.mask = WinCreateOffscreenWindow(newsize, 8, screenFormat, &err);
	ErrFatalDisplayIf(err, "Error creating paddle mask, chief.");

	bmphand = DmGetResource(bitmapRsc, bmp_paddlemid);
	bmpptr = MemHandleLock(bmphand);
	ErrFatalDisplayIf(!bmpptr, "Error (1) drawing paddle (mid), chief.");
	midwin = WinCreateOffscreenWindow(1, 8, screenFormat, &err);
	ErrFatalDisplayIf(err, "Error (2) drawing paddle (mid), chief.");
	WinSetDrawWindow(midwin);
	WinDrawBitmap(bmpptr, 0, 0);
	MemHandleUnlock(bmphand);
	DmReleaseResource(bmphand);

	WinSetDrawWindow(gv.paddle.pic);

	RctSetRectangle(&lamerect, 0, 0, 1, 8);
	for(i = PADDLEEDGEWIDTH; i < newsize-PADDLEEDGEWIDTH; ++i)
	{
		WinCopyRectangle(midwin, gv.paddle.pic, &lamerect, i, 0, winPaint);
	}
	WinDeleteWindow(midwin, false);

	bmphand = DmGetResource(bitmapRsc, bmp_paddleleft);
	bmpptr = MemHandleLock(bmphand);
	ErrFatalDisplayIf(!bmpptr, "Error drawing paddle (left), chief.");
	WinDrawBitmap(bmpptr, 0, 0);
	MemHandleUnlock(bmphand);
	DmReleaseResource(bmphand);

	bmphand = DmGetResource(bitmapRsc, bmp_paddleright);
	bmpptr = MemHandleLock(bmphand);
	ErrFatalDisplayIf(!bmpptr, "Error drawing paddle (right), chief.");
	WinDrawBitmap(bmpptr, newsize-PADDLEEDGEWIDTH, 0);
	MemHandleUnlock(bmphand);
	DmReleaseResource(bmphand);

	WinSetDrawWindow(gv.paddle.mask);

    if(deviceflags & df_color)
    {
    	oldForeColor = WinSetForeColor(255);
	    oldBackColor = WinSetBackColor(0);
    }

	RctSetRectangle(&lamerect, PADDLEEDGEWIDTH, 0, newsize-PADDLEEDGEWIDTH, 8);
	WinDrawRectangle(&lamerect, 0);

	bmphand = DmGetResource(bitmapRsc, bmp_paddleleft_mask);
	bmpptr = MemHandleLock(bmphand);
	ErrFatalDisplayIf(!bmpptr, "Error drawing paddle mask (left), chief.");
	WinDrawBitmap(bmpptr, 0, 0);
	MemHandleUnlock(bmphand);
	DmReleaseResource(bmphand);

	bmphand = DmGetResource(bitmapRsc, bmp_paddleright_mask);
	bmpptr = MemHandleLock(bmphand);
	ErrFatalDisplayIf(!bmpptr, "Error drawing paddle mask (right), chief.");
	WinDrawBitmap(bmpptr, newsize-PADDLEEDGEWIDTH, 0);
	MemHandleUnlock(bmphand);
	DmReleaseResource(bmphand);

	WinSetDrawWindow(oldwin);

    if(deviceflags & df_color)
    {
    	WinSetForeColor(oldForeColor);
    	WinSetBackColor(oldBackColor);
    }

	if(gv.paddle.x >= (newsize-gv.paddle.width)>>1)
	{
		if(gv.paddle.x+((newsize-gv.paddle.width)>>1) <= SCREENWIDTH-newsize)
		{
			gv.paddle.x += (gv.paddle.width-newsize)>>1;
		}
		else
		{
			gv.paddle.x = SCREENWIDTH-newsize;
		}
	}
	else
	{
		gv.paddle.x = 0;
	}

	gv.paddle.width = newsize;
}


/**************************************************
Draw the border at the top of the screen...
***************************************************/
void drawborder(void)
{
	RectangleType borderrect;
	WinHandle oldwin=WinGetDrawWindow();

	if(deviceflags & df_color)
	{
		const RGBColorType color={0, 0x33, 0x33, 0xaa};
		WinPushDrawState();
		WinSetForeColor(WinRGBToIndex(&color));
	}

	RctSetRectangle(&borderrect, 0, TOPBUFFER, SCREENWIDTH, gv.borderstrength);
	WinSetDrawWindow(screenbuffer);
	WinDrawRectangle(&borderrect, 0);
	WinSetDrawWindow(oldwin);

	if(deviceflags & df_color)
	{
		WinPopDrawState();
	}
}


void drawbgtiles(void)
{
	UInt8 i, j;
	RectangleType lamerect;
	WinHandle oldwin=WinGetDrawWindow();
	MemHandle bghand=DmGetResource(bitmapRsc, bgstart+gv.board%NBACKGROUNDS);
	BitmapPtr bgptr=MemHandleLock(bghand);
	ErrFatalDisplayIf(!bgptr, "Error drawing background, chief");

	WinSetDrawWindow(bgwin);
	WinDrawBitmap(bgptr, 0, 0);

	for(j=0; j<(SCREENHEIGHT-1)/BGHEIGHT+1; j++)
	{
		for(i=0; i<(SCREENWIDTH-1)/BGWIDTH+1; i++)
		{
			RctSetRectangle(&lamerect, 0, 0, BGWIDTH, BGHEIGHT);
			WinCopyRectangle(bgwin, screenbuffer, &lamerect,
				i*BGWIDTH, j*BGHEIGHT+TOPBUFFER, winPaint);
		}
	}

	WinSetDrawWindow(oldwin);
	MemHandleUnlock(bghand);
	DmReleaseResource(bghand);
	return;
}


void drawboard(void)
{
	if((deviceflags&df_color)
	&& (prefs.settings.flags&set_background))
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
	drawsides();
}


/************************************************************
Load the board from either a resource or an external db and
initialize board-related data...
*************************************************************/
Boolean loadboard(void)
{
	DmOpenRef dbref = NULL;
	MemHandle boardhandle;
	boardset* boardsetptr;
	UInt8 i, j;
	Boolean loadflag = false;

	if(prefs.settings.flags & set_extboards)
	{
		LocalID boardsetlid = DmFindDatabase(prefs.settings.boardsetcard,
			prefs.settings.boardsetname);
		ErrFatalDisplayIf(!boardsetlid, "Error opening board set, chief.");
		dbref = DmOpenDatabase(prefs.settings.boardsetcard,
			boardsetlid, dmModeReadOnly);
		boardhandle = DmGetRecord(dbref, 1);
	}
	else
	{
		boardhandle = DmGetResource(boardrc, boardset0);
	}

	boardsetptr = (boardset*)MemHandleLock(boardhandle);
	ErrFatalDisplayIf(boardsetptr==NULL, "Error loading the board, chief.");

	if(prefs.settings.flags & set_random)
	{
		gv.board = SysRandom(0)%(boardsetptr->nboards);
	}

#ifdef CHEAT
	settopboardprefs(boardsetptr->nboards - 1);
#endif

	if(boardsetptr->nboards > gv.board)
	{
		loadflag = true;
		
		gv.bonusflags = boardsetptr->boards[gv.board].bonusflags;

		for(j=0; j<BOARDHEIGHT; j++)
		{
			for(i=0; i<BOARDWIDTH; i++)
			{
				gv.brickarray[i][j] = boardsetptr->boards[gv.board].bricks[j*BOARDWIDTH+i];

				if((isbrickon(gv.brickarray[i][j]))
				&& (!checkbrick(gv.brickarray[i][j])))
				{
					gv.brickarray[i][j] = brick_4hit;
				}

				switch(gv.brickarray[i][j]>=REGBRICKSTART ?
				(gv.brickarray[i][j]&BRICKMASK)+REGBRICKSTART :
				gv.brickarray[i][j])
				{
					case brick_bonus0:
						addbrickanim(i, j, gv.brickarray[i][j],
							gv.brickarray[i][j]+3, ANIMATEFOREVER);
						break;
					case brick_attract0:
						addbrickanim(i, j, brick_attract0, brick_attract2,
							ANIMATEFOREVER+ACTIVE);
						break;
					case brick_attract2:
						addbrickanim(i, j, brick_attract2, brick_attract0,
							ANIMATEFOREVER+ACTIVE+REVERSE);
						break;
					case brick_teleport0:
						addbrickanim(i, j, brick_teleport0, brick_teleport2,
							ANIMATEFOREVER+REFLECT);
						break;
				}
			}
		}

		resetpaddle(false);
		gv.borderstrength = MAXBORDERSTRENGTH;
		gv.escapes = 0;
	}

	MemHandleUnlock(boardhandle);

	if(prefs.settings.flags & set_extboards)
	{
		DmReleaseRecord(dbref, 1, false);
		DmCloseDatabase(dbref);
	}
	else
	{
		DmReleaseResource(boardhandle);
	}

	return loadflag;
}


void boardtostr(char* text)
{
	StrIToA(text, gv.board+1);
}


void drawboardtext(void)
{
	RectangleType	rect;
	char			boardtext[maxStrIToALen+6];
	FontID			oldfont = FntSetFont(2);
	WinHandle		oldwin = WinGetDrawWindow();
	
	WinSetDrawWindow(screenbuffer);
	RctSetRectangle(&rect, 22, 46, 100, 26);
	WinEraseRectangle(&rect, 0);
	WinDrawRectangleFrame(roundFrame, &rect);
	StrCopy(boardtext, "Board ");
	boardtostr(&boardtext[6]);
	WinDrawChars(boardtext, StrLen(boardtext),
		22+((100-FntCharsWidth(boardtext, StrLen(boardtext)))>>1), 52);				
	FntSetFont(oldfont);
	WinSetDrawWindow(oldwin);
}


void elapsestate(void)
{
	gv.counter++;

	if(!(gv.gamestate&~INPLAYMASK))
	{
		UInt32 oldscore = gv.score;

		switch(gv.gamestate)
		{
		case PLAY:
			updategamegfx();
			updategameplay();
			break;
		case WINLEVEL:
			if(tonextlevel())
			{
				gv.gamestate=ENTER;
			}
			else
			{
				gv.finishflag=true;
				if(checkhighscore()==NOHIGHSCORE)
				{
					FrmPopupForm(form_wingame);
				}
				else
				{
					FrmPopupForm(form_wingamehigh);
				}
				gv.gamestate=NOPLAY;
			}
			break;
		case DIE:
			if(gv.lives>0)
			{
				resetpaddle(true);
				gv.gamestate=ENTER;
			}
			else
			{
				if(checkhighscore()==NOHIGHSCORE)
				{
					FrmPopupForm(form_gameover);
				}
				else
				{
					FrmPopupForm(form_gameoverhigh);
				}
				removeallbrickanims();
				killallboni();
				gv.gamestate=NOPLAY;
			}
			break;
		case ENTER:
		{
			updategamegfx();

			if(paddleup())
			{
				addguy();
				gv.gamestate=PLAY;
				drawboard();
			}
			break;
		}
		case RETURN:
		{
			WinHandle oldwin=WinGetDrawWindow();
			WinSetDrawWindow(screenbuffer);
			WinEraseWindow();
			WinSetDrawWindow(oldwin);

			FrmEraseForm(FrmGetActiveForm());
			FrmGotoForm(form_title);
			gv.gamestate=NOPLAY;
			break;
		}
		}

		//technically, this should be in updategameplay(), but animatebricks()
		//can occasionally affect the score...
		if(gv.score!=oldscore)
		{
			drawscore();
			if((gv.score/EXTRALIFESCORE != oldscore/EXTRALIFESCORE)
			&& (gv.lives<99))
			{
				gv.lives++;
				drawlives();
				setsound(sound_extralife);
			}
		}
	}
	else if(!(gv.gamestate&~(INPLAYMASK|PAUSE|POPUP)))
	{
		switch(FrmGetActiveFormID())
		{
			case form_bonusinfo:
			{
				UInt8 i;
				RectangleType lamerect;

				IndexedColorType oldBackColor = 0;

                if(deviceflags & df_color)
                {
                    oldBackColor = WinSetBackColor(0);
                }

				for(i = 0; i < NBONITYPES; i++)
				{
					RctSetRectangle(&lamerect, (i&1)?134:6, 18+i*12,
						BONUSWIDTH-2, BONUSHEIGHT-2);
					WinEraseRectangleFrame(simpleFrame, &lamerect);
					RctSetRectangle(&lamerect, 0, 0, BONUSWIDTH, BONUSHEIGHT);
					WinCopyRectangle(bonuspics[(gv.counter/ANIMINTERVAL)%NBONIFRAMES],
						NULL, &lamerect, (i&1)?133:5, 17+i*12, winOverlay);
				}

                if(deviceflags & df_color)
                {
    				WinSetBackColor(oldBackColor);
                }
				break;
			}
			case form_brickinfo:
			{
				RectangleType lamerect;

				RctSetRectangle(&lamerect, 0, 0, 16, 8);
				WinCopyRectangle(brickhandles[brick_bonus0+(gv.counter/ANIMINTERVAL)%4],
					NULL, &lamerect, 6, 60, winPaint);
				break;
			}
			case form_brickinfo2:
			{
				RectangleType lamerect;

				RctSetRectangle(&lamerect, 0, 0, 16, 8);
				WinCopyRectangle(brickhandles[brick_attract0+(gv.counter/ANIMINTERVAL)%3],
					NULL, &lamerect, 6, 37, winPaint);
				WinCopyRectangle(brickhandles[brick_attract2-(gv.counter/ANIMINTERVAL)%3],
					NULL, &lamerect, 6, 48, winPaint);
				WinCopyRectangle(brickhandles[brick_teleport0 +
					((gv.counter/ANIMINTERVAL)%4==3?1:(gv.counter/ANIMINTERVAL)%4)],
					NULL, &lamerect, 6, 59, winPaint);
			}
			break;
		}
	}
}