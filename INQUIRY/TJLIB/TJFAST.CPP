/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJFAST.CPP
//---
// Thomas Jefferson Software Development Group Library
// Fast Video Routines Source File
//---
// History:
//    07/06/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Right now, this makes use of the Fastgraph library routines, and thus
//    can never be distributed in its current form.  However, this is the only
//    one of the library files that accesses Fastgraph routines, and as soon
//    as I get a chance to write some of my own, the dependent libraries will
//    still work and be completely PD.  K-Rad!
//===

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <fastgraf.h>
#include "tjfast.h"

scr *winstack = NULL;
int oldmode;

void tjClose( )
// Close up the tj system.
{
   while( tjRmWin( ) );
   textmode( oldmode );
   textcolor( LIGHTGRAY );
   textbackground( BLACK );
   clrscr( );
   printf( "Copyright (c) 1991 ADinfinitum.  All rights reserved.\n" );
}

int tjScrHeight( )
// Returns the screen height (in characters).
{
   text_info ti;
   gettextinfo( &ti );
   return( ti.screenheight );
}

int tjScrWidth( )
// Returns the screen width (in characters).
{
   text_info ti;
   gettextinfo( &ti );
   return( ti.screenwidth );
}

int tjGetMode( )
// Gets the current text mode.
{
   text_info ti;
   gettextinfo( &ti );
   return( ti.currmode );
}

void tjSetMode( int mode )
// Sets the text mode.
{
   textmode( mode );
   fg_setmode( -1 );
   textmode( mode );
}

void tjInit( )
// Initialize the tj system.
{
   tjSetMode( oldmode = tjGetMode( ) );
   clrscr( );
}

int tjColorMode( )
// Is the current mode in color?
{
   int temp = tjGetMode( );

   return( (temp == C80) || (temp == C40) || (temp == C4350) );
}

scr *tjSaveScr( int ux, int uy, int lx, int ly )
// Save a region of the screen.
{
   scr *temp = new( scr );
   // Allocate new screen.

   if( temp == NULL )
      return( NULL );
   // Not enough memory for variable!

   temp->ux = ux;
   temp->uy = uy;
   temp->rows = ly-uy+1;
   temp->cols = lx-ux+1;
   // Store dimensions for later use.

   temp->buffer = new( char[temp->rows * temp->cols * 2] );
   // Allocate buffer with requested size.

   if( temp->buffer == NULL )
   {
      delete( temp );
      return( NULL );
   }
   // Not enough memory for buffer!

   if( gettext( ux, uy, lx, ly, temp->buffer ) )
      return( temp );
   // Success.

   delete( temp->buffer );
   delete( temp );
   return( NULL );
   // Miscellaneous error with gettext!
}

int tjRestScr( scr *restore, int ux, int uy )
// Restore a region of the screen.
{
   if( restore == NULL )
      return( 0 );
   // Screen does not exist!

   if( restore->buffer == NULL )
      return( 0 );
   // Nothing in buffer!

   if( puttext( ux, uy, ux+restore->cols-1, uy+restore->rows-1, restore->buffer ) )
      return( 1 );
   // Success.

   return( 0 );
   // Failure!
}

int tjKillScr( scr *kill )
// Kill a screen buffer once allocated.
{
   if( kill == NULL )
      return( 1 );

   if( kill->buffer == NULL )
   {
      delete( kill );
      return( kill == NULL );
   }

   delete( kill->buffer );
   delete( kill );
   return( kill == NULL );
}

void tjStrRep( char *str, char ch, int num )
// Make a repeated character string.
{
   int index;

   for( index = 0; index < num; index++ )
      str[index] = ch;
   str[num] = 0;
}

void tjWriteAt( int x, int y, int fore, int back, char *str )
// Write a string at a specific x and y location in a specific color.
{
   fg_locate( y-1, x-1 );
   fg_setattr( fore, back, 0 );
   fg_text( str, strlen( str ) );
}

