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

#define form_title 1
#define form_main 2
#define form_highscores 3
#define form_gameover 4
#define form_gameoverhigh 5
#define form_wingame 6
#define form_wingamehigh 7
#define form_restoring 8
#define form_pause 9
#define form_settings 10
#define form_bonusinfo 11
#define form_brickinfo 12
#define form_about 13
#define form_loading 14
#define form_boards 15
#define form_keys 16
#define form_brickinfo2 17

#define alert_error			30
#define alert_rconf			31
#define alert_suiconf		32
#define alert_failconvert	33

#define palrc 'PPAL'
#define palette_bricks 1000

#define masklistrc 'PMSK'
#define masklist_bricks 1100

#define button_titlestart		0
#define button_titlehs			1
#define button_titlesettings	2
#define button_titleboards		3
#define button_highscoreok		100
#define button_highscorecancel	101
#define button_pauseresume		200
#define label_pauseboard		201
#define button_settingsok		300
#define button_settingscancel	301
#define checkbox_autosave		302
#define button_settingskeys		303
#define list_pen				304
#define popup_pen				305
#define checkbox_rconf			306
#define checkbox_datehs			307
#define list_volume				309
#define popup_volume			310
#define checkbox_background		311
#define checkbox_compat			312
#define checkbox_graf			313
#define list_diffs				314
#define popup_diffs				315
#define button_bonidone			400
#define button_bonibricks		401
#define button_brickdone		500
#define button_brickmore		501
#define button_brickmore2		502
#define button_brickboni		503
#define button_aboutdone		600
#define list_boards				700
#define button_boardsok			701
#define button_boardscancel		702
#define button_boardsplus		703
#define button_boardsminus		704
#define label_startboard		705
#define STARTBOARDX				120
#define STARTBOARDY				82
#define button_boardsdelete		706
#define alert_boardsdelete		707
#define checkbox_random			708
#define label_nboards			709
#define button_keysok			800
#define button_keyscancel		801
#define list_keys				802
#define popup_rightkey			803
#define popup_leftkey			804
#define popup_frightkey			805
#define popup_fleftkey			806
#define popup_actionkey			807
#define popup_pausekey			808
#define button_keysdefaults		809
#define list_altkeys			810
#define checkbox_kevents		811

#define menu_main			100
#define menu_title			101
#define menu_pause			102

#define menuitem_pause		100
#define menuitem_retire		101
#define menuitem_settings	102
#define menuitem_suicide	103
#define menuitem_quit		104
#define menuitem_start		105
#define menuitem_scores		106
#define menuitem_boards		107
#define menuitem_resume		108
#define menuitem_convert	109
#define menuitem_bonusinfo	110
#define menuitem_brickinfo	111
#define menuitem_about		112


#define field_highscore 0

#define guy_start 0
#define bmp_guyform0 0
#define bmp_guyform1 1
#define bmp_guyform2 2
#define bmp_guyform3 3
#define bmp_guyform4 4
#define bmp_guyform5 5
#define bmp_guy0 6
#define bmp_guy1 7
#define bmp_guy2 8
#define bmp_guy3 9
#define bmp_guy4 10
#define bmp_guy5 11
#define bmp_guy6 12
#define bmp_guy7 13
#define bmp_guymask 14

#define bmp_paddleright 20
#define bmp_paddleleft 21
#define bmp_paddlemid 22
#define bmp_paddleright_mask 23
#define bmp_paddleleft_mask 24

#define bmp_border_side 30

#define brick_start 40
#define bmp_brick_explode0 40
#define bmp_brick_explode1 41
#define bmp_brick_explode2 42
#define bmp_brick_explode3 43
#define bmp_brick_unbreakable0 44
#define bmp_brick_unbreakable1 45
#define bmp_brick_unbreakable2 46
#define bmp_brick_unbreakable3 47
#define bmp_brick_bomb 48
#define bmp_brick_attract0 49
#define bmp_brick_attract1 50
#define bmp_brick_attract2 51
#define bmp_brick_teleport0 52
#define bmp_brick_teleport1 53
#define bmp_brick_teleport2 54
#define bmp_brick_plain 56
#define bmp_brick_2hit 57
#define bmp_brick_3hit 58
#define bmp_brick_4hit 59
#define bmp_brick_bonus0 60
#define bmp_brick_bonus1 61
#define bmp_brick_bonus2 62
#define bmp_brick_bonus3 63
#define bmp_brick_jewel 64
#define bmp_brick_accel 65
#define bmp_brick_tophit 66
#define bmp_brick_regen 67

#define brick_none 0xff
#define brick_explode0 0
#define brick_explode1 1
#define brick_explode2 2
#define brick_explode3 3
#define brick_unbreakable0 4
#define brick_unbreakable1 5
#define brick_unbreakable2 6
#define brick_unbreakable3 7
#define brick_bomb 8
#define brick_attract0 9
#define brick_attract1 10
#define brick_attract2 11
#define brick_teleport0 12
#define brick_teleport1 13
#define brick_teleport2 14
#define brick_PLACEHOLDER3 15
#define brick_plain 16
#define brick_2hit 17
#define brick_3hit 18
#define brick_4hit 19
#define brick_bonus0 20
#define brick_bonus1 21
#define brick_bonus2 22
#define brick_bonus3 23
#define brick_jewel 24
#define brick_accel 25
#define brick_tophit 26
#define brick_regen 27
#define brick_PLACEHOLDER5 28
#define brick_PLACEHOLDER6 29
#define brick_PLACEHOLDER7 30
#define brick_disabled 31

#define boni_start 80
#define bmp_bonus_50point 80
#define bmp_bonus_decel 81
#define bmp_bonus_clone 82
#define bmp_bonus_through 83
#define bmp_bonus_enlarge 84
#define bmp_bonus_extralife 85
#define bmp_bonus_extrapoint 86
#define bmp_bonus_charge 87

#define bonus_50point 0
#define bonus_decel 1
#define bonus_clone 2
#define bonus_through 3
#define bonus_enlarge 4
#define bonus_extralife 5
#define bonus_extrapoint 6
#define bonus_charge 7

#define boniframe_start 100
#define bmp_bonusframe0 100
#define bmp_bonusframe1 101
#define bmp_bonusframe2 102
#define bmp_bonusframe3 103
#define bmp_bonusframe4 104
#define bmp_bonusframe5 105

#define brickmask_start 150
#define bmp_brickmask_jewel 150

#define bgstart 200
#define bmp_bg0 200
#define bmp_bg1 201
#define bmp_bg2 202
#define bmp_bg3 203
#define bmp_bg4 204
#define bmp_bg5 205
#define bmp_bg6 206

#define bmp_title 220

#define boardrc 'PRBO'
#define boardset0 0

#define soundrc 'PSND'
#define sound_start 0
#define sound_hitside 0
#define sound_hitpaddle 1
#define sound_hitborder 2
#define sound_hitplain 3
#define sound_hitbonus 4
#define sound_accelerate 5
#define sound_hitmagnet 6
#define sound_hitjewel 7
#define sound_boom 8
#define sound_collect 9
#define sound_die 10
#define sound_escape 11
#define sound_bang 12
#define sound_extralife 13
#define sound_break 14
#define sound_teleport 15
#define sound_regen 16