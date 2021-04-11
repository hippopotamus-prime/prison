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
**      April 2, 2006 - Added checking for divide-by-zero in decelerate.
** (yeah, I know....)
**
**      March 23, 2006 - Rewrote decelerate to bound the ball's speed by
** a more accurate value.
**
**      July 12, 2005 - Cleaned up addbonus a bit, made it so it checks if
** all bonus flags are disabled (crashed previously).
****************************************************************************/

#include <PalmOS.h>
#include "rc.h"
#include "boni.h"
#include "sound.h"
#include "guys.h"
#include "game.h"
#include "bricks.h"

#define bmask_50point 1
#define bmask_decel 2
#define bmask_clone 4
#define bmask_through 8
#define bmask_enlarge 16
#define bmask_extralife 32
#define bmask_extrapoint 64
#define bmask_charge 128

#define BWEIGHTS {7, 10, 7, 3, 8, 3, 5, 6}

WinHandle bonuspics[NBONITYPES+NBONIFRAMES];

static void killbonus(UInt8 index);
static void collectbonus(const UInt8 i);
static void decelerate(void);
static void addbrickthrough(const UInt8 amt);
static void addcharge(const UInt8 amt);


void collectbonus(const UInt8 i)
{
	UInt8 soundindex=sound_collect;

	switch(gv.bonuslist[i]->type)
	{
		case bonus_50point:
			gv.score+=50;
			break;
		case bonus_decel:
			decelerate();
			break;
		case bonus_clone:
			if(!addguy())
			{
				gv.score += 20;
			}
			break;
		case bonus_through:
			addbrickthrough((gv.brickval-gv.difficulty)>>1);
			break;
		case bonus_enlarge:
			if(gv.paddle.width<60)
			{
				resizepaddle(gv.paddle.width+12);
			}
			break;
		case bonus_extralife:
			if(gv.lives<99)
			{
				gv.lives++;
				drawlives();
				soundindex=sound_extralife;
			}
			break;
		case bonus_extrapoint:
			gv.brickval+=2;
			break;
		case bonus_charge:
			addcharge((gv.brickval-gv.difficulty)>>1);
	}

	setsound(soundindex);
	return;
}


void addcharge(const UInt8 amt)
{
	UInt8 i;
	for(i=0; i<gv.nguys; i++)
	{
		if((gv.guylist[i]->status>=STATUS_GO)
		|| (gv.nguys==1))
		{
			gv.guylist[i]->charge+=amt;
		}
	}
	return;
}


void addbrickthrough(const UInt8 amt)
{
	UInt8 i;
	for(i=0; i<gv.nguys; i++)
	{
		if((gv.guylist[i]->status>=STATUS_GO)
		|| (gv.nguys==1))
		{
			gv.guylist[i]->brickthrough+=amt;
		}
	}
	return;
}


void decelerate(void)
{
	UInt8 i;
    Int16 speed;

	for(i = 0; i < gv.nguys; ++i)
	{
		if(gv.guylist[i]->status >= STATUS_GO)
        {
            gv.guylist[i]->dx = (gv.guylist[i]->dx * 3) >> 2;
            gv.guylist[i]->dy = (gv.guylist[i]->dy * 3) >> 2;
    
            speed = sqroot((Int32)(gv.guylist[i]->dx) * (Int32)(gv.guylist[i]->dx) +
    		    (Int32)(gv.guylist[i]->dy) * (Int32)(gv.guylist[i]->dy));
    
            //If below the minimum speed, scale
            //the guy's speed up to the minimum.
            //But ONLY if the speed is not zero (heh, oops).
            if((speed < MIN_SPEED)
            && (speed != 0))
            {
                gv.guylist[i]->dx =
                    (Int16)((Int32)(gv.guylist[i]->dx) * (Int32)(MIN_SPEED) / speed);
                gv.guylist[i]->dy =
                    (Int16)((Int32)(gv.guylist[i]->dy) * (Int32)(MIN_SPEED) / speed);
            }
        }
	}
}


void moveboni(void)
{
	UInt8 i;
	UInt8 x, y;

	for(i=0; i<gv.nboni; i++)
	{
		x=gv.bonuslist[i]->x;
		y=gv.bonuslist[i]->y++;

		if(y>=TOPBUFFER+SCREENHEIGHT)
		{
			killbonus(i);
		}
		else if((y>TOPBUFFER+SCREENHEIGHT-8-BONUSHEIGHT) && (x+BONUSWIDTH>gv.paddle.x)
		&& (x<gv.paddle.x+gv.paddle.width))
		{
			collectbonus(i);
			killbonus(i);
		}
	}
}


void eraseboni(void)
{
	Int8 i;
	RectangleType lamerect;

	RctSetRectangle(&lamerect, 0, 0, BONUSWIDTH, BONUSHEIGHT);
	for(i=gv.nboni-1; i>=0; i--)
	{
		WinCopyRectangle(gv.bonuslist[i]->savebehind, screenbuffer, &lamerect,
			gv.bonuslist[i]->x, gv.bonuslist[i]->y, winPaint);
	}
	return;
}


