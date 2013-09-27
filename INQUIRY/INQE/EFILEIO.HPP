/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */


// EFILEIO.HPP
// File I/O functions used by Inquiry/E
 /===

int fexist( char *fname )
{
   int x;

   FILE *testfile = fopen( fname, "rb" );
   x = (testfile != NULL);
   fclose( testfile );
   return( x );
}

#define MAX 38
#define PASSWORD "\x21\x26\x60\xFE\x36\x11\x11\x17\x7E\x57\x55\x32\x0C\x01\x13\x33\x34\x31\x01\x17\x05\x31\x32\x12\x11\x11\x17\x05"
typedef char str[MAX];

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
      };
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

void SkipCryptFlist( FILE *infile )
// Load an encrypted list of strings... assuming str is the type.
// the file must already be opened, and the first number encountered should
// be the number of lines...
{
   int num, index;
   str temp;

   num = fgetc( infile );
   if( !feof( infile ) )
      for( index = 1; index <= num; index++ )
         fread( temp, sizeof( str ), 1, infile );
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
   strcpy( temp, "*Edit this entry to add a new one" );
   *(int*)(&temp[36]) = total;
   iquedptr( LoadMe, temp );
}

int YesNoBox( str thing )
{
   fg_cursor( 0 );
   char item[4];
   Flist temp = mkFlist( sizeof( item ), 0 );
   tjPickOpts Opts;

   strcpy( item, "Yes" );
   iquedptr( temp, item );
   strcpy( item, "No" );
   iquedptr( temp, item );

   Opts.ux = 35;
   Opts.uy = 10;
   Opts.lines = 2;
   Opts.fore = BLACK;
   Opts.back = LIGHTGRAY;
   Opts.hifore = WHITE;
   Opts.hiback = BLACK;
   Opts.width = 3;
   Opts.topline = 1; Opts.current = 2;
   Opts.dispfun = &strdispfun;
   Opts.keyhan = NULL;
   Opts.PickMe = temp;

   tjFBoxWinTitle( Opts.ux-7, Opts.uy-1, Opts.ux+Opts.width+6, Opts.uy+Opts.lines, RED, LIGHTGRAY, SINGLE, WHITE, LIGHTGRAY, thing, TOP );

   int x = tjPick( Opts );

   rmFlist( &temp );
   tjRmWin( );

   fg_cursor( 1 );
   return( x == 1 );
}

int AbandonChanges( )
{
   return( YesNoBox( "Abandon Changes?" ) );
}