void tjPlainWrite( int x, int y, char *str )
// Write a string without disturbing the attributes underneath.
{
   fg_locate( y-1, x-1 );
   fg_chgtext( str, strlen( str )-1 );
}

void tjAttrWrite( int x, int y, int fore, int back, int num )
// Write an attribute a specified number of times without disturbing chars.
{
   fg_setattr( fore, back, 0 );
   fg_locate( y-1, x-1 );
   fg_chgattr( num-1 );
}

void tjWriteChar( int x, int y, int fore, int back, char ch )
// Put a character at a specific x and y location.
{
   char str[2] = { ch, 0 };

   tjWriteAt( x, y, fore, back, str );
}

void tjPutChar( int x, int y, char ch )
// Put a character at x, y but don't disturb attribute.
{
   char str[2] = { ch, 0 };

   tjPlainWrite( x, y, str );
}

void tjPutAttr( int x, int y, int fore, int back )
// Put an attribute at x, y but don't disturb character.
{
   tjAttrWrite( x, y, fore, back, 1 );
}

void tjFill( int ux, int uy, int lx, int ly, char ch, int fore, int back )
// Fill an area with a single character and attribute.
{
   int index;
   char *str = new( char[lx-ux+2] );

   tjStrRep( str, ch, lx-ux+1 );

   for( index = uy; index < ly+1; index++ )
      tjWriteAt( ux, index, fore, back, str );

   delete( str );
}

void tjFillChar( int ux, int uy, int lx, int ly, char ch )
// Fill a region with text characters, leaving attributes undisturbed.
{
   int index;
   char *str = new( char[lx-ux+2] );

   tjStrRep( str, ch, lx-ux+1 );

   for( index = uy; index < ly+1; index++ )
      tjPlainWrite( ux, index, str );

   delete( str );
}

void tjFillAttr( int ux, int uy, int lx, int ly, int fore, int back )
// Fill a region with an attribute, leaving the characters undisturbed.
{
   int index;

   for( index = uy; index < ly+1; index++ )
      tjAttrWrite( ux, index, fore, back, lx-ux+1 );
}

void tjBox( int ux, int uy, int lx, int ly, int fore, int back, frame style )
// Make an innocent-looking empty box on the screen, don't store underneath.
{
   int index;
   char *str = new( char[lx-ux+2] );

   tjStrRep( str, style[TOP], lx-ux+1 );
   str[0] = style[TOPLEFT];
   str[lx-ux] = style[TOPRIGHT];
   tjWriteAt( ux, uy, fore, back, str );

   for( index = uy+1; index < ly; index++ )
   {
      tjWriteChar( ux, index, fore, back, style[LEFT] );
      tjWriteChar( lx, index, fore, back, style[RIGHT] );
   }

   tjStrRep( str, style[BOTTOM], lx-ux+1 );
   str[0] = style[BOTTOMLEFT];
   str[lx-ux] = style[BOTTOMRIGHT];
   tjWriteAt( ux, ly, fore, back, str );

   delete( str );
}

void tjFBox( int ux, int uy, int lx, int ly, int fore, int back, frame style )
// Make a filled box on the screen, still don't store underneath.
{
   int index;
   char *str = new( char[lx-ux+2] );

   tjStrRep( str, style[TOP], lx-ux+1 );
   str[0] = style[TOPLEFT];
   str[lx-ux] = style[TOPRIGHT];
   tjWriteAt( ux, uy, fore, back, str );

   tjStrRep( str, ' ', lx-ux+1 );
   str[0] = style[LEFT];
   str[lx-ux] = style[RIGHT];

   for( index = uy+1; index < ly; index++ )
      tjWriteAt( ux, index, fore, back, str );

   tjStrRep( str, style[BOTTOM], lx-ux+1 );
   str[0] = style[BOTTOMLEFT];
   str[lx-ux] = style[BOTTOMRIGHT];
   tjWriteAt( ux, ly, fore, back, str );

   delete( str );
}

