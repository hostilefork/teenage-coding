/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJKEYS.H
//---
// Thomas Jefferson Software Development Group Library
// Keyboard Input Routines Header File
//---
// History:
//    07/07/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Uses the TechnoJock toolkit system of translating double key sequences
//    into a single key.  Defines the new "key" type, which is basically an
//    unsigned character.  (easier to type "key").  To keep from having all
//    these key things defined, #define __NOKEYS.
//===

///////////////////////////////////////////////////////// TYPE DEFINITIONS

typedef unsigned char key;

//////////////////////////////////////////////////// CONSTANT DECLARATIONS

#ifndef __NOKEYS
#define __NOKEYS

#define    BackSp       8
#define    PgUp         201
#define    CtrlPgUp     138
#define    Tab          9
#define    PgDn         209
#define    CtrlPgDn     246
#define    Enter        13
#define    End          207
#define    CtrlEnd      245
#define    Esc          27
#define    Home         199
#define    CtrlHome     247
#define    STab         143
#define    Ins          210
#define    Del          211
#define    LArr         203
#define    CtrlLArr     243
#define    CtrlPrtsc    242
#define    RArr         205
#define    CtrlRArr     244
#define    UArr         200
#define    DArr         208
#define    CtrlA        1
#define    AltA         158
#define    Alt1         248
#define    CtrlB        2
#define    AltB         176
#define    Alt2         249
#define    CtrlC        3
#define    AltC         174
#define    Alt3         250
#define    CtrlD        4
#define    AltD         160
#define    Alt4         251
#define    CtrlE        5
#define    AltE         146
#define    Alt5         252
#define    CtrlF        6
#define    AltF         161
#define    Alt6         253
#define    CtrlG        7
#define    AltG         162
#define    Alt7         254
#define    CtrlH        8
#define    AltH         163
#define    Alt8         255
#define    CtrlI        9
#define    AltI         151
#define    Alt9         134
#define    CtrlJ        10
#define    AltJ         164
#define    Alt0         135
#define    CtrlK        11
#define    AltK         165
#define    Altminus     136
#define    CtrlL        12
#define    AltL         166
#define    Altequals    137
#define    CtrlM        13
#define    AltM         178
#define    CtrlN        14
#define    AltN         177
#define    CtrlO        15
#define    AltO         152
#define    CtrlP        16
#define    AltP         153
#define    CtrlQ        17
#define    AltQ         144
#define    CtrlR        18
#define    AltR         147
#define    CtrlS        19
#define    AltS         159
#define    CtrlT        20
#define    AltT         148
#define    CtrlU        21
#define    AltU         150
#define    CtrlV        22
#define    AltV         175
#define    CtrlW        23
#define    AltW         145
#define    CtrlX        24
#define    AltX         173
#define    CtrlY        25
#define    AltY         149
#define    CtrlZ        26
#define    AltZ         172
#define    F1           187
#define    sF1          212
#define    F2           188
#define    sF2          213
#define    F3           189
#define    sF3          214
#define    F4           190
#define    sF4          215
#define    F5           191
#define    sF5          216
#define    F6           192
#define    sF6          217
#define    F7           193
#define    sF7          218
#define    F8           194
#define    sF8          219
#define    F9           195
#define    sF9          220
#define    F10          196
#define    sF10         221
#define    F11          139
#define    sF11         141
#define    F12          140
#define    sF12         142
#define    CtrlF1       222
#define    AltF1        232
#define    CtrlF2       223
#define    AltF2        233
#define    CtrlF3       224
#define    AltF3        234
#define    CtrlF4       225
#define    AltF4        235
#define    CtrlF5       226
#define    AltF5        236
#define    CtrlF6       227
#define    AltF6        237
#define    CtrlF7       228
#define    AltF7        238
#define    CtrlF8       229
#define    AltF8        239
#define    CtrlF9       230
#define    AltF9        240
#define    CtrlF10      231
#define    AltF10       241
#define    CtrlF11      154
#define    AltF11       156
#define    CtrlF12      155
#define    AltF12       157

#endif

////////////////////////////////////////////////////// FUNCTION PROTOTYPES

int tjKeyIn( key ch, key Lo, key Hi );
// Check to see if a key is in a particular range.

int tjVideoKey( key ch );
// Check to see if the key pressed has a video character.

key tjGetKey( );
// Get a key from the keyboard, return its keycode.