int AreYouSure( )
{
   return( YesNoBox( "Are you sure?" ) );
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

char choosetype( )
{
   fg_cursor( 0 );
   str item;
   Flist temp = mkFlist( sizeof( str ), 0 );
   tjPickOpts Opts;

   strcpy( item, "Lecture (no question)" );
   iquedptr( temp, item );
   strcpy( item, "Multiple Choice Question" );
   iquedptr( temp, item );
   strcpy( item, "Short Answer Question" );
   iquedptr( temp, item );

   Opts.ux = 20;
   Opts.uy = 10;
   Opts.lines = 3;
   Opts.fore = BLACK;
   Opts.back = LIGHTGRAY;
   Opts.hifore = WHITE;
   Opts.hiback = BLACK;
   Opts.width = sizeof( str );
   Opts.topline = Opts.current = 1;
   Opts.dispfun = &strdispfun;
   Opts.keyhan = NULL;
   Opts.PickMe = temp;

   tjFBoxWinTitle( Opts.ux-2, Opts.uy-1, Opts.ux+Opts.width+1, Opts.uy+Opts.lines, RED, LIGHTGRAY, SINGLE, WHITE, LIGHTGRAY, "Choose type", TOP );

   int x = tjPick( Opts );

   rmFlist( &temp );
   tjRmWin( );

   switch( x )
   {
      case 1 :
         return( 'L' );
      case 2:
         return( 'M' );
      case 3:
         return( 'S' );
      default:
         return( 0 );
   }
   fg_cursor( 1 );
}

void CopyCryptFile( char *fname )
{
   str temp;
   int num;
   int index;

   FILE *infile = fopen( fname, "rb" );
   FILE *outfile = fopen( "TEMP.$$$", "wb" );

   while( !feof( infile ) )
   {
      num = fgetc( infile );
      if( !feof( infile ) )
      {
         fputc( num, outfile );
         for( index = 1; index <= num; index++ )
         {
            fread( temp, sizeof( str ), 1, infile );
            fwrite( temp, sizeof( str ), 1, outfile );
         }
      }
   }
   fclose( infile );
   fclose( outfile );
}

void FirstHalf( FILE *&infile, FILE *&outfile, char *fname, int recno )
{
   int index;
   int num, subindex;
   str temp;

   CopyCryptFile( fname );
   infile = fopen( "TEMP.$$$", "rb" );
   outfile = fopen( fname, "wb" );

   for( index = 1; index < recno; index++ )
   {
      num = fgetc( infile );
      if( !feof( infile ) )
      {
         fputc( num, outfile );
         for( subindex = 1; subindex <= num; subindex++ )
         {
            fread( temp, sizeof( str ), 1, infile );
            fwrite( temp, sizeof( str ), 1, outfile );
         }
      }
   }
}

void SecondHalf( FILE *&infile, FILE *&outfile )
{
   int num, subindex;
   str temp;

   while( !feof( infile ) )
   {
      num = fgetc( infile );
      if( !feof( infile ) )
      {
         fputc( num, outfile );
         for( subindex = 1; subindex <= num; subindex++ )
         {
            fread( temp, sizeof( str ), 1, infile );
            fwrite( temp, sizeof( str ), 1, outfile );
         }
      }
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

void menudispfun( tjPickOpts Opts, int linenum )
{
   str item;
   str fname;
   Flist temp;
   int index;

   if( rcld( Opts.PickMe, &temp, linenum ) )
   {
      if( !rcld( temp, fname, 1 ) )
         strcpy( fname, "" );
      if( !rcld( temp, item, 2 ) )
         strcpy( item, "" );
      tjStrPad( item, 37 );
      tjStrPad( fname, 12 );
      if( linenum == Opts.current )
      {
         tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.hifore, Opts.hiback, item );
         tjWriteAt( Opts.ux+13, 1, WHITE, RED, fname );
         for( index = 1; index < 20; index++ )
         {
            if( !rcld( temp, item, 2+index ) )
               strcpy( item, "" );
            tjStrPad( item, 37 );
            tjWriteAt( Opts.ux, 1+index, WHITE, BLUE, item );
         }
      }
      else
         tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, item );
   }
   else
   {
      strcpy( item, "" );
      tjStrPad( item, Opts.width );
      tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, item );
   }
}

int menuhan( tjPickOpts Opts, key ch )
{
   Flist temp;
   str item;

   switch( ch )
   {
      case Ins:
         return( 1 );
         break;

      case 'b':
      case 'B':
         return( 1 );
         break;

      case 'd':
      case 'D':
         return( 1 );
         break;
   }
   return( 0 );
}

int dialoghan( tjPickOpts Opts, key ch )
{
   Flist temp;
   str item;

   switch( ch )
   {
      case Ins:                         // Insert new element before current
         return( 1 );
         break;

      case Del:                         // Delete current element
         return( 1 );
         break;

      case 'U':                         // Cut to buffer...
      case 'u':
         break;

      case 'C':                         // Copy to buffer...
      case 'c':
         break;

      case 'P':                         // Paste from buffer...
      case 'p':
         break;
   }
   return( 0 );
}


key ShowMenu( str fname )
{
   tjPickOpts pOpts;
   Flist temp;

   extern int LISTDATA_WIDTH;
   extern int LISTDATA_DEPTH;
   extern int LISTDATA_LENGTH;
   extern unsigned char LISTDATA [];

   void far *screen;
   int offset = 0;
   int x;

   for (x=0; x<LISTDATA_DEPTH; x++)
   {
       (long) screen = 0xB8000000+x*160+offset;
       memcpy (screen,&LISTDATA[x*LISTDATA_WIDTH*2],LISTDATA_WIDTH*2);
   }


   fg_cursor( 0 );
   pOpts.ux = 42;
   pOpts.uy = 22;
   pOpts.fore = BLACK;
   pOpts.back = LIGHTGRAY;
   pOpts.hifore = WHITE;
   pOpts.hiback = BLACK;
   pOpts.topline = pOpts.current = 1;
   pOpts.width = 37;
   LoadMenu( pOpts.PickMe, "*.TXR" );
   pOpts.dispfun = &menudispfun;
   pOpts.keyhan = &menuhan;
   pOpts.lines = 3;

   if( !nempty( pOpts.PickMe ) )
   {
      rmFlist( &pOpts.PickMe );
      int done = 0;
      do
      {
         pOpts.lastkey = tjGetKey( );
         if( pOpts.lastkey == Ins )
            done = 1;
         if( pOpts.lastkey == Esc );
            done = 1;
      } while( !done );
      return( pOpts.lastkey );
   }

   if( x = tjPick( pOpts ) )
   {
      rcld( pOpts.PickMe, &temp, x );
      rcld( temp, fname, 1 );
   }

   KillMenu( pOpts.PickMe );
   fg_cursor( 1 );
   return( pOpts.lastkey );
}

