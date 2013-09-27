/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



// Simple Basic Editing routines.

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <flexlist.h>
#include <fastgraf.h>
#include <mem.h>
#include <stdlib.h>
#include <dir.h>
#include "ctype.h"
#include "gxlib.h"
#include "pcxlib.h"
#include "tjfast.h"
#include "tjkeys.h"
#include "tjpick.h"
#include "tjread.h"
#include "efileio.hpp"

int viewhan( tjPickOpts Opts, key ch )
{
   str temp;

   if( (ch == 'v') || (ch == 'V') )
   {
      tjMkWin( 1, 1, 80, 25 );
      gxSetDisplay( gxEGA_10 );
      gxSetMode( gxGRAPHICS );
      rcld( Opts.PickMe, temp, Opts.current );
      pcxFileDisplay( temp, 0, 0, 0 );
      tjGetKey( );
      gxSetMode( gxTEXT );
      tjInit( );
      tjRmWin( );
   }
   return( 0 );
}

void DeleteLine( Flist DelMe, int LineNum )
{
   mkcdptr( DelMe, LineNum );
   delete( deln( DelMe ) );
}

void InsertLine( Flist InsertMe, int LineNum, str Thing )
// Inserts AFTER specified line
{
   mkcdptr( InsertMe, LineNum );
   insdptr( InsertMe, Thing );
}

int FirstWord( char *CheckMe, char *StoreInMe )
//===
// Extracts the first word from a string.
//---
// Returns:
//    Length of the string.
//===
{
   int Index;

   if( strlen( CheckMe ) )
   {
      strcpy( StoreInMe, CheckMe );
      for( Index=1; (Index < strlen(CheckMe)) && (CheckMe[Index] != ' ') && (CheckMe[Index] != 0); Index++ );
      if( CheckMe[Index] )
      {
         strcpy( CheckMe, &CheckMe[Index+1] );
         StoreInMe[Index+1] = 0;
      }
      else
         strcpy( CheckMe, "" );
   }
   else
      strcpy( StoreInMe, "" );

   return( strlen( StoreInMe ) );
}

int LastWord( char *CheckMe, char *StoreInMe )
//===
// Extracts the last word from a string.
//---
// Returns:
//    Length of the string.
//===
{
   int Index;

   if( strlen( CheckMe ) )
   {
      strcpy( StoreInMe, CheckMe );
      for( Index=strlen( CheckMe )-1; (Index > 0) && (CheckMe[Index] != ' ') && (CheckMe[Index] != 0); Index-- );
      if( Index )
      {
         strcpy( StoreInMe, &StoreInMe[Index+1] );
         CheckMe[Index+1] = 0;
      }
      else
         strcpy( StoreInMe, "" );
   }
   else
      strcpy( StoreInMe, "" );

   return( strlen( StoreInMe ) );
}


struct Edit_Options
{
   int ux, uy;          // upper x and y locations of the editing window.
   int lines;           // number of lines to edit.
   int max;             // maximum number of characters per line.
   int insert;          // is insert mode on?
   int current;         // current line number.
   int topline;         // top line in window.
   int pos;             // position in line.
   int fore, back;      // foreground and background of text.
   int wfore, wback;    // foreground and background of window.
   int tfore, tback;    // foreground and background of window title.
   str title;           // window title.
   Flist EditMe;        // flexlist to edit.
   Edit_Options *next;  // what window to go to next...
   int permitnum;       // permit weird number entry.
   int choice;          // # o' choices
   int maxlines;        // # o' lines
   int titled;          // titled
};

Edit_Options    LectureText,            // Full left half, blue.
                LecturePics,            // Full right half, purple
                ShortText,              // Full left half, blue, same as MultText
                ShortPics,              // Small 2-line, purple, same as MultPics
                ShortAnswer,            // Rest of right, green
                MultAnswer,             // Half of (right-2)... green
                MultTrick,              // Other Half of (right-2)... red.
                Description;            // Limited edit region... for database descriptions.

void Display_Lines( Edit_Options *Opts )
//===
// Redraws the current viewfield.
//===
{
   int Index;
   char temp[MAX+1];

   fg_cursor( 0 );
   for( Index = Opts->topline; Index < Opts->topline+Opts->lines; Index++ )
   {
      if( !rcld( Opts->EditMe, temp, Index ) )
         strcpy( temp, "" );
      tjStrPad( temp, MAX );
      tjWriteAt( Opts->ux, Opts->uy+(Index-Opts->topline), Opts->fore, Opts->back, temp );
   }
   fg_cursor( 1 );
}

