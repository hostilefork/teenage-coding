/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJKEYS.CPP
//---
// Thomas Jefferson Software Development Group Library
// Keyboard Input Routines Source File
//---
// History:
//    07/07/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Uses the TechnoJock toolkit system of translating double key sequences
//    into a single key.  Defines the new "key" type, which is basically an
//    unsigned character.  (easier to type "key").
//===

#include <conio.h>
#include "tjkeys.h"

int tjKeyIn( key ch, key Lo, key Hi )
// Check to see if a key is in a particular range.
{
   return( ( Lo <= ch ) && ( ch <= Hi ) );
}

int tjVideoKey( key ch )
// Check to see if the key pressed has a video character.
{
   return( tjKeyIn( ch, 32, 126 ) );
}

key tjGetKey( )
// Get a key from the keyboard, return its keycode.
{
   key ch = getch( );
   if( ch )
      return( ch );

   ch = getch( );
   if( tjKeyIn( ch,15,25 ) || tjKeyIn( ch,30,38 ) || tjKeyIn( ch,44,50 ) ||
       tjKeyIn( ch,59,68 ) || tjKeyIn( ch,71,73 ) || tjKeyIn( ch,75,75 ) ||
       tjKeyIn( ch,77,77 ) || tjKeyIn( ch,79,127 ) )
      ch += 128;
   if( tjKeyIn( ch,128,140 ) )
      ch += 6;

   return( ch );
}