void dialogdispfun( tjPickOpts Opts, int linenum )
{
   str title;
   str category;
   int index;

   if( rcld( Opts.PickMe, title, linenum ) )
   {
      switch( title[0] )
      {
         case 'L':
            strcpy( category, "Lecture" );
            break;
         case '*':
            strcpy( category, "NEW ENTRY!!!" );
            break;
         case 'M':
            strcpy( category, "Mult. Choice" );
            break;
         case 'S':
            strcpy( category, "Short Answer" );
            break;
      }
      strcpy( title, &title[1] );
      title[32] = 0;
      tjStrPad( title, 32 );
      tjStrPad( category, 12 );
      if( linenum == Opts.current )
      {
         tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.hifore, Opts.hiback, title );
         tjWriteAt( Opts.ux-15, Opts.uy+(linenum-Opts.topline), Opts.hifore, Opts.hiback, category );
      }
      else
      {
         tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, title );
         tjWriteAt( Opts.ux-15, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, category );
      }
   }
   else
   {
      strcpy( title, "" );
      tjStrPad( title, 32 );
      strcpy( category, "" );
      tjStrPad( category, 12 );
      tjWriteAt( Opts.ux, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, title );
      tjWriteAt( Opts.ux-15, Opts.uy+(linenum-Opts.topline), Opts.fore, Opts.back, category );
   }
}

void FlexSort( Flist sortme )
// sort AND remove duplicates.
{
   int index, subindex, maximum;
   str compare, loop;

   for( index = 1; index < nempty( sortme ); index++ )
   {
      maximum = index;
      rcld( sortme, compare, index );
      for( subindex = index+1; subindex <= nempty( sortme ); subindex++ )
      {
         rcld( sortme, loop, subindex );
         if( strcmp( compare, loop ) > 0 )
         {
            maximum = subindex;
            strcpy(compare, loop );
         }
      }
      rcld( sortme, loop, maximum );
      rcld( sortme, compare, index );
      stod( sortme, compare, maximum );
      stod( sortme, loop, index );
   }

   for( index = 2; index <= nempty( sortme ); index++ )
   {
      rcld( sortme, loop, index-1 );
      rcld( sortme, compare, index );
      if( !strcmp( compare, loop ) )
      {
         mkcdptr( sortme, index );
         delete( deln( sortme ) );
         index--;
      }
   }
}

#define CNOTICE "<<< INQUIRY/G Composite Compiled Graphics File! >>>"
#define VNOTICE "Format(c)1988-90 GENUS Microprogramming"
#define NNOTICE "Inquiry / G (c)1991 ADinfinitum"

long thusfar = 0;

struct GXLIBhdr
{
   unsigned int IDnum;
   char Copyright[52];
   char Volume[40];
   unsigned int NumImages;
   char Nonsense[32];
}; // Followed by a 0... then filetags.

struct GXLIBtag
{
   char FileName[13];
   unsigned long Offset;
   unsigned long Size;
   unsigned int Time;
   unsigned int Date;
}; // All optionally followed by a 0 if another tag follows...


void writeGXLIBhdr( FILE *outfile, int number )
{
   GXLIBhdr Temp;

   Temp.IDnum = 0xCA01;
   strcpy( Temp.Copyright, CNOTICE );
   strcpy( Temp.Volume, VNOTICE );
   Temp.NumImages = number;
   strcpy( Temp.Nonsense, NNOTICE );

   fwrite( &Temp, sizeof( Temp ), 1, outfile );
   fputc( 0, outfile );

   thusfar = sizeof( Temp ) + number*( sizeof( GXLIBtag )+1 );
}

