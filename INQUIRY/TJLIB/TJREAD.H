/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJREAD.H
//---
// Thomas Jefferson Software Development Group Library
// String Reading Routines Header File
//---
// History:
//    07/07/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Editing routines are somewhat rudimentary.  Perhaps I'll modify them
//    later.  Short time and all, you know.
//===

////////////////////////////////////////////////////// FUNCTION PROTOTYPES

int tjRead( int x, int y, int fore, int back, char *readme, int num );
// Read a string from the user.
