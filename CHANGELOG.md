# Changelog

## 1.9.2 - 2021-04-11

* Project uploaded to GitHub. References to the defunct Mobile Game Lab have
been removed.
* License updated to GPL v3.

## 1.9.1 - 2006-04-02

### Game Fixes

* Fixed divide by zero error when catching decelerate power-ups

## 1.9 - 2006-04-01

### Game Fixes

* Fixed crash when opening the settings form on some devices (Treo)
* Bounded the ball speed more accurately; in rare cases when playing
nightmare mode, the ball could go fast enough to skip off the screen

### Editor Changes

* Added file insertion commands
* "Append" changed to "Insert After"

## 1.8.7 - 2005-07-14

### Game Fixes

* Disabling all the power-ups would cause a crash

## 1.8.6 - 2005-01-13

### Game Fixes

* Restored OS 3.0 compatibility (broken in 1.8.4)

## 1.8.5 - 2005-01-08

### Editor Changes

* Pris-Ed released under the GNU GPL

## 1.8.4 - 2004-12-30

### Game Fixes

* The game should be more tolerant of changes to the system palette

### Other Changes

* Prison released under the GNU GPL

## 1.8.3 - 2004-08-27

### Game Fixes

* The "board #" box should always disappear correctly now

## 1.8.2 - 2004-07-06

### Game Fixes

* Teleports should explode much less readily
* The first ball released should always be at a fixed angle (this didn't happen
before if you had several un-released balls)

## 1.8.1 - 2004-05-29

### Game Fixes

* Restored the aggressive key blocking from previous versions, but with an
option on the keys form to turn it off

## 1.8 - 2004-05-26

### New Game Features

* Added a menu command to convert extra lives into clones
* Teleports can explode if balls have bounced around enough (before they would
only bounce off, potentially creating loops)
* Key presses are blocked less aggressively if compatibility mode is on
* Added support for the 5-way navigator on the Treo 600 (untested)
* Collecting a clone power-up now gives some points if the maximum number of
balls is already in play
* Added navigation resources for most forms
* The boards form now shows the number of boards in a set

## 1.7 - 2004-02-11

### New Game Features

* The board number is now displayed while paused
* Added an option to play random boards
* Revised the info forms a little
* Ball movement now prevents auto-off

### Game Fixes

* Hopefully fixed various pen control problems (esp. on the Tungsten C)
* Added Graffiti state indicators where there are editable fields

### Other Changes

* Web page is up - mobilegamelab.com

## 1.6 - 2003-10-02

### New Game Features

* Added 10 new boards
* Added regenerating bricks
* The menu can now be accessed while paused
* Unpaired teleports now explode when hit
* The board number is displayed when starting a new board
* The "extra points" bonus amplifies some of the other bonus effects

### Game Fixes

* Cleaned up some bad data on board 43
* Fixed the score not being displayed after explosions
* Fixed a bug with version checking for external boards (_**Warning**_ - New
boards created with Pris-Ed can crash earlier versions)

## 1.5.1 - 2003-08-16

### Game Fixes

* Pen movement should now prevent auto-off

### Editor Fixes

* Exported boards should now have the right format internally

## 1.5 - 2003-08-06

### New Game Features

* Added Jog Dial support
* Added a suicide command in case a ball gets stuck
* The pause key should now also un-pause
* There is now a minimum angle at which the balls can bounce
* Added a "Quit Game" item to the menu

### Game Fixes

* Improved compatibility with 68k / hi-res Sony models

### Editor Changes

* Removed the rather useless .bin option from the export dialog

## 1.4 - 2003-05-28

### New Game Features

* Added 5-way navigator support
* Added an option to add the date to new high scores
* Added optional retirement confirmation 
* The settings form arranges itself a little better now

### Game Fixes

* Fixed loading of plain-gray bricks in external boards
* "API Mode" is now "Compatibility Mode"
* Fixed a bug that allowed you to try to play deleted boards
* Hopefully got rid of the occasional extrememly low-angle bounces

## 1.3 - 2003-04-18

### New Game Features

* Pen taps can now be tied to things besides paddle movement
* Pausing can now be done by a key
* Moved the key settings onto a separate page

### Game Fixes

* OS events are now generated for the correct keys

## 1.2 - 2003-02-07

### New Game Features

* Difficulty levels added

### Game Fixes

* Graffiti (text input) is now turned off when using Graffiti control
* Tweaked the game timing a little
* Using the settings form won't revert you to the internal boards now

## 1.1 - 2003-01-18

### New Game Features

* Graffiti control added

### Game Fixes

* High scores for external boards should be saved correctly now
* Improved Visor Prism compatibility
* Better error handling on game start-up
* Under some conditions the background was drawn to replace bricks that were
hit, even though it was turned off

## 1.0 - 2003-01-10

### New Editor Features

* Pris-Ed is now included in the archive

### New Game Features

* External board support is enabled
* There are now 50 internal boards
* Power-ups now have a little more color

### Game Fixes

* Fixed odd-angled bounces when hitting the paddle's corners

## Beta Release 2 (limited)

### New Game Features

* Added a title graphic
* The highest board reached is now stored along side the high scores
* The start board is also stored
* Added teleport bricks
* Animated bricks can now reverse their direction
* The info menu is now accessible from the title screen
* The "extra point" bonus now gives two points instead of one

### Game Fixes

* The info screens now erase themselves a little better
* The paddle should always start in the same place now
* Board db's are now closed properly when you win the game

## Beta Release 1 (limited)

* First release (_**Warning**_ - incompatible with the newer versions, delete
this if upgrading)