void writeGXLIBtag( FILE *outfile, char *name, unsigned long size, int more )
{
   GXLIBtag Temp;
   char newname[13];
   char *extension;
   int length, index;

   if( extension = strchr( name, '.' ) )
      length = strlen( name )-strlen( extension );
   else
      length = strlen( name );

   for( index = 0; index < length; index++ )
      newname[index] = toupper( name[index] );
   for( index = index; index < 8; index++ )
      newname[index] = ' ';
   for( index = 8; index < 13; index++ )
      newname[index] = toupper( extension[index-8] );

   strcpy( Temp.FileName, newname );
   Temp.Offset = thusfar;
   Temp.Size = size;
   Temp.Time = 0x16DB;
   Temp.Date = 0x064A;
   fwrite( &Temp, sizeof( Temp ), 1, outfile );
   if( more )
      fputc( 0, outfile );
   thusfar = thusfar+size;
}

long filesize( char *name )
{
   int useme = open( name, O_RDONLY );
   long temp;

   temp = filelength( useme );
   close( useme );

   return( temp );
};

void BuildMsg( char *fname )
{
   tjFBoxWin( 10, 10, 50, 13, LIGHTRED, RED, BSINGLE );
   tjWriteAt( 12, 11, YELLOW, RED, "WARNING: Could not find graphic file!" );
   tjWriteAt( 12, 12, WHITE, RED, fname );
   tjGetKey( );
   tjRmWin( );
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

void BuildGXL( char *fname )
{
   FILE *infile = fopen( fname, "rb" );
   Flist temp;
   str item;
   Flist biglist = mkFlist( sizeof( str ), 0 );

   tjFBoxWin( 10, 10, 50, 13, BLACK, LIGHTGRAY, BDOUBLE );
   tjWriteAt( 12, 11, DARKGRAY, LIGHTGRAY, "Building GXR library for file:" );
   tjWriteAt( 12, 12, WHITE, LIGHTGRAY, fname );

   skiprecs( infile, 1 );

   while( !feof( infile ) )
   {
      LoadCryptFlist( temp, infile );
      rcld( temp, item, 1 );
      switch( item[0] )
      {
         case 'L':
           break;
         case 'S':
           skiprecs( infile, 1 );
           break;
         case 'M':
           skiprecs( infile, 2 );
           break;
      }
      LoadCryptFlist( temp, infile );
      mkcdptr( temp, 1 );
      delete( deln( temp ) );
      while( nempty( temp ) )
      {
         popd( temp, item );
         strform( item );
         if( fexist( item ) )
            iquedptr( biglist, item );
         else
            if( strcmp( item, "" ) )
               BuildMsg( item );
      }
      rmFlist( &temp );
   }
   FlexSort( biglist );

   str newname;
   strcpy( newname, fname );

   forcefile( newname );
   forcefname( newname );
   strcat( newname, ".GXR" );

   FILE *outfile = fopen( newname, "wb" );
   writeGXLIBhdr( outfile, nempty( biglist ) );

   GXHEADER vhptr;
   PCXHEADER header;
   int index;
   long size;
   for( index = 1; index <= nempty( biglist ); index++ )
   {
      rcld( biglist, item, index );
      pcxGetFileHeader( item, &header );
      if( header.x2-header.x1 > 319 )
      {
         pcxFileImage( gxCMM, item, &vhptr, gxEGA_10 );
         pcxVirtualFile( &vhptr, 0, 0, 319, 349, "TEMP.$$$" );
         pcxFreeImage( &vhptr );
         size = filesize( "TEMP.$$$" );
      }
      else
         size = filesize( item );
      forcefile( item );
      writeGXLIBtag( outfile, item, size, index != nempty( biglist ) );
   }

   FILE *pcxfile;
   char *buffer = new( char[4096] );
   int num;

   for( index = 1; index <= nempty( biglist ); index++ )
   {
      rcld( biglist, item, index );
      pcxGetFileHeader( item, &header );
      if( header.x2-header.x1 > 319 )
      {
         pcxFileImage( gxCMM, item, &vhptr, gxEGA_10 );
         pcxVirtualFile( &vhptr, 0, 0, 319, 349, "TEMP.$$$" );
         pcxFreeImage( &vhptr );
         pcxfile = fopen( "TEMP.$$$", "rb" );
      }
      else
         pcxfile = fopen( item, "rb" );

      do
      {
         num = fread( buffer, sizeof( char ), 4096/*sizeof( buffer )*/, pcxfile );
         fwrite( buffer, sizeof( char ), num, outfile );
      } while( num == 4096/*sizeof( buffer )*/ );

      fclose( pcxfile );
   }

   rmFlist( &biglist );
   fclose( outfile );
   delete( buffer );
   tjRmWin( );
}
