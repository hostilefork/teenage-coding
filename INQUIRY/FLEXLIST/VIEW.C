/*

	view.c
	7-25-89

	Copyright 1989
	John W. Small
	All rights reserved

	PSW / Power SoftWare
	P.O. Box 10072
	McLean, Virginia 22102 8072

*/

#include <stdlib.h>		/*  exit(), atol()  */
#include <stdio.h> 		/*  fopen(), fclose(),  */
						/*  fgets()  */
#include <string.h>		/*  strlen(), strcpy()  */
#include <io.h>         /*  open(), close(), read()  */
#include <fcntl.h>      /*  O_RDONLY, O_BINARY  */
#include <conio.h>		/*  clrscr(), clreol(),  */
						/*  gotoxy(), cprintf(),  */
						/*  putch(), cgets(),  */
						/*  textcolor(),  */
						/*  textbackgound(),  */
						/*  window()  */
#include <dos.h>		/*  int86(), union REGS;  */
#include <flexlist.h>	/*  mkFlist(), rmFlist(),  */
						/*  nempty(), iquedptr(),   */
						/*  nextdptr(), prevdptr(),  */
						/*  rcld()  */


#define tabstops 4
#define tabdisp ' '
#define HSCROLL  8
#define HPAN	 50
#define VSCROLL  7
#define MAXLINES 23
#define MAXCOLS 79
#define FNSIZE  65
#define BUFSIZE 2048
static char buf[BUFSIZE];
static int color, bgrd, ccolor, cbgrd, mcolor, mbgrd;
static long lineno;

Flist loadpgs(char *fname)      /*  return list of pages  */
{
	Flist pgs;          /*  pages Flist  */
	long fpos, bpos, y; /*  file and base positions */
	int fd, eob;		/*  end of buf  */
	register int i;

    if (!fname)
        return (Flist) 0;
    if (strlen(fname) >= FNSIZE)
		return (Flist) 0;
	if ((fd = open(fname,O_RDONLY|O_BINARY)) < 0)
		return (Flist) 0;
	if ((pgs = mkFlist(sizeof(long),FNSIZE))
        == (Flist) 0)
        return (Flist) 0;
	strcpy(Flistdptr(pgs),fname);
	fpos = bpos = 0;
	if (!iquedptr(pgs,&fpos))  {
		rmFlist(&pgs);
		close(fd);
		return (Flist) 0;
	}
	for (y = 1; (eob = read(fd,buf,BUFSIZE)) != 0;
		bpos += eob)
		for (i = 0 ; i < eob; i++)
			if (buf[i] == '\n')  {
				if (!(y % VSCROLL))  {
					fpos = bpos + i + 1;
					if (!iquedptr(pgs,&fpos))  {
						rmFlist(&pgs);
						close(fd);
						return (Flist) 0;
					}
				}
				y++;
			}
	close(fd);
	return pgs;
}

#define  Home	71
#define  UpArr  72
#define  PgUp   73
#define  LArr   75
#define  RArr   77
#define  EndKey 79
#define  DnArr  80
#define  PgDn   81
#define  CtrlLArr    115
#define  CtrlRArr    116
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

