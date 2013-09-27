/*

    hypertxt.c
	7/25/89
    display hypertext file

    Copyright 1989
    John W. Small
    All rights reserved

    PSW / Power SoftWare
    P.O. Box 10072
    McLean, Virginia 22102 8072


    The required format of a hypertext file to be readable
    by this program is as follows:

    1.  A topic is defined on a line by itself begining
        in column 1 with a vertical tab (ctrl K), i.e.

        \vtopic

        The topic continues on until the next topic or EOF.
        Topic is any descriptive text that must be matched
        by a hyperlink inorder to be selected.

        A topic can have multiple pages by having a single
        line with only a vertical tab in column 1 to
        indicate a page break, i.e.

        \v

        Topics can also be ended with a '\x18', <ctrl X>,
        in column one, i.e.

        \x18

        Each hypertext file requires at least one topic.
        The first topic of a file is the default topic.

    2.  A topic can have hyperlinks to other topics by
        embedding the following:

        \x05 text \vtopic\x18

        where text can be any text and is visible and its
        hyperlink is \vtopic which is invisible.  Notice
        that '\x05', <ctrl E>, introduces a hyperlink.  This
        was chosen with the implied meaning: begin possible
        enquiry.  Note also that '\x18', <ctrl X>, ends the
        hyperlink with the implied meaning: cancel display
        of topic.  Hyperlinks must appear unbroken on one
        line.

        Hyperlinks can also link to topics in other hyper-
        text files:

        \x05 text \vtopic|aux.htx\x18

        In this hyperlink topic is in aux.htx, a hypertext
        file.

        Hyperlinks can also envoke other programs or system
        commands:

        \x05 text \v|myprog.exe\x18

        Notice that the topic is missing, thus it is implied
        the file following the '|' separator is something
        other than a hypertext file.

        \x05 text \v|command.com /cdir\x18

        The above hyperlink is an example of executing a
        system command.


    3.  It's up to the hypertext file to provide formating
        for pages and lines.


    The prominent data structure of this program is a list
    of topics each one pointing to a list of hyperlinks.
    Associated with each hyperlinks list is list of
    hyperpages and a specific hyperpage.  For each hypertext
    file in memory there exists a hyperpages list.  Perhaps
    the following diagram will help the discussion.



    topics      ->  hls ->  hls ->  hls ->  hls -> ...

                     |       |       |       |
                     |       v       v       v
                     |
                     |
                     |
                     |
                     |
                     v

    (hyperlinks)    hls ->  hl  ->  hl  ->  hl  -> ...

                     |      (hlink)
                     |
                     |
    (hlinks_info)    |
                     |
                     |--------------|
                     |              |
                     |              |
                     v              v

    (hyperpages)    hps ->  hp  ->  hp  ->  hp  -> ...

                                    (hpage)


    The hyperpages' list is actually a list of topics
    appearing in a hypertext file and the positions of the
    topic's text in the hypertext file.   The hypertext
    file itself is never stored in RAM.  When the topic is
    displayed on the screen a list of hyperlinks and their
    screen positions is built.  This allows cursor
    positioning on the hyperlinks again without the need of
    storing the hypertext in RAM.  If a hyperlink is
    selected then its hyperlinks' list is pushed onto the
    topics list.  If the newly selected topic is in a
    existing hyperpages list it is diplayed on the screen
    and again a hyperlinks' list is built for this page
    otherwise a new hyperpages list is built first by
    loading the proper hypertext file.  Note that several
    hyperlinks lists can reference one hyperpages list.


*/



#include <stdio.h>      /*  fopen(), fgets(), fclose(),  */
						/*  fseek()  */
#include <string.h>     /*  strlen(), strncpy(),  */
                        /*  strcpy(), strcmp(),  */
                        /*  strdup(), strcat(), strtok()  */
#include <stdlib.h>     /*  exit(), calloc(),  */
						/*  free(), abs()  */
#include <io.h>         /*  open(), close(), read()  */
#include <fcntl.h>      /*  O_RDONLY, O_BINARY  */
#include <process.h>    /*  spawnv()  */
#include <conio.h>      /*  wherex(), wherey(),  */
                        /*  gotoxy(), clrscr(),  */
                        /*  clreol(), textcolor(),  */
                        /*  textbackground(), textmode(), */
						/*  putch(), cprintf()  */
