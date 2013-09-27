/*
 * Archival code written when I was 15, see blog about it:
 *     http://hostilefork.com/2013/09/16/manykeys-my-open-source-roots/
 */



/*
   MANYKEYS.C - Read multiple keypresses
   Created by Brian Dickens
   Special thanks to Richard Biffl for all his help and advice

   Released to the public domain.
   These routines may not be sold in any way shape or form.
*/

#include <dos.h>
   /* Needed for outportb() and inportb() */
#include "manykeys.h"

int manykeystatus = 0;
   /* Is MANYKEYS installed via manykeyon()? */

struct PRESSREC
   /* Record used to store information for a single key */
{
    int pressed:1;
       /* Is the key CURRENTLY pressed? */
    int count:7;
       /* Number of times pressed since last check */
} volatile keystatus[128];
   /* keyboard status variable... CURRENT readings */

#ifdef __CPLUSPLUS
void interrupt (*oldint9)(...);
#else
void interrupt (*oldint9)();
#endif
   /* Old keyboard interrupt storage variable */

#ifdef __CPLUSPLUS
void interrupt newint9(...)
#else
void interrupt newint9()
#endif
{
   static unsigned char keyval = 0;
   static unsigned char temp = 0;
   static unsigned char extended = 0;

   keyval = inportb( 0x60 );
      /* 0x60 is keyboard port */

   if( keyval == 0xE0 )
      /* extended key code, loop out. */
   {
      extended = 1;
      keyval = inportb( 0x61 );
      outportb( 0x61, keyval | 0x80 );
      outportb( 0x61, keyval );
         /* Reset keyboard */
      outportb( 0x20, 0x20 );
      return;
   }

   if( extended ) /* extended key code */
   {
      extended = 0;
      switch( keyval%128 )
      {
         case 0x52: temp = 0x00; break;
         case 0x47: temp = 0x01; break;
         case 0x49: temp = 0x02; break;
         case 0x53: temp = 0x03; break;
         case 0x4F: temp = 0x04; break;
         case 0x51: temp = 0x05; break;
         case 0x48: temp = 0x06; break;
         case 0x4B: temp = 0x07; break;
         case 0x50: temp = 0x08; break;
         case 0x4D: temp = 0x09; break;
         case 0x35: temp = 0x0A; break;
         case 0x1C: temp = 0x0B; break;
         case 0x38: temp = 0x0C; break;
         case 0x1D: temp = 0x0D; break;
         default:
             keyval = inportb( 0x61 );
             outportb( 0x61, keyval | 0x80 );
             outportb( 0x61, keyval );
                /* Reset keyboard */
             outportb( 0x20, 0x20 );
             return;
      }
      if( keyval > 127 )
         keyval = 128+temp+0x59;
      else
         keyval = temp+0x59;
   }

   if( keyval > 127 )
      keystatus[keyval-128].pressed = 0;
         /* Signal that key is NOT currently pressed */
   else
   {
      keystatus[keyval].pressed = 1;
         /* Signal that key is currently pressed */
      keystatus[keyval].count++;
         /* Increment press count for key */
   }

   keyval = inportb( 0x61 );
   outportb( 0x61, keyval | 0x80 );
   outportb( 0x61, keyval );
      /* Reset keyboard */

   outportb( 0x20, 0x20 );
}

void manykeyson()
   /* Turn off BIOS method of getting keystrokes and enable manykey */
{
   if( manykeystatus == 0 )
   {
      oldint9 = getvect( 0x09 );
      setvect( 0x09, newint9 );
      manykeystatus = 1;
   }
}

void manykeysoff()
   /* Turn off many key support and return to BIOS method */
{
   if( manykeystatus == 1 )
      setvect( 0x09, oldint9 );
}

int checkcount( int key )
   /* Get a press count for a particular key but DON'T clear it */
{
   int temp;

   disable();
   temp = keystatus[key].count;
   enable();
   return( temp );
}

int presscount( int key )
   /* Get a press count for a particular key and clear press count */
{
   int temp;

   disable();
   temp = keystatus[key].count;
   keystatus[key].count = 0;
   enable();
   return( temp );
}

int ispressed( int key )
   /* Is a particular key pressed AT THIS MOMENT? */
{
   return( keystatus[key].pressed );
}

int firstpressed( )
   /* Returns code of the lowest order key pressed, NIL if no keys pressed */
{
   int index;
   int temp = 0;

   disable();
   for( index = 0; index < 128; index++ )
      if( keystatus[index].pressed )
      {
         temp = index;
         index = 128;
      }
   enable();
   return( temp );
}

int firstcount( )
   /* Returns code of the lowest order key with a presscount > 0 */
{
   int index;
   int temp = 0;

   disable();
   for( index = 0; index < 128; index++ )
      if( keystatus[index].count )
      {
         temp = index;
         index = 128;
      }
   enable();
   return( temp );
}

int totalpressed( )
   /* Counts the number of keys currently pressed */
{
   int index;
   int temp = 0;

   disable();
   for( index = 0; index < 128; index++ )
      if( keystatus[index].pressed )
         temp++;
   enable();
   return( temp );
}

void clearallcounts( )
   /* Clear all press counts to 0 */
{
   int index;

   disable();
   for( index = 0; index < 128; index++ )
      keystatus[index].count = 0;
   enable();
}

char *keyname( int key )
   /* Given a key returns an ASCII name for it (good for config programs) */
{
   static char *keynames[] =
   {
      "", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
      "BKSP", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[",
      "]", "ENTER", "LCTRL", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";",
      "'", "`", "LSHIFT", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
      "/", "RSHIFT", "N*", "LALT", "SPACE", "CAPS", "F1", "F2", "F3", "F4",
      "F5", "F6", "F7", "F8", "F9", "F10", "NUM", "SCROLL", "N7", "N8", "N9",
      "N-", "N4", "N5", "N6", "N+", "N1", "N2", "N3", "N0", "N.", "", "", "",
      "F11", "F12", "INS", "HOME", "PGUP", "DEL", "END", "PGDN", "UP", "LEFT",
      "DOWN", "RIGHT", "N/", "NENTER", "RALT", "RCTRL"
   };

   if( key <= RCTRL )
      return( keynames[key] );
   else
      return( keynames[0] );
}
