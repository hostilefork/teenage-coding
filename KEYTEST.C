/*
 * Archival code written when I was 15, see blog about it:
 *     http://hostilefork.com/2013/09/16/manykeys-my-open-source-roots/
 */



/*
   KEYTEST.C
   by Brian Dickens

   Demonstration of the multikey capabilities of MANYKEYS.

   Gets stressed out at 9 keys or so, sometimes fewer, sometimes more.
   Limitation of the keyboard hardware.

   Quit by pressing escape.

   COMPILE USING:
      (compiler name) keytest.c manykeys.c
*/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "manykeys.h"

void printpressed()
{
   int index;
   static char keystring[80] = "";
   char newstring[80];
   char *dest;

   strcpy( newstring, "" );
   dest = newstring;

   for( index = 0; index < 128; index++ )
      if( ispressed( index ) )
      {
         sprintf( dest, "[%s]", keyname( index ) );
         dest += strlen( dest );
      }

   if( strcmp( newstring, keystring ) )
   {
      strcpy( keystring, newstring );
      clrscr();
      puts( newstring );
   }
}

int main()
{
   printf( "MANYKEYS demonstration program - by Brian Dickens\n" );
   printf( "hit ESCAPE to quit, any other key combination to demonstrate.\n" );
   manykeyson( );

   clearallcounts( );
   while( !presscount( ESCAPE ) )
      printpressed();

   manykeysoff( );
   return( 0 );
}
