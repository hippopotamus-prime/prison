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
#include "bricks.h"
#include "game.h"
#include "rc.h"
#include "boni.h"
#include "sound.h"
#include "guys.h"

#define MAGRANGE 4000
#define MAGPOWER 90000
#define MAXTELEPORTS 5

#define teleport_fail		0
#define teleport_bounce		1
#define teleport_ok			2
#define teleport_explode	3

typedef struct
{
	UInt8 i;
	UInt8 j;
}teledest;

static void removebrickanim(brickanim* balistptr, brickanim* prev);
static void magnet(const UInt8 x, const UInt8 y, const Int32 power);
static UInt8 teleport(const UInt8 srci, const UInt8 srcj, const UInt8 guyi);
static void regenbricks(const UInt8 type, const Boolean allbricks);
static Boolean checkforbrick(const UInt8 type);

WinHandle* brickhandles;
static UInt8* brickmasklist;
static WinHandle brickmasks[NBRICKMASKS];


/**********************************************************
Replace any disabled bricks with a given brick, if the
colors match.
***********************************************************/
void regenbricks(const UInt8 type, const Boolean allbricks)
{
	UInt8 i, j;
	
	for(j = 0; j < BOARDHEIGHT; j++)
	{
		for(i = 0; i < BOARDWIDTH; i++)
		{
			if((gv.brickarray[i][j] >= REGBRICKSTART)
			&& ((gv.brickarray[i][j]&BRICKMASK) + REGBRICKSTART == brick_disabled)
			&& ((gv.brickarray[i][j]&~BRICKMASK) == (type&~BRICKMASK)))
			{
				if((allbricks)
				|| (SysRandom(0)&1))
				{
					gv.brickarray[i][j] = type;
					drawbrick(i, j, false);
				}
			}
		}
	}
}


/**********************************************************
Check for the presence of a given brick in the board
(used with regenerating bricks).
***********************************************************/
Boolean checkforbrick(const UInt8 type)
{
	UInt8 i, j;
	Boolean found = false;

	for(j = 0; (j < BOARDHEIGHT) && (!found); j++)
	{
		for(i = 0; i < BOARDWIDTH; i++)
		{
			if(gv.brickarray[i][j] == type)
			{
				found = true;
				break;
			}
		}
	}

	return found;
}


/**********************************************************
Find a teleport location and send a specified guy there.
***********************************************************/
UInt8 teleport(const UInt8 srci, const UInt8 srcj, const UInt8 guyi)
{
	UInt8 i, j;
	UInt8 returncode = teleport_fail;
	UInt8 ndests = 0;
	teledest dests[MAXTELEPORTS-1];

	for(j=0; j<BOARDHEIGHT; j++)
	{
		for(i=0; i<BOARDWIDTH; i++)
		{
			if((gv.brickarray[i][j]>=brick_teleport0)
			&& (gv.brickarray[i][j]<=brick_teleport2)
			&& (ndests<MAXTELEPORTS-1)
			&& ((i!=srci) || (j!=srcj)))
			{
				dests[ndests].i=i;
				dests[ndests++].j=j;
			}
		}
	}

	if(ndests)
	{
		if(gv.guylist[guyi]->karma < KL_NOTELE)
		{
			UInt8 destindex=SysRandom(0)%ndests;

			gv.guylist[guyi]->x = ((dests[destindex].i<<4)+(16-GUYWIDTH)/2)<<8;
	//		gv.guylist[guyi]->x += (dests[destindex].i-srci)<<(4+8);

			if(gv.guylist[guyi]->dy > 0)
			{
				gv.guylist[guyi]->y = (((dests[destindex].j+1)<<3)+BOARDSTARTY)<<8;
			}
			else
			{
				gv.guylist[guyi]->y = (((dests[destindex].j-1)<<3)+BOARDSTARTY)<<8;
			}
	
			addkarma(guyi, KV_TELEPORT);
			returncode = teleport_ok;
		}
		else if(gv.guylist[guyi]->karma < KL_EXTELE)
		{
			addkarma(guyi, KV_TELEBOUNCE);
			returncode = teleport_bounce;
		}
		else
		{
			addkarma(guyi, KV_TELEXPLODE);
			returncode = teleport_explode;
		}
	}

	return returncode;
}