int putpgs(char *fname)
{
	FILE *pgsfile, *newpgsfile;
	Flist pgs, newpgs;
	long fpos;
	int c, x, y, i, startCol;

	if ((pgs = loadpgs(fname)) == (Flist) 0)
		return 1;

	if ((pgsfile = fopen(Flistdptr(pgs),"r"))
		== (FILE *) 0)
		return 2;

	for (c = lineno? 0 : -Home, startCol = 1; c != ESC;
		c = getPC())  {
		switch (c)  {
		case 0:
			if (!rcld(pgs,&fpos,
				((unsigned) ((lineno - 1) / VSCROLL)) + 1))
				rcld(pgs,&fpos,1);
			break;
		case -PgDn:
			if ((i = ncur(pgs) + 3) < nempty(pgs))
				rcld(pgs,&fpos,i);
			else if (ncur(pgs) < nempty(pgs))
				rcld(pgs,&fpos,nempty(pgs));
			else
				continue;
			break;
		case -PgUp:
			if ((i = ncur(pgs) - 3) > 0)
				rcld(pgs,&fpos,i);
			else if (i > -2)
				rcld(pgs,&fpos,1);
			else
				continue;
			break;
		case -Home:
			rcld(pgs,&fpos,1);
			break;
		case -EndKey:
			rcld(pgs,&fpos,nempty(pgs));
			break;
		case -UpArr:
			if (ncur(pgs) > 1)
				prevdptr(pgs,&fpos);
			else
				continue;
			break;
		case -DnArr:
			if (ncur(pgs) < nempty(pgs))
				nextdptr(pgs,&fpos);
			else
				continue;
			break;
		case -LArr:
			if (startCol == 1)
				continue;
			if ((startCol -= HSCROLL) < 1)
				startCol = 1;
			break;
		case -CtrlLArr:
			if (startCol == 1)
				continue;
			if ((startCol -= HPAN) < 1)
				startCol = 1;
			break;
		case -RArr:
			startCol += HSCROLL;
			break;
		case -CtrlRArr:
			startCol += HPAN;
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
			if ((newpgs = loadpgs(newFile()))
				!= (Flist) 0)  {
				if ((newpgsfile =
					fopen(Flistdptr(newpgs),"r"))
					!= (FILE *) 0)  {
					fclose(pgsfile);
					pgsfile = newpgsfile;
					rmFlist(&pgs);
					pgs = newpgs;
					rcld(pgs,&fpos,1);
				}
				else  {
					rmFlist(&newpgs);
					continue;
				}
			}
			else
				continue;
			break;
		default:
			continue;
		}
		fseek(pgsfile,fpos,0);
		textcolor(color);
		textbackground(bgrd);
		window(1,1,80,24);
		clrscr();
		for (y = 1; (y <= MAXLINES) &&
			fgets(buf,BUFSIZE,pgsfile); y++)  {
			gotoxy(1,y);
			for (x = 1, i = 0;
				buf[i] && buf[i] != '\n' &&
				x < startCol + MAXCOLS;
				i++)  {
				if (buf[i] == '\t')  { 	/* expand tab */
					for (;x % tabstops; x++)
						if (x >= startCol) putch(tabdisp);
					if (x >= startCol) putch(tabdisp);
					x++;
				}
				else if (x < startCol)
					x++;
				else if (buf[i] < 26)  {  /* ctrl char */
					textcolor(ccolor);
					textbackground(cbgrd);
					putch(buf[i] + 'A' - 1);
					textcolor(color);
					textbackground(bgrd);
					x++;
				}
				else {
					putch(buf[i]);
					x++;
				}
			}
		}
		textcolor(mcolor);
		textbackground(mbgrd);
		window(1,1,80,25);
		gotoxy(1,25);
		cprintf("View file: %s   Line %d   Col %d",
			(char *) Flistdptr(pgs),
			(ncur(pgs) - 1) * VSCROLL + 1,
			startCol);
		clreol();
	}
	fclose(pgsfile);
	rmFlist(&pgs);
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
		mbgrd  = RED;
	}
	textcolor(color);
	textbackground(bgrd);
	clrscr();
}

main(int argc, char *argv[])
{
    int i;

	if (argc > 3)  {
		puts("\nusage:  list  filename  [lineno]");
		exit(1);
	}
	if (argc == 3)
		lineno = atol(argv[2]);
	else
		lineno = 0L;
    scrInit();
	if (argc >= 2)
		i = putpgs(argv[1]);
	else
		i = putpgs(newFile());
    textcolor(LIGHTGRAY);
    textbackground(BLACK);
    clrscr();
    exit(i);
}