#include <dos.h>        /*  int86(), union REGS;  */
#include <flexlist.h>   /*  mkFlist(), rmFlist(),  */
                        /*  nempty(), Flistdptr()  */
                        /*  pushdptr(), popd(),  */
                        /*  iquedptr(),  */
                        /*  mkcdptr(), ncur(), getd(),  */
                        /*  nextdptr(), prevdptr()  */
                        /*  rcld()  */

#define HTOPIC  '\v'    /*  hyperpage topic begin ^K  */
#define HLINK   '\x05'  /*  hyperlink text  begin ^E */
#define HACTION '|'     /*  hypertext subfile or spawn  */
#define HEND    '\x18'  /*  hyperlink end ^X, or */
                        /*  hyperpage topic end ^X  */
#define FNSIZE  65
#define TSIZE	80

typedef struct  {       /*  hyperpage node          */
    char *topic;        /*  topic heading           */
    long fpos;          /*  file position of page   */
} hpage;

typedef struct {        /*  hyperpages global info  */
    char fname[FNSIZE]; /*  hypertext filename      */
    int refs;           /*  # of hyperlinks references  */
} hpages_info;

typedef struct  {       /*  hyperlink node  */
    char *txt, *topic;  /*  hyperlink text, topic */
    int scrX, scrY;     /*  hyperlink screen position  */
} hlink;

typedef struct  {       /*  hyperlinks global info  */
    Flist hps;          /*  respective hyperpages list */
    unsigned hpNo;      /*  and hyperpage number  */
} hlinks_info;



#define BUFSIZE  2048
#define TOPICSIZE 65
static char buf[BUFSIZE];
static char topic[TOPICSIZE];
static int color = BLACK;
static int bgrd  = LIGHTGRAY;
static int hcolor = BLUE;
static int hbgrd  = LIGHTGRAY;
static int scolor = WHITE;
static int sbgrd  = BLACK;
static int mcolor = LIGHTGRAY;
static int mbgrd  = BLUE;
static int TabStop = 4;
static int ColumnGrab = 4;
static char helpFile[FNSIZE];

Flist loadhps(char *fname)      /*  return hyperpages  */
{
    Flist hps;          /*  hyperpages Flist  */
	hpage hp;           /*  hyperpage */
	hpages_info *hpsi;  /*  hyperpages info */
	long bpos, eob;		/*  base position, end of buf  */
	int fd, i, j, col;

    if (!fname)
        return (Flist) 0;
    if (strlen(fname) >= FNSIZE)
		return (Flist) 0;
	if ((fd = open(fname,O_RDONLY|O_BINARY)) < 0)
		return (Flist) 0;
    if ((hps = mkFlist(sizeof(hpage),sizeof(hpages_info)))
        == (Flist) 0)
        return (Flist) 0;
    hpsi = Flistdptr(hps);
    strcpy(hpsi->fname,fname);
	hpsi->refs = 0;
	/*  scan file for topics  */
	for (
		bpos = 0, col = 1;
		(eob = read(fd,buf,BUFSIZE)) != 0;
		bpos += eob
	)
		for (i = 0 ; i < eob; i++, col++)
			if (buf[i] == '\n')
				col = 0;
			else if (buf[i] == '\v' && col == 1)  {
				/*  found topic page  */
				for (
					j = 0;
					j < TOPICSIZE && i < eob &&
					buf[i] != '\n';
				)  {
					if (buf[i] != '\r')
						topic[j++] = buf[i++];
					else
						i++;
					if (i >= eob)  {
						bpos += eob;
						eob = read(fd,buf,BUFSIZE);
						i = 0;
					}
				}
				if (j < TOPICSIZE && i < eob &&
					buf[i] == '\n'
				)  {
					col = 0;
					topic[j] = '\0';
					if ((hp.topic = strdup(topic))
						!= (char *) 0
					)  {
						hp.fpos = bpos + i + 1;
						if (iquedptr(hps,&hp))
							continue;
					}
				}
				/*  error in topic */
				while (popd(hps,&hp))
					free(hp.topic);
				rmFlist(&hps);
				close(fd);
				return (Flist) 0;
			}
	close(fd);
    /*  if no hyperpages then discard  */
    if (!nempty(hps))
        rmFlist(&hps);
    mkcdptr(hps,1);
    return hps;
}

