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

#ifndef __GAME__
#define __GAME__

#include "guys.h"
#include "boni.h"
#include "bricks.h"
#include "highscores.h"

#define CR_ID 'Prsn'
//device flags...
#define df_apionly	1
#define df_oldrom	2
#define df_color	4
#define df_5way		8
#define df_hires	16
//screen buffer size / position...
#define SCREENSTARTX 8
#define SCREENSTARTY 16
#define SCREENWIDTH 144
#define SCREENHEIGHT 144
#define TOPBUFFER 8
#define BOTTOMBUFFER 8
//side border graphics constants
#define BORDERSIDEWIDTH 8
#define BORDERSIDEHEIGHT 4
#define BORDERSIDESTARTY 16
//background graphics constants
#define BGWIDTH 32		//must be a multiple of 16
#define BGHEIGHT 32		//"" 8
#define NBACKGROUNDS 4
//other...
#define PADDLESTARTX 60
#define PADDLESTARTWIDTH 24
#define PADDLEEDGEWIDTH 8
#define ACTIONINTERVAL 6
#define ACCINTERVAL 200
#define ANIMINTERVAL 3
#define MAXBORDERSTRENGTH 8
#define DISPBOARDLEN 60
//game states
#define NOPLAY		0
#define RETURN		1
#define PLAY		2
#define WINLEVEL	3
#define DIE			4
#define ENTER		5
#define INPLAYMASK	7
#define PAUSE		16
#define POPUP		32
#define RESTORE		64
#define INTERRUPT	128

#define BOARDWIDTH 9
#define BOARDHEIGHT 12
#define BOARDSTARTY (8+TOPBUFFER)
#define EXTRALIFESCORE 4000

#define NOTARGET 0xff
#define PENPAUSE 0xfe
#define PENACTION 0xfd

#define NOHIGHSCORE 0xff
#define NOGRAF 0xff

#define MAXBOARDSETS 64
#define BOARDVERSION 1

//2048 corresponds to moving 8 pixels per frame;
//more would have the ball skipping over blocks.
#define MAX_SPEED_EASY      1400
#define MAX_SPEED_NORMAL    1800
#define MAX_SPEED_HARD      2040
#define MAX_SPEED_NIGHTMARE 2040

#define MIN_SPEED           280

//These are divided by 4, so on easy, the ball
//speeds up 20% each interval.
#define ACCELERATION_FACTOR_EASY        5
#define ACCELERATION_FACTOR_NORMAL      5
#define ACCELERATION_FACTOR_HARD        6
#define ACCELERATION_FACTOR_NIGHTMARE   6

typedef struct
{
	UInt16 npenbtns;
	const PenBtnInfoType* penbtnlist;
	Coord right;
	Coord left;
}grafinfo;

typedef struct
{
	UInt8 rightkey;
	UInt8 leftkey;
	UInt8 fastrightkey;
	UInt8 fastleftkey;
	UInt8 actionkey;
}keydefV12;

#define key_right		0
#define key_left		1
#define key_fastright	2
#define key_fastleft	3
#define key_action		4
#define key_pause		5
#define key_reserved1	6
#define key_reserved2	7

typedef struct
{
	UInt16 boardsetcard;
	char boardsetname[32];
	keydefV12 keys;
	UInt8 flags;
	UInt8 volume;
	UInt8 difficulty;
	UInt8 reserved1;
	UInt8 reserved2;
	UInt8 reserved3;
}settingsV12;

typedef struct
{
	UInt16 boardsetcard;
	char boardsetname[32];
	UInt8 keys[8];
	UInt8 flags;
	UInt8 volume;
	UInt8 difficulty;
	UInt8 pencontrol;
	UInt8 moreflags;		//*cough* someone should have made flags 16-bit
	UInt8 reserved3;
}settings;

#define diff_easy		0
#define diff_normal 	1
#define diff_hard		2
#define diff_nightmare	3

#define set_autosave		1
#define set_autosave_bit	0
#define set_background		2
#define set_background_bit	1
#define set_api				4
#define set_api_bit			2
#define set_extboards		8
#define set_extboards_bit	3
#define set_graf			16
#define set_graf_bit		4
#define set_rconf			32
#define set_rconf_bit		5
#define set_datehs			64
#define set_datehs_bit		6
#define set_random			128
#define set_random_bit		7
#define mset_kevents		1
#define mset_kevents_bit	0

#define pc_movement	0
#define pc_action	1
#define pc_pause	2
#define pc_off		255

typedef struct
{
	settings settings;
	boarddata data;
}prisprefs;

typedef struct
{
	settingsV12 settings;
	boarddata data;
}prisprefsV12;

typedef union
{
	prisprefs prefs;
	prisprefsV12 prefsV12;
}tpref;

typedef struct
{
	UInt8 x;
	UInt8 y;
	UInt8 width;
	WinHandle pic;
	WinHandle savebehind;
	WinHandle mask;
}paddletype;

typedef struct
{
	UInt16 bonusflags;
	UInt8 bricks[BOARDWIDTH*BOARDHEIGHT];
}boardtype;

typedef struct
{
	UInt16 nboards;
	boardtype boards[0];
}boardset;

typedef struct
{
	UInt8 nguys;
	UInt8 nboni;
	guytype* guylist[MAXGUYS];
	bonus* bonuslist[MAXBONI];
	brickanim* bahead;
	paddletype paddle;
	UInt32 counter;
	UInt32 actiontime;
	UInt32 score;
	UInt32 dispboardtime;
	UInt32 reserved0;
	UInt16 bonusflags;
	UInt16 board;
	UInt8 brickarray[BOARDWIDTH][BOARDHEIGHT];
	UInt8 escapes;
	UInt8 lives;
	UInt8 brickval;
	UInt8 gamestate;
	UInt8 borderstrength;
	UInt8 cursound;
	UInt8 curbeep;
	UInt8 difficulty;
	Boolean redraw;		//this tells the game to redraw the board after the "board ##" box disappears
	UInt8 reserved2;
	UInt8 reserved3;
	Boolean finishflag;
}gamevars;

extern gamevars gv;
extern WinHandle screenbuffer;
extern WinHandle bgwin;
extern UInt8 pentarget;
extern UInt8 deviceflags;
extern prisprefs prefs;
extern grafinfo grinfo;
extern Int32 gametimefornext;
extern UInt32 timeinterval;

extern UInt32 keylist[10];
extern UInt16 vcharlist[10];
extern UInt16 codelist[10];

extern void updategamegfx(void);
extern void updategameplay(void);
extern void drawboard(void);
extern Boolean loadboard(void);
extern void drawborders(void);
extern Boolean tonextlevel(void);
extern void resetpaddle(Boolean deathflag);
extern void resizepaddle(const UInt8 newsize);
extern void resetgamevars(void);
extern void drawlives(void);
extern UInt16 sqroot(const UInt32 c);
extern Boolean paddleup(void);
extern void hitborder(void);
extern void elapsestate(void);
extern UInt8* getwinbits(WinHandle win);
extern void drawbgtiles(void);
extern void drawborder(void);
extern void drawsides(void);
extern void boardtostr(char* text);
extern void setkeymask(void);
extern void convert_life(void);

#endif