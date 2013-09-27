/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */

 

// GFILEIO.HPP
// File I/O functions used by Inquiry/G.
//===

#define MAX 38
#define PASSWORD "\x21\x26\x60\xFE\x36\x11\x11\x17\x7E\x57\x55\x32\x0C\x01\x13\x33\x34\x31\x01\x17\x05\x31\x32\x12\x11\x11\x17\x05"
typedef char str[MAX];

void ChangeDirectory( char ch )
{
   char path[41];
   gxSetMode( gxTEXT );
   tjInit( );
   tjSetMode( C80 );
   int done = 0;

   strcpy( leftgraf, "" );
   strcpy( rightgraf, "" );

   getcwd( path, 41 );

   while( !done )
   {
      clrscr( );
      if( ch == 'V' )
         tjWriteAt( 1, 1, YELLOW, RED, " ERROR!!!  No databases found in current directory!!! " );
      else
         tjWriteAt( 1, 1, BLACK, LIGHTGRAY, " Change Database Directory function: " );

      tjWriteAt( 18, 24, WHITE, BLUE, " Have NO IDEA where your database files are??? " );
      tjWriteAt( 16, 25, LIGHTGRAY, BLUE, " Press ESC to return to DOS so you can FIND them. " );

      tjWriteAt( 10, 5, WHITE, BLACK, "Enter full path to location where databases are located >>>" );
      if( !tjRead( 15, 7, BLACK, LIGHTGRAY, path, 40 ) )
      {
         tjClose( );
         exit( 0 );
      }
      done = !chdir( path );
      if( !done )
      {
         tjWriteAt( 10, 10, WHITE, RED, " INVALID PATH SPECIFIED!!! " );
         tjWriteAt( 10, 11, WHITE, RED, " Path should be of the form 'A:\\DATABASES', 'C:\\INQG\\DATA', etc. " );
         tjGetKey( );
      }
   }

   fg_setmode( fg_bestmode( 640, 350, 1 ) );
   gxSetMode( gxGRAPHICS );
   retcode = grSetColor( grBLACK );
   retcode = grDrawLine( 1, 1, 10, 10 );
}

void Crypt( const void *ptr, size_t size, char *lock )
// Perform a rotating XOR based encryption on any data structure!
{
   int index;

   for(index=0;index<size;index++)
      *((char*)ptr+index)=*((char*)ptr+index)^lock[index%strlen(lock)];
}

void LoadCryptFlist( Flist &LoadMe, FILE *infile )
// Load an encrypted list of strings... assuming str is the type.
// the file must already be opened, and the first number encountered should
// be the number of lines...
{
   int num, index;
   str temp;

   LoadMe = mkFlist( sizeof( str ), 0 );

   num = fgetc( infile );
   if( !feof( infile ) )
      for( index = 1; index <= num; index++ )
      {
         fread( temp, sizeof( str ), 1, infile );
         Crypt( temp, sizeof( str ), PASSWORD );
         iquedptr( LoadMe, temp );
      }
}

void SaveCryptFlist( Flist SaveMe, FILE *outfile )
// Save an encrypted list of strings, refer to LoadCryptFlist.
{
   int num, index;
   str temp;

   num = nempty( SaveMe );
   fputc( num, outfile );
   for( index = 1; index <= num; index++ )
   {
      rcld( SaveMe, temp, index );
      Crypt( temp, sizeof( str ), PASSWORD );
      fwrite( temp, sizeof( str ), 1, outfile );
   }
}

void LoadIndex( Flist &LoadMe, FILE *infile )
// Goes through a file and builds an index flexlist.
{
   int num, index;
   str temp;
   int total = 1;

   LoadCryptFlist( LoadMe, infile );
   rmFlist( &LoadMe );

   LoadMe = mkFlist( sizeof( str ), 0 );

   while( !feof( infile ) )
   {
      num = fgetc( infile );
      if( !feof( infile ) )
      {
         fread( temp, sizeof( str ), 1, infile );
         Crypt( temp, sizeof( str ), PASSWORD );
         *(int*)(&temp[36]) = total;
         total++;
         if( temp[0] != 'C' )
            iquedptr( LoadMe, temp );
         for( index = 1; index < num; index++ )
            fread( temp, sizeof( str ), 1, infile );
      }
   }
}

void skiprecs( FILE *infile, int num )
{
   int index;
   Flist temp;

   for( index = 0; index < num; index++ )
   {
      LoadCryptFlist( temp, infile );
      rmFlist( &temp );
   }
}

void CopyCryptFile( char *fname )
{
   Flist temp;

   FILE *infile = fopen( fname, "rb" );
   FILE *outfile = fopen( "TEMP.$$$", "wb" );

   while( !feof( infile ) )
   {
      LoadCryptFlist( temp, infile );
      SaveCryptFlist( temp, outfile );
      rmFlist( &temp );
   }
   fclose( infile );
   fclose( outfile );
}

