/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJPICK.CPP
//---
// Thomas Jefferson Software Development Group Library
// Pick List Routines Source File
//---
// History:
//    07/07/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Innovative system places no limitations on the length of the list, or
//    the functionality.  Can be used as a basis for anything from a menu
//    system to a directory box!  (And believe me, it is!)  Again, it relies
//    on a commercial library (Flexlist).  Oh well.  Maybe someday I'll write
//    my own stuff.  No time right now.
//===

#include <dir.h>
#include <string.h>
#include <flexlist.h>
#include "tjfast.h"
#include "tjkeys.h"
#include "tjpick.h"

void tjStrPad( char *PadMe, int width )
{
   int index;

   for( index = strlen( PadMe ); index < width; index++ )
      PadMe[index] = ' ';
   PadMe[width] = 0;
}

void strdispfun( tjPickOpts Opts, int linenum )
// Just a string display function, in case you need it!
{
   char *item = new( char[Opts.width+1] );

   if( rcld( Opts.PickMe, item, linenum ) )
   {
      tjStrPad( item, Opts.width );
      if( linenum == Opts.current )
         tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.hifore, Opts.hiback, item );
      else
         tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, item );
   }
   else
   {
      strcpy( item, "" );
      tjStrPad( item, Opts.width );
      tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, item );
   }
   delete( item );
}

int tjPick( tjPickOpts &Opts )
// Does a pick list of items, using the options set in the given options.
{
   key ch;
   int done = 0;
   int index;
   int redraw = 1;

   do
   {
      if( Opts.current < Opts.topline )
      {
         Opts.topline = Opts.current;
         redraw = 1;
      }

      if( Opts.current > Opts.topline+Opts.lines-1 )
      {
         Opts.topline = Opts.current-Opts.lines+1;
         redraw = 1;
      }

      if( redraw )
      {
         redraw = 0;
         for( index = Opts.topline; index < Opts.topline+Opts.lines; index++ )
            Opts.dispfun( Opts, index );
      }

      ch = tjGetKey( );
      switch( ch )
      {
         case Esc:
            done = 1;
            break;

         case Enter:
            done = 1;
            break;

         case UArr:
            if( Opts.current > 1 )
            {
               redraw = 1;
               Opts.current--;
            }
            break;

         case DArr:
            if( Opts.current < nempty( Opts.PickMe ) )
            {
               redraw = 1;
               Opts.current++;
            }
            break;

         default:
            if( Opts.keyhan != NULL )
               done = Opts.keyhan( Opts, ch );
            break;
      }
   } while( !done );
   Opts.lastkey = ch;
   if( ch != Esc )
      return( Opts.current );
   else
      return( 0 );
}

int tjDirPick( char filename[13], char *mask, tjPickOpts Opts )
// Pick a directory entry.  Conveeeenient.
{
   ffblk ffblk;
   int done;
   Opts.PickMe = mkFlist( (size_t) 13, 0 );
   Opts.width = 12;
   Opts.topline = Opts.current = 1;
   Opts.dispfun = &strdispfun;

   done = findfirst( mask, &ffblk, 0);
   while (!done)
   {
      iquedptr( Opts.PickMe, &ffblk.ff_name );
      done = findnext(&ffblk);
   }

   if( done = tjPick( Opts ) )
   {
      rcld( Opts.PickMe, filename, done );
      rmFlist( &Opts.PickMe );
      return( 1 );
   }
   else
   {
      strcpy( filename, "" );
      rmFlist( &Opts.PickMe );
      return( 0 );
   }
}
