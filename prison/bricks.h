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

#ifndef __BRICKS__
#define __BRICKS__

#define NBRICKSERIES_COLOR 14
#define NBRICKSERIES_GRAY 2
#define BRICKSERIESLEN 16
#define NREGBRICKS 12
#define NSPECBRICKS 15
#define BRICKMASK 15
#define REGBRICKSTART BRICKSERIESLEN
#define NBRICKMASKS 1
#define NOMASK 0xff

//brickanim flags
#define ANIMATEFOREVER 1
#define ACTIVE 2
#define REVERSE 4
#define REMOVE 8
#define REFLECT 16

struct animbrick
{
	UInt8 i;
	UInt8 j;
	UInt8 start;
	UInt8 end;
	UInt8 flags;
	struct animbrick* next;
};
typedef struct animbrick brickanim;

typedef struct
{
	UInt8 colors[4];
}ipalentry;

extern WinHandle* brickhandles;

extern void addbrickanim(const UInt8 i, const UInt8 j, const UInt8 start,
	const UInt8 end, const UInt8 flags);
extern void removeallbrickanims(void);
extern void animatebricks(void);
extern void drawbrick(const UInt8 i, const UInt8 j, const Boolean allowerase);
extern Boolean hitbrick(const UInt8 i, const UInt8 j, const UInt8 guyi);
extern void detonate(const UInt8 centeri, const UInt8 centerj);
extern UInt16 loadbrickpics(void);
extern void freebrickpics(void);
extern void drawbrickarray(void);
extern void tint(WinHandle win, const ipalentry* tintcolor, WinHandle mask);

#define checkbrick(brick)	((brick >= REGBRICKSTART) ? \
	(((brick&BRICKMASK) < NREGBRICKS) && (brick < NBRICKSERIES_COLOR*BRICKSERIESLEN)) : \
	(brick < NSPECBRICKS))

#define isbrickon(brick)	((brick < REGBRICKSTART) || ((brick&BRICKMASK) != 0x0f))
	//returns false for brick_none and brick_disabled

#endif