Flist mkchp(Flist hps, char *topic)
/*  return hps with hp current  */
{
    hpage hp;

    if (!topic)
        return (Flist) 0;
    for (mkcdptr(hps,0); nextdptr(hps,&hp); )
        if (!strcmp(hp.topic,topic))
            break;
    return hps;
}

int rmhps(Flist *hpsAddr)
/*  remove hps if no other references */
{
    hpage hp;

    if (!hpsAddr)
        return 0;
    if (!*hpsAddr)
        return 0;
    if (--((hpages_info *) Flistdptr(*hpsAddr))->refs > 0)
        return 0;
    for (mkcdptr(*hpsAddr,0); nextdptr(*hpsAddr,&hp); )
        if (hp.topic)
            free(hp.topic);
    return rmFlist(hpsAddr);
}

Flist findhps(Flist topics, char *fname)
/*  find hps in RAM else loadhps()  */
{
    hpages_info * hpsi;
    hlinks_info * hlsi;
    Flist hls;

    if (!topics || !fname)
        return (Flist) 0;
    for (mkcdptr(topics,0); nextdptr(topics,&hls); )  {
        hlsi = Flistdptr(hls);
        if ((hpsi = Flistdptr(hlsi->hps))
            != (hpages_info *) 0)
            if (!strcmp(hpsi->fname,fname))
                return hlsi->hps;
    }
    return loadhps(fname);
}

char **strvec(const char *s)
/*  convert a string into vector of (char *)  */
{
    char *d, *t, **vec;
    Flist v;
    int n;

    if (!s)  return (char **) 0;
    if (!*s) return (char **) 0;
    if ((d = strdup(s)) == (char *) 0) return (char **) 0;
    if ((v = mkFlist(sizeof(char *),0)) == (Flist) 0)  {
        free(d);
        return (char **) 0;
    }
    for (t = strtok(d," "); t; t = strtok((char *)0," "))
        if ((t = strdup(t)) != (char *) 0)  {
            if (!iquedptr(v,&t))  {
                free(t);
                break;
            }
        }
        else
            break;
    if (nempty(v))
        if ((vec = calloc(nempty(v)+1,sizeof(char *)))
            != (char **) 0)  {
            for (n = 0; nextdptr(v,&t); n++)
                vec[n] = t;
            vec[n] = (char *) 0;
        }
        else while (popd(v,&t))
            free(t);
    rmFlist(&v);
    free(d);
    return vec;
}

void freestrvec(char **vec)
{
    int i;

    if (vec)  {
        for (i = 0; vec[i]; i++)
            free(vec[i]);
        free(vec);
    }
}