int tjMkWin( int ux, int uy, int lx, int ly )
// Make a window, store contents in the winstack, etc.
{
   scr *temp = tjSaveScr( ux, uy, lx, ly );

   if( temp == NULL )
      return( 0 );

   temp->next = winstack;
   winstack = temp;
   return( 1 );
}

int tjRmWin( )
// Remove a window, redisplay winstack contents.
{
   if( winstack == NULL )
      return( 0 );

   scr *temp = winstack;
   winstack = winstack->next;

   return( tjRestScr( temp, temp->ux, temp->uy ) );
}

int tjBoxWin( int ux, int uy, int lx, int ly, int fore, int back, frame style )
// Make a window with a box, store contents in the winstack, etc.
{
   if( tjMkWin( ux, uy, lx, ly ) )
   {
      tjBox( ux, uy, lx, ly, fore, back, style );
      return( 1 );
   }
   return( 0 );
}

int tjFBoxWin( int ux, int uy, int lx, int ly, int fore, int back, frame style )
// Make a window with a filled box, store contents in the winstack, etc.
{
   if( tjMkWin( ux, uy, lx, ly ) )
   {
      tjFBox( ux, uy, lx, ly, fore, back, style );
      return( 1 );
   }
   return( 0 );
}

void tjTitle( int ux, int uy, int lx, int ly, int tfore, int tback, char *title,
              position titlepos )
// Write a title at one of six positions.
{
   char *temp = new( char[lx-ux] );
   int index;
   for( index = 0; index < lx-ux; index++ )
      temp[index] = title[index];
   temp[lx-ux-1] = 0;

   switch( titlepos )
   {
      case TOPLEFT:
         tjWriteAt( ux+1, uy, tfore, tback, temp );
         break;

      case TOP:
         tjWriteAt( ux+((lx-ux)-strlen(temp))/2+1, uy, tfore, tback, temp );
         break;

      case TOPRIGHT:
         tjWriteAt( lx-strlen( temp ), uy, tfore, tback, temp );
         break;

      case BOTTOM:
         tjWriteAt( ux+((lx-ux)-strlen(temp))/2+1, ly, tfore, tback, temp );
         break;

      case BOTTOMLEFT:
         tjWriteAt( ux+1, ly, tfore, tback, temp );
         break;

      case BOTTOMRIGHT:
         tjWriteAt( lx-strlen( temp ), ly, tfore, tback, temp );
         break;
   }

   delete( temp );
}

int tjBoxWinTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                   int tfore, int tback, char *title, position titlepos )
// Make a window with a box, store contents, put a title.
{
   if( !tjBoxWin( ux, uy, lx, ly, fore, back, style ) )
      return( 0 );

   tjTitle( ux, uy, lx, ly, tfore, tback, title, titlepos );
   return( 1 );
}

int tjFBoxWinTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                    int tfore, int tback, char *title, position titlepos )
// Make a window with a filled box, store contents, put a title.
{
   if( !tjFBoxWin( ux, uy, lx, ly, fore, back, style ) )
      return( 0 );

   tjTitle( ux, uy, lx, ly, tfore, tback, title, titlepos );
   return( 1 );
}

void tjBoxTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                 int tfore, int tback, char *title, position titlepos )
// Make a window with a box, DO NOT store contents, put a title.
{
   tjBox( ux, uy, lx, ly, fore, back, style );
   tjTitle( ux, uy, lx, ly, tfore, tback, title, titlepos );
}

void tjFBoxTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                  int tfore, int tback, char *title, position titlepos )
// Make a window with a filled box, DO NOT store contents, put a title.
{
   tjFBox( ux, uy, lx, ly, fore, back, style );
   tjTitle( ux, uy, lx, ly, tfore, tback, title, titlepos );
}