void FirstHalf( FILE *&infile, FILE *&outfile, char *fname, int recno )
{
   int index;
   Flist temp;

   CopyCryptFile( fname );
   infile = fopen( "TEMP.$$$", "rb" );
   outfile = fopen( fname, "wb" );

   for( index = 1; index < recno; index++ )
   {
      LoadCryptFlist( temp, infile );
      SaveCryptFlist( temp, outfile );
      rmFlist( &temp );
   }
}

void SecondHalf( FILE *&infile, FILE *&outfile )
{
   Flist temp;

   while( !feof( infile ) )
   {
      LoadCryptFlist( temp, infile );
      if( nempty( temp ) )
         SaveCryptFlist( temp, outfile );
      rmFlist( &temp );
   }
   fclose( infile );
   fclose( outfile );
}

void LoadMenu( Flist &LoadMe, char *mask )
// Goes through a file and reads a menu flexlist.
{
   ffblk ffblk;
   Flist temp;
   int done;
   LoadMe = mkFlist( sizeof( Flist ), 0 );

   done = findfirst( mask, &ffblk, 0);
   while (!done)
   {
      FILE *infile = fopen( ffblk.ff_name, "rb" );
      LoadCryptFlist( temp, infile );
      fclose( infile );

      mkcdptr( temp, 0 );
      insdptr( temp, &ffblk.ff_name );
      done = findnext(&ffblk);
      iquedptr( LoadMe, &temp );
   }
}

void KillMenu( Flist &KillMe )
{
   int index;
   Flist temp;

   for( index = 1; index <= nempty( KillMe ); index++ )
   {
      rcld( KillMe, &temp, index );
      rmFlist( &temp );
   }
   rmFlist( &KillMe );
}

int ShowMenu( str fname )
{
   Flist Menu;
   Flist node;
   int current = 1, index, redraw = 1, done=0;
   str item;
   key ch;

   LoadHalf( "MBOX.PCC", fxRIGHT, syspath );
   LoadHalf( "STARTUP.PCC", fxLEFT, syspath );
   LoadMenu( Menu, "*.TXR" );
   ShowHalf( fxRIGHT+fxLEFT );
   do
   {
      if( redraw && rcld( Menu, &node, current ) )
      {
         rcld( node, item, 2 );
         StripTrailing( item );
         PutTitle( item, grBLUE );
         for( index = 3; index < 3+20; index++ )
            if( rcld( node, item, index ) )
               PutText( index-3, item );
            else
               PutText( index-3, "" );
      }
      if( nempty( Menu ) )
         ch = tjGetKey( );
      else
         ch = 'V';
      switch( ch )
      {
         case F1:
           Help( "MHELP.PCC" );
           break;
         case UArr:
           if( current > 1 )
              current--;
           else
              current = nempty( Menu );
           redraw = 1;
           break;

         case DArr:
           if( current < nempty( Menu ) )
              current++;
           else
              current = 1;
           redraw = 1;
           break;

         case 'V':
         case 'd':
         case 'D':
            ChangeDirectory( ch );
            LoadHalf( "MBOX.PCC", fxRIGHT, syspath );
            LoadHalf( "STARTUP.PCC", fxLEFT, syspath );
            LoadHalf( "TBOX.PCC", fxRIGHT, syspath );
            LoadMenu( Menu, "*.TXR" );
            ShowHalf( fxRIGHT+fxLEFT );
            redraw = 1;
            break;

         case Esc:
         case Enter:
            done = 1;
            break;
       }
   } while( !done );

   rcld( Menu, &node, current );
   rcld( node, item, 1 );
   strcpy( fname, item );

   KillMenu( Menu );
   return( ch == Esc );
}

#define RESPUY 326

void PutChar( int x, int y, char outme, char fore, char back )
{
   char temp[2];
   temp[0] = outme;
   temp[1] = 0;

   grSetColor( fore );
   grSetBkColor( back );
   grMoveTo( x, y );
   grOutText( temp );
}

void OutHiText( int x, int y, str outme, int pos )
{
   str temp;
   strcpy( temp, outme );
   tjStrPad( temp, 37 );
   temp[pos] = ' ';

   retcode = grSetTextStyle( grTXT8X14, grOPAQUE );
   grSetColor( grBLACK );
   grSetBkColor( grWHITE );
   grMoveTo( x, y );
   grOutText( temp );
   strcpy( temp, outme );
   tjStrPad( temp, 37 );
   PutChar( x+pos*8, y, temp[pos], WHITE, GREEN );
}

