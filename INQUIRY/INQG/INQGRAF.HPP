/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



typedef char set[32];

void beep( int time, int freq )
{
   sound( freq );
   delay( time );
   nosound( );
}

void ClearSet( set ClearMe )
{
   int Index;

   for( Index = 0; Index < 32; Index++ )
      ClearMe[Index] = 0;
}

int InSet( set SearchMe, char FindMe )
{
   return( SearchMe[FindMe/8] & (1 << (FindMe%8)) );
}

void AddSet( set AddToMe, char AddMe )
{
   AddToMe[AddMe/8] = AddToMe[AddMe/8] | (1 << (AddMe%8));
}

void AddLet( Flist Add, char num )
{
   char temp[38];
   strcpy( temp, "(   )\xC4\xC4\xC4 \xC4\xC4\xC4\xC4\xC4 \xC4\xC4\xC4\xC4 \xC4\xC4\xC4 \xC4\xC4 \xC4\xC4 \xC4 \xC4 \xC4 " );
   temp[2] = num;
   iquedptr( Add, temp );
}

void AddBlank( Flist Add )
{
   char temp[38];
   strcpy( temp, "" );
   iquedptr( Add, temp );
}

void forcefname( char *way )
{
   char *ptr;

   ptr = strrchr( way, '.' );
   if( ptr != NULL )
      *ptr = 0;
}

void forcefile( char *way )  // No way!  Way!  NO WAY!  WAY!
{
   char *ptr;

   ptr = strrchr( way, '\\' );
   if( ptr != NULL )
      strcpy( way, ptr );
}

void StripTrailing( char *stripme )
{
   while( (strlen( stripme ) > 0) && (stripme[strlen( stripme )-1] == ' ') )
      stripme[strlen( stripme )-1] = 0;
}

void strform( char *dome )
{
   int Index = 0;

   while( Index < strlen( dome ) )
      if((dome[Index] == ' ') && ((dome[Index+1] == ' ') || (Index == 0)))
         strcpy( &dome[Index], &dome[Index+1] );
      else
      {
         dome[Index] = toupper( dome[Index] );
         Index++;
      }
   StripTrailing( dome );
}

int LoadHalf( char *FileName, int which, char *libname )
//===
// Display a graphic on either the left or right of the screen.
//===
{
   GXLIB lib;

   retcode = gxOpenLib( gxGXL, libname, &lib, NULL, 0 );
   strform( FileName );

   if( which == fxRIGHT )
      if( !strcmp( "TBOX.PCC", FileName ) )
         retcode = pcxLibDisplay( &lib, FileName, 320, 0, 1 );
      else
         if( strcmp( rightgraf, FileName ) )
         {
            retcode = pcxLibDisplay( &lib, FileName, 320, 296, 1 );
            strcpy( rightgraf, FileName );
            strcpy( rightlib, libname );
         } else;
   else
      if( strcmp( leftgraf, FileName ) )
      {
         retcode = pcxLibDisplay( &lib, FileName, 0, 0, 1 );
         strcpy( leftgraf, FileName );
         strcpy( leftlib, libname );
      }

   retcode = gxCloseLib( &lib );
   return( retcode );
};

void ShowHalf( int which )
// Show hidden pages
{
   fg_sethpage( 1 );
   switch( which )
   {
      case fxRIGHT:
         fg_save( 0, 319, 0, 349 );
         break;
      case fxLEFT:
         fg_save( 320, 639, 0, 349 );
         break;
      case fxLEFT+fxRIGHT:
         break;
   }
   fg_fadein( 0 );
   fg_setcolor( grWHITE );
   if( which == fxLEFT )
      LoadHalf( "TBOX.PCC", fxRIGHT, syspath );
}

void Help( char *fname )
{
   char temp[13];
   char templib[80];

   strcpy( temp, leftgraf );
   strcpy( templib, leftlib );
   LoadHalf( fname, fxLEFT, syspath );
   ShowHalf( fxLEFT );
   tjGetKey( );
   LoadHalf( "NOTHING.PCC", fxLEFT, syspath );
   LoadHalf( temp, fxLEFT, templib );
   ShowHalf( fxLEFT );
}

void PutArrow( int Position, int OnOff )
//===
// Put (or erase) either an up or down arrow on the window.
//===
{
   if( Position == fxDOWN )
      retcode = gxPutImage( OnOff ? Arrows[2] : Arrows[0], gxSET,
                            76*8, 8, 0 );
   else
      retcode = gxPutImage( OnOff ? Arrows[1] : Arrows[0], gxSET,
                            78*8, 8, 0 );
}

void PutNum( int num )
// Put a number in the little Tab--STab vacancy
{
   char temp[10];

   grSetColor( grWHITE );
   grSetBkColor( grBLACK );
   itoa( num, temp, 10 );
   temp[2] = 0;
   if( strlen( temp ) < 2 )
   {
     temp[1] = temp[0];
     temp[0] = ' ';
   }
   grMoveTo( 400, 300 );
   retcode = grSetTextStyle( grTXT8X14, grOPAQUE );
   grOutText( temp );
}