/**********************************************************
Attract / repel all the active guys to / from a specified
point...
***********************************************************/
void magnet(const UInt8 x, const UInt8 y, const Int32 power)
{
	UInt8 i;

	for(i=0; i<gv.nguys; i++)
	{
		if((gv.guylist[i]->status>=STATUS_GO)
		&& ((gv.guylist[i]->karma < KL_MAGIMMUNE)
		|| !(SysRandom(0)%3)))
		{
			Int32 distsq=((Int32)x-(Int32)(gv.guylist[i]->x>>8)) *
			((Int32)x-(Int32)(gv.guylist[i]->x>>8)) +
			((Int32)y-(Int32)(gv.guylist[i]->y>>8)) *
			((Int32)y-(Int32)(gv.guylist[i]->y>>8));

			if((distsq <= MAGRANGE)
			&& ((distsq)))
			{
				Int32 acc=power / distsq;
				Int32 dist=sqroot(distsq);

				Int16 ddx=acc * ((Int32)x-(Int32)(gv.guylist[i]->x>>8)) / dist;
				Int16 ddy=acc * ((Int32)y-(Int32)(gv.guylist[i]->y>>8)) / dist;

				gv.guylist[i]->dx += ddx;
				gv.guylist[i]->dy += ddy;

				if(gv.counter & 1)
				{
					addkarma(i, KV_MAG);
				}
			}
		}
	}
	return;
}


/*******************************************************
Add an animation to the end of the anim list, with
specified parameters...
********************************************************/
void addbrickanim(const UInt8 i, const UInt8 j, const UInt8 start,
const UInt8 end, const UInt8 flags)
{
	brickanim* newba=MemPtrNew(sizeof(brickanim));
	ErrFatalDisplayIf(newba==NULL, "Error creating animation, chief.");

	newba->i=i;
	newba->j=j;
	newba->start=start;
	newba->end=end;
	newba->flags=flags;
	newba->next=NULL;
	if((gv.bahead))
	{
		brickanim* balistptr=gv.bahead;
		while((balistptr->next))
		{
			balistptr=balistptr->next;
		}
		balistptr->next=newba;
	}
	else
	{
		gv.bahead=newba;
	}
	return;
}


void removeallbrickanims(void)
{
	brickanim* balistptr=gv.bahead;
	brickanim* next;

	while((balistptr))
	{
		next=balistptr->next;
		MemPtrFree(balistptr);
		balistptr=next;
	}
	gv.bahead=NULL;
	return;
}


void removebrickanim(brickanim* balistptr, brickanim* prev)
{
	if((prev))
	{
		prev->next=balistptr->next;
	}
	else
	{
		gv.bahead=balistptr->next;
	}
	MemPtrFree(balistptr);
	return;
}


/******************************************************
Hit the 8 bricks surrounding a specified brick, add
explosion animations where bricks were destroyed or
didn't exist
*******************************************************/
void detonate(const UInt8 centeri, const UInt8 centerj)
{
	UInt8 i, j;

	for(j=centerj-1; j!=centerj+2; j++)
	{
		for(i=centeri-1; i!=centeri+2; i++)
		{
			if((i<BOARDWIDTH) && (j<BOARDHEIGHT)
			&& ((i!=centeri) || (j!=centerj)))
			{
				hitbrick(i, j, NOGUY);
				if(!isbrickon(gv.brickarray[i][j]))
				{
					gv.brickarray[i][j]=brick_explode0;
					addbrickanim(i, j, brick_explode0, brick_explode3, REMOVE);
				}
			}
		}
	}

	return;
}