int GoAnything( char *fname, int tcolor, Flist Text, Flist Graphics, Flist Responses, int range = 0 )
{
   int Index;
   str Item;

   for( Index = 1; Index <= nempty( Graphics ); Index++ )
   {
      rcld( Graphics, Item, Index );
      strform( Item );
      if( !strcmp( Item, "" ) )
      {
         mkcdptr( Graphics, Index );
         delete( deln( Graphics ) );
         Index--;
      }
   }

   int Line = 1;
   int pos;
   key ch;
   int done = 0;
   int current = nempty( Responses )  ? 1 : 0;
   int currentgraf = nempty( Graphics ) > 1 ? 1 : 0;
   str node;
   char type;
   str libname;
   str title;
   strcpy( libname, fname );
   int restoreflag = 0;

   forcefile( libname );
   forcefname( libname );
   strcat( libname, ".GXR" );
   LoadHalf( "NOTHING.PCC", fxLEFT, syspath );

   if( currentgraf )
   {
      rcld( Graphics, Item, currentgraf+1 );
      forcefile( Item );
      LoadHalf( Item, fxLEFT, libname );
   }

   ShowHalf( fxLEFT+fxRIGHT );

   rcld( Text, title, 1 );
   StripTrailing( &title[1] );
   type = title[0];
   PutTitle( &title[1], tcolor );

   if( current > 0 )
   {
      rcld( Responses, node, current );
      StripTrailing( node );
      pos = strlen( node );
      OutHiText( TEXTUX, RESPUY, node, pos );
   }

   if( current > 0 )
      PutNum( current );

   if( nempty( Text ) < MAXROWS )
   {
      PutArrow( fxUP, gxFALSE );
      PutArrow( fxDOWN, gxFALSE );
      Line = 0;
   }
   else
   {
      if( Line >= nempty( Text )-MAXROWS )
      {
         Line = nempty( Text )-MAXROWS;
         PutArrow( fxDOWN, gxFALSE );
      }
      else
         PutArrow( fxDOWN, gxTRUE );

      if( Line == 0 )
         PutArrow( fxUP, gxFALSE );
      else
         PutArrow( fxUP, gxTRUE );
   }
   Line = 1;

   for( Index = Line; Index < Line+MAXROWS; Index++ )
   {
      if( rcld( Text, &Item, Index+1 ) )
         PutText( Index-Line, Item );
      else
         PutText( Index-Line, "" );
   }

   do
   {
      ch = tjGetKey( );
      if( restoreflag )
         PutTitle( &title[1], tcolor );
      restoreflag = 0;

      switch( ch )
      {
         case F1:
            if( type == 'L' )
               Help( "LHELP.PCC" );
            else
               Help( "QHELP.PCC" );
            break;
         case UArr:
         if( Line > 1 )
         {
            if( Line+MAXROWS == nempty( Text )-1 )
               PutArrow( fxDOWN, gxTRUE );
            Line--;
            rcld( Text, &Item, Line+1 );
            ScrollText( fxDOWN, Item );
            if( Line == 0 )
               PutArrow( fxUP, gxFALSE );
         }
         else ;
         break;

         case DArr:
         if( Line+MAXROWS < nempty( Text ) )
         {
            if( Line == 0 )
               PutArrow( fxUP, gxTRUE );
            Line++;
            rcld( Text, &Item, Line+MAXROWS );
            ScrollText( fxUP, Item );
            if( Line+MAXROWS == nempty( Text )-1 )
               PutArrow( fxDOWN, gxFALSE );
         }
         else ;
         break;

         case PgDn:
         if( Line < (int) (nempty( Text )-MAXROWS) )
         {
            if( Line == 0 )
               PutArrow( fxUP, gxTRUE );
            if( (Line = Line+MAXROWS-1) >= nempty( Text )-MAXROWS-1 )
            {
               Line = nempty( Text )-MAXROWS;
               PutArrow( fxDOWN, gxFALSE );
            }
            if( Line < 1 )
               Line = 1;
            for( Index = Line; Index < Line+MAXROWS; Index++ )
            {
               if( rcld( Text, &Item, Index+1 ) )
                  PutText( Index-Line, Item );
               else
                  PutText( Index-Line, "" );
            }
         }
         else ;
         break;

         case PgUp:
         if( Line > 0 )
         {
            if( Line == nempty( Text )-MAXROWS-1 )
               PutArrow( fxDOWN, gxTRUE );
            if( (Line = Line-MAXROWS+1) <= 0 )
            {
               Line = 0;
               PutArrow( fxUP, gxFALSE );
            }

            if( Line < 1 )
               Line = 1;

            for( Index = Line; Index < Line+MAXROWS; Index++ )
            {
               if( rcld( Text, &Item, Index+1 ) )
                  PutText( Index-Line, Item );
               else
                  PutText( Index-Line, "" );
            }

         }
         else ;
         break;

         case STab:                  // Previous Response
            if( (current != 0) && ( current > 1 ) )
            {
               current--;
               PutNum( current );
               rcld( Responses, node, current );
               StripTrailing( node );
               pos = strlen( node );
               OutHiText( TEXTUX, RESPUY, node, pos );
            }
            break;

         case Tab:                   // Next Response
            if( (current != 0) && (current < nempty( Responses ) ) )
            {
               current++;
               PutNum( current );
               rcld( Responses, node, current );
               StripTrailing( node );
               pos = strlen( node );
               OutHiText( TEXTUX, RESPUY, node, pos );
            }
            break;

         case CtrlLArr:
         case LArr:
         if( (((ch == CtrlLArr) && (current != 0))) || (((ch == LArr) && (current == 0 ))) )
         {
            if( (currentgraf > 1) && (currentgraf != 0) )
            {
               currentgraf--;
               rcld( Graphics, Item, currentgraf+1 );
               forcefile( Item );
               LoadHalf( "NOTHING.PCC", fxLEFT, syspath );
               LoadHalf( Item, fxLEFT, libname );
               ShowHalf( fxLEFT );
            }
         }
         else
            if( (ch == LArr) && (current > 0) && (pos > 0) )
            {
               rcld( Responses, node, current );
               tjStrPad( node, 37 );
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grBLACK, grWHITE );
               pos--;
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grWHITE, grGREEN );
            }
         break;

         case CtrlRArr:
         case RArr:
         if( (((ch == CtrlRArr) && (current != 0))) || (((ch == RArr) && (current == 0 ))) )
         {
            if( (currentgraf < nempty( Graphics )-1) && (currentgraf != 0))
            {
               currentgraf++;
               rcld( Graphics, Item, currentgraf+1 );
               forcefile( Item );
               LoadHalf( "NOTHING.PCC", fxLEFT, syspath );
               LoadHalf( Item, fxLEFT, libname );
               ShowHalf( fxLEFT );
            }
         }
         else
            if( (ch == RArr) && (current > 0) && (pos < 36) )
            {
               rcld( Responses, node, current );
               tjStrPad( node, 37 );
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grBLACK, grWHITE );
               pos++;
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grWHITE, grGREEN );
            }
         break;

         case Home:
         case CtrlHome:
            if( (ch == Home) && (current != 0) )
            {
               rcld( Responses, node, current );
               tjStrPad( node, 37 );
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grBLACK, grWHITE );
               pos = 0;
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grWHITE, grGREEN );
            }
            else
            {
               if( (currentgraf != 1) && (currentgraf != 0 ))
               {
                  currentgraf = 1;
                  rcld( Graphics, Item, currentgraf+1 );
                  forcefile( Item );
                  LoadHalf( "NOTHING.PCC", fxLEFT, syspath );
                  LoadHalf( Item, fxLEFT, libname );
                  ShowHalf( fxLEFT );
               }
            }
            break;

         case End:
         case CtrlEnd:
            if( (ch == End) && (current != 0) )
            {
               rcld( Responses, node, current );
               tjStrPad( node, 37 );
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grBLACK, grWHITE );
               StripTrailing( node );
               pos = strlen( node );
               tjStrPad( node, 37 );
               PutChar( TEXTUX+pos*8, RESPUY, node[pos], grWHITE, grGREEN );
            }
            else
            {
               if( (currentgraf != nempty( Graphics )-1) && (currentgraf != 0 ))
               {
                  currentgraf = nempty( Graphics )-1;
                  rcld( Graphics, Item, currentgraf+1 );
                  forcefile( Item );
                  LoadHalf( "NOTHING.PCC", fxLEFT, syspath );
                  LoadHalf( Item, fxLEFT, libname );
                  ShowHalf( fxLEFT );
               }
            }
            break;

         case Del:
         case BackSp:
            if( (ch == Del) && (pos < 36) )
               pos++;
            if( (current != 0) && (pos > 0 ))
            {
               rcld( Responses, node, current );
               tjStrPad( node, 37 );
               pos--;
               strcpy( &node[pos], &node[pos+1] );
               stod( Responses, node, current );
               OutHiText( TEXTUX, RESPUY, node, pos );
            }
            break;

         case CtrlD:
            strcpy( node, "" );
            pos = 0;
            stod( Responses, node, current );
            OutHiText( TEXTUX, RESPUY, node, pos );
            break;

         case Enter:
            if( current == 0 )
               done = 1;
            else
            {
               done = 1;
               for( Index = 1; Index <= nempty( Responses ); Index++ )
               {
                  rcld( Responses, node, Index );
                  strform( node );
                  if( !strcmp( node, "" ) )
                     done = 0;
               }
               if( !done )
               {
                  PutTitle( "More Answers Expected: Use TAB", grRED );
                  beep( 200, 400 );
                  restoreflag = 1;
               }
            }

            if( (type == 'M') && done )
            {
               rcld( Responses, node, 1 );
               strform( node );
               for( Index = 0; Index < strlen( node ); Index++ )
               if( ((node[Index] < 'A') || (node[Index] >= 'A'+range)) && (node[Index] != ' '))
                  done = 0;
               if( !done )
               {
                  PutTitle( "Invalid Letters Entered!", grRED );
                  beep( 200, 400 );
                  restoreflag = 1;
               }
            }
            break;

         case Esc:
            done = 1;
            break;

         default:
            if( (current != 0) && (pos < 36) && (tjVideoKey( ch ) ))
            {
               grSetColor( grBLACK );
               grSetBkColor( grWHITE );
               grMoveTo( TEXTUX, RESPUY );
               rcld( Responses, node, current );
               tjStrPad( node, 37 );
               for( Index = 34; Index >= pos; Index-- )
                  node[Index+1] = node[Index];
               node[pos] = ch;
               stod( Responses, node, current );
               pos++;
               OutHiText( TEXTUX, RESPUY, node, pos );
            }
            break;
      }
   } while( !done );
   return( ch == Esc );
}

