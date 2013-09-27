/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */

 

//===
// Inquiry/G 2.01
//---
// Revision History:
//    1.00  - No graphics, no multiple choice.
//    1.01  - No graphics, multiple choice added
//    1.10  - No graphics, picklist of label regions added
//       SUBMITTED TO THOMAS JEFFERSON SPANISH HONOR SOCIETY
//    2.00a - Added support of graphics
//       DEMONSTRATED AT GWU
//   *2.01b - Larger letters, reworking of graphic display logic
//   *2.01  - Integrated editor added, library construction.
//---
// * - Do not yet exist.
//===

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <dir.h>
#include <alloc.h>
#include <flexlist.h>
// Standard libraries required

#include <gxlib.h>
#include <pcxlib.h>
#include <grlib.h>
#include <fastgraf.h>
int retcode;
// GENUS microprogramming graphics libraries

#define fxUP 1
#define fxDOWN 2
#define fxLEFT 4
#define fxRIGHT 8

#define gxTYPE gxEGA_10
#define MAXROWS 20
#define MAXCOLS 37
#define TEXTUX (41*8+4)
#define TEXTUY (2*12)+2
#define TITLEUX (43*8)
#define TITLEUY (8)
#define TITLECOLS (32)
GXHEADER *Arrows[2];

char leftgraf[13] = { 0 }, rightgraf[13] = { 0 };
char leftlib[80] = { 0 }, rightlib[80] = { 0 };
char syspath[80];

#include "tjkeys.h"
#include "tjpick.h"
#include "tjread.h"
#include "tjfast.h"
#include "inqgraf.hpp"
#include "fileio.hpp"

int main( int argc, char *argv[] )
//===
// Main function of Inquiry/G
//===
{
   int Index;
   strcpy( syspath, argv[0] );
   *(strrchr( syspath, '.' )) = 0;
   strcat( syspath, ".DAT" );

   fg_setmode( fg_bestmode( 640, 350, 1 ) );
   retcode = gxSetDisplay( gxTYPE );
   retcode = gxSetMode( gxGRAPHICS );

   retcode = grSetColor( grBLACK );
   retcode = grDrawLine( 1, 1, 10, 10 );
   retcode = grSetBkColor( grWHITE );

   randomize( );
// Initialization routines

   int done = 0;
   str temp;

   LoadHalf( "TBOX.PCC", fxRIGHT, syspath );

   while( !done )
   {
      done = ShowMenu( temp );
      if( !done )
      {
         if( OfferIndex( temp ) )
         {
         }
      }
   }

   retcode = gxSetMode( gxTEXT );
   return( 0 );
// Closeup routines
}
