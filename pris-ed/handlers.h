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

#ifndef __HANDLERS__
#define __HANDLERS__

#include <windows.h>

LRESULT CALLBACK mainhandler(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK boardhandler(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK brickhandler(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK palettehandler(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK abouthandler(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK flagshandler(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK pdbnamehandler(HWND, UINT, WPARAM, LPARAM);

#endif