int GoLecture( char *fname, Flist Text, Flist Graphics )
{
   Flist Responses = mkFlist( sizeof( str ), 0 );

   LoadHalf( "LBOX.PCC", fxRIGHT, syspath );

   int x = GoAnything( fname, grBLUE, Text, Graphics, Responses );
   rmFlist( &Responses );
   return( x );
}

int tabulate( Flist TabMe, int &total )
{
   int index;
   str item;
   str prev;
   int value;

   total = 0;

   rcld( TabMe, item, 1 );
   value = *(int*)(&item[2]);
   strcpy( item, "" );
   stod( TabMe, item, 1 );

   for( index = 2; index <= nempty( TabMe ); index++ )
   {
      rcld( TabMe, prev, index-1 );
      rcld( TabMe, item, index );
      strform( item );
      strform( prev );
      if( !strcmp( prev, "" ) )
         stod( TabMe, prev, index-1 );
      if( !strcmp( item, "" ) && !strcmp( prev, "" ) )
      {
         mkcdptr( TabMe, index );
         delete( deln( TabMe ) );
         index--;
      }
   }

   for( index = 2; index <= nempty( TabMe ); index++ )
   {
      rcld( TabMe, prev, index-1 );
      rcld( TabMe, item, index );
      strform( item );
      strform( prev );
      if( strcmp( item, "" ) && !strcmp( prev, "" ) )
      {
         total++;
         prev[1] = 1;
         stod( TabMe, prev, index-1 );
      }
   }
   if( !value )
      return( total );
   else
      return( total < value ? total : value );
}

