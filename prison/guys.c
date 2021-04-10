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

/****************************************************************************
** Updates
**
**      March 23, 2006 - Rewrote accelerate to bound the ball's speed by
** a more accurate value.  Previously only the dx / dy components were
** bounded.
****************************************************************************/

#include <PalmOS.h>
#include "game.h"
#include "guys.h"
#include "rc.h"
#include "sound.h"

//checkforstuff codes
#define cfs_reflect 0
#define cfs_continue 1
#define cfs_die 2
#define cfs_paddle 3
#define cfs_brick 4
#define cfs_win 5
#define cfs_paddle_right 6
#define cfs_paddle_left 7

#define NIGHTMARE_GRAV 21

//minimum inverse slope, e.g. dx/dy
#define MININVSLOPE 10

WinHandle guyhandles[GUYFRAMES];

static void killguy(UInt8 index, const Boolean affectstate);
static UInt8 checkforstuff(const UInt8 x, const UInt8 y, const UInt8 guyi);


void addkarma(const UInt8 guyi, const Int8 val)
{
	if((gv.guylist[guyi]->karma+val <= MAXKARMA)
	&& (gv.guylist[guyi]->karma+val >= MINKARMA))
	{
		gv.guylist[guyi]->karma+=val;
	}
	else if(val>0)
	{
		gv.guylist[guyi]->karma=MAXKARMA;
	}
	else
	{
		gv.guylist[guyi]->karma=MINKARMA;
	}
}


/************************************************
Animate all the guys on the screen... *shrug*
*************************************************/
void animateguys(void)
{
	UInt8 i;

	for(i=0; i<gv.nguys; i++)
	{
		switch(gv.guylist[i]->status)
		{
		case STATUS_GO:
			gv.guylist[i]->frame=
				(gv.guylist[i]->frame-GUYACTFRAMESTART+1)%8+GUYACTFRAMESTART;
			break;
		case STATUS_FORM:
			if(++gv.guylist[i]->frame >= 6)
			{
				gv.guylist[i]->status=STATUS_WAIT;
			}
		}
	}
	return;
}


/****************************************************
Accelerate all the guys in play...
*****************************************************/
void accelerateall(void)
{
	UInt8 i;
	Boolean playsound=false;

	for(i=0; i<gv.nguys; i++)
	{
		playsound|=accelerate(i);
	}
	if((playsound))
	{
		setsound(sound_accelerate);
	}
	return;
}


/***************************************************
** Increase a guy's speed, but not beyond a certain
** limit.
****************************************************/
Boolean accelerate(const UInt8 i)
{
	Boolean accelerated = false;

    if((i != NOGUY)
    && (gv.guylist[i]->status >= STATUS_GO))
    {
        Int16 speed;
        Int16 max_speed;
        UInt8 acceleration_factor;

        switch(gv.difficulty)
        {
            case diff_easy:
                max_speed = MAX_SPEED_EASY;
                acceleration_factor = ACCELERATION_FACTOR_EASY;
                break;
        
            default:
            case diff_normal:
                max_speed = MAX_SPEED_NORMAL;
                acceleration_factor = ACCELERATION_FACTOR_NORMAL;
                break;
        
            case diff_hard:
                max_speed = MAX_SPEED_HARD;
                acceleration_factor = ACCELERATION_FACTOR_HARD;
                break;
        
            case diff_nightmare:
                max_speed = MAX_SPEED_NIGHTMARE;
                acceleration_factor = ACCELERATION_FACTOR_NIGHTMARE;
        }

		gv.guylist[i]->dx = (Int16)((gv.guylist[i]->dx * acceleration_factor) >> 2);
		gv.guylist[i]->dy = (Int16)((gv.guylist[i]->dy * acceleration_factor) >> 2);

        speed = sqroot((Int32)(gv.guylist[i]->dx) * (Int32)(gv.guylist[i]->dx) +
            (Int32)(gv.guylist[i]->dy) * (Int32)(gv.guylist[i]->dy));

        //If speed changed significantly, set the
        //return status to true.  Significantly means
        //it wasn't already close to the max.
        if((speed - max_speed) < (max_speed >> 3))
        {
            accelerated = true;
        }

        //If we went over the max speed,
        //scale it down to the maximum.
        if(speed > max_speed)
        {
            gv.guylist[i]->dx = (Int16)
                ((Int32)(gv.guylist[i]->dx) * (Int32)(max_speed) / speed);

            gv.guylist[i]->dy = (Int16)
                ((Int32)(gv.guylist[i]->dy) * (Int32)(max_speed) / speed);
        }

		addkarma(i, KV_ACC);
    }

	return accelerated;
}