void PutText( int YLoc, char *PutMe )
//===
// Completely overwrite a text string on the screen in the window.
//===
{
   char Temp[MAXCOLS+1];
   int Index;

   for( Index = 0; Index < MAXCOLS; Index++ )
      Temp[Index] = PutMe[Index];
   for( Index = strlen( PutMe ); Index < MAXCOLS; Index++ )
      Temp[Index] = ' ';
   Temp[MAXCOLS] = 0;
   retcode = grMoveTo( TEXTUX, TEXTUY+YLoc*13 );
   retcode = grSetColor( grBLACK );
   retcode = grSetBkColor( grWHITE );
   retcode = grSetTextStyle( grTXT8X14, grOPAQUE );
   retcode = grOutText( Temp );
}

void SelMark( int YLoc, int on )
{
   char Temp;
   retcode = grMoveTo( TEXTUX, TEXTUY+YLoc*13 );
   retcode = grSetColor( grRED );
   retcode = grSetBkColor( grWHITE );
   retcode = grSetTextStyle( grTXT8X14, grOPAQUE );
   if( on )
      retcode = grOutText( "\x10" );
   else
      retcode = grOutText( " " );

   retcode = grMoveTo( TEXTUX+36*8, TEXTUY+YLoc*13 );
   if( on )
      retcode = grOutText( "\x11" );
   else
      retcode = grOutText( " " );
   retcode = grSetColor( grBLACK );
}

void IsChosen( int YLoc, char *Temp )
{
   grSetColor( grBLACK );
   grSetBkColor( grWHITE );

   if( Temp[0] == 'L' )
   {
      grMoveTo( TEXTUX+1*8, TEXTUY+YLoc*13 );
      if( Temp[35] )
         retcode = grOutText( "\xFE" );
      else
         retcode = grOutText( "\xF0" );

   }
   else
   {
      grMoveTo( TEXTUX+2*8, TEXTUY+YLoc*13 );
      if( Temp[35] )
         retcode = grOutText( "\x07" );
      else
         retcode = grOutText( "\x09" );
   }
}

void PutSelText( int YLoc, char *PutMe )
//===
// Completely overwrite a text string on the screen in the window.
//===
{
   char Temp[MAXCOLS+1];
   int Index;

   if( PutMe[0] == 'L' )
   {
      for( Index = 1; Index <= 33; Index++ )
         Temp[Index+2] = PutMe[Index];
      Temp[0] = Temp[1] = Temp[2] = ' ';
   }
   else
   {
      for( Index = 1; Index <= 33; Index++ )
         Temp[Index+3] = PutMe[Index];
      Temp[0] = Temp[1] = Temp[2] = Temp[3] = ' ';
   }
   PutText( YLoc, Temp );
   IsChosen( YLoc, PutMe );
}


void PutGray( int YLoc, char *PutMe, int chosen )
//===
// Completely overwrite a text string on the screen in the window.
//===
{
   char Temp[MAXCOLS+1];
   int Index;

   for( Index = 0; Index < MAXCOLS; Index++ )
      Temp[Index] = PutMe[Index];
   for( Index = strlen( PutMe ); Index < MAXCOLS; Index++ )
      Temp[Index] = ' ';
   Temp[MAXCOLS] = 0;
   retcode = grMoveTo( TEXTUX, 299+YLoc*14 );
   if( chosen )
   {
      retcode = grSetColor( grWHITE );
      retcode = grSetBkColor( grBLACK );
   }
   else
   {
      retcode = grSetColor( grBLACK );
      retcode = grSetBkColor( grGRAY );
   }
   retcode = grSetTextStyle( grTXT8X14, grOPAQUE );
   retcode = grOutText( Temp );
}

void CoolRect( int ux, int uy, int lx, int ly, int color )
//===
// Draw a really cool rectangle.
//===
{
   retcode = grSetFillStyle( grFSOLID, color, grOPAQUE );
   retcode = grDrawRect( ux+1, uy+1, lx, ly, grFILL );
   retcode = grSetColor( color+8 );
   retcode = grDrawLine( ux, uy, ux, ly );
   retcode = grDrawLine( ux, uy, lx, uy );
}

void PutTitle( char *PutMe, int color )
//===
// Center a title at one of the two title locations.
//===
{
   char Temp[33];
   int Index;

   CoolRect( TITLEUX, TITLEUY, TITLEUX+TITLECOLS*8-1, TITLEUY+8-1, color );
   retcode = grSetTextStyle( grTXT8X8, grTRANS );

   for( Index = 0; Index < 32; Index++ )
      Temp[Index] = PutMe[Index];
   Temp[32] = '\0';

   Index = ( 32-strlen( Temp ) ) / 2;

   retcode = grSetColor( WHITE );
   retcode = grMoveTo( TITLEUX+Index*8, TITLEUY );
   retcode = grOutText( Temp );
   retcode = grSetTextStyle( grTXT8X14, grOPAQUE );
};


void ScrollWin( int Direction )
//===
// Scroll the window of text either up or down and put a string on the
// newly opened line.
//===
{
   if( Direction == fxUP )
      fg_scroll( TEXTUX, TEXTUX+MAXCOLS*8, TEXTUY+13, TEXTUY+MAXROWS*13,
                 -13, 1 );
   else
      fg_scroll( TEXTUX, TEXTUX+MAXCOLS*8, TEXTUY, TEXTUY+MAXROWS*13-13,
                 13, 1 );
}

void ScrollText( int Direction, char *ReplaceMe )
//===
// Scroll the window of text either up or down and put a string on the
// newly opened line.
//===
{
   ScrollWin( Direction );
   if( Direction == fxUP )
      PutText( MAXROWS-1, ReplaceMe );
   else
      PutText( 0, ReplaceMe );
}