void GetWhy( Flist Answers, Flist &Why )
{
   Why = mkFlist( sizeof( str ), 0 );

   int index = 0;
   str item;
   int subindex;

   do
   {
      index++;
      if( !rcld( Answers, item, index ) )
         strcpy( item, "@" );
      strform( item );
   } while( strcmp( "@", item ) );

   while( index <= nempty( Answers ) )
   {
      mkcdptr( Answers, index );
      delete( deln( Answers ) );
      if( rcld( Answers, item, index ) )
         iquedptr( Why, item );
   }
}

int ShowEval( int right, char *fname, Flist Why, Flist Answers, Flist Tricks, Flist Graphics, Flist Resp2, char mode, int num, int range = 0 );

int SAnswerIn( Flist Answers, str look )
{
   str temp;
   int subindex;
   int index;

   strform( look );

   for( index = 1; index <= nempty( Answers ); index++ )
   {
      rcld( Answers, temp, index );
      strform( temp );
      if( !strcmp( temp, "" ) && (temp[1] == 1) )
      {
         subindex = index;
         do
         {
            subindex++;
            if( !rcld( Answers, temp, subindex ) )
               strcpy( temp, "" );
            strform( temp );
            if( !strcmp( temp, look ) )
            {
               rcld( Answers, temp, index );
               temp[1] = 0;
               stod( Answers, temp, index );
               return( 1 );
            }
         } while( strcmp( temp, "" ) );
         index = subindex-1;
      }
   }
   return( 0 );
}