int puthp(Flist hps, Flist *hlsAddr)
/* put current hyperpage, build hyperlinks Flist */
{
    FILE *hpsfile;
    hpage hp;
    hpages_info *hpsi;
    Flist hls;
    hlink hl;
    hlinks_info *hlsi;
    int f, r, n;    /*  front & rear string pointers  */

    if (hlsAddr)  {
        *hlsAddr = (Flist) 0;
        if ((hls =
            mkFlist(sizeof(hlink),sizeof(hlinks_info)))
            == (Flist) 0)
            return -1;
    }
    else
        hls = (Flist) 0;
    if (!getd(hps,&hp))  {
        rmFlist(&hls);
        return -2;
    }
    hpsi = Flistdptr(hps);
    if ((hpsfile = fopen(hpsi->fname,"r")) == (FILE *) 0)  {
        rmFlist(&hls);
        return -3;
	}
    if (hls)  {
        /* Associate hyperlinks with hyperpage */
        hlsi = Flistdptr(hls);
        hlsi->hps = hps;
        hlsi->hpNo = ncur(hps);
        hpsi->refs++;
    }
    fseek(hpsfile,hp.fpos,0);
    textcolor(color);
    textbackground(bgrd);
    clrscr();
    while (fgets(buf,BUFSIZE,hpsfile))  {
        if ((r = strlen(buf)) != 0)
            if (buf[r-1] == '\n')
                buf[r-1] = '\0';
        if (*buf == HTOPIC || *buf == HEND)
            break;
        else for (r = 0; buf[r];)  {
            for (f = r; buf[r] && (buf[r] != HLINK); r++);
            if (buf[r])  {      /*  found hyperlink  */

                /*  print preceding string  */
                for (;f<r;f++)
					if (buf[f] == '\t')  {
						for (n = wherex(); n % TabStop; n++)
							putch(' ');
						putch(' ');
					}
                    else
                        putch(buf[f]);

                /*  extract hyperlink text  */
                for (f = ++r;
                    buf[r] && (buf[r] != HTOPIC);
                    r++);
                if (buf[r])  {

                    /*  print hyperlink text  */
                    hl.txt = (char *) 0;
                    if (hls)  { /* build hyperlink */
                        buf[r] = '\0';
                        hl.txt = strdup(&buf[f]);
                        hl.scrX = wherex();
                        hl.scrY = wherey();
                        buf[r] = HTOPIC;
                    }
                    textcolor(hcolor);
                    textbackground(hbgrd);
                    for (;f<r;f++)
						if (buf[f] == '\t')  {
							for (n = wherex(); n % TabStop; n++)
								putch(' ');
							putch(' ');
						}
                        else
                            putch(buf[f]);
                    textcolor(color);
                    textbackground(bgrd);


                    /* skip hyperlink topic */
                    for (f = r++;
                        buf[r] && buf[r] != HEND;
                        r++);
                    if (buf[r])  {
                        buf[r++] = '\0';
                        if (hls)  { /* build hyperlink */
                            hl.topic = strdup(&buf[f]);
                            if (!iquedptr(hls,&hl))  {
                                if (hl.txt)
                                    free(hl.txt);
                                if (hl.topic)
                                    free(hl.topic);
                            }
                            /*  no report for  */
                            /*  missing hyperlinks  */
                        }
                        /* continue processing */
                    }
                    else  {
                        /*  hyperlink error:  */
                        /*  topic ends at EOL  */
                        if (hl.txt)
                            free(hl.txt);
                        break;
                    }
                }
                else  {
                    /* hyperlink error: text ends at EOL  */
                    for (;buf[f];f++)
						if (buf[f] == '\t')  {
							for (n = wherex(); n % TabStop; n++)
								putch(' ');
							putch(' ');
						}
                        else
                            putch(buf[f]);
                    break;
                }
            }
            else  {
                /*  no more hyperlinks in string  */
                for (;buf[f];f++)
					if (buf[f] == '\t')  {
						for (n = wherex(); n % TabStop; n++)
							putch(' ');
						putch(' ');
					}
                    else
                        putch(buf[f]);
                break;
            }
        }
        putch('\n');
        putch('\r');
    }
    fclose(hpsfile);
    mkcdptr(hls,1);
    if (hlsAddr)
        *hlsAddr = hls;
    return 0;
}


#define selhl(hls) puthl(hls,scolor,sbgrd)
#define clrhl(hls) puthl(hls,hcolor,hbgrd)

void puthl(Flist hls, int color, int bgrd)
/* rewrite current hyperlink */
{
    hlink hl;
    int f, n;

    if (!getd(hls,&hl))
        return;
    if (!hl.txt)
        return;
    gotoxy(hl.scrX,hl.scrY);
    textcolor(color);
    textbackground(bgrd);
    for (f = 0; hl.txt[f]; f++)
        if (hl.txt[f] == '\t')
            for (putch(' '),
                n = wherex() % TabStop + 1;
                n;
                putch(' '), n--);
        else
            putch(hl.txt[f]);
}

int rmhls(Flist *hlsAddr)
/*  remove hls  */
{
    hlinks_info *hlsi;
    hlink hl;

    if (!hlsAddr)
        return 0;
    if (!*hlsAddr)
        return 0;
    hlsi = Flistdptr(*hlsAddr);
    if (hlsi->hps)
        rmhps(&hlsi->hps);
    for (mkcdptr(*hlsAddr,0); nextdptr(*hlsAddr,&hl);)  {
        if (hl.txt)
            free(hl.txt);
        if (hl.topic)
            free(hl.topic);
    }
    return rmFlist(hlsAddr);
}

int PgUpOk(Flist hps)
/*  Does the current topic have a previous page?  */
{
    hpage hp;

    if (getd(hps,&hp))
        if (!strcmp(hp.topic,"\v"))
            if (ncur(hps) > 1)
                return 1;
    return 0;
}

int PgDnOk(Flist hps)
/*  Does the current topic have an additional page?  */
{
    hpage hp;
    int n, ok;

    ok = 0;
    n = ncur(hps);
    if (nextdptr(hps,&hp))
        if (!strcmp(hp.topic,"\v"))
            ok = 1;
    mkcdptr(hps,n);
    return ok;
}

