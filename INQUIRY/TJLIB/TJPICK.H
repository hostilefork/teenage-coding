/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJPICK.H
//---
// Thomas Jefferson Software Development Group Library
// Pick List Routines Header File
//---
// History:
//    07/07/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Not so innovative "pick a string" from a flexlist of strings.
//===

#include <flexlist.h>

///////////////////////////////////////////////////////// TYPE DEFINITIONS

struct tjPickOpts
{
   int ux, uy;                  // ux and uy of first pick item.
   int lines;                   // number of items to display at once.
   int fore, back;              // foreground and background of items not picked.
   int hifore, hiback;          // foreground and background of items picked.
   int topline, current;        // set the position beforehand.
   int width;                   // width of the representation desired.

typedef void (*display_function)( tjPickOpts, int );
// Options to display from, entry to display.

typedef int (*key_handler)( tjPickOpts, unsigned char );
// Flexlist, entry, character to decide about, returns if quit necessary

   key_handler keyhan;          // key handler.
   display_function dispfun;    // display function.
   Flist PickMe;                // Flexlist to pick.
   unsigned char lastkey;       // last key pressed...
};


////////////////////////////////////////////////////// FUNCTION PROTOTYPES

void strdispfun( tjPickOpts Opts, int );
// Just a string display function, in case you need it!

void tjStrPad( char *PadMe, int width );
// Pad a string with spaces, to make its strlen == width.

int tjPick( tjPickOpts &Opts );
// Does a pick list of items, using the options set in the given options.

int tjDirPick( char filename[13], char *mask, tjPickOpts Opts );
// Pick a directory entry.  Conveeeenient.