int GoShort( char *fname, Flist Text, Flist Answers, Flist Graphics, int &correct, int &total )
{
   Flist Responses = mkFlist( sizeof( str ), 0 );
   Flist Why;

   GetWhy( Answers, Why );
   int index;
   str nada;
   int atotal;
   int tab = tabulate( Answers, atotal );

   strcpy( nada, "" );
   for( index = 1; index <= tab; index++ )
      iquedptr( Responses, nada );

   LoadHalf( "QBOX.PCC", fxRIGHT, syspath );

   int x;

   x = GoAnything( fname, grBLUE, Text, Graphics, Responses );

   int right = 0;

   if( !x )
   {
      for( index = 1; index <= nempty( Responses ); index++ )
      {
         rcld( Responses, nada, index );
         if( SAnswerIn( Answers, nada ) )
         {
            right++;
            mkcdptr( Responses, index );
            delete( deln( Responses ) );
            index--;
         }
      }
      x = ShowEval( right, fname, Why, Answers, NULL, Graphics, Responses, 'S', tab );
      total += tab;
      correct += right;
   }

   rmFlist( &Why );
   rmFlist( &Responses );
   if( x )
      return( x );
   if( right == tab )
      return( 2 );
   return( 0 );
}

int Mark( Flist MarkMe, int which, char use )
{
   int index;
   int subindex = 1;
   str item;

   rcld( MarkMe, item, 1 );
   for( index = 0; index < which; index++ )
      do
      {
         subindex++;
         if( !rcld( MarkMe, item, subindex ) )
         {
            strcpy( item, "" );
            item[1] = 'Z';
         }
      } while( strcmp( item, "" ) );
   if( item[1] != 1 )
      return( 1 );

   rcld( MarkMe, item, subindex );
   item[1] = use;
   stod( MarkMe, item, subindex );
   return( 0 );
}

void MarkRandom( Flist Answers, int tab, int sum, int total, set Taken )
{
   str temp;
   char randchar;
   int randnum;
   int index;

   for( index = 0; index < tab; index++ )
   {
      do
      {
         randchar = random( sum )+'A';
      } while( InSet( Taken, randchar ) );
      AddSet( Taken, randchar );

      do
      {
         randnum = random( total );
      } while( Mark( Answers, randnum, randchar ) );
   }
}

void PrintMarked( Flist Print, Flist PrintTo, char use )
{
   int index;
   str item;

   for( index = 1; index <= nempty( Print ); index++ )
   {
      rcld( Print, item, index );
      if( !strcmp( item, "" ) && (item[1] == use) )
      {
         do
         {
            index++;
            if( !rcld( Print, item, index ) )
               strcpy( item, "" );
            if( !strcmp( item, "" ) )
               index--;
            else
               iquedptr( PrintTo, item );
         } while( strcmp( item, "" ) );
      }
   }
}

int AnswerIn( Flist CheckMe, char FindMe )
{
   int index;
   str temp;

   for( index = 1; index <= nempty( CheckMe ); index++ )
   {
      rcld( CheckMe, temp, index );
      if( !strcmp( temp, "" ) )
         if( temp[1] == FindMe )
            return( 1 );
   }
   return( 0 );
}

int ShowEval( int right, char *fname, Flist Why, Flist Answers, Flist Tricks, Flist Graphics, Flist Resp2, char mode, int num, int range )
{
   int index;
   str item;
   Flist Hybrid = mkFlist( sizeof( str ), 0 );
   Flist Responses = mkFlist( sizeof( str ), 0 );
   str prev;

   LoadHalf( "LBOX.PCC", fxRIGHT, syspath );

   if( mode == 'M' )
   {
       if( right )
          strcpy( item, "SEVALUATION: Correct Answer" );
       else
         strcpy( item, "SEVALUATION: Incorrect Answer" );
       iquedptr( Hybrid, item );
       if( num == 1 )
          strcpy( item, "The correct choice is:" );
       else
          strcpy( item, "The correct choices are:" );
       iquedptr( Hybrid, item );
       AddBlank( Hybrid );
       for( index = 'A'; index < 'A'+range; index++ )
       {
          if( AnswerIn( Answers, index ) )
          {
             AddLet( Hybrid, index );
             PrintMarked( Answers, Hybrid, index );
          }
       }
       AddBlank( Hybrid );
       for( index = 1; index <= nempty( Why ); index++ )
       {
          rcld( Why, item, index );
          iquedptr( Hybrid, item );
       }
       AddBlank( Hybrid );
       if( !right )
       {
          AddBlank( Hybrid );
          rcld( Resp2, prev, 1 );
          strform( prev );
          strcpy( item, "Your Invalid Response(s):" );
          iquedptr( Hybrid, item );
          AddBlank( Hybrid );
          for( index = 0; index < strlen( prev ); index++ )
          {
             if( AnswerIn( Tricks, prev[index] ) )
             {
                AddLet( Hybrid, prev[index] );
                PrintMarked( Tricks, Hybrid, prev[index] );
                AddBlank( Hybrid );
             }
          }
       }
   }
   else
   {
       if( right == num )
          strcpy( item, "SEVALUATION: All Correct" );
       else
          sprintf( item, "SEVALUATION: %d Incorrect", num-right );
       iquedptr( Hybrid, item );
       strcpy( item, "The acceptable answer(s):" );
       iquedptr( Hybrid, item );
       AddBlank( Hybrid );
       for( index = 2; index <= nempty( Answers ); index++ )
       {
          rcld( Answers, prev, index-1 );
          rcld( Answers, item, index );
          strform( prev );
          strform( item );
          if( !strcmp( prev, "" ) )
             iquedptr( Hybrid, item );
       }
       AddBlank( Hybrid );
       for( index = 1; index <= nempty( Why ); index++ )
       {
          rcld( Why, item, index );
          iquedptr( Hybrid, item );
       }
       if( right < num )
       {
          right = 0;
          AddBlank( Hybrid );
          strcpy( item, "Your Invalid or Repeated Response(s):" );
          iquedptr( Hybrid, item );
          AddBlank( Hybrid );
          for( index = 1; index <= nempty( Resp2 ); index++ )
          {
             rcld( Resp2, item, index );
             strform( item );
             iquedptr( Hybrid, item );
          }
       }
   }
   AddBlank( Hybrid );
   strcpy( item, "Press [ENTER] to proceed..." );
   iquedptr( Hybrid, item );

   int x = GoAnything( fname, right ? grGREEN : grRED, Hybrid, Graphics, Responses );

   rmFlist( &Responses );
   rmFlist( &Hybrid );
   return( x );
}

