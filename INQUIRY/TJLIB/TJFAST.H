/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/09/27/inquiry-more-teenage-turbo-c-coding/
 */



//===
// TJFAST.H
//---
// Thomas Jefferson Software Development Group Library
// Fast Video Routines Header File
//---
// History:
//    07/06/1991 -
//       v1.0 created by Brian Dickens.
//---
// Notes:
//    Right now, this makes use of the Fastgraph library routines, and thus
//    can never be distributed in its current form.  However, this is the only
//    one of the library files that accesses Fastgraph routines, and as soon
//    as I get a chance to write some of my own, the dependent libraries will
//    still work and be completely PD.  K-Rad!
//===

///////////////////////////////////////////////////////// TYPE DEFINITIONS

struct scr
{
   int rows;
   int cols;
   char *buffer;
   scr *next; // used only for windows...
   int ux, uy; // again only for windows...
};

enum position
{ TOPLEFT, TOP, TOPRIGHT,
  LEFT, RIGHT,
  BOTTOMLEFT, BOTTOM, BOTTOMRIGHT };

typedef char frame[8];

///////////////////////////////////////////////////// CONSTANT DECLARATIONS

const frame NONE =
{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
const frame SINGLE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame DOUBLE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame SINGLESIDE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame DOUBLESIDE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame BSINGLE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame BDOUBLE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame BSINGLESIDE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame BDOUBLESIDE =
{ '�', '�', '�', '�', '�', '�', '�', '�' };
const frame SOLID =
{ '�', '�', '�', '�', '�', '�', '�', '�' };

//////////////////////////////////////////// INITIALIZATION AND DESTRUCTION

void tjInit( );
// Initialize the tj system.

void tjClose( );
// Close up the tj system.

///////////////////////////////////// GETTING AND SETTING VIDEO INFORMATION

int tjScrHeight( );
// Returns the screen height (in characters).

int tjScrWidth( );
// Returns the screen width (in characters).

int tjGetMode( );
// Gets the current text mode.

void tjSetMode( int mode );
// Sets the current text mode.

int tjColorMode( );
// Is the current mode color?

///////////////////////////////////////// SAVING AND RESTORING SCREEN AREAS

scr *tjSaveScr( int ux = 1, int uy = 1, int lx = 80, int ly = 25 );
// Save a region of the screen.

int tjRestScr( scr *restore, int ux = 1, int uy = 1 );
// Restore a region of the screen.

int tjKillScr( scr *kill );
// Kill a screen buffer once allocated.

/////////////////////////////////////////////// FAST VIDEO WRITING ROUTINES

void tjWriteChar( int x, int y, int fore, int back, char ch );
// Put a character at a specific x and y location.

void tjPutChar( int x, int y, char ch );
// Put a character at x, y but don't disturb attribute.

void tjPutAttr( int x, int y, int fore, int back );
// Put an attribute at x, y but don't disturb character.

void tjWriteAt( int x, int y, int fore, int back, char *str );
// Write a string at a specific x and y location in a specific color.

void tjPlainWrite( int x, int y, char *str );
// Write a string without disturbing the attributes underneath.

void tjAttrWrite( int x, int y, int fore, int back, int num );
// Write an attribute a specified number of times without disturbing chars.

void tjFill( int ux, int uy, int lx, int ly, char ch, int fore, int back );
// Fill an area with a single character and attribute.

void tjFillChar( int ux, int uy, int lx, int ly, char ch );
// Fill a region with text characters, leaving attributes undisturbed.

void tjFillAttr( int ux, int uy, int lx, int ly, int fore, int back );
// Fill a region with an attribute, leaving the characters undisturbed.

//////////////////////////////////////////////////////// WINDOWING ROUTINES

void tjBox( int ux, int uy, int lx, int ly, int fore, int back, frame style );
// Make an innocent-looking little box on the screen, don't store underneath.

void tjFBox( int ux, int uy, int lx, int ly, int fore, int back, frame style );
// Make a filled box on the screen, still don't store underneath.

int tjMkWin( int ux, int uy, int lx, int ly );
// Make a window, store contents in the winstack, etc.

int tjRmWin( );
// Remove a window, redisplay winstack contents.

int tjBoxWin( int ux, int uy, int lx, int ly, int fore, int back, frame style );
// Make a window with a box, store contents in the winstack, etc.

int tjFBoxWin( int ux, int uy, int lx, int ly, int fore, int back, frame style );
// Make a window with a filled box, store contents in winstack, etc.

void tjTitle( int ux, int uy, int lx, int ly, int tfore, int tback, char *title,
              position titlepos );
// Write a title at one of six positions.

int tjBoxWinTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                   int tfore, int tback, char *title, position titlepos );
// Make a window with a box, store contents, put a title.

int tjFBoxWinTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                    int tfore, int tback, char *title, position titlepos );
// Make a window with a filled box, store contents, put a title.

void tjBoxTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                 int tfore, int tback, char *title, position titlepos );
// Make a window with a box, DO NOT store contents, put a title.

void tjFBoxTitle( int ux, int uy, int lx, int ly, int fore, int back, frame style,
                  int tfore, int tback, char *title, position titlepos );
// Make a window with a filled box, DO NOT store contents, put a title.