/***********************************************************
Execute all the animations in the anim list, removing any
that are outside their given range or have finished
animating.  Also perform any actions tied to "active"
animations.
************************************************************/
void animatebricks(void)
{
	brickanim* balistptr=gv.bahead;
	brickanim* prev=NULL;
	Boolean removeflag;

	while(balistptr!=NULL)
	{
		removeflag=false;
		if(balistptr->flags&REVERSE)
		{
			if((gv.brickarray[balistptr->i][balistptr->j] > balistptr->start)
			|| (gv.brickarray[balistptr->i][balistptr->j] < balistptr->end))
			{
				//brick has been hit mid-anim
				removebrickanim(balistptr, prev);
				removeflag=true;
			}
		}
		else
		{
			if((gv.brickarray[balistptr->i][balistptr->j] < balistptr->start)
			|| (gv.brickarray[balistptr->i][balistptr->j] > balistptr->end))
			{
				removebrickanim(balistptr, prev);
				removeflag=true;
			}
		}

		if(!removeflag)
		{
			if(balistptr->flags&ACTIVE)
			{
				switch(gv.brickarray[balistptr->i][balistptr->j])
				{
				case brick_explode2:
					setsound(sound_boom);
					detonate(balistptr->i, balistptr->j);
					break;
				case brick_attract2:
				case brick_attract1:
				case brick_attract0:
					if(balistptr->flags&REVERSE)
					{
						magnet((balistptr->i<<4)+8-GUYWIDTH/2,
							(balistptr->j<<3)+BOARDSTARTY+4-GUYHEIGHT/2,
							-MAGPOWER);
					}
					else
					{
						magnet((balistptr->i<<4)+8-GUYWIDTH/2,
							(balistptr->j<<3)+BOARDSTARTY+4-GUYHEIGHT/2,
							MAGPOWER);
					}
					break;
				}
			}

			if((balistptr->flags&REFLECT)
			&& (gv.brickarray[balistptr->i][balistptr->j]==balistptr->end))
			{
				balistptr->flags^=REVERSE;
				balistptr->end=balistptr->start;
				balistptr->start=gv.brickarray[balistptr->i][balistptr->j];
			}

			if(gv.brickarray[balistptr->i][balistptr->j]!=balistptr->end)
			{
				if(balistptr->flags&REVERSE)
				{
					gv.brickarray[balistptr->i][balistptr->j]--;
				}
				else
				{
					gv.brickarray[balistptr->i][balistptr->j]++;
				}
				drawbrick(balistptr->i, balistptr->j, true);
			}
			else
			{
				if(balistptr->flags&REMOVE)
				{
					gv.brickarray[balistptr->i][balistptr->j]=brick_none;
				}
				else
				{
					gv.brickarray[balistptr->i][balistptr->j]=balistptr->start;
				}
				drawbrick(balistptr->i, balistptr->j, true);
				if(!(balistptr->flags&ANIMATEFOREVER))
				{
					removebrickanim(balistptr, prev);
					removeflag=true;
				}
			}
		}

		if(!removeflag)
		{
			prev=balistptr;
			balistptr=balistptr->next;
		}
		else if((prev))
		{
			balistptr=prev->next;
		}
		else
		{
			balistptr=gv.bahead;
		}
	}
	return;
}


