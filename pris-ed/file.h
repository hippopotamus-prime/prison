/****************************************************************************
** Pris-Ed, The Prison Editor
** Copyright 2004 Aaron Curtis
** 
** This file is part of The Prison.
** 
** Pris-Ed is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** Pris-Ed is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with The Prison; if not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#ifndef __PFILE__
#define __PFILE__

#include <windows.h>
#include "board.h"

extern char curfile[MAX_PATH];

void savefileas(void);
void savefile(void);
void exportfile(void);
void OpenBoardFile(boardtype** board, BOOL set_current);
void ReopenFile(void);
void storefile(HANDLE fhand);
short LoadFile(HANDLE fhand, boardtype** board);
int asktosave(void);
void writepdb(HANDLE fhand, const char* pdbname);
void setcurfile(char* filename);

#endif