/**************************************************
Put into play the first guy found waiting on the
paddle...
***************************************************/
Boolean releaseguy(void)
{
	UInt8 i;
	UInt8 nactive = 0;

	//count active guys
	for(i = 0; i < gv.nguys; i++)
	{
		if(gv.guylist[i]->status == STATUS_GO)
		{
			nactive++;
		}
	}

	for(i=0; i<gv.nguys; i++)
	{
		if(gv.guylist[i]->status==STATUS_WAIT)
		{
			const Int32 sqspeedvals[]={73728, 131072, 204800, 294912};

			gv.guylist[i]->status=STATUS_GO;

			if(nactive)
			{
				gv.guylist[i]->dx = SysRandom(0)%513-256;
			}
			else
			{
				//these all correspond to about 45 degrees
				const Int16 startxvels[]={165, 256, 328, 332};

				gv.guylist[i]->dx = startxvels[gv.difficulty];
			}

			gv.guylist[i]->dy=-sqroot(sqspeedvals[gv.difficulty]-((Int32)(gv.guylist[i]->dx) *
				(Int32)(gv.guylist[i]->dx)));
			return true;
		}
	}
	return false;
}


/**********************************************************
Try and move a guy to some coordinates x and y, make the
board respond appropriately, and return a code for what
the guy should do (i.e. continue moving, turn around, etc.)
***********************************************************/
UInt8 checkforstuff(const UInt8 x, const UInt8 y, const UInt8 guyi)
{
	Boolean hitflag=cfs_continue;

	if(((signed)x<=0) || (x+GUYWIDTH>SCREENWIDTH))
	{
		setsound(sound_hitside);
		hitflag=cfs_reflect;
	}
	else if(((signed)y<=TOPBUFFER+gv.borderstrength) && ((gv.borderstrength)))
	{
		setsound(sound_hitborder);
		hitborder();
		hitflag=cfs_reflect;
	}
	else if(y>=TOPBUFFER+SCREENHEIGHT)
	{
		hitflag=cfs_die;
	}
	else if(y<=7)
	{
		hitflag=cfs_win;
	}
	else if((y>TOPBUFFER+SCREENHEIGHT-8-GUYHEIGHT)
	&& (gv.guylist[guyi]->dy > 0))
	{
		Boolean rightflag=false;
		Boolean leftflag=false;

		if((x >= gv.paddle.x)
		&& (x < gv.paddle.x+gv.paddle.width))
		{
			rightflag=true;
		}
		if((x+GUYWIDTH > gv.paddle.x)
		&& (x+GUYWIDTH <= gv.paddle.x+gv.paddle.width))
		{
			leftflag=true;
		}

		if((leftflag)
		&& (!rightflag)
		&& (gv.guylist[guyi]->dx > 0)
		&& (gv.guylist[guyi]->dx > gv.guylist[guyi]->dy))
		{
			hitflag = cfs_paddle_left;
		}
		else if((rightflag)
		&& (!leftflag)
		&& (gv.guylist[guyi]->dx < 0)
		&& (-gv.guylist[guyi]->dx > gv.guylist[guyi]->dy))
		{
			hitflag = cfs_paddle_right;
		}
		else if(rightflag || leftflag)
		{
			hitflag=cfs_paddle;
		}
	}

	else
	{
		if(((y-BOARDSTARTY)>>3 < BOARDHEIGHT) && (y >= BOARDSTARTY))
		{
			if(isbrickon(gv.brickarray[x>>4][(y-BOARDSTARTY)>>3]))
			{
				if(hitbrick(x>>4, (y-BOARDSTARTY)>>3, guyi))
				{
					hitflag=cfs_brick;
				}
			}
			if(((x+GUYWIDTH-1)>>4 != x>>4)
			&& (isbrickon(gv.brickarray[(x+GUYWIDTH-1)>>4][(y-BOARDSTARTY)>>3])))
			{
				if(hitbrick((x+GUYWIDTH-1)>>4, (y-BOARDSTARTY)>>3, guyi))
				{
					hitflag=cfs_brick;
				}
			}
		}
		if(((y+GUYHEIGHT-1)>>3 != y>>3)
		&& ((y+GUYHEIGHT-1-BOARDSTARTY)>>3 < BOARDHEIGHT)
		&& (y+GUYHEIGHT-1 >= BOARDSTARTY))
		{
			if(isbrickon(gv.brickarray[x>>4][(y+GUYHEIGHT-1-BOARDSTARTY)>>3]))
			{
				if(hitbrick(x>>4, (y+GUYHEIGHT-1-BOARDSTARTY)>>3, guyi))
				{
					hitflag=cfs_brick;
				}
			}
			if(((x+GUYWIDTH-1)>>4 != x>>4)
			&& (isbrickon(gv.brickarray[(x+GUYHEIGHT-1)>>4][(y+GUYHEIGHT-1-BOARDSTARTY)>>3])))
			{
				if(hitbrick((x+GUYHEIGHT-1)>>4, (y+GUYHEIGHT-1-BOARDSTARTY)>>3, guyi))
				{
					hitflag=cfs_brick;
				}
			}
		}
	}

	return hitflag;
}