char *curTopic(Flist hps)
/*  Return string of current topic  */
{
    hpage hp;
    int n;
    char *t;

    t = (char *) 0;
    n = ncur(hps);
    while (getd(hps,&hp))  {
        if (strcmp(hp.topic,"\v"))  {
            t = hp.topic;
            break;
        }
        /*  skip page breaks  */
        prevdptr(hps,(void *)0);
    }
    mkcdptr(hps,n);
    return t;
}

void PgMess(Flist hls)
/*  put page status line  */
{
    hpage hp;
    hpages_info *hpsi;
    hlink hl;
    hlinks_info *hlsi;
    char *t;

	if ((hlsi = Flistdptr(hls)) != (hlinks_info *) 0)  {
		mkcdptr(hlsi->hps,hlsi->hpNo);
        gotoxy(1,25);
        textcolor(mcolor);
        textbackground(mbgrd);
        clreol();
        gotoxy(70,25);
        if (PgUpOk(hlsi->hps))  {
            cprintf("PgUp");
            if (PgDnOk(hlsi->hps))
                cprintf("/PgDn");
        }
        else if (PgDnOk(hlsi->hps))
            cprintf("PgDn");
        gotoxy(1,25);
        if ((t = curTopic(hlsi->hps)) != (char *) 0)
            cprintf("%s",t);
        if ((hpsi = Flistdptr(hlsi->hps))
            != (hpages_info *) 0)
            cprintf("|%s",hpsi->fname);
        if (getd(hls,&hl))  {
            gotoxy(45,25);
            textcolor(scolor);
            textbackground(sbgrd);
            cprintf("%s",hl.topic);
        }
        gotoxy(80,25);
    }
}

#define  Home   71
#define  UpArr  72
#define  PgUp   73
#define  LArr   75
#define  RArr   77
#define  EndKey 79
#define  DnArr  80
#define  PgDn   81
#define  ESC    27
#define  CR     13
#define  BackSp  8
#define  F1     59
#define  F3     61
#define  AltF3  106
#define  CtrlHome    119
#define  CtrlPgUp    132
#define  CtrlEnd     117
#define  CtrlPgDn    118

int getPC(void)		/*  PC special keys return  */
{					/*  negative scan codes  */
  union REGS rgs;

  rgs.x.ax  =  0;
  int86(0x16,&rgs,&rgs);
  if (rgs.h.al) {
     return (int ) rgs.h.al;
  }
  return - (int) rgs.h.ah;
}

char *newFile(void)
{
	static char fname[FNSIZE+3];

	gotoxy(1,25);
	textcolor(mcolor);
	textbackground(mbgrd);
	clreol();
	fname[0] = FNSIZE;
	cprintf("Enter file: ");
	return cgets(fname);
}

char *newTopic(void)
{
	static char topic[TSIZE+3];

	gotoxy(1,25);
	textcolor(mcolor);
	textbackground(mbgrd);
	clreol();
	topic[0] = TSIZE;
	cprintf("Enter topic: ");
	cgets(topic);
	if (topic[1])  {
		topic[1] = '\v';
		return &topic[1];
	}
	return (char *) 0;
}

