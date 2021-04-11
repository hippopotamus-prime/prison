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
#include "game.h"
#include "sound.h"
#include "rc.h"

#define NSOUNDS 17

static noise* soundlist[NSOUNDS];

Int32 soundtimefornext;
UInt16 soundamp;


void loadsounds(void)
{
	UInt8 i;
	MemHandle beepshand;
	for(i=0; i<NSOUNDS; i++)
	{
		beepshand=DmGetResource(soundrc, sound_start+i);
		soundlist[i]=(noise*)MemHandleLock(beepshand);
	}

	setvolume();
	return;
}


void setvolume(void)
{
	if((prefs.settings.volume))
	{
		const UInt8 amps[4]={0, sndMaxAmp>>2, sndMaxAmp>>1, sndMaxAmp};
		soundamp=amps[prefs.settings.volume-1];
	}
	else
	{
		soundamp=PrefGetPreference(prefGameSoundVolume);
	}
	return;
}


void freesounds(void)
{
	UInt8 i;
	MemHandle beepshand;

	for(i=0; i<NSOUNDS; i++)
	{
		if((soundlist[i]))		//in case startapp failed
		{
			beepshand=MemPtrRecoverHandle(soundlist[i]);
			MemHandleUnlock(beepshand);
			DmReleaseResource(beepshand);
		}
	}
	return;
}


void setsound(const UInt8 index)
{
	if((index==NOSOUND) || (!soundamp))
	{
		gv.cursound=NOSOUND;
		soundtimefornext=evtWaitForever;
	}
	else if((gv.cursound==NOSOUND)
	|| (soundlist[index]->priority >= soundlist[gv.cursound]->priority))
	{
		gv.cursound=index;
		gv.curbeep=0;

		soundtimefornext=TimGetTicks();
//		dosound();
//		soundtimefornext=getnextbeeptime();
//		gv.curbeep++;
	}
	return;
}


void dosound(void)
{
	if(gv.cursound!=NOSOUND)
	{
		UInt16 amp;

		if(soundamp < -soundlist[gv.cursound]->beeps[gv.curbeep].relamp)
		{
			amp=0;
		}
		else if(soundamp+soundlist[gv.cursound]->beeps[gv.curbeep].relamp > sndMaxAmp)
		{
			amp=sndMaxAmp;
		}
		else
		{
			amp=soundamp+soundlist[gv.cursound]->beeps[gv.curbeep].relamp;
		}

		if((amp)
		&& (soundlist[gv.cursound]->beeps[gv.curbeep].freq)
		&& (soundlist[gv.cursound]->beeps[gv.curbeep].duration))
		{
			SndCommandType command;

			command.cmd=sndCmdFrqOn;
			command.param1=soundlist[gv.cursound]->beeps[gv.curbeep].freq;
			command.param2=soundlist[gv.cursound]->beeps[gv.curbeep].duration;
			command.param3=amp;

			SndDoCmd(NULL, &command, 0);
		}

		if(gv.curbeep >= soundlist[gv.cursound]->nbeeps-1)
		{
			gv.cursound=NOSOUND;
		}
	}
	return;
}


Int32 getnextbeeptime(void)
{
	if(gv.cursound==NOSOUND)
	{
		return evtWaitForever;
	}
	return TimGetTicks() + (SysTicksPerSecond() *
		soundlist[gv.cursound]->beeps[gv.curbeep].duration / 1000);
}