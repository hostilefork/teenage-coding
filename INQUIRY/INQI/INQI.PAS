{
/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */
}

{ Inquiry/I installation program for databases }

PROGRAM INQI;

{$M $4000, 0, 0}

USES
  CRT, DOS;

PROCEDURE MakeBatch( Name : STRING );
  VAR
    OUTFILE : TEXT;

  BEGIN
    ASSIGN( OUTFILE, Name+'.BAT' );
    REWRITE( OUTFILE );
    WRITELN( OUTFILE, 'CD \', Name );
    WRITELN( OUTFILE, 'INQG' );
    WRITELN( OUTFILE, 'CD \' );
    CLOSE( OUTFILE );
  END;

PROCEDURE GetTag( Path : STRING; VAR Disk : INTEGER; VAR Name : STRING; VAR Last : BOOLEAN );
  VAR
     INFILE : TEXT;
     LastTemp : STRING;

  BEGIN
     ASSIGN( INFILE, Path+'DISK.TAG' );
     RESET( INFILE );
     READLN( INFILE, Disk );
     READLN( INFILE, Name );
     READLN( INFILE, LastTemp );
     Last := (LastTemp = 'Last');
     CLOSE( INFILE );
  END;

PROCEDURE CopyFile( Source, Dest : STRING );
  BEGIN
    SwapVectors;
    EXEC( GetEnv( 'COMSPEC' ), '/C COPY '+Source+' '+Dest );
    SwapVectors;
  END;

FUNCTION Drive( Path : STRING ) : BYTE;
  BEGIN
    IF ( Path[2] <> ':' )
       THEN Path := ParamStr( 0 );
    Drive := ORD( Path[1] )-ORD( 'A' )
  END;

FUNCTION SpaceFree( Source, Dest : STRING ) : BOOLEAN;
  VAR
     INFILE : FILE OF BYTE;
     Size : LONGINT;
     Room : LONGINT;
     Old : STRING;

  BEGIN
     ASSIGN( INFILE, Source );
     RESET( INFILE );
     Size := FileSize( INFILE );
     CLOSE( INFILE );
     Getdir( 0, Old );
     Chdir( Dest );
     Room := DiskFree( 0 );
     Chdir( Old );
     SpaceFree := Room >= Size;
  END;

FUNCTION Exist( Path : STRING ) : BOOLEAN;
  VAR
    INFILE : TEXT;

  BEGIN
    ASSIGN( INFILE, Path );
    {$I-}
    reset( INFILE );
    {$I+}
    if( IOresult = 0 )
      THEN
        BEGIN
           Exist := TRUE;
           CLOSE( INFILE );
        END
      ELSE Exist := FALSE;
  END;

PROCEDURE Main;
  VAR
     Last : BOOLEAN;
     Index : INTEGER;
     Disk : INTEGER;
     Destination : STRING;
     SysPath : STRING;
     Name : STRING;
     NumFiles : INTEGER;
     Val : INTEGER;

  BEGIN
     SysPath := ParamStr( 0 );
     Index := LENGTH( SysPath );
     WHILE( SysPath[Index] <> '\' ) DO
        Index := Index-1;
     SysPath := COPY( SysPath, 1, Index );
     textcolor( LIGHTGRAY );
     textbackground( BLACK );
     clrscr;
     GetTag( SysPath, Disk, Name, Last );
     WRITELN( 'Inquiry database installation program' );
     WRITELN( 'Copyright (c) 1991 ADinfinitum' );
     WRITELN( '-----===>>> For non-commercial purposes' );
     WRITELN( 'Please enter path to install this database to.' );
     WRITELN( 'Default is C:\', Name );
     WRITE( '>>> ' );
     READLN( Destination );
     IF( Destination = '' )
        THEN Destination := 'C:\'+Name;
     {$I-}
     CHDIR( Destination );
     {$I+}
     if( IOresult <> 0 )
       THEN BEGIN
        MKDIR( Destination );
        CHDIR( Destination );
       END;
         BEGIN
           CopyFile( Syspath+'*.?XR', Destination );
           WHILE( NOT( Last ) )DO
             BEGIN
               Val := Disk+1;
               REPEAT
                  IF( Val <> Disk+1 )
                     THEN WRITELN( 'WRONG DISK!  TRY AGAIN!!!' );
                  WRITELN( 'Please insert disk ', disk+1, ' into ', SysPath,' !!!' );
                  WRITELN( 'Press [ENTER] when ready.' );
                  READLN;
                  GetTag( SysPath, Val, Name, Last );
               UNTIL (Val = Disk+1);
               Disk := Val;
               CopyFile( SysPath+'*.?XR', Destination );
             END;
             WRITELN( 'INSTALLATION COMPLETED!  Make sure the Inquiry/G directory is in your path!' );
             WRITELN( 'Type ', Name, ' from your root directory to start.' );
         END;
     CHDIR( '\' );
     MakeBatch( Name );
  END;

BEGIN
   Main;
END.