/**************************************************************
Draw the brick contained in gv.brickarray at a specified
location...
***************************************************************/
void drawbrick(const UInt8 i, const UInt8 j, const Boolean allowerase)
{
	if(isbrickon(gv.brickarray[i][j]))
	{
		WinHandle brickhand;
		UInt8 lowindex=gv.brickarray[i][j]&BRICKMASK;
		Boolean maskflag =
			((deviceflags&df_color)
			&& (gv.brickarray[i][j]>=REGBRICKSTART)
			&& (brickmasklist[lowindex]!=NOMASK));

		if((deviceflags&df_color) || (gv.brickarray[i][j]<REGBRICKSTART))
		{
			brickhand=brickhandles[gv.brickarray[i][j]];
		}
		else
		{
			brickhand=brickhandles[lowindex+REGBRICKSTART];
		}

		if((deviceflags & df_apionly)
		|| (prefs.settings.flags & set_api)
		|| (maskflag))
		{
			RectangleType lamerect;
			UInt8 operation=winPaint;

			RctSetRectangle(&lamerect, 0, 0, 16, 8);
			if((maskflag))
			{
				WinCopyRectangle(brickmasks[brickmasklist[lowindex]],
					screenbuffer, &lamerect, i<<4, BOARDSTARTY+(j<<3), winMask);
				operation=winOverlay;
			}
			WinCopyRectangle(brickhand, screenbuffer,
				&lamerect, i<<4, BOARDSTARTY+(j<<3), operation);
		}
		else
		{
			UInt8* bufferbits=getwinbits(screenbuffer);
			UInt8* brickbits=getwinbits(brickhand);

			if(deviceflags&df_color)
			{
				bufferbits+=((BOARDSTARTY+8*j)*SCREENWIDTH+16*i);

				asm("
					movem.l %%a0-%%a1/%%d0-%%d4, -(%%a7)

					movea.l %0, %%a0
					movea.l %1, %%a1
					move.l #7, %%d0

				brickloop:
					movem.l (%%a0)+, %%d1-%%d4
					movem.l %%d1-%%d4, (%%a1)
					adda.l %2, %%a1
					dbra %%d0, brickloop

					movem.l (%%a7)+, %%a0-%%a1/%%d0-%%d4
				" : : "m" (brickbits), "m" (bufferbits), "i" (SCREENWIDTH));
			}
			else
			{
				bufferbits+=((BOARDSTARTY+8*j)*SCREENWIDTH+16*i)/4;

				asm("
					movea.l %0, %%a0
					movea.l %1, %%a1

					move.l (%%a0), (%%a1)
					move.l 4(%%a0), 36(%%a1)
					move.l 2*4(%%a0), 2*36(%%a1)
					move.l 3*4(%%a0), 3*36(%%a1)
					move.l 4*4(%%a0), 4*36(%%a1)
					move.l 5*4(%%a0), 5*36(%%a1)
					move.l 6*4(%%a0), 6*36(%%a1)
					move.l 7*4(%%a0), 7*36(%%a1)
				" : : "m" (brickbits), "m" (bufferbits)
				: "%%a0", "%%a1");
			}
		}
	}
	else if((allowerase))
	{
		if((prefs.settings.flags & set_api)
		|| (deviceflags & df_apionly))
		{
			RectangleType eraserect;

			if(prefs.settings.flags&set_background)
			{
				RctSetRectangle(&eraserect, (i<<4)%BGWIDTH,
					((j<<3)+8)%BGHEIGHT, 16, 8);
				WinCopyRectangle(bgwin, screenbuffer, &eraserect,
					i*16, BOARDSTARTY+j*8, winPaint);
			}
			else
			{
				WinHandle oldwin=WinGetDrawWindow();

				WinSetDrawWindow(screenbuffer);
				RctSetRectangle(&eraserect, (i<<4), BOARDSTARTY+(j<<3), 16, 8);
				WinEraseRectangle(&eraserect, 0);
				WinSetDrawWindow(oldwin);
			}
		}
		else
		{
			UInt8* bufferbits=getwinbits(screenbuffer);

			if(deviceflags&df_color)
			{
				bufferbits+=((BOARDSTARTY+(j<<3))*SCREENWIDTH+(i<<4));

				if(prefs.settings.flags&set_background)
				{
					UInt8* bgbits=getwinbits(bgwin)
						+ (((j<<3)+8)%BGHEIGHT)*BGWIDTH + (i<<4)%BGWIDTH;

					asm("
						movem.l %%a0-%%a1/%%d0-%%d4, -(%%a7)
						movea.l %0, %%a0
						movea.l %1, %%a1
						move.l #7, %%d0

					eraseloop:
						movem.l (%%a0), %%d1-%%d4
						movem.l %%d1-%%d4, (%%a1)
						adda.l %2, %%a0
						adda.l %3, %%a1
						dbra %%d0, eraseloop

						movem.l (%%a7)+, %%a0-%%a1/%%d0-%%d4
					" : : "m" (bgbits), "m" (bufferbits),
					"i" (BGWIDTH), "i" (SCREENWIDTH));
				}
				else
				{
					asm("
						movea.l %0, %%a0
						move.l #7, %%d0

					alteraseloop:
						clr.l (%%a0)
						clr.l 4(%%a0)
						clr.l 8(%%a0)
						clr.l 12(%%a0)
						adda.l %1, %%a0
						dbra %%d0, alteraseloop
					" : : "m" (bufferbits), "i" (SCREENWIDTH)
					: "%%a0", "%%d0");
				}
			}
			else
			{
				bufferbits+=((BOARDSTARTY+(j<<3))*SCREENWIDTH+(i<<4))>>2;

				asm("
					movea.l %0, %%a0

					clr.l (%%a0)
					clr.l 36(%%a0)
					clr.l 2*36(%%a0)
					clr.l 3*36(%%a0)
					clr.l 4*36(%%a0)
					clr.l 5*36(%%a0)
					clr.l 6*36(%%a0)
					clr.l 7*36(%%a0)
				" : : "m" (bufferbits) : "%%a0");
			}
		}
	}

	return;
}


/***********************************************************
Draw the entire 9x12 brick array...
************************************************************/
void drawbrickarray(void)
{
	UInt8 i, j;

	for(j=0; j<BOARDHEIGHT; j++)
	{
		for(i=0; i<BOARDWIDTH; i++)
		{
			drawbrick(i, j, false);
		}
	}
}


/***********************************************************
Hit a specified brick and have it respond appropriately,
i.e. remove itself from the board, give points, explode,
etc.  Return true if the brick was solid.
************************************************************/
Boolean hitbrick(const UInt8 i, const UInt8 j, const UInt8 guyi)
{
	Boolean solidflag=true;
	UInt8 soundindex=NOSOUND;

	switch(gv.brickarray[i][j]>=REGBRICKSTART ?
	(gv.brickarray[i][j]&BRICKMASK)+REGBRICKSTART :
	gv.brickarray[i][j])
	{
		case brick_regen:
		{
			UInt8 brick = gv.brickarray[i][j];

			gv.brickarray[i][j] = (brick&~BRICKMASK) | (brick_disabled-REGBRICKSTART);
			gv.score += gv.brickval>>1;

			if(checkforbrick(brick))
			{
				soundindex = sound_hitplain;
				drawbrick(i, j, true);
			}
			else
			{
				soundindex = sound_regen;
				if(guyi!=NOGUY)
				{
					regenbricks(brick, gv.guylist[guyi]->karma < KL_NOREGEN);
				}
				else
				{
					regenbricks(brick, true);
				}
			}
			break;
		}
		case brick_teleport0:
		case brick_teleport1:
		case brick_teleport2:
			if(guyi!=NOGUY)
			{
				UInt8 code = teleport(i, j, guyi);

				if(code == teleport_ok)
				{
					solidflag = false;
					soundindex = sound_teleport;
				}
				else if(code == teleport_bounce)
				{
					soundindex = sound_hitside;
				}
				else
				{
					//teleport_fail or teleport_explode
					gv.brickarray[i][j]=brick_explode0;
					drawbrick(i, j, false);
					addbrickanim(i, j, brick_explode0, brick_explode3,
						ACTIVE | REMOVE);
					gv.score+=gv.brickval;
					soundindex=sound_bang;
				}
			}
			else
			{
				gv.brickarray[i][j]=brick_none;
				drawbrick(i, j, true);
				gv.score+=gv.brickval;
			}
			break;
		case brick_attract0:
		case brick_attract1:
		case brick_attract2:
			gv.brickarray[i][j]=brick_none;
			drawbrick(i, j, true);
			gv.score+=(gv.brickval<<2);
			soundindex=sound_hitmagnet;
			break;
		case brick_accel:
			gv.brickarray[i][j]=brick_none;
			drawbrick(i, j, true);
			gv.score+=gv.brickval;
			if(accelerate(guyi))
			{
				soundindex=sound_accelerate;
			}
			else
			{
				soundindex = sound_hitplain;
			}
			break;
		case brick_jewel:
			gv.brickarray[i][j]=brick_none;
			drawbrick(i, j, true);
			gv.score+=gv.brickval;
			solidflag=false;
			soundindex=sound_hitjewel;
			break;
		case brick_explode0:
		case brick_explode1:
		case brick_explode2:
		case brick_explode3:
			if(guyi==NOGUY)
			{
				gv.brickarray[i][j]=brick_explode0;
			}
			solidflag=false;
			break;
		case brick_bomb:
			gv.brickarray[i][j]=brick_explode0;
			drawbrick(i, j, false);
			addbrickanim(i, j, brick_explode0, brick_explode3,
				ACTIVE | REMOVE);
			gv.score+=gv.brickval;
			soundindex=sound_bang;
			break;
		case brick_bonus0:
		case brick_bonus1:
		case brick_bonus2:
		case brick_bonus3:
			addbonus(16*i+1, 8*j+BOARDSTARTY);
			gv.brickarray[i][j]=brick_none;
			drawbrick(i, j, true);
			gv.score+=gv.brickval;
			soundindex=sound_hitbonus;
			break;
		case brick_unbreakable3:
		case brick_unbreakable2:
		case brick_unbreakable1:
			if((guyi!=NOGUY)
			&& (gv.guylist[guyi]->karma >= KL_BREAKALL))
			{
				gv.brickarray[i][j]=brick_none;
				drawbrick(i, j, true);
				soundindex=sound_break;
				addkarma(guyi, KV_BREAK);
			}
			else
			{
				gv.brickarray[i][j]=brick_unbreakable0;
				drawbrick(i, j, false);
				soundindex=sound_hitside;
			}
			break;
		case brick_unbreakable0:
			if((guyi!=NOGUY)
			&& (gv.guylist[guyi]->karma >= KL_BREAKALL))
			{
				gv.brickarray[i][j]=brick_none;
				drawbrick(i, j, true);
				soundindex=sound_break;
				addkarma(guyi, KV_BREAK);
			}
			else
			{
				addbrickanim(i, j, brick_unbreakable0, brick_unbreakable3, 0);
				soundindex=sound_hitside;
			}
			break;
		case brick_tophit:
			if((guyi==NOGUY) || (gv.guylist[guyi]->dy > 0))
			{
				gv.brickarray[i][j]=brick_none;
				drawbrick(i, j, true);
				gv.score+=gv.brickval;
				soundindex=sound_hitplain;
			}
			else
			{
				soundindex=sound_hitside;
			}
			break;
		case brick_2hit:
		case brick_3hit:
		case brick_4hit:
			gv.brickarray[i][j]--;
			drawbrick(i, j, true);
			gv.score+=gv.brickval;
			soundindex=sound_hitplain;
			break;
		case brick_plain:
			gv.brickarray[i][j]=brick_none;
			drawbrick(i, j, true);
			gv.score+=gv.brickval;
			soundindex=sound_hitplain;
			if(!(SysRandom(0)%BONUSFREQ))
			{
				addbonus(16*i+1, 8*j+BOARDSTARTY);
			}
	}

	if((guyi!=NOGUY) && (solidflag)
	&& (gv.guylist[guyi]->charge))
	{
		detonate(i, j);
		gv.guylist[guyi]->charge--;
		soundindex=sound_boom;
	}

	if(soundindex!=NOSOUND)
	{
		setsound(soundindex);
	}
	return solidflag;
}


UInt16 loadbrickpics(void)
{
	UInt8 i, j;
	MemHandle mh;
	BitmapPtr bmpptr;
	UInt16 err;
	ipalentry* palindices=NULL;

	WinSetDrawWindow(WinGetDisplayWindow());

	if(deviceflags&df_color)
	{
		RGBColorType tempcolor;
		MemHandle brickmasklisthand=NULL;

		//load palette for regular bricks...
		//	get palette in raw rgb form
		MemHandle colorlisthand=DmGetResource(palrc, palette_bricks);
		UInt8* colorlist=MemHandleLock(colorlisthand);
		if(!colorlist) return -1;

		//	allocate memory for indexed palette
		palindices=(ipalentry*)MemPtrNew(sizeof(ipalentry) *
			(NBRICKSERIES_COLOR-2));
		if(!palindices) return -1;

		//	convert rgb palette to 8-bit indices
		for(i=0; i<NBRICKSERIES_COLOR-2; i++)
		{
			for(j=0; j<4; j++)
			{
				tempcolor.r=colorlist[i*12 + j*3 + 0];
				tempcolor.g=colorlist[i*12 + j*3 + 1];
				tempcolor.b=colorlist[i*12 + j*3 + 2];
				palindices[i].colors[j]=WinRGBToIndex(&tempcolor);
			}
		}
		MemHandleUnlock(colorlisthand);
		DmReleaseResource(colorlisthand);

		//allocate memory for brick window handles
		brickhandles=(WinHandle*)MemPtrNew(sizeof(WinHandle) *
			NBRICKSERIES_COLOR * BRICKSERIESLEN);
		if(!brickhandles) return -1;
		MemSet(brickhandles, sizeof(WinHandle) *
			NBRICKSERIES_COLOR * BRICKSERIESLEN, 0);

		//load brick masks...
		brickmasklisthand=DmGetResource(masklistrc, masklist_bricks);
		brickmasklist=MemHandleLock(brickmasklisthand);
		if(!brickmasklist) return -1;

		for(i=0; i<NBRICKMASKS; i++)
		{
			mh=DmGetResource(bitmapRsc, brickmask_start+i);
			bmpptr=MemHandleLock(mh);
			if(!bmpptr) return -1;

			brickmasks[i]=WinCreateOffscreenWindow(16, 8, screenFormat, &err);
			if((err)) return err;
			WinSetDrawWindow(brickmasks[i]);
			WinDrawBitmap(bmpptr, 0, 0);

			MemHandleUnlock(mh);
			DmReleaseResource(mh);
		}
	}
	else
	{
		brickhandles=(WinHandle*)MemPtrNew(sizeof(WinHandle) *
			NBRICKSERIES_GRAY * BRICKSERIESLEN);
		if(!brickhandles) return -1;
		MemSet(brickhandles, sizeof(WinHandle) *
			NBRICKSERIES_GRAY * BRICKSERIESLEN, 0);
	}

	//load special bricks...
	for(i=0; i<NSPECBRICKS; i++)
	{
		mh=DmGetResource(bitmapRsc, brick_start+i);
		bmpptr=MemHandleLock(mh);
		if(!bmpptr) return -1;

		brickhandles[i]=WinCreateOffscreenWindow(16, 8, screenFormat, &err);
		if((err)) return err;
		WinSetDrawWindow(brickhandles[i]);
		WinDrawBitmap(bmpptr, 0, 0);

		MemHandleUnlock(mh);
		DmReleaseResource(mh);
	}

	//load regular bricks...
	for(i=0; i<NREGBRICKS; i++)
	{
		mh=DmGetResource(bitmapRsc, brick_start+REGBRICKSTART+i);
		bmpptr=MemHandleLock(mh);
		if(!bmpptr) return -1;

		if(deviceflags&df_color)
		{
			brickhandles[REGBRICKSTART+i] =
				WinCreateOffscreenWindow(16, 8, screenFormat, &err);
			if((err)) return err;
			WinSetDrawWindow(brickhandles[REGBRICKSTART+i]);
			WinDrawBitmap(bmpptr, 0, 0);

			for(j=2; j<NBRICKSERIES_COLOR; j++)
			{
				brickhandles[j*BRICKSERIESLEN+i] =
					WinCreateOffscreenWindow(16, 8, screenFormat, &err);
				if((err)) return err;
				WinSetDrawWindow(brickhandles[j*BRICKSERIESLEN+i]);
				WinDrawBitmap(bmpptr, 0, 0);

				if(brickmasklist[i]==NOMASK)
				{
					tint(brickhandles[j*BRICKSERIESLEN+i],
						&palindices[j-2], NULL);
				}
				else
				{
					tint(brickhandles[j*BRICKSERIESLEN+i],
						&palindices[j-2],
						brickmasks[brickmasklist[i]]);
				}
			}
		}
		else
		{
			brickhandles[BRICKSERIESLEN+i] =
				WinCreateOffscreenWindow(16, 8, screenFormat, &err);
			if((err)) return err;
			WinSetDrawWindow(brickhandles[BRICKSERIESLEN+i]);
			WinDrawBitmap(bmpptr, 0, 0);
		}

		MemHandleUnlock(mh);
		DmReleaseResource(mh);
	}

	if(deviceflags&df_color)
	{
		MemPtrFree(palindices);
	}
	return 0;
}


void tint(WinHandle win, const ipalentry* tintcolor, WinHandle mask)
{
	BitmapPtr bmpptr=WinGetBitmap(win);
	UInt8* winbits=(UInt8*)BmpGetBits(bmpptr);
	const UInt16 size=BmpBitsSize(bmpptr);
	UInt16 i;
	RGBColorType color;
	UInt8* maskbits=NULL;

	if((mask))
	{
		maskbits=BmpGetBits(WinGetBitmap(mask));
	}

	WinSetDrawWindow(WinGetDisplayWindow());
	for(i=0; i<size; i++)
	{
		if((!maskbits) || (maskbits[i]))
		{
			WinIndexToRGB(winbits[i], &color);
			winbits[i]=tintcolor->colors[color.r>>6];
		}
	}
}


void freebrickpics(void)
{
	UInt8 i;

	if((brickhandles))
	{
		if(deviceflags&df_color)
		{
			if((brickmasklist))
			{
				MemHandle brickmasklisthand=MemPtrRecoverHandle(brickmasklist);
				MemHandleUnlock(brickmasklisthand);
				DmReleaseResource(brickmasklisthand);
			}

			for(i=0; i<NBRICKMASKS; i++)
			{
				if((brickmasks[i]))
				{
					WinDeleteWindow(brickmasks[i], false);
				}
			}

			for(i=0; i < NBRICKSERIES_COLOR*BRICKSERIESLEN; i++)
			{
				if((brickhandles[i]))
				{
					WinDeleteWindow(brickhandles[i], false);
				}
			}
		}
		else
		{
			for(i=0; i < NBRICKSERIES_GRAY*BRICKSERIESLEN; i++)
			{
				if((brickhandles[i]))
				{
					WinDeleteWindow(brickhandles[i], false);
				}
			}
		}

		MemPtrFree(brickhandles);
	}
}