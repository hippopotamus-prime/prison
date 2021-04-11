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

#ifndef __GUYS__
#define __GUYS__

//guy graphics constants
#define GUYHEIGHT 8
#define GUYWIDTH 8
#define GUYFRAMES 15
#define GUYACTFRAMESTART 6
#define GUYMASKFRAME 14
//gameplay-related
#define MAXGUYS 5

//karma-related
#define MAXKARMA		95
#define MINKARMA		0
#define KL_BREAKALL		50		//break unbreakable bricks
#define KL_MAGIMMUNE	65		//ignore magnets (~75%)
#define KL_NOTELE		60		//bounce off teleports
#define KL_NOREGEN		65		//stop regenerating bricks (~50%)
#define KL_EXTELE		95		//blow up teleports
#define KV_PADDLE		-50
#define KV_BRICK		3
#define KV_MAG			1
#define KV_SIDE			2
#define KV_ACC			-8
#define KV_BREAK		-21
#define KV_TELEPORT		1		//this is added on top of KV_BRICK
#define KV_TELEBOUNCE	-2		//so is this
#define KV_TELEXPLODE	-90		//and this too

//guy status codes
#define STATUS_FORM 1
#define STATUS_WAIT 2
#define STATUS_GO 4
#define STATUS_ALL 0xff

#define NOGUY 0xff

typedef struct
{
	UInt16 x;
	UInt16 y;
	Int16 dx;
	Int16 dy;
	UInt8 status;
	UInt8 frame;
	UInt8 brickthrough;
	UInt8 charge;
	Int8 karma;
	WinHandle savebehind;
}guytype;

extern WinHandle guyhandles[GUYFRAMES];

extern void animateguys(void);
extern void killallguys(const UInt8 mask);
extern Boolean addguy(void);
extern void eraseguys(void);
extern void drawguys(void);
extern void moveguys(void);
extern Boolean releaseguy(void);
extern Boolean accelerate(const UInt8 i);
extern void accelerateall(void);
extern void addkarma(const UInt8 guyi, const Int8 val);

#endif