int GoMult( char *fname, Flist Text, Flist Answers, Flist Tricks, Flist Graphics, int &correct, int &total )
{
   Flist Responses = mkFlist( sizeof( str ), 0 );
   Flist Why;

   GetWhy( Answers, Why );
   int index;
   str nada;
   int atotal, ttotal;
   int atab = tabulate( Answers, atotal );
   int ttab = tabulate( Tricks, ttotal );
   set Taken;
   ClearSet( Taken );

   strcpy( nada, "" );
   iquedptr( Responses, nada );

   MarkRandom( Answers, atab, atab+ttab, atotal, Taken );
   MarkRandom( Tricks, ttab, atab+ttab, ttotal, Taken );

   LoadHalf( "QBOX.PCC", fxRIGHT, syspath );
   AddBlank( Text );
   for( index = 'A'; index < 'A'+atab+ttab; index++ )
   {
      AddLet( Text, index );
      PrintMarked( Answers, Text, index );
      PrintMarked( Tricks, Text, index );
      AddBlank( Text );
   }

   int x;
   x = GoAnything( fname, grBLUE, Text, Graphics, Responses, atab+ttab );

   int right = 1;

   if( !x )
   {
      rcld( Responses, nada, 1 );
      strform( nada );
      for( index = 0; index < strlen( nada ); index++ )
         if( nada[index] != ' ' )
            if( !AnswerIn( Answers, nada[index] ) )
               right = 0;
      x = ShowEval( right, fname, Why, Answers, Tricks, Graphics, Responses, 'M', atab, atab+ttab );
      total++;
      correct += right;
   }

   rmFlist( &Why );
   rmFlist( &Responses );
   if( x )
      return( x );
   if( right )
      return( 2 );
   return( 0 );
}

void GoRecords( char *fname, Flist Selected )
{
   int index;
   str item;
   int correct = 0, total = 0, done = 0;
   Flist A, B, C, D;
   float percent;

   FILE *infile = fopen( fname, "rb" );
   skiprecs( infile, 1 );

   for( index = 1; index <= nempty( Selected ); index++ )
   {
      rcld( Selected, item, index );
      if( item[35] )
         switch( item[0] )
         {
            case 'L':
               LoadCryptFlist( A, infile );
               LoadCryptFlist( B, infile );
               done = GoLecture( fname, A, B );
               rmFlist( &A );
               rmFlist( &B );
               break;
            case 'S':
               LoadCryptFlist( A, infile );
               LoadCryptFlist( B, infile );
               LoadCryptFlist( C, infile );
               done = GoShort( fname, A, B, C, correct, total );
               rmFlist( &A );
               rmFlist( &B );
               rmFlist( &C );
               break;
            case 'M':
               LoadCryptFlist( A, infile );
               LoadCryptFlist( B, infile );
               LoadCryptFlist( C, infile );
               LoadCryptFlist( D, infile );
               done = GoMult( fname, A, B, C, D, correct, total );
               rmFlist( &A );
               rmFlist( &B );
               rmFlist( &C );
               rmFlist( &D );
               break;
         }
      else
         switch( item[0] )
         {
            case 'L':
               skiprecs( infile, 2 );
               break;
            case 'S':
               skiprecs( infile, 3 );
               break;
            case 'M':
               skiprecs( infile, 4 );
               break;
         }
      if( done == 2 )
      {
         done = 0;
         item[35] = 0;
         stod( Selected, item, index );
      }
      if( done )
         index = nempty( Selected )+1;
   }
   fclose( infile );

   LoadHalf( "PBOX.PCC", fxRIGHT, syspath );
   LoadHalf( "STARTUP.PCC", fxLEFT, syspath );
   ShowHalf( fxLEFT+fxRIGHT );
   PutTitle( "EVALUATION: Entire Database", grCYAN );

   sprintf( item, "There were %d total questions asked.", total );
   PutText( 1, item );
   sprintf( item, "You got %d of those questions right.", correct );
   PutText( 3, item );
   sprintf( item, "You got %d of those questions wrong.", total-correct );
   PutText( 5, item );

   percent = total;
   if( total != 0 )
      sprintf( item, "Your percentage was %4.2f%%.", correct/percent*100 );
   else
      strcpy( item, "Percentage is not applicable." );

   PutText( 9, item );
   strcpy( item, "Press [ENTER] to go to review menu." );
   PutText( 11, item );

   char ch;

   do
   {
      ch = tjGetKey( );
   } while( (ch != Enter) && (ch != Esc) );
}

