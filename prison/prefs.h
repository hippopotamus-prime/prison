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

#ifndef __PREFS__
#define __PREFS__

#define SPREFVERSION 4
#define UPREFVERSION 2

#define PREFGAMEVARS 1


extern void getprefs(void);
extern void killgamevars(void);
extern void storegamevars(void);
extern UInt16 loadgamevars(void);

#endif