int puthps(char *fname)
/*  display hypertext file  */
{
    Flist topics, hps, hls, newhls;
    hpage hp;
    hpages_info *hpsi;
    hlink hl, newhl;
    hlinks_info *hlsi;
    int c, i;
    char **vec;

    if ((topics = mkFlist(sizeof(Flist),0)) == (Flist) 0)
        return 1;
    if ((hps = loadhps(fname)) == (Flist) 0)  {
        rmFlist(&topics);
        return 1;
    }
    if (puthp(hps,&hls))  {
        rmFlist(&topics);
        rmhps(&hps);
        return 1;
    }
    selhl(hls);
    PgMess(hls);
    while (((c = getPC()) != ESC) ||  nempty(topics))  {
        clrhl(hls);
        switch (c)  {
        case -Home:
            mkcdptr(hls,1);
            break;
        case -CtrlHome:
            hlsi = Flistdptr(hls);
            i = ncur(hlsi->hps);
            rcld(hlsi->hps,&hp,1);
            while ((ncur(hlsi->hps) < i)
                && !strcmp(hp.topic,"\v"))
                nextdptr(hlsi->hps,&hp);
            if (ncur(hlsi->hps) < i)
                if (!puthp(hlsi->hps,&newhls))  {
                    rmhls(&hls);
                    hls = newhls;
                    break;
                }
            mkcdptr(hlsi->hps,i);
            break;
        case -EndKey:
            mkcdptr(hls,nempty(hls));
            break;
        case -CtrlEnd:
            hlsi = Flistdptr(hls);
            i = ncur(hlsi->hps);
            rcld(hlsi->hps,&hp,nempty(hlsi->hps));
            while ((ncur(hlsi->hps) > i)
                && !strcmp(hp.topic,"\v"))
                prevdptr(hlsi->hps,&hp);
            if (ncur(hlsi->hps) > i)
                if (!puthp(hlsi->hps,&newhls))  {
                    rmhls(&hls);
                    hls = newhls;
                    break;
                }
            mkcdptr(hlsi->hps,i);
            break;
        case -PgUp:
            hlsi = Flistdptr(hls);
            if (PgUpOk(hlsi->hps))  {
                prevdptr(hlsi->hps,(void *)0);
                if (!puthp(hlsi->hps,&newhls))  {
                    rmhls(&hls);
                    hls = newhls;
                }
                else
                    nextdptr(hlsi->hps,(void *)0);
            }
            break;
        case -CtrlPgUp:
            hlsi = Flistdptr(hls);
            i = ncur(hlsi->hps);
            getd(hlsi->hps,&hp);
            /* go to begining of current topic  */
            while (ncur(hlsi->hps)
                && !strcmp(hp.topic,"\v"))
                prevdptr(hlsi->hps,&hp);
            if (ncur(hlsi->hps))
                prevdptr(hlsi->hps,&hp);
            while (ncur(hlsi->hps)
                && !strcmp(hp.topic,"\v"))
                prevdptr(hlsi->hps,&hp);
            if (ncur(hlsi->hps))
                if (!puthp(hlsi->hps,&newhls))  {
                    rmhls(&hls);
                    hls = newhls;
                    break;
                }
            mkcdptr(hlsi->hps,i);
            break;
        case -PgDn:
            hlsi = Flistdptr(hls);
            if (PgDnOk(hlsi->hps))  {
                nextdptr(hlsi->hps,(void *)0);
                if (!puthp(hlsi->hps,&newhls))  {
                    rmhls(&hls);
                    hls = newhls;
                    }
                else
                    prevdptr(hlsi->hps,(void *)0);
            }
            break;
        case -CtrlPgDn:
            hlsi = Flistdptr(hls);
            i = ncur(hlsi->hps);
            getd(hlsi->hps,&hp);
            if (ncur(hlsi->hps)
                && strcmp(hp.topic,"\v"))
                nextdptr(hlsi->hps,&hp);
            while (ncur(hlsi->hps)
                && !strcmp(hp.topic,"\v"))
                nextdptr(hlsi->hps,&hp);
            if (ncur(hlsi->hps))
                if (!puthp(hlsi->hps,&newhls))  {
                    rmhls(&hls);
                    hls = newhls;
                    break;
                }
            mkcdptr(hlsi->hps,i);
            break;
        case -UpArr:
            if ((i = ncur(hls)) != 0)  {
                getd(hls,&hl);
                while (prevdptr(hls,&newhl))
                    if (abs(newhl.scrX - hl.scrX)
                        < ColumnGrab)
                        break;
                if (ncur(hls))
                    break;
                mkcdptr(hls,i);
                /*  no item grabbed so fall thru  */
            }
        case -LArr:
            if (nempty(hls))
                while (!prevdptr(hls,(void *)0));
            break;
        case -DnArr:
            if ((i = ncur(hls)) != 0)  {
                getd(hls,&hl);
                while (nextdptr(hls,&newhl))
                    if (abs(newhl.scrX - hl.scrX)
                        < ColumnGrab)
                        break;
                if (ncur(hls))
                    break;
                mkcdptr(hls,i);
                /*  no item grabbed so fall thru  */
            }
        case -RArr:
            if (nempty(hls))
                while (!nextdptr(hls,(void *)0));
            break;
		case ESC:   /* drop back to first topic */
            while (nempty(topics))  {
                rmhls(&hls);
                popd(topics,&hls);
            }
            hlsi = Flistdptr(hls);
            mkcdptr(hlsi->hps,hlsi->hpNo);
            if (puthp(hlsi->hps,(Flist *)0))
                return 1;
            break;
        case -F1:
            hlsi = Flistdptr(hls);
            hpsi = Flistdptr(hlsi->hps);
            if (strcmp(helpFile,hpsi->fname))
				if ((hps = loadhps(helpFile)) != (Flist) 0)
					if (pushdptr(topics,&hls))
						if (puthp(hps,&hls))  {
							rmhps(&hps);
							popd(topics,&hls);
						}
						else
							break;
					else
						rmhps(&hps);
			break;
		case -F3:
			if ((hps = loadhps(newFile())) != (Flist) 0)
				if (puthp(hps,&newhls))
					rmhps(&hps);
				else  {
					do  { rmhls(&hls); }
					while (popd(topics,&hls));
					hls = newhls;
				}
			break;
		case -AltF3:
			if (pushdptr(topics,&hls))  {
				hlsi = Flistdptr(hls);
				if (puthp(mkchp(hlsi->hps,newTopic()),&hls))
					popd(topics,&hls);
			}
			break;
        case CR:
            if (pushdptr(topics,&hls))  {
                hlsi = Flistdptr(hls);
                if (getd(hls,&hl))  {
                    for (i = 0;
                        hl.topic[i] && hl.topic[i]
                        != HACTION;
                        i++);
                    if (hl.topic[i])
                        if (i == 1)  {  /*  spawn  */
                            if ((vec =
                                strvec(&hl.topic[i+1]))
                                != (char **) 0)  {
                                textcolor(color);
                                textbackground(bgrd);
                                clrscr();
                                spawnv(P_WAIT,vec[0],vec);
								freestrvec(vec);
								textmode(C80);
                                puthp(hlsi->hps,
                                    (Flist *) 0);
                            }
                            popd(topics,&hls);
                        }
                        else    /*  hypertext subfile  */
                        if ((hps =
                            findhps(topics,&hl.topic[i+1]))
                            != (Flist) 0)  {
                            hl.topic[i] = '\0';
                            if (puthp(mkchp(hps,hl.topic),
                                &hls))  {
                                hpsi = Flistdptr(hps);
                                if (hpsi->refs == 0)
                                    rmhps(&hps);
                            }
                            hl.topic[i] = HACTION;
                        }
                        else
                        /* unable to load hypertext file  */
							popd(topics,&hls);
                    else    /*  local hypertext topic  */
                    if (puthp(mkchp(hlsi->hps,hl.topic),
                        &hls))
                        popd(topics,&hls);
                }
                else    /*  no hyperlink chosen  */
                    popd(topics,&hls);
            }
            break;
        case BackSp:
            while (nempty(topics))  {
                rmhls(&hls);
                popd(topics,&hls);
                hlsi = Flistdptr(hls);
                mkcdptr(hlsi->hps,hlsi->hpNo);
                if (!puthp(hlsi->hps,(Flist *)0))
                    break;
                else if (!nempty(topics))
                    return 1;
            }
            break;
        }
        selhl(hls);
        PgMess(hls);
    }
    for (; hls; popd(topics,&hls))
        rmhls(&hls);
    rmFlist(&topics);
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
        color = BLACK;
        bgrd  = LIGHTGRAY;
        hcolor = LIGHTGRAY;
        hbgrd  = BLACK;
        scolor = WHITE;
        sbgrd  = BLACK;
        mcolor = BLACK;
        mbgrd  = LIGHTGRAY;
    }
    else  {
        color = BLACK;
        bgrd  = LIGHTGRAY;
        hcolor = BLUE;
        hbgrd  = LIGHTGRAY;
        scolor = WHITE;
        sbgrd  = BLACK;
        mcolor = LIGHTGRAY;
        mbgrd  = BLUE;
	}
	textcolor(color);
	textbackground(bgrd);
	clrscr();
}

main(int argc, char *argv[])
{
    int i;

	if (argc > 2)  {
		puts("\nusage:  hypertxt  filename");
		exit(1);
	}
    /*  helpfile is the same directory and  */
    /*  name as program only with .HTX extension  */
    strncpy(helpFile,argv[0],strlen(argv[0])-4);
    strcat(helpFile,".HTX");
    scrInit();
	if (argc == 1)
		i = puthps(newFile());
	else
		i = puthps(argv[1]);
    textcolor(LIGHTGRAY);
    textbackground(BLACK);
    clrscr();
    exit(i);
}