int OfferIndex( char *fname )
{
   Flist Select;
   FILE *infile = fopen( fname, "rb" );
   LoadIndex( Select, infile );
   fclose( infile );

   int done=0, current=1, topline=1, redraw=1, index;
   str item;
   key ch;
   for( index = 1; index <= nempty( Select ); index++ )
   {
      rcld( Select, item, index );
      item[35] = 1;
      stod( Select, item, index );
   }

   do
   {
      if( redraw )
      {
         LoadHalf( "PBOX.PCC", fxRIGHT, syspath );
         LoadHalf( "STARTUP.PCC", fxLEFT, syspath );
         ShowHalf( fxRIGHT+fxLEFT );
         PutTitle( "Select Materials to Cover", grBLUE );
         redraw = 0;
         for( index = topline; index < topline+20; index++ )
         {
            if( rcld( Select, item, index ) )
               PutSelText( index-topline, item );
            else
               PutText( index-topline, "" );
         }
         SelMark( current-topline, 1 );
      }
      ch = tjGetKey( );
      switch( ch )
      {
         case F1:
           Help( "PHELP.PCC" );
           break;
         case UArr:
           if( current > 1 )
              if( current == topline )
              {
                 SelMark( current-topline, 0 );
                 ScrollWin( fxDOWN );
                 current--;
                 topline--;
                 rcld( Select, item, current );
                 PutSelText( current-topline, item );
                 SelMark( current-topline, 1 );
              }
              else
              {
                 SelMark( current-topline, 0 );
                 current--;
                 SelMark( current-topline, 1 );
              }
           break;

         case DArr:
           if( current < nempty( Select ) )
              if( current == topline+19 )
              {
                 SelMark( current-topline, 0 );
                 ScrollWin( fxUP );
                 current++;
                 topline++;
                 rcld( Select, item, current );
                 PutSelText( current-topline, item );
                 SelMark( current-topline, 1 );
              }
              else
              {
                 SelMark( current-topline, 0 );
                 current++;
                 SelMark( current-topline, 1 );
              }
           break;

         case ' ':
            rcld( Select, item, current );
            item[35] = !item[35];
            stod( Select, item, current );
            IsChosen( current-topline, item );
            break;

         case Enter:
         {
            GoRecords( fname, Select );
            redraw = 1;
         } break;

         case Esc:
            done = 1;
            break;

         case 'a':
         case 'A':
            for( index = 1; index <= nempty( Select ); index++ )
            {
               rcld( Select, item, index );
               item[35] = 1;
               if( (index >= topline) && (index < topline+20) )
                  IsChosen( index-topline, item );
               stod( Select, item, index );
            }
            break;

         case 'L':
         case 'l':
            for( index = 1; index <= nempty( Select ); index++ )
            {
               rcld( Select, item, index );
               if( item[0] != 'L' )
                  item[35] = 0;
               if( (index >= topline) && (index < topline+20) )
                  IsChosen( index-topline, item );
               stod( Select, item, index );
            }
            break;

         case 'Q':
         case 'q':
            for( index = 1; index <= nempty( Select ); index++ )
            {
               rcld( Select, item, index );
               if( item[0] == 'L' )
                  item[35] = 0;
               if( (index >= topline) && (index < topline+20) )
                  IsChosen( index-topline, item );
               stod( Select, item, index );
            }
            break;
       }
   }
   while( !done );

   rmFlist( &Select );
   return( ch == Esc );
}