/******************************************************
Attempt to move all the active guys in whatever
direction they're going, have them do other various
things when they hit parts of the playfield...
*******************************************************/
void moveguys(void)
{
	UInt8 i;
	Boolean moveflag = false;
	Boolean dieflag;

	for(i=0; i<gv.nguys; i++)
	{
		dieflag = false;

		if(gv.guylist[i]->status >= STATUS_GO)
		{
			if((!(gv.counter & 3)) && (!moveflag))
			{
				//only want to do this once if any balls are moving
				EvtResetAutoOffTimer();
				moveflag = true;
			}

			switch(checkforstuff((gv.guylist[i]->x + gv.guylist[i]->dx)>>8,
			gv.guylist[i]->y>>8, i))
			{
				case cfs_paddle:
				case cfs_continue:
					gv.guylist[i]->x += gv.guylist[i]->dx;
					break;
				case cfs_brick:
					if((gv.guylist[i]->brickthrough))
					{
						gv.guylist[i]->x += gv.guylist[i]->dx;
					}
					else
					{
						gv.guylist[i]->dx = -gv.guylist[i]->dx;
						addkarma(i, KV_BRICK);
					}
					break;
				case cfs_reflect:
					gv.guylist[i]->dx = -gv.guylist[i]->dx;
					addkarma(i, KV_SIDE);
					break;
				case cfs_paddle_right:
				{
					Int16 tempdx = gv.guylist[i]->dx;
					gv.guylist[i]->dx = gv.guylist[i]->dy;
					gv.guylist[i]->dy = tempdx;
					setsound(sound_hitpaddle);
					addkarma(i, KV_PADDLE);
					break;
				}
				case cfs_paddle_left:
				{
					Int16 tempdx = gv.guylist[i]->dx;
					gv.guylist[i]->dx = -gv.guylist[i]->dy;
					gv.guylist[i]->dy = -tempdx;
					setsound(sound_hitpaddle);
					addkarma(i, KV_PADDLE);
					break;
				}
			}

			switch(checkforstuff(gv.guylist[i]->x>>8,
			(gv.guylist[i]->y + gv.guylist[i]->dy)>>8, i))
			{
				case cfs_continue:
					gv.guylist[i]->y += gv.guylist[i]->dy;
					if(gv.difficulty==diff_nightmare)
					{
						gv.guylist[i]->dy += NIGHTMARE_GRAV;
					}
					break;
				case cfs_brick:
					if((gv.guylist[i]->brickthrough))
					{
						gv.guylist[i]->y += gv.guylist[i]->dy;
					}
					else
					{
						gv.guylist[i]->dy = -gv.guylist[i]->dy;
						addkarma(i, KV_BRICK);
					}
					break;
				case cfs_reflect:
					gv.guylist[i]->dy = -gv.guylist[i]->dy;
					break;
				case cfs_die:
					dieflag=true;
					setsound(sound_die);
					break;
				case cfs_win:
					dieflag=true;
					gv.escapes++;
					gv.score+=(10+(gv.difficulty*10));
					setsound(sound_escape);
					break;
				case cfs_paddle:
				case cfs_paddle_right:
				case cfs_paddle_left:
				{
					UInt16 oldhyp, newhyp;
					Int16 newdx, newdy;
					Int16 ddx = ((gv.guylist[i]->x>>8) - gv.paddle.x
						- (gv.paddle.width>>1)+GUYWIDTH/2)
						* (700/gv.paddle.width);

					oldhyp = sqroot((Int32)(gv.guylist[i]->dx) * (Int32)(gv.guylist[i]->dx)
						+ (Int32)(gv.guylist[i]->dy) * (Int32)(gv.guylist[i]->dy));
					newdx = gv.guylist[i]->dx + ddx;
					newhyp = sqroot((Int32)(newdx) * (Int32)(newdx)
						+ (Int32)(gv.guylist[i]->dy) * (Int32)(gv.guylist[i]->dy));

					newdx = (Int16)(((Int32)(newdx)
						* (Int32)(oldhyp))/(Int32)(newhyp));
					newdy = (Int16)(((Int32)(gv.guylist[i]->dy)
						* (Int32)(oldhyp))/(Int32)(newhyp));
				

					if(((newdx >= 0) && (newdy*MININVSLOPE >= newdx))
					|| ((newdx < 0) && (newdy*MININVSLOPE >= -newdx)))
					{
						gv.guylist[i]->dy = -newdy;
						gv.guylist[i]->dx = newdx;
					}
					else
					{
						gv.guylist[i]->dy = -gv.guylist[i]->dy;
					}

					if((gv.guylist[i]->brickthrough))
					{
						gv.guylist[i]->brickthrough--;
					}
					addkarma(i, KV_PADDLE);
					setsound(sound_hitpaddle);

					break;
				}
			}

			if(dieflag)
			{
				killguy(i, true);
			}
		}
	}
	return;
}


