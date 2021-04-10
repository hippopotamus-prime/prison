/****************************************************************************
** Pris-Ed, The Prison Editor
** Copyright 2004 Aaron Curtis
** 
** This file is part of The Prison.
** 
** Pris-Ed is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Pris-Ed is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with Pris-Ed; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****************************************************************************/

#include <windows.h>
#include <time.h>
#include "file.h"
#include "main.h"
#include "board.h"
#include "resource.h"
#include "handlers.h"

char curfile[MAX_PATH]="";

static BOOL fwritebigend(HANDLE fhand,
	const unsigned int val, const short nbytes);


/*******************************************
Ask the user if they want to save the
board set that's currently open.  If yes,
save it.
********************************************/
int asktosave(void)
{
	int returnval=IDNO;

	if((editflag))
	{
		returnval=MessageBox(NULL, "Save current board set first?",
			"Uh, chief...", MB_YESNOCANCEL | MB_DEFBUTTON2);
		if(returnval==IDYES)
		{
			savefile();
		}
	}
	return returnval;
}


/**************************************************
** Ask the user to select a file, then load its
** contents into a given boardtype*
***************************************************/
void OpenBoardFile(boardtype** board, BOOL set_current)
{
	OPENFILENAME ofn;
	char name[MAX_PATH]="";
	char initialdir[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize=sizeof(ofn);
	ofn.lpstrFilter="Prison Boards (.pbd)\0*.pbd\0";
	ofn.lpstrFile=name;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt="pbd";
	if(curfile[0]!='\0')
	{
		ofn.lpstrInitialDir=curfile;
	}
	else
	{
		getexepath(initialdir);
		ofn.lpstrInitialDir=initialdir;
	}

	if(GetOpenFileName(&ofn))
	{
		HANDLE fhand = CreateFile(name, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL);

		if(fhand != INVALID_HANDLE_VALUE)
		{
			if(LoadFile(fhand, board) && set_current)
			{
				setcurfile(name);
			}

			CloseHandle(fhand);
		}
	}
}


/************************************************
** Re-open the most recently opened file and load
** its contents...
************************************************/
void ReopenFile(void)
{
	HANDLE fhand = CreateFile(curfile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);

	if(fhand != INVALID_HANDLE_VALUE)
	{
		if(LoadFile(fhand, &curboard))
		{
			setcurfile(curfile);
		}
		CloseHandle(fhand);
	}
	else
	{
		if(asktosave()!=IDCANCEL)
		{
			DestroyBoardSet(&curboard);
			curboard = NewBoard(NULL);
			setcurfile(NULL);
		}
	}
}


/**************************************************
** Load a board set from a given file handle...
***************************************************/
short LoadFile(HANDLE fhand, boardtype** board)
{
	DWORD bytesread;
	BYTE boardinfo[2]={0, 0};

	if((ReadFile(fhand, boardinfo, 2,
	&bytesread, NULL)) && (bytesread==2))
	{
		int nboards = boardinfo[1]+(boardinfo[0]<<8);
		DestroyBoardSet(board);

		if(!nboards)
		{
			AppendBoard(board);
		}
		else
		{
			while(nboards)
			{
				short i, j;

				AppendBoard(board);

				ReadFile(fhand, &((*board)->highbonusflags),
					1, &bytesread, NULL);
				ReadFile(fhand, &((*board)->lowbonusflags),
					1, &bytesread, NULL);

				for(j=0; j<BOARDHEIGHT; j++)
				{
					for(i=0; i<BOARDWIDTH; i++)
					{
						if((!ReadFile(fhand, &((*board)->boardarray[i][j]),
						1, &bytesread, NULL)) || (!bytesread))
						{
							MessageBox(NULL, "That's not a good file, chief.",
								NULL, MB_OK);
							DestroyBoardSet(board);
                            *board = NewBoard(NULL);

							return 0;
						}
					}
				}
				nboards--;
			}
		}
		return 1;
	}
	return 0;
}


/********************************************************
Save the current board set under the most recently used
file name...
*********************************************************/
void savefile(void)
{
	HANDLE fhand=CreateFile(curfile, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fhand!=INVALID_HANDLE_VALUE)
	{
		storefile(fhand);
		CloseHandle(fhand);
		reseteditflag();
	}
	else
	{
		savefileas();
	}
}


/**************************************************
Ask the user for a file name and format, then save
the current board appropriately.
***************************************************/
void exportfile(void)
{
	OPENFILENAME ofn;
	char name[MAX_PATH]="";
	char initialdir[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "Palm Database (.pdb)\0*.pdb\0";
	ofn.lpstrFile = name;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT
		| OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "pdb";
	ofn.lpstrTitle = "Export";
	if(curfile[0]!= '\0')
	{
		ofn.lpstrInitialDir=curfile;
	}
	else
	{
		getexepath(initialdir);
		ofn.lpstrInitialDir=initialdir;
	}

	if(GetSaveFileName(&ofn))
	{
		HANDLE fhand=CreateFile(name, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(fhand!=INVALID_HANDLE_VALUE)
		{
			if(ofn.nFilterIndex==1)
			{
				char* pdbname =
					(char*)DialogBox(GetModuleHandle(NULL),
					MAKEINTRESOURCE(idd_pdbname),
					GetParent(boardwin), pdbnamehandler);
				if((pdbname))
				{
					writepdb(fhand, pdbname);
					GlobalFree(pdbname);
				}
			}
			else
			{
				storefile(fhand);
			}
			CloseHandle(fhand);
		}
	}
}


/*******************************************************
Ask the user for a file name and save the current
board under it.
********************************************************/
void savefileas(void)
{
	OPENFILENAME ofn;
	char name[MAX_PATH]="";
	char initialdir[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize=sizeof(ofn);
	ofn.lpstrFilter="Prison Boards (.pbd)\0*.pbd\0";
	ofn.lpstrFile=name;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT
		| OFN_HIDEREADONLY;
	ofn.lpstrDefExt="pbd";
	if(curfile[0]!='\0')
	{
		ofn.lpstrInitialDir=curfile;
	}
	else
	{
		getexepath(initialdir);
		ofn.lpstrInitialDir=initialdir;
	}

	if(GetSaveFileName(&ofn))
	{
		HANDLE fhand=CreateFile(name, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(fhand!=INVALID_HANDLE_VALUE)
		{
			storefile(fhand);
			CloseHandle(fhand);
			setcurfile(name);
		}
	}
}


/*****************************************************
** Write the current board to a given file handle.
******************************************************/
void storefile(HANDLE fhand)
{
	DWORD byteswritten;
	int nboards = CountBoards(NULL);
	boardtype* aboard = FindFirstBoard(curboard);

	fwritebigend(fhand, nboards, 2);

	while((aboard))
	{
		short i, j;

		WriteFile(fhand, &(aboard->highbonusflags),
			1, &byteswritten, NULL);
		WriteFile(fhand, &(aboard->lowbonusflags),
			1, &byteswritten, NULL);

		for(j=0; j<BOARDHEIGHT; j++)
		{
			for(i=0; i<BOARDWIDTH; i++)
			{
				WriteFile(fhand, &(aboard->boardarray[i][j]),
					1, &byteswritten, NULL);
			}
		}

		aboard=aboard->next;
	}
}


void writepdb(HANDLE fhand, const char* pdbname)
{
	DWORD nbw;
	short i;
	unsigned long now;
	struct tm footime={0, 0, 0, 1, 0, 4, 0, 0, -1};

	//name
	WriteFile(fhand, pdbname, 32, &nbw, NULL);
	//attributes
	fwritebigend(fhand, 0, 2);
	//version
	fwritebigend(fhand, BOARDVERSION, 2);
	//creation date
	now=(long)time(NULL)+2082844800;
	fwritebigend(fhand, now, 4);
	//modification date
	fwritebigend(fhand, now, 4);
	//backup date
	fwritebigend(fhand, 0, 4);
	//modification number
	fwritebigend(fhand, 0, 4);
	//app info
	fwritebigend(fhand, 0, 4);
	//sort info
	fwritebigend(fhand, 0, 4);
	//type
	WriteFile(fhand, "Pbrd", 4, &nbw, NULL);
	//creator
	WriteFile(fhand, "Prsn", 4, &nbw, NULL);
	//id seed
	fwritebigend(fhand, 0, 4);
	//chained record list id
	fwritebigend(fhand, 0, 4);
	//number of records
	fwritebigend(fhand, 2, 2);
	//first record entry
	fwritebigend(fhand, 0x60, 4);
	fwritebigend(fhand, 0, 1);
	fwritebigend(fhand, 0, 3);
	//second record entry
	fwritebigend(fhand, (10*22)+10+0x60, 4);
	fwritebigend(fhand, 0, 1);
	fwritebigend(fhand, 0, 3);
	//2-byte gap
	fwritebigend(fhand, 0, 2);

	//first record data
	for(i=0; i<5; i++)
	{
		fwritebigend(fhand, 0, 2);
	}
	for(i=0; i<10; i++)
	{
		fwritebigend(fhand, 100*(20-i), 4);
		fwritebigend(fhand, 0, 1);
		WriteFile(fhand, "----------------", 17, &nbw, NULL);
	}

	//second record data
	storefile(fhand);
}


BOOL fwritebigend(HANDLE fhand, const unsigned int val, const short nbytes)
{
	DWORD nbw;
	BYTE beval[4];
	short i;

	for(i=0; i<nbytes; i++)
	{
		beval[i]=(val>>((nbytes-i-1)<<3))&255;
	}

	return WriteFile(fhand, beval, nbytes, &nbw, NULL);
}


void setcurfile(char* filename)
{
	char wintitle[MAX_PATH+12]="Pris-Ed";

	reseteditflag();
	if((filename))
	{
		char shortname[MAX_PATH];

		GetFileTitle(filename, shortname, MAX_PATH);
		lstrcat(wintitle, "  -  ");
		lstrcat(wintitle, shortname);
		lstrcpy(curfile, filename);
	}
	else
	{
		curfile[0]='\0';
	}

	SetWindowText(GetParent(boardwin), wintitle);
}
