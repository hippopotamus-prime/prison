# The Prison
> An open source Breakout-style game for PalmOS

![Screenshot1](https://i.imgur.com/IzvYAgh.gif)
![Screenshot2](https://i.imgur.com/pVb4ylw.gif)
![Screenshot3](https://i.imgur.com/qj9HOTi.gif)
![Screenshot4](https://i.imgur.com/hHpFJpk.gif)
![Screenshot5](https://i.imgur.com/J3W5HbE.gif)

## Overview
The Prison is a Breakout-style game featuring a wide assortment of
power-ups, special bricks, and other oddities.

For the uninitiated, the object of the game is to destroy bricks by
bouncing a ball (or in this case, a little man trapped in a ball) into
them, while keeping the ball from falling off the bottom of the screen.

One of the (relatively) unique points in The Prison's gameplay is that
you don't need to hit every brick to complete a level; instead you just
need to hit the top border a few times.

## Free Software
The Prison is free software as described by the GNU General Public
License (v3). See LICENSE.md for details.

The color background graphics (gfx/bg[0-3]_color.bmp) are from public domain
sources.

## System Requirements
Feature | System Requirements
------- | -------------------
2-bit grayscale | OS 3.0
8-bit color | OS 3.5 (and hardware capable of displaying color)

## Controls
By default, the controls are set up as follows:

Button | Action
------ | ------
**To Do List** | Move right
**Memo Pad** | Move right (fast)
**Address Book** | Move left
**Date Book** | Move left (fast)
**Up** | Release the ball
**Down** | Pause

The game will also respond to pen input, by moving the paddle to where ever
you tap.

## Settings
The settings menu is accessible from the title screen or by the in-game
menu. The controls for sound volume and the keys should be fairly
self-explanatory. As for the rest:

Setting | Description
------- | -----------
**Difficulty** | Controls the speed of the ball in-game, as well as the scoring. Nightmare difficulty also adds gravity. The setting only takes effect when starting a new game.
**Pen Control** | Determines how the game responds to pen taps in the display area (but not Graffiti).
**Autosave** | If checked, any games in progress are saved when you exit the program and restored the next time the program is run.
**Date High Scores** | If checked, the default name for high scores has the current date appended to it, if there is enough space for both the name and the date.
**Background** | If checked, a background picture is tiled behind the bricks. (Color devices only.)
**Compatibility Mode** | If checked, API calls are used to draw the game's graphics, instead of faster assembly routines. Generally, you should leave this off unless there are graphical errors. (This option is not present on hi-res and non-68k devices.)
**Graffiti Control** | If checked, the game will respond to pen input in the Graffiti area, by moving the paddle. (Not present if the game can't figure out where the Graffiti area is, and even if it can, it probably won't be in exactly the right place.)

## Building
The game was originally developed with [PRC Tools](http://prc-tools.sourceforge.net/), which
unfortunately has not been maintained for modern operating systems. An easy alternative to build
it is to use [prc-tools-remix](https://github.com/jichu4n/prc-tools-remix). The project offers pre-built
binaries for 64-bit Ubuntu/Debian systems and a convenient setup script to install the Palm SDK.

Run `make all` to build prison.prc, then transfer it to a PalmOS device
or the [Palm OS Emulator](https://sourceforge.net/projects/pose/).