key Edit_Flist( Edit_Options *Opts )
//===
// Edit a flexlist in a rather word-processorish fashion.
// No blocking yet.
//---
// Returns:
//    The key pressed that exited editing.
//===
{
   key ch;
      // key pressed by user
   int Done = 0, Redraw = 1;
      // Flow control flags.
   str item, temp, next, prev;
      // item is usually the current line, and temp is usually the prev or next.
   int Index;
      // general purpose index.
   Flist keybuf = mkFlist( sizeof( key ), 0 );
      // key buffer... used for several important commands.

   Edit_Options *loop;

   loop = Opts;
   do
   {
      ch = AltW;
      iquedptr( keybuf, &ch );
      while( nempty( loop->EditMe ) < 2 )
      {
         strcpy( temp, "" );
         iquedptr( loop->EditMe, &temp );
      }
      loop = loop->next;
   }
   while( loop != Opts );

   do
   {
      if( !nempty( keybuf ) )
      {
         if( Opts->current > Opts->maxlines+1 )
            Opts->current = Opts->maxlines+1;
         if( Opts->current < Opts->topline )
            Redraw = Opts->topline = Opts->current;
         if( Opts->current > Opts->topline+Opts->lines-1 )
            Redraw = Opts->topline = Opts->topline+(Opts->current-Opts->topline-Opts->lines+1);
         if( Redraw )
         {
            Redraw = 0;
                // Draw window
            Display_Lines( Opts );
         }
         gotoxy( Opts->ux+Opts->pos, Opts->uy+(Opts->current-Opts->topline) );
            // Screen redraw and repositioning routines, only if there are no
            // keys left to process.  Would make things look choppy...
      }

      if( nempty( keybuf ) )
         popd( keybuf, &ch );
      else
         ch = tjGetKey( );
         // Get a key, or as the case may be, pull a key off the buffer stack.

      rcld( Opts->EditMe, item, Opts->current );

      switch( ch )
      {
         case AltW:
            tjBoxTitle( Opts->ux-1, Opts->uy-1, Opts->ux+Opts->max+1, Opts->uy+Opts->lines,
                  Opts->wfore, Opts->wback, BDOUBLE,
                  Opts->wfore, Opts->tback, Opts->title, TOP );
            if( Opts->permitnum )
            {
               if( Opts->choice > 0 )
                  itoa( Opts->choice, temp, 10 );
               else
                  strcpy( temp, "*" );

               tjStrPad( temp, 2 );
               tjWriteAt( Opts->ux+Opts->max-1, Opts->uy-1, Opts->wfore, Opts->wback, temp );
            }
            Display_Lines( Opts );
            Redraw = 1;
            Opts = Opts->next;
            tjBoxTitle( Opts->ux-1, Opts->uy-1, Opts->ux+Opts->max+1, Opts->uy+Opts->lines,
                           Opts->wfore, Opts->wback, BDOUBLE,
                           Opts->tfore, Opts->tback, Opts->title, TOP );
            if( Opts->permitnum )
            {
               if( Opts->choice > 0 )
                  itoa( Opts->choice, temp, 10 );
               else
                  strcpy( temp, "*" );

               tjStrPad( temp, 2 );
               tjWriteAt( Opts->ux+Opts->max-1, Opts->uy-1, YELLOW, Opts->wback, temp );
            }

            break;

         case F10:
            Done = 1;
            break;

         case Enter:            // Split line.
            if( Opts->pos < strlen( item ) )
            {
               strcpy( temp, &item[Opts->pos] );
               item[Opts->pos] = 0;
               stod( Opts->EditMe, item, Opts->current );
            }
            else
               strcpy( temp, "" );

            InsertLine( Opts->EditMe, Opts->current, temp );
            Redraw = 1;
            Opts->current++;
            Opts->pos = 0;
            break;

         case AltF:                     // Insert filename in text.
            fg_cursor( 0 );
            tjPickOpts pOpts;
            pOpts.ux = 30;
            pOpts.uy = 11;
            pOpts.lines = 8;
            pOpts.fore = LIGHTGRAY;
            pOpts.back = BLACK;
            pOpts.hifore = WHITE;
            pOpts.hiback = CYAN;
            pOpts.width = 12;
            pOpts.keyhan = &viewhan;
            tjFBoxWinTitle( 25, 9, 46, 21, BLACK, LIGHTGRAY, DOUBLE, WHITE, LIGHTGRAY,
                            " Choose a picture ", TOP );

            tjBox( pOpts.ux-1, pOpts.uy-1, pOpts.ux+pOpts.width, pOpts.uy+pOpts.lines,
                   BLACK, LIGHTGRAY, SINGLE );
            tjWriteAt( 27, 20, DARKGRAY, LIGHTGRAY, "V)iew PCX/PCC file" );

            if( tjDirPick( temp, "*.PC?", pOpts ) )
            {
               for( Index = 0; Index < strlen( temp ); Index++ )
               {
                  ch = temp[Index];
                  iquedptr( keybuf, &ch );
               }
               ch = Enter;
               iquedptr( keybuf, &ch );
            }
            tjRmWin( );
            fg_cursor( 1 );
            break;

         case AltN:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 164;
            iquedptr( keybuf, &ch );
            break;

         case AltA:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 160;
            iquedptr( keybuf, &ch );
            break;

         case AltI:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 161;
            iquedptr( keybuf, &ch );
            break;

         case AltO:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 162;
            iquedptr( keybuf, &ch );
            break;

         case AltE:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 130;
            iquedptr( keybuf, &ch );
            break;

         case AltU:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 163;
            iquedptr( keybuf, &ch );
            break;

         case AltQ:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 168;
            iquedptr( keybuf, &ch );
            break;

         case AltM:
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 173;
            iquedptr( keybuf, &ch );
            break;

         case Esc:              // Escape hit, exit.
            Done = AbandonChanges( );
            break;

         case Del:
            Opts->pos++;
            ch = BackSp;
            iquedptr( keybuf, &ch );
            break;

         case Ins:              // Toggle Insert mode.
            Opts->insert = !Opts->insert;
            break;

         case UArr:             // Move cursor up.
            if( Opts->current > 2 )
               Opts->current--;
            break;

         case DArr:            // Move cursor down.
            if( Opts->current < nempty( Opts->EditMe ) )
               Opts->current++;
            break;

         case CtrlLArr:         // Go a word left.
            if( (Opts->pos == 0) && (Opts->current != 2) )
            {
               Opts->current--;
               Opts->pos = Opts->max-1;
               ch = CtrlLArr;
               iquedptr( keybuf, &ch );
            }
            else
               if( Opts->pos > 0 )
               {
                  if( Opts->pos > strlen( item ) )
                     Opts->pos = strlen( item );
                  for( Index = Opts->pos-1; (Index > 0) &&
                       !((item[Index-1] == ' ') &&
                       (item[Index] != ' ')); Index-- );
                  if( Index < 0 )
                     Opts->pos = 0;
                  else
                     Opts->pos = Index;
                  if( (Opts->pos == 0) && ((item[0] == ' ') || (item[0] == 0)))
                  {
                     ch = CtrlLArr;
                     iquedptr( keybuf, &ch );
                  }
               }
            break;

         case CtrlRArr:         // Go a word right.
            if( (Opts->pos == Opts->max-1) && (Opts->current < nempty( Opts->EditMe )) )
            {
               Opts->current++;
               Opts->pos = 0;
               rcld( Opts->EditMe, item, Opts->current );
               if( (item[0] == ' ') || (item[0] == 0) )
               {
                  ch = CtrlRArr;
                  iquedptr( keybuf, &ch );
               }
            }
            else
               if( Opts->pos < Opts->max-1 )
               {
                  for( Index = Opts->pos+1; (Index < Opts->max ) &&
                       !((item[Index-1] == ' ') &&
                       (item[Index] != ' ')); Index++ );
                  if( Index > Opts->max-1 )
                     Opts->pos = Opts->max-1;
                  else
                     Opts->pos = Index;
                  if( Opts->pos >= strlen( item ) )
                     if( Opts->current == nempty( Opts->EditMe ) )
                        Opts->pos = strlen( item );
                     else
                        Opts->pos = Opts->max-1;
                  if( Opts->pos == Opts->max-1 )
                  {
                     ch = CtrlRArr;
                     iquedptr( keybuf, &ch );
                  }
               }
            break;

         case AltT:            // Enter a title.
            if( Opts->titled != 3 )
            {
               Edit_Options *loop = Opts;

               while( !loop->titled )
                 loop = loop->next;

               rcld( loop->EditMe, item, 1 );
               tjWriteAt( 1, 1, RED, LIGHTGRAY, " Enter a title:                                                                 " );
               if( tjRead( 20, 1, BLACK, LIGHTGRAY, &item[1], 33 ) )
                  stod( loop->EditMe, item, 1 );
               else
                  rcld( loop->EditMe, item, 1 );
               tjWriteAt( 1, 1, RED, LIGHTGRAY, "                                            F10 - Save changes   Esc - Abandon  " );
               tjWriteAt( 2, 1, BLACK, LIGHTGRAY, &item[1] );
            }
            break;

         case CtrlP:            // Reformat paragraph.
            int finished = 0;
            int store;

            store = Opts->current;
            Redraw = 1;

            while( !finished )
            {
               if( (Opts->current < nempty( Opts->EditMe )) || !strlen( item ) )
               {
                  rcld( Opts->EditMe, item, Opts->current );
                  rcld( Opts->EditMe, next, Opts->current+1 );

                  if( (strlen( item ) == 0) )
                     finished = 1;
                  else
                  {
                     while( (FirstWord( next, temp )+strlen( item ) <
                            Opts->max) && (strlen( temp ) > 0) )
                     {
                        strcat( item, temp );
                        stod( Opts->EditMe, item, Opts->current );
                        if( strlen( next ) == 0 )
                        {
                           DeleteLine( Opts->EditMe, Opts->current+1 );
                           if( !rcld( Opts->EditMe, next, Opts->current+1 ) )
                           {
                              finished = 1;
                              strcpy( next, "" );
                           }
                        }
                        else
                           stod( Opts->EditMe, next, Opts->current+1 );
                     }
                     Opts->current++;
                  }
               }
               else
                  finished = 1;
            }
            Opts->current = store;
            break;

         case PgUp:
            Redraw = 1;
            Opts->current -= Opts->lines-1;
            Opts->topline -= Opts->lines-1;
            if( Opts->topline < 2 )
            {
               Opts->topline = 2;
               Opts->current = 2;
            }
            break;

         case PgDn:
            Redraw = 1;
            Opts->topline += Opts->lines-1;
            Opts->current += Opts->lines-1;
            if( Opts->topline+Opts->lines-1 > nempty( Opts->EditMe ) )
            {
               Opts->topline = nempty( Opts->EditMe )-Opts->lines+1;
               Opts->current = nempty( Opts->EditMe );
            }
            if( Opts->topline < 2 )
            {
               Opts->topline = 2;
               Opts->current = nempty( Opts->EditMe );
            }
            break;

         case AltB:             // Insert a bullet.
            ch = CtrlG;
            iquedptr( keybuf, &ch );
            ch = 7;
            iquedptr( keybuf, &ch );
            break;

         case AltC:             // Number to Choose when giving quiz.
            if( Opts->permitnum )
            {
               strcpy( temp, "" );
               tjMkWin( Opts->ux+Opts->max-2, Opts->uy-1, Opts->ux+Opts->max-2, Opts->uy-1 );
               if( tjRead( Opts->ux+Opts->max-2, Opts->uy-1, YELLOW,
                           Opts->wback, temp, 3 ) )
               tjRmWin( );
               Opts->choice = atoi( temp );
               if( Opts->choice > 0 )
                  itoa( Opts->choice, temp, 10 );
               else
                  strcpy( temp, "*" );

               tjStrPad( temp, 2 );
               tjWriteAt( Opts->ux+Opts->max-1, Opts->uy-1, YELLOW, Opts->wback, temp );
            }
            break;

         case AltL:             // Insert graphic line.
            Redraw = 1;
            strcpy( item, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴" );
            InsertLine( Opts->EditMe, Opts->current-1, item );
            break;

         case CtrlI:            // Insert 5 spaces.
            ch = ' ';
            for( Index = 0; Index < 5; Index++ )
               iquedptr( keybuf, &ch );
            break;

         case CtrlC:            // Center current line.
            Redraw = 1;
            for( Index = 0; Index < Opts->max; Index++ )
               temp[Index] = ' ';
            for( Index = 0; (Index < strlen( item )) && (item[Index] == ' '); Index++ );
            strcpy( item, &item[Index] );
            strcpy( &temp[(Opts->max-strlen(item))/2], item );
            strcpy( item, temp );
            stod( Opts->EditMe, item, Opts->current );
            break;

         case CtrlL:            // Left justify line.
            Redraw = 1;
            for( Index = 0; ( Index < strlen( item )) && (item[Index] == ' '); Index++ );
            strcpy( item, &item[Index] );
            stod( Opts->EditMe, item, Opts->current );
            break;

         case CtrlR:            // Right justify line.
            Redraw = 1;
            for( Index = 0; Index < Opts->max; Index++ )
               temp[Index] = ' ';
            for( Index = 0; (Index < strlen( item )) && (item[Index] == ' '); Index++ );
            strcpy( item, &item[Index] );
            strcpy( &temp[Opts->max-strlen(item)], item );
            strcpy( item, temp );
            stod( Opts->EditMe, item, Opts->current );
            break;

         case CtrlD:            // Delete current line.
            Redraw = 1;
            if( nempty( Opts->EditMe ) > 2 )
            {
               DeleteLine( Opts->EditMe, Opts->current );
               if( Opts->current > nempty( Opts->EditMe ) )
                  Opts->current--;
            }
            else
            {
               strcpy( item, "" );
               stod( Opts->EditMe, item, Opts->current );
            }
            break;

         case CtrlN:            // Insert a line prior to current line.
            Redraw = 1;
            strcpy( item, "" );
            InsertLine( Opts->EditMe, Opts->current-1, item );
            break;

         case LArr:             // Move cursor left.
            if( Opts->pos > 0 )
               Opts->pos--;
            break;

         case RArr:             // Move cursor right.
            if( Opts->pos < Opts->max-1 )
               Opts->pos++;
            break;

         case Home:             // Go to beginning of line.
            Opts->pos = 0;
            break;

         case End:              // Go to end of line.
            if( ( Opts->pos = strlen( item ) ) > Opts->max-1 )
               Opts->pos = Opts->max-1;
            break;

         case BackSp:           // Delete character to left of cursor
            if( Opts->pos == 0 )
               if( Opts->current != 2 )
               {
                  rcld( Opts->EditMe, prev, Opts->current-1 );
                  Opts->pos = strlen( prev );
                  while( (FirstWord( item, temp ) <
                         ( Opts->max-strlen( prev ) ) ) &&
                         ( strlen( temp ) > 0 ) )
                  {
                     stod( Opts->EditMe, item, Opts->current );
                     strcat( prev, temp );
                     stod( Opts->EditMe, prev, Opts->current-1 );
                  }
                  rcld( Opts->EditMe, item, Opts->current );
                  if( !strlen( item ) )
                     DeleteLine( Opts->EditMe, Opts->current );
                  Opts->current--;
                  Redraw = 1;
               }
               else;
            else
               if( Opts->pos > strlen( item ) )
                  Opts->pos--;
               else
               {
                  Redraw = 1;
                  Opts->pos--;
                  strcpy( &item[Opts->pos], &item[Opts->pos+1] );
                  stod( Opts->EditMe, item, Opts->current );
               }
            break;

         default:               // Put a character in line.
            if( (ch == CtrlG) || tjVideoKey( ch ) )
            {
            if( ch == CtrlG )
               popd( keybuf, &ch );
            Redraw = 1;
            if( Opts->insert )
               if( (strlen( item ) > Opts->max-1) && !(Opts->pos > Opts->max-1))
               {
                  int Index;

                  for( Index = Opts->pos; (Index < strlen( item ) ) && (item[Index] != ' '); Index++ );
                  if( Index == strlen( item ) )
                  {
                     if( LastWord( item, temp ) )
                     {
                        stod( Opts->EditMe, item, Opts->current );
                        Opts->pos = Opts->pos-strlen( item );
                        for( Index = strlen( temp )+1; Index > Opts->pos; Index-- )
                           temp[Index] = temp[Index-1];
                        temp[Opts->pos] = ch;
                        InsertLine( Opts->EditMe, Opts->current, temp );
                        Opts->pos++;
                        Opts->current++;
                     }
                     else
                     {
                        strcpy( temp, &item[Opts->pos] );
                        item[Opts->pos] = ch;
                        item[Opts->pos+1] = 0;
                        stod( Opts->EditMe, item, Opts->current );
                        InsertLine( Opts->EditMe, Opts->current, temp );
                     }
                  }
                  else
                  {
                     int SubIndex;

                     strcpy( temp, &item[Index+1] );
                     item[Index] = 0;
                     for( SubIndex = Index+1; SubIndex > Opts->pos; SubIndex-- )
                        item[SubIndex] = item[SubIndex-1];
                     item[Opts->pos] = ch;
                     stod( Opts->EditMe, item, Opts->current );
                     InsertLine( Opts->EditMe, Opts->current, temp );
                     Opts->pos++;
                  }
               }
               else
               {
                  if( Opts->pos > Opts->max-1 )
                  {
                     char catme[2];
                     catme[0] = ch;
                     catme[1] = 0;

                     Opts->pos = LastWord( item, temp )+1;
                     stod( Opts->EditMe, item, Opts->current );
                     strcat( temp, catme );
                     InsertLine( Opts->EditMe, Opts->current, temp );
                     Opts->current++;
                  }
                  else
                  {
                     int Index;

                     while( Opts->pos > strlen( item ) )
                        strcat( item, " " );
                     for( Index = strlen( item )+1; Index > Opts->pos; Index-- )
                        item[Index] = item[Index-1];
                     item[Opts->pos] = ch;
                     Opts->pos++;
                     stod( Opts->EditMe, item, Opts->current );
                  }
               }
            else
               if( Opts->pos > Opts->max-1 )
               {
                  char catme[2];
                  catme[0] = ch;
                  catme[1] = 0;

                  Opts->pos = LastWord( item, temp )+1;
                  stod( Opts->EditMe, item, Opts->current );
                  strcat( temp, catme );
                  InsertLine( Opts->EditMe, Opts->current, temp );
                  Opts->current++;
               }
               else
               {
                  while( Opts->pos >= strlen( item ) )
                    strcat( item, " " );
                  item[Opts->pos] = ch;
                  Opts->pos++;
                  stod( Opts->EditMe, item, Opts->current );
               }
            }
            break;
      }
   } while( !Done );

   while( nempty( Opts->EditMe ) > Opts->maxlines+1 )
      DeleteLine( Opts->EditMe, Opts->maxlines+2 );

   return( ch );
}

void InitOptions( )
{
   LectureText.ux = 2;
   LectureText.uy = 4;
   LectureText.lines = 21;
   LectureText.max = 37;
   LectureText.insert = 1;
   LectureText.pos = 0;
   LectureText.current = 2;
   LectureText.topline = 2;
   LectureText.fore = LIGHTGRAY;
   LectureText.back = BLACK;
   LectureText.wfore = LIGHTBLUE;
   LectureText.wback = BLUE;
   LectureText.tfore = WHITE;
   LectureText.tback = BLUE;
   LectureText.maxlines = 32760;
   LectureText.titled = 1;
   strcpy( LectureText.title, " Lecture Text " );
   LectureText.permitnum = 0;

   Description = LectureText;
   Description.ux = 42;
   Description.uy = 2;
   Description.fore = WHITE;
   Description.back = BLUE;
   Description.wfore = BLACK;
   Description.wback = LIGHTGRAY;
   Description.tfore = WHITE;
   Description.tback = RED;
   Description.maxlines = 19;
   Description.lines = 19;
   Description.titled = 3;
   strcpy( Description.title, " Long Database Description " );
   Description.next = &Description;

   ShortText = LectureText;
   strcpy( ShortText.title, " Question Text " );

   LecturePics = LectureText;
   LecturePics.titled = 0;
   LecturePics.ux = 42;
   strcpy( LecturePics.title, " Lecture Pictures " );
   LecturePics.wfore = LIGHTMAGENTA;
   LecturePics.wback = MAGENTA;
   LecturePics.tback = MAGENTA;

   ShortPics = LecturePics;
   ShortPics.uy = 23;
   ShortPics.lines = 2;
   strcpy( ShortPics.title, " Question Pictures " );

   ShortAnswer = LecturePics;
   ShortAnswer.lines = 17;
   ShortAnswer.wfore = LIGHTGREEN;
   ShortAnswer.wback = GREEN;
   ShortAnswer.tback = GREEN;
   ShortAnswer.permitnum = 1;
   ShortAnswer.choice = 0;
   strcpy( ShortAnswer.title, " Correct Answers " );

   MultAnswer = ShortAnswer;
   MultAnswer.lines = 7;

   MultTrick = MultAnswer;
   MultTrick.uy = 13;
   MultTrick.lines = 8;
   MultTrick.wfore = LIGHTRED;
   MultTrick.wback = RED;
   MultTrick.tback = RED;
   strcpy( MultTrick.title, " Deceptive (wrong!) Answers " );
}

void DoAnEdit( char *fname, char classification, int recno, int insertflag )
{
   int index;
   Flist temp;
   FILE *infile, *outfile;
   str title;
   str other;
   str value;

   tjFill( 1, 1, 80, 25, ' ', LIGHTGRAY, BLACK );

   if( !insertflag )
   {
      infile = fopen( fname, "rb" );
      for( index = 1; index < recno+1; index++ )
         SkipCryptFlist( infile );
      switch( classification )
      {
         case 'L':
            LoadCryptFlist( LectureText.EditMe, infile );
            LoadCryptFlist( LecturePics.EditMe, infile );
            rcld( LectureText.EditMe, title, 1 );
            break;

         case 'M':
            LoadCryptFlist( ShortText.EditMe, infile );
            LoadCryptFlist( MultAnswer.EditMe, infile );
            LoadCryptFlist( MultTrick.EditMe, infile );
            rcld( ShortText.EditMe, title, 1 );
            LoadCryptFlist( ShortPics.EditMe, infile );
            rcld( MultAnswer.EditMe, value, 1 );
            MultAnswer.choice = *(int*)(&value[2]);
            rcld( MultTrick.EditMe, value, 1 );
            MultTrick.choice = *(int*)(&value[2]);
           break;

         case 'S':
            LoadCryptFlist( ShortText.EditMe, infile );
            LoadCryptFlist( ShortAnswer.EditMe, infile );
            LoadCryptFlist( ShortPics.EditMe, infile );
            rcld( ShortText.EditMe, title, 1 );
            rcld( ShortAnswer.EditMe, value, 1 );
            ShortAnswer.choice = *(int*)(&value[2]);
            break;
      }
      fclose( infile );
   }
   else
   {
      strcpy( &title[1], "" );
      tjWriteAt( 1, 1, RED, LIGHTGRAY, " Enter a title:                                                                 " );
      fg_cursor( 1 );

      index = tjRead( 20, 1, BLACK, LIGHTGRAY, &title[1], 33 );
      strcpy( other, &title[1] );
      strform( other );
      if( !index || !strlen( other ) )
      {
         switch( classification )
         {
            case 'L' : strcpy( &title[1], "Lecture (no name)" ); break;
            case 'M' : strcpy( &title[1], "Multiple Choice (no name)" ); break;
            case 'S' : strcpy( &title[1], "Short Answer (no name)" ); break;
         }
      }
      title[0] = classification;
   }
   strcpy( other, "C" );
   tjWriteAt( 1, 1, RED, LIGHTGRAY, "                                            F10 - Save changes   Esc - Abandon  " );
   tjWriteAt( 2, 1, BLACK, LIGHTGRAY, &title[1] );

   switch( classification )
   {
      case 'L':
         LecturePics.next = &LectureText;
         LectureText.next = &LecturePics;
         if( insertflag )
         {
            LectureText.EditMe = mkFlist( sizeof( str ), 0 );
            iquedptr( LectureText.EditMe, title );
            LecturePics.EditMe = mkFlist( sizeof( str ), 0 );
         }
         if( Edit_Flist( &LectureText ) == F10 )
         {
            stod( LecturePics.EditMe, other, 1 );
            FirstHalf( infile, outfile, fname, recno+1 );
            if( !insertflag )
               skiprecs( infile, 2 );
            SaveCryptFlist( LectureText.EditMe, outfile );
            SaveCryptFlist( LecturePics.EditMe, outfile );
            SecondHalf( infile, outfile );
         }
         rmFlist( &LectureText.EditMe );
         rmFlist( &LecturePics.EditMe );
         break;

      case 'S':
         ShortText.next = &ShortAnswer;
         ShortAnswer.next = &ShortPics;
         ShortPics.next = &ShortText;
         if( insertflag )
         {
            ShortText.EditMe = mkFlist( sizeof( str ), 0 );
            iquedptr( ShortText.EditMe, title );
            ShortAnswer.EditMe = mkFlist( sizeof( str ), 0 );
            ShortPics.EditMe = mkFlist( sizeof( str ), 0 );
         }
         if( Edit_Flist( &ShortText ) == F10 )
         {
            stod( ShortPics.EditMe, other, 1 );
            *(int*)(&other[2]) = ShortAnswer.choice;
            stod( ShortAnswer.EditMe, other, 1 );
            FirstHalf( infile, outfile, fname, recno+1 );
            if( !insertflag )
               skiprecs( infile, 3 );
            SaveCryptFlist( ShortText.EditMe, outfile );
            SaveCryptFlist( ShortAnswer.EditMe, outfile );
            SaveCryptFlist( ShortPics.EditMe, outfile );
            SecondHalf( infile, outfile );
         }
         rmFlist( &ShortText.EditMe );
         rmFlist( &ShortPics.EditMe );
         rmFlist( &ShortAnswer.EditMe );
         break;

      case 'M':
         ShortText.next = &MultAnswer;
         MultAnswer.next = &MultTrick;
         MultTrick.next = &ShortPics;
         ShortPics.next = &ShortText;
         if( insertflag )
         {
            ShortText.EditMe = mkFlist( sizeof( str ), 0 );
            iquedptr( ShortText.EditMe, title );
            ShortPics.EditMe = mkFlist( sizeof( str ), 0 );
            MultTrick.EditMe = mkFlist( sizeof( str ), 0 );
            MultAnswer.EditMe = mkFlist( sizeof( str ), 0 );
         }
         if( Edit_Flist( &ShortText ) == F10 )
         {
            stod( ShortPics.EditMe, other, 1 );
            *(int*)(&other[2]) = MultAnswer.choice;
            stod( MultAnswer.EditMe, other, 1 );
            *(int*)(&other[2]) = MultTrick.choice;
            stod( MultTrick.EditMe, other, 1 );
            FirstHalf( infile, outfile, fname, recno+1 );
            if( !insertflag )
               skiprecs( infile, 4 );
            SaveCryptFlist( ShortText.EditMe, outfile );
            SaveCryptFlist( MultAnswer.EditMe, outfile );
            SaveCryptFlist( MultTrick.EditMe, outfile );
            SaveCryptFlist( ShortPics.EditMe, outfile );
            SecondHalf( infile, outfile );
         }
         rmFlist( &ShortText.EditMe );
         rmFlist( &ShortPics.EditMe );
         rmFlist( &MultTrick.EditMe );
         rmFlist( &MultAnswer.EditMe );
         break;
   }
}

key ShowDialog( str fname )
// Loads index of database items (for file FILENAME) and does a dialog box.
{
   tjPickOpts pOpts;
   Flist temp;

   extern int EDITDATA_WIDTH;
   extern int EDITDATA_DEPTH;
   extern int EDITDATA_LENGTH;
   extern unsigned char EDITDATA [];

   void far *screen;
   int offset = 0;
   int x;


   fg_cursor( 0 );
   pOpts.ux = 32;
   pOpts.uy = 4;
   pOpts.fore = WHITE;
   pOpts.back = BLUE;
   pOpts.hifore = WHITE;
   pOpts.hiback = CYAN;
   pOpts.topline = pOpts.current = 1;
   pOpts.width = 32;
   pOpts.dispfun = &dialogdispfun;
   pOpts.keyhan = &dialoghan;
   pOpts.lines = 17;

   str thing;

   char classification;
   int insertflag;
   int endflag;

   do
   {
       FILE *infile = fopen( fname, "rb" );
       LoadIndex( pOpts.PickMe, infile );
       fclose( infile );

       InitOptions( );
       insertflag = 1;
       endflag = 0;
       classification = 0;
       fg_cursor( 0 );
       for (x=0; x<EDITDATA_DEPTH; x++)
       {
          (long) screen = 0xB8000000+x*160+offset;
          memcpy (screen,&EDITDATA[x*EDITDATA_WIDTH*2],EDITDATA_WIDTH*2);
       }
       x = tjPick( pOpts );
       fg_cursor( 1 );
       if( pOpts.lastkey == Enter )
       {
          rcld( pOpts.PickMe, thing, x );
          if( thing[0] != '*' )
             classification = thing[0];
          else
             endflag = 1;
          pOpts.lastkey = Ins;
          insertflag = 0;
       }
       if( pOpts.lastkey == Ins )
       {
          rcld( pOpts.PickMe, thing, x );
          if( classification == 0 )
             classification = choosetype( );
          if( classification )
             DoAnEdit( fname, classification, *(int*)(&thing[36]), insertflag || endflag );
       }
       if( (pOpts.lastkey == Del) && !(x==nempty( pOpts.PickMe )) && AreYouSure( ) )
       {
          rcld( pOpts.PickMe, thing, x );
          FILE *one, *two;

          FirstHalf( one, two, fname, *(int*)(&thing[36])+1 );
          switch( thing[0] )
          {
             case 'L':
                skiprecs( one, 2 );
                break;
             case 'M':
                skiprecs( one, 4 );
                break;
             case 'S':
                skiprecs( one, 3 );
                break;
          }
          SecondHalf( one, two );
       }
   } while( pOpts.lastkey != Esc );

   rmFlist( &pOpts.PickMe );
   fg_cursor( 1 );
   return( pOpts.lastkey );
}

int main( )
{
   Flist Test;
   tjInit( );

   extern int IMAGEDATA_WIDTH;
   extern int IMAGEDATA_DEPTH;
   extern int IMAGEDATA_LENGTH;
   extern unsigned char IMAGEDATA [];

   void far *screen;
   int offset = 0;
   int x;

   for (x=0; x<IMAGEDATA_DEPTH; x++)
   {
       (long) screen = 0xB8000000+x*160+offset;
       memcpy (screen,&IMAGEDATA[x*IMAGEDATA_WIDTH*2],IMAGEDATA_WIDTH*2);
   }

   fg_cursor( 0 );
   getch( );
   fg_cursor( 1 );
   clrscr( );

   str temp;
   key ch;

   while( (ch = ShowMenu( temp )) != Esc )
   {
       switch( ch )
       {
          case Ins:
             fg_cursor( 1 );
             InitOptions( );
             str filename;
             str shortdesc;
             strcpy( filename, "" );
             strcpy( shortdesc, "" );
             tjFBoxWin( 1, 1, 80, 3, BLACK, LIGHTGRAY, BDOUBLE );
             tjWriteAt( 2, 2, RED, LIGHTGRAY, " Enter a filename: (No spaces!  No periods!) " );
             if( tjRead( 52, 2, WHITE, RED, filename, 9 ) )
             {
                tjRmWin( );
                strform( filename );
                strcat( filename, ".TXR" );
                if( !fexist( filename ) )
                {
                   tjFBoxWin( 1, 1, 80, 3, BLACK, LIGHTGRAY, BDOUBLE );
                   tjWriteAt( 2, 2, RED, LIGHTGRAY, " Enter a short description: " );
                   if( tjRead( 32, 2, WHITE, RED, shortdesc, 33 ) )
                   {
                      tjRmWin( );
                      Description.EditMe = mkFlist( sizeof( str ), 0 );
                      tjFBox( 41, 21, 80, 25, BLACK, LIGHTGRAY, BDOUBLE );
                      tjWriteAt( 43, 23, RED, LIGHTGRAY, "Press F10 to save or ESC to abort" );
                      if( Edit_Flist( &Description ) == Esc )
                         rmFlist( &Description.EditMe );
                      else
                      {
                         FILE *writetome = fopen( filename, "wb" );
                         stod( Description.EditMe, shortdesc, 1 );
                         SaveCryptFlist( Description.EditMe, writetome );
                         rmFlist( &Description.EditMe );
                         fclose( writetome );
                      }
                   } else tjRmWin( );
                }
                else
                {
                   tjFBoxWin( 1, 1, 80, 3, BLACK, LIGHTGRAY, BDOUBLE );
                   tjWriteAt( 3, 2, YELLOW, RED, " That filename already exists!!! " );
                   tjGetKey( );
                   tjRmWin( );
                }
             }
             else
                tjRmWin( );
             break;
          case Enter:
             ShowDialog( temp );
             break;
          case 'D':
          case 'd':
             {
                InitOptions( );
                FILE *infile, *outfile;
                FirstHalf( infile, outfile, temp, 0 );
                LoadCryptFlist( Description.EditMe, infile );
                rcld( Description.EditMe, shortdesc, 1 );
                tjFBoxWin( 1, 1, 80, 3, BLACK, LIGHTGRAY, BDOUBLE );
                tjWriteAt( 2, 2, RED, LIGHTGRAY, " Enter a short description: " );
                if( tjRead( 32, 2, WHITE, RED, shortdesc, 32 ) )
                {
                   tjRmWin( );
                   tjFBox( 41, 21, 80, 25, BLACK, LIGHTGRAY, BDOUBLE );
                   tjWriteAt( 43, 23, RED, LIGHTGRAY, "Press F10 to save or ESC to abort" );
                   if( Edit_Flist( &Description ) != Esc )
                   {
                      stod( Description.EditMe, shortdesc, 1 );
                      SaveCryptFlist( Description.EditMe, outfile );
                      rmFlist( &Description.EditMe );
                      SecondHalf( infile, outfile );
                   }
                } else tjRmWin( );
             }
             break;
          case 'B':
          case 'b':
             BuildGXL( temp );
             break;
       }
   }

   tjClose( );
   fg_cursor( 1 );
   remove( "TEMP.$$$" );
   return( 0 );
}
