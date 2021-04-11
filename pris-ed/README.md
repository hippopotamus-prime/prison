# Pris-Ed
> Board editor for The Prison

## About
Pris-Ed is a Windows-based tool for creating add-on boards for
The Prison, a Breakout-style game for devices running PalmOS.

Boards created using this version of Pris-Ed require version 1.6
or higher of The Prison.

## Free Software
Pris-Ed is free software as described by the GNU General Public
License (v3). See LICENSE.md in the top-level directory for details.

## Usage
Essentially, Pris-Ed works like a paint program.  Left-clicking
on the main window places the currently selected brick, and right-
clicking erases. To the right of the main window is a color
palette and a brick palette. Clicking on a color changes the
bricks that are available, although some of the bricks are
color-independent.

In the brick palette, the top magnet is attractive and the bottom
one is repulsive. This can be confusing since they aren't
animated like in the game.

_**About teleport bricks**_ - For these to work properly, there
must be at least two and at most five per board.

## Menu Commands

### File
The Open, Save, and Save As commands work with files in .pbd
(Prison BoarD) format; you need to use the Export command to
create a Palm Database (.pdb) that the game can read.

Pris-Ed can not read from .pdb files, so be sure to save your
boards in the native format before exporting them.

New is used to create a new board _set_; to add new boards to
an existing set, use the insert commands in the Board menu.

### Board

Command | Description
------- | -----
Next/Prev | Go to the next or previous board in the current set. Pris-Ed will automatically start on the last board in a set.
Insert Before/After | Create a new board and place it before or after the current one.
Delete | Remove the current board.
Insert File Before/After | Import boards from another file and place them before or after the current board.
Flags | Display the bonus flags dialog. Checked power-ups will appear on the current board during the game. The settings carry over when creating new boards.

### Zoom
This menu controls the size of Pris-Ed's windows.  It has no
effect on the game itself.

### Help
The About command displays contact information.