void drawguys(void)
{
	UInt8 i;
	RectangleType lamerect;

	for(i = 0; i < gv.nguys; ++i)
	{
		//grab the background beneath the graphic, so
		//we can erase it later
		RctSetRectangle(&lamerect, gv.guylist[i]->x>>8,
			gv.guylist[i]->y>>8, GUYWIDTH, GUYHEIGHT);
		WinCopyRectangle(screenbuffer, gv.guylist[i]->savebehind,
			&lamerect, 0, 0, winPaint);

		//now draw it...
		RctSetRectangle(&lamerect, 0, 0, GUYWIDTH, GUYHEIGHT);
		WinCopyRectangle(guyhandles[GUYMASKFRAME], screenbuffer,
			&lamerect, gv.guylist[i]->x>>8,
			gv.guylist[i]->y>>8, winMask);
		WinCopyRectangle(guyhandles[gv.guylist[i]->frame], screenbuffer,
			&lamerect, gv.guylist[i]->x>>8,
			gv.guylist[i]->y>>8, winOverlay);
	}
}


void eraseguys(void)
{
	Int8 i;
	RectangleType lamerect;
	RctSetRectangle(&lamerect, 0, 0, GUYWIDTH, GUYHEIGHT);

	for(i=gv.nguys-1; i>=0; i--)
	{
		WinCopyRectangle(gv.guylist[i]->savebehind, screenbuffer,
			&lamerect, gv.guylist[i]->x>>8,
			gv.guylist[i]->y>>8, winPaint);
	}
	return;
}


void killallguys(const UInt8 mask)
{
	Int8 i;

	for(i = gv.nguys-1; i >= 0; i--)
	{
		if(gv.guylist[i]->status & mask)
		{
			killguy(i, (mask != STATUS_ALL));
		}
	}
}


void killguy(UInt8 index, const Boolean affectstate)
{
	WinDeleteWindow(gv.guylist[index]->savebehind, false);
	MemPtrFree(gv.guylist[index]);

	while(index+1 < gv.nguys)
	{
		gv.guylist[index] = gv.guylist[index+1];
		index++;
	}

	if((!(--gv.nguys))
	&& (affectstate))
	{
		if(gv.escapes)
		{
			gv.gamestate = (gv.gamestate & ~INPLAYMASK) | WINLEVEL;
		}
		else
		{
			gv.gamestate = (gv.gamestate & ~INPLAYMASK) | DIE;
		}
	}
}


Boolean addguy(void)
{
	Boolean added = false;
	UInt16 err;

	if(gv.nguys < MAXGUYS)
	{
		gv.guylist[gv.nguys]=(guytype*)MemPtrNew(sizeof(guytype));
		ErrFatalDisplayIf(!gv.guylist[gv.nguys], "Error creating guy, chief.");

		gv.guylist[gv.nguys]->status=STATUS_FORM;
		gv.guylist[gv.nguys]->x=(gv.paddle.x+(gv.paddle.width>>1)-(GUYWIDTH/2))<<8;
		gv.guylist[gv.nguys]->y=(TOPBUFFER+SCREENHEIGHT-8-GUYHEIGHT)<<8;
		gv.guylist[gv.nguys]->dx=0;
		gv.guylist[gv.nguys]->dy=0;
		gv.guylist[gv.nguys]->frame=0;
		gv.guylist[gv.nguys]->charge=0;
		gv.guylist[gv.nguys]->brickthrough=0;
		gv.guylist[gv.nguys]->karma=0;

		gv.guylist[gv.nguys++]->savebehind=WinCreateOffscreenWindow(GUYWIDTH, GUYHEIGHT,
			screenFormat, &err);
		ErrFatalDisplayIf(err, "Error creating guy background window, chief.");

		added = true;
	}

	return added;
}