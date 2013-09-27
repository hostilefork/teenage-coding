{
/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */
}

{ Inquiry/M database disk making program }
{$M $4000, 0, 0 }

PROGRAM INQM;

USES
  CRT,
  DOS;

TYPE
   FileName = STRING[12];
   Directory = ARRAY [1..100] OF FileName;
   BigBool = ARRAY [1..100] OF BOOLEAN;

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

PROCEDURE DiskTag( Path : STRING; Disk : INTEGER; Name : STRING; Last : BOOLEAN );
  VAR
     OUTFILE : TEXT;

  BEGIN
     ASSIGN( OUTFILE, Path+'DISK.TAG' );
     REWRITE( OUTFILE );
     WRITELN( OUTFILE, Disk );
     WRITELN( OUTFILE, Name );
     if( Last )
        THEN WRITELN( OUTFILE, 'Last' )
     else
        WRITELN( OUTFILE, 'More' );
     CLOSE( OUTFILE );
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

PROCEDURE AddFile( Source, Dest : STRING; VAR Disk : INTEGER; Name : STRING );
  VAR
    Index : INTEGER;
    Val : INTEGER;

  BEGIN
     Val := Disk;
     WHILE( NOT( SpaceFree( Source, Dest ) ) ) DO
       BEGIN
          if( Val = Disk )
             THEN Disk := Disk+1;
          WRITELN( 'More room needed!!!' );
          WRITELN( 'Insert another blank (formatted) disk in drive ', Dest, '.' );
          WRITELN( 'And press [RETURN].  This should be disk ', disk, '.' );
          READLN;
       END;
     if( Disk <> Val )
        THEN DiskTag( Dest, Disk, Name, FALSE );

     CopyFile( Source, Dest );

     Index := LENGTH( Source );
     WHILE( Source[Index] <> '.' ) DO
        Index := Index-1;
     Source := COPY( Source, 1, Index )+'GXR';

     if( Exist( Source ) )
       THEN
         BEGIN
            Val := Disk;
            WHILE( NOT( SpaceFree( Source, Dest ) ) ) DO
              BEGIN
                if( Val = Disk )
                   THEN Disk := Disk+1;
                WRITELN( 'More room needed!!!' );
                WRITELN( 'Insert another blank (formatted) disk in drive ', Dest, '.' );
                WRITELN( 'And press [RETURN].  This should be disk ', disk, '.' );
                READLN;
              END;
            if( Disk <> Val )
               THEN DiskTag( Dest, Disk, Name, FALSE );

           CopyFile( Source, Dest )
         END;
  END;

PROCEDURE DeSelect( VAR X : BigBool );
  VAR
    Index : INTEGER;

  BEGIN
    FOR Index := 1 TO 100 DO
       X[Index] := FALSE;
  END;

FUNCTION LoadDir( VAR LoadMe : Directory; Mask : STRING ) : INTEGER;
  VAR
    DirInfo : SearchRec;
    Index : INTEGER;

  BEGIN
    Index := 0;
    FindFirst( Mask, Archive, DirInfo);
    while DosError = 0 do
    begin
      Index := Index+1;
      LoadMe[Index] := DirInfo.Name;
      FindNext(DirInfo);
    end;
    LoadDir := Index;
  END;

FUNCTION ChosenCount( Countme : BigBool; NumFiles:INTEGER ) : INTEGER;
  VAR
    Index, Count : INTEGER;

  BEGIN
     Count := 0;
     FOR Index := 1 TO NumFiles DO
       IF( Countme[Index] )
         THEN Count := Count+1;
     ChosenCount := Count;
  END;

PROCEDURE PickDir( VAR PickMe : Directory; VAR Chosen : BigBool; NumFiles:INTEGER );
  VAR
    Current : INTEGER;
    Key : CHAR;
    Index : INTEGER;

  BEGIN
    Current := 1;
    REPEAT
       textcolor( WHITE );
       textbackground( CYAN );
       FOR Index := Current TO Current+10 DO
          BEGIN
             if( Index <= NumFiles )
               THEN
                  BEGIN
                    gotoxy( 4, 7+Index-Current );
                    IF( Chosen[Index] )
                      THEN WRITE( '*' )
                      ELSE WRITE( ' ' );
                    WHILE( LENGTH( PickMe[Index] ) < 12 ) DO
                       PickMe[Index] := PickMe[Index]+' ';
                    WRITELN( PickMe[Index] );
                    textbackground( BLUE );
                    textcolor( LIGHTGRAY );
                 END
              ELSE
                BEGIN
                   textcolor( LIGHTGRAY );
                   textbackground( BLUE );
                   gotoxy( 4, 7+Index-Current  );
                   WRITELN( '             ' );
                END;
          END;
       gotoxy( 80, 25 );
       Key := ReadKey;
       CASE Key OF
          #72 :
             IF( Current > 1 )
                THEN Current := Current-1;
          #80 :
             IF( Current < NumFiles )
                THEN Current := Current+1;
          ' ' :
             BEGIN
               Chosen[Current] := NOT( Chosen[Current] );
               textcolor( WHITE );
               textbackground( CYAN );
               gotoxy( 4, 7+Current );
               IF( Chosen[Current] )
                  THEN WRITE( '*' )
                  ELSE WRITE( ' ' );
             END;
       END;
    UNTIL( Key IN [#27,#13] );
    IF( Key = #27 )
       THEN DeSelect( Chosen );
    textcolor( Lightgray );
    textbackground( BLACK );
  END;

PROCEDURE Main;
  VAR
     Listing : Directory;
     Chosen : BigBool;
     Index : INTEGER;
     Disk : INTEGER;
     Destination : STRING;
     SysPath : STRING;
     Name : STRING;
     NumFiles : INTEGER;

  BEGIN
     SysPath := ParamStr( 0 );
     Index := LENGTH( SysPath );
     WHILE( SysPath[Index] <> '\' ) DO
        Index := Index-1;
     SysPath := COPY( SysPath, 1, Index );
     textcolor( LIGHTGRAY );
     textbackground( BLACK );
     clrscr;
     WRITELN( 'INQuiry/G database disk maker.' );
     WRITELN( 'Copyright (c) 1991 ADinfinitum' );
     WRITELN;
     WRITELN( 'Choose databases you want to transfer.  SPACE Toggle: ENTER Accept: Esc QUIT' );
     NumFiles := LoadDir( Listing, '*.TXR' );
     DeSelect( Chosen );
     PickDir( Listing, Chosen, NumFiles );
     Disk := 1;

     if( ChosenCount( Chosen, NumFiles ) > 0 )
       THEN
         BEGIN
           clrscr;
           WRITELN( 'Enter 8 character name to group these databases under...' );
           WRITE( '>>> ' );
           READLN( Name );
           WRITELN( 'Enter Destination for selected files: (Default is A:\)' );
           WRITE( '>>> ' );
           READLN( Destination );
           IF( Destination = '' )
              THEN Destination := 'A:\';
           WRITELN( 'Insert DISK 1 and press [ENTER].' );
           READLN;
           if( Destination[LENGTH( Destination )] <> '\' )
              THEN Destination := Destination+'\';

            CopyFile( SysPath+'INQINST.DAT', Destination+'INSTALL.EXE' );

            DiskTag( Destination, Disk, Name, FALSE );

            FOR Index := 1 TO NumFiles DO
              IF Chosen[Index]
                THEN AddFile( Listing[Index], Destination, Disk, Name );
            DiskTag( Destination, Disk, Name, TRUE );
            WRITELN( 'Disk set completed.' );
         END;
  END;

BEGIN
   Main;
END.
