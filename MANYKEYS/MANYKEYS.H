/*
 * Archival code written when I was 15, see blog about it:
 *     http://hostilefork.com/2013/09/16/manykeys-my-open-source-roots/
 */



/*
   MANYKEYS.H - Read multiple keypresses
   Created by Brian Dickens
   Special thanks to Richard Biffl for all his help and advice

   Released to the public domain.
   These routines may not be sold in any way shape or form.
*/

#ifndef __MANYKEYS_H
#define __MANYKEYS_H

enum keyconst
{
   NIL, ESCAPE, M1, M2, M3, M4, M5, M6, M7, M8, M9, M0, MINUS, EQUALS,
   BACKSPACE, TAB, Q, W, E, R, T, Y, U, I, O, P, LBRACKET, RBRACKET, ENTER,
   LCTRL, A, S, D, F, G, H, J, K, L, SEMICOLON, APOSTROPHE, BAPOSTROPHE,
   LSHIFT, BACKSLASH, Z, X, C, V, B, N, M, COMMA, PERIOD, SLASH, RSHIFT, NSTAR,
   LALT, SPACE, CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUM, SCROLL, N7,
   N8, N9, NMINUS, N4, N5, N6, NPLUS, N1, N2, N3, N0, NPERIOD, NIL2, NIL3,
   NIL4, F11, F12, INS, HOME, PGUP, DEL, END, PGDN, UP, LEFT, DOWN, RIGHT,
   NSLASH, NENTER, RALT, RCTRL
};

/* Note: "M" prefixed constants mean "MAIN" as in M1 means MAIN - 1 key.
         "N" prefixed constants mean "NUMERIC" as in N1 means NUMERIC - 1 key
         "L" prefixed constants mean "LEFT" as in LALT means LEFT ALT KEY
         "R" prefixed constants mean "RIGHT" as in RALT means RIGHT ALT KEY */

void manykeyson();
   /* Turn off BIOS method of getting keystrokes and enable manykey */

void manykeysoff();
   /* Turn off many key support and return to BIOS method */

int checkcount( int key );
   /* Get a press count for a particular key but DON'T clear it */

int presscount( int key );
   /* Get a press count for a particular key and clear press count */

int ispressed( int key );
   /* Is a particular key pressed AT THIS MOMENT? */

int firstpressed( );
   /* Returns code of the lowest order key pressed, NIL if no keys pressed */

int firstcount( );
   /* Returns code of the lowest order key with a presscount > 0 */

int totalpressed( );
   /* Counts the number of keys currently pressed */

void clearallcounts( );
   /* Clear all press counts to 0 */

char *keyname( int key );
   /* Given a key returns an ASCII name for it (good for config programs) */

#endif
