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

#ifndef __SOUND__
#define __SOUND__

#define NOSOUND 0xff
//volume settings
#define vol_sys 0
#define vol_off 1
#define vol_low 2
#define vol_med 3
#define vol_high 4

typedef struct
{
	UInt16 freq;
	UInt16 duration;
	Int16 relamp;
}beep;

typedef struct
{
	UInt8 priority;
	UInt8 nbeeps;
	beep beeps[0];
}noise;

extern Int32 soundtimefornext;
extern UInt16 soundamp;

extern void loadsounds(void);
extern void freesounds(void);
extern void setsound(const UInt8 index);
extern void dosound(void);
extern Int32 getnextbeeptime(void);
extern void setvolume(void);

#endif