void drawboni(void)
{
	UInt8 i;
	RectangleType lamerect;

	for(i=0; i<gv.nboni; i++)
	{
		//save the background
		RctSetRectangle(&lamerect, gv.bonuslist[i]->x,
			gv.bonuslist[i]->y, BONUSWIDTH, BONUSHEIGHT);
		WinCopyRectangle(screenbuffer, gv.bonuslist[i]->savebehind, &lamerect,
			0, 0, winPaint);

		//draw the frame
		RctSetRectangle(&lamerect, 0, 0, BONUSWIDTH, BONUSHEIGHT);
		WinCopyRectangle(bonuspics[(gv.counter/ANIMINTERVAL)%NBONIFRAMES],
			screenbuffer, &lamerect, gv.bonuslist[i]->x, gv.bonuslist[i]->y,
			winPaint);

		//draw the icon
		RctSetRectangle(&lamerect, 0, 0, BONUSWIDTH-2, BONUSHEIGHT-2);
		WinCopyRectangle(bonuspics[NBONIFRAMES+gv.bonuslist[i]->type], screenbuffer,
			&lamerect, gv.bonuslist[i]->x+1, gv.bonuslist[i]->y+1, winPaint);
	}
	return;
}


void killallboni(void)
{
	Int8 i;

	for(i=gv.nboni-1; i>=0; i--)
	{
		killbonus(i);
	}
	return;
}


void killbonus(UInt8 index)
{
	WinDeleteWindow(gv.bonuslist[index]->savebehind, false);
	MemPtrFree(gv.bonuslist[index]);

	while(index+1 < gv.nboni)
	{
		gv.bonuslist[index]=gv.bonuslist[index+1];
		index++;
	}
	gv.nboni--;
	return;
}


/**********************************************************************
** Randomly create a new bonus at the given coordinates, taking into
** account the board's bonus flags.
**********************************************************************/
void addbonus(const UInt8 x, const UInt8 y)
{
	if((gv.nboni < MAXBONI)
	&& (gv.bonusflags))
	{
		const UInt8 weights[NBONITYPES] = BWEIGHTS;
        UInt8 weight_sum = 0;
		UInt8 i;

        //Calculate the sum of the weights...
        for(i = 0; i < NBONITYPES; ++i)
        {
            if(gv.bonusflags & (1<<i))
            {
                weight_sum += weights[i];
            }
        }

        //weight_sum could be 0 if all the bonus flags were disabled,
        //this would then crash the next section...
        if(weight_sum)
        {
            //Choose a critical value in the range of enabled
            //bonus weights...
		    UInt8 crit = SysRandom(0) % weight_sum;
		    UInt8 sum = 0;
		    UInt16 err;

            //Create the actual bonus...
		    gv.bonuslist[gv.nboni] = MemPtrNew(sizeof(bonus));
		    ErrFatalDisplayIf(!gv.bonuslist[gv.nboni], "Error creating bonus, chief.");

		    gv.bonuslist[gv.nboni]->x = x;
		    gv.bonuslist[gv.nboni]->y = y;

		    gv.bonuslist[gv.nboni]->savebehind = WinCreateOffscreenWindow(BONUSWIDTH,
			    BONUSHEIGHT, screenFormat, &err);
		    ErrFatalDisplayIf(err, "Error creating bonus background window, chief.");

            i = -1;

            //Now loop until the critical value is reached -
            //this gives a nice, weighted distribution of
            //bonus types.
		    while(crit >= sum)
		    {
			    i = (i+1) % NBONITYPES;

			    if(gv.bonusflags & (1<<i))
			    {
				    sum += weights[i];
			    }
		    }

		    gv.bonuslist[gv.nboni++]->type = i;
        }
	}
}



UInt16 loadbonipics(void)
{
	UInt16 err=0;
	MemHandle mh;
	BitmapPtr bmpptr;
	UInt8 i;

	for(i=0; i<NBONIFRAMES; i++)
	{
		mh=DmGetResource(bitmapRsc, boniframe_start+i);
		bmpptr=MemHandleLock(mh);
		if(!bmpptr)
		{
			err=-1;
			break;
		}

		bonuspics[i]=WinCreateOffscreenWindow(BONUSWIDTH, BONUSHEIGHT,
			screenFormat, &err);
		if(err) break;
		WinSetDrawWindow(bonuspics[i]);
		WinDrawBitmap(bmpptr, 0, 0);

		MemHandleUnlock(mh);
		DmReleaseResource(mh);
	}
	if((err)) return err;

	for(i=0; i<NBONITYPES; i++)
	{
		mh=DmGetResource(bitmapRsc, boni_start+i);
		bmpptr=MemHandleLock(mh);
		if(!bmpptr)
		{
			err=-1;
			break;
		}

		bonuspics[NBONIFRAMES+i]=WinCreateOffscreenWindow(BONUSWIDTH-2,
			BONUSHEIGHT-2, screenFormat, &err);
		if(err) break;
		WinSetDrawWindow(bonuspics[NBONIFRAMES+i]);
		WinDrawBitmap(bmpptr, 0, 0);
		if(deviceflags&df_color)
		{
			const ipalentry bpal={{0xcb, 0x64, 0x38, 0x00}};
			tint(bonuspics[NBONIFRAMES+i], &bpal, NULL);
		}

		MemHandleUnlock(mh);
		DmReleaseResource(mh);
	}

	return err;
}


void freebonipics(void)
{
	UInt8 i;

	for(i=0; i<NBONITYPES+NBONIFRAMES; i++)
	{
		if((bonuspics[i]))
		{
			WinDeleteWindow(bonuspics[i], false);
		}
	}
}