/*

	dump.c
	7/20/89

	Copyright 1989
	John W. Small
	All rights reserved

	PSW / Power SoftWare
	P.O. Box 10072
	McLean, Virginia 22102 8072

*/

#include <stdlib.h>		/*  exit()  */
#include <string.h>     /*  strlen(), strcpy()  */
#include <io.h>         /*  open(), close(), read()  */
#include <fcntl.h>      /*  O_RDONLY, O_BINARY  */
#include <conio.h>      /*  clrscr(), clreol(),  */
						/*  gotoxy(), window(),  */
						/*  textcolor(), cprintf(),  */
						/*  textbackground(), putch(),  */
						/*  window()  */
#include <dos.h>		/*  int86(), union REGS;  */
#include <flexlist.h>	/*  mkFlist(), rmFlist(),  */
						/*  nempty(), iquedptr(),  */
						/*  ncur(), mkcdptr(),  */
						/*  nextdptr(), prevdptr()  */

#define NSIZE   256
#define FNSIZE   65

static int color, bgrd, ccolor, cbgrd, mcolor, mbgrd;

Flist loaddf(char *fname)
{
	Flist df;
	int fd, n;
	char *txt;

	if (!fname)
		return (Flist) 0;
	if (strlen(fname) >= FNSIZE)
		return (Flist) 0;
	if ((fd = open(fname,O_RDONLY|O_BINARY)) < 0)
		return (Flist) 0;
	if ((df = mkFlist(NSIZE,FNSIZE)) == (Flist) 0)  {
		close(fd);
		return (Flist) 0;
	}
	strcpy(Flistdptr(df),fname);
	while ((txt = iquedptr(df,(void *) 0)) != (char *) 0)
		if ((n = read(fd,txt,NSIZE)) < NSIZE)  {
			for (; n < NSIZE; n++)
				txt[n] = '\0';
			break;
		}
	close(fd);
	if (!nempty(df))
		rmFlist(&df);
	return df;
}

#define  Home	71
#define  UpArr  72
#define  PgUp   73
#define  LArr   75
#define  RArr   77
#define  EndKey 79
#define  DnArr  80
#define  PgDn   81
#define  ESC    27
#define	 F1     59
#define  F3     61

int getPC(void)
{
  union REGS rgs;

  rgs.x.ax  =  0x0000;
  int86(0x16,&rgs,&rgs);
  if (rgs.h.al) {
     return (int ) rgs.h.al;
  }
  return - (int) rgs.h.ah;
}

char *newFile(void)
{
	static char fname[FNSIZE];
	int c;

	gotoxy(1,25);
	textcolor(mcolor);
	textbackground(mbgrd);
	clreol();
	fname[0] = FNSIZE;
	cprintf("Enter file: ");
	return cgets(fname);
}

int dump(char *fname)
{
	Flist df, newdf;
	char c, *txt;
	int i, r, n;

	if ((df = loaddf(fname))
		== (Flist) 0)
		return 1;
	for (c = -Home; c != ESC; c = getPC())  {
		switch (c)  {
		case -PgDn:
			if (ncur(df) < nempty(df))
				txt = nextdptr(df,(void *) 0);
			else
				continue;
			break;
		case -PgUp:
			if (ncur(df) > 1)
				txt = prevdptr(df,(void *) 0);
			else
				continue;
			break;
		case -Home:
			txt = mkcdptr(df,1);
			break;
		case -EndKey:
			txt = mkcdptr(df,nempty(df));
			break;
		case -F1:
			textcolor(mcolor);
			textbackground(mbgrd);
			gotoxy(1,25);
			clreol();
			cprintf("Keys: PgUp, PgDn, Home = top,"
			" End = bottom, F3 = new file, ESC = Quit");
			continue;
		case -F3:
			if ((newdf = loaddf(newFile()))
				!= (Flist) 0)  {
				rmFlist(&df);
				df = newdf;
				txt = mkcdptr(df,1);
			}
			break;
		default:
			continue;
		}
		textcolor(color);
		textbackground(bgrd);
		window(1,1,80,24);
		clrscr();
		cprintf(" Addr      Dword        Dword   "
			"     Dword        Dword           ASCII"
			"\n\r\n\r\n\r");
		for (n = ncur(df) - 1, r = 0;
			r < 16; r++, txt += 16)  {
			cprintf("\n\r%06x ",n * NSIZE + r * 16);
			for (i = 0; i < 16; i++)  {
				if (i % 4 == 0)
					putch(' ');
				cprintf("%02X ",(unsigned char) txt[i]);
			}
			cprintf("  ");
			for (i = 0; i < 16; i++)
				switch (txt[i])  {
					case 7: case 8: case 9:
					case 10: case 13:
						putch(' ');
						break;
					default:
						if (txt[i] < 26)  {  /* ctrl char */
							textcolor(ccolor);
							textbackground(cbgrd);
							putch(txt[i] + 'A' - 1);
							textcolor(color);
							textbackground(bgrd);
						}
						else
						putch(txt[i]);
				}
		}
		textcolor(mcolor);
		textbackground(mbgrd);
		window(1,1,80,25);
		gotoxy(1,25);
		clreol();
		cprintf("Dump file: %s",(char *) Flistdptr(df));
	}
	rmFlist(&df);
	return 0;
}

int vmode(void)
{
    union REGS rgs;

    rgs.x.ax = 0x0F00;
    int86(0x10,&rgs,&rgs);
    return (int) rgs.h.al;
}

void scrInit(void)
{
    textmode(C80);
    if (vmode() == 7)  {
		color  = BLACK;
		bgrd   = LIGHTGRAY;
		ccolor = LIGHTGRAY;
		cbgrd  = BLACK;
        mcolor = BLACK;
        mbgrd  = LIGHTGRAY;
    }
    else  {
		color  = BLACK;
		bgrd   = LIGHTGRAY;
		ccolor = BLUE;
		cbgrd  = LIGHTGRAY;
        mcolor = LIGHTGRAY;
		mbgrd  = MAGENTA;
	}
	textcolor(color);
	textbackground(bgrd);
	clrscr();
}

main(int argc, char *argv[])
{
    int i;

	if (argc > 2)  {
		puts("\nusage:  dump  filename");
		exit(1);
	}
    scrInit();
	if (argc == 1)
		i = dump(newFile());
	else
		i = dump(argv[1]);
    textcolor(LIGHTGRAY);
    textbackground(BLACK);
    clrscr();
    exit(i);
}
