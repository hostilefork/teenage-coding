/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJREAD.CPP
//---
// Thomas Jefferson Software Development Group Library
// String Reading Routines Source File
//---
// History:
//    07/07/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Editing routines are somewhat rudimentary.  Perhaps I'll modify them
//    later.  Short time and all, you know.
//===

#include <conio.h>
#include <string.h>
#include "tjfast.h"
#include "tjkeys.h"
#include "tjpick.h"
#include "tjread.h"

int tjRead( int x, int y, int fore, int back, char *readme, int num )
// Read a string from the user.
{
   key ch;
   int done = 0;
   int pos = 0;
   int index;

   tjStrPad( readme, num );

   do
   {
      tjWriteAt( x, y, fore, back, readme );
      gotoxy( x+pos, y );
      ch = tjGetKey( );
      switch( ch )
      {
         case Enter:
         case Esc:
            done = 1;

         case LArr:
            if( pos > 0 )
               pos--;
            break;

         case RArr:
            if( pos < num-1 )
               pos++;
            break;

         case Del:
            if( pos < num-1 )
            {
               pos++;
               strcpy( &readme[pos-1], &readme[pos] );
               readme[num-1] = ' ';
               readme[num] = 0;
               pos--;
            }
            break;

         case Home:
            pos = 0;
            break;

         case End:
            pos = 0;
            for( index = num-1; index > 0; index-- )
               if( readme[index-1] != ' ' )
               {
                  pos = index;
                  index = 1;
               }
            break;

         case BackSp:
            if( pos > 0 )
            {
               strcpy( &readme[pos-1], &readme[pos] );
               readme[num-1] = ' ';
               readme[num] = 0;
               pos--;
            }
            break;

         default:
            if( pos < num-1 )
            {
               for( index = num-2; index >= pos; index-- )
                  readme[index+1] = readme[index];
               readme[pos] = ch;
               readme[num] = 0;
               readme[num-1] = ' ';
               pos++;
            }
            break;
      }
   } while( !done );

   return( ch == Enter );
}
