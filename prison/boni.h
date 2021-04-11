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

#ifndef __BONI__
#define __BONI__

//boni graphics constants
#define NBONITYPES 8
#define NBONIFRAMES 6
#define BONUSWIDTH 14
#define BONUSHEIGHT 14
//gameplay-related
#define BONUSFREQ 10	//i.e. 1 in 10 hits
#define MAXBONI 3

typedef struct
{
	UInt8 x;
	UInt8 y;
	UInt8 type;
	WinHandle savebehind;
}bonus;

extern WinHandle bonuspics[NBONITYPES+NBONIFRAMES];

extern void addbonus(const UInt8 x, const UInt8 y);
extern void killallboni(void);
extern void drawboni(void);
extern void eraseboni(void);
extern void moveboni(void);
extern UInt16 loadbonipics(void);
extern void freebonipics(void);

#endif