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

#ifndef __HANDLERS__
#define __HANDLERS__

extern Boolean titlehandler(EventPtr eventptr);
extern Boolean mainhandler(EventPtr eventptr);
extern Boolean highscoreshandler(EventPtr eventptr);
extern Boolean gameoverhandler(EventPtr eventptr);
extern Boolean gameoverhighhandler(EventPtr eventptr);
extern Boolean restorehandler(EventPtr eventptr);
extern Boolean pausehandler(EventPtr eventptr);
extern Boolean settingshandler(EventPtr eventptr);
extern Boolean bonusinfohandler(EventPtr eventptr);
extern Boolean nohandler(EventPtr eventptr);
extern Boolean boardshandler(EventPtr eventptr);
extern Boolean keyshandler(EventPtr eventptr);
extern Boolean infohandler(EventPtr eventptr);

#endif