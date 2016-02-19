/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#include <stdio.h>
#include "dmain.h"
#include "pmccabe.h"

/* Global */
int ncss_Line;

/* $Id: dmain.c,v 1.16 2001/01/26 23:00:32 bame Exp $ */

static int Lastc = '\n';

short Pipe[SIZE];
short *Piperead = Pipe;
short *Pipewrite = Pipe;
short *Pipeend = Pipe + sizeof Pipe;

FILE *Input;
char Inputfile[1030];

int Cppflag = 0;

int
decomment()
{
    register int c, c1;

    enum {	NORMAL,
		C_COMMENT,
		CPP,
		CC_COMMENT,
		STRINGLITERAL,
		CHARLITERAL} state = NORMAL;

    Piperead = Pipewrite = Pipe;

    do
    {
	if ((c = getc(Input)) == EOF)
	{
	    PUTCHAR(EOF);
	    break;
	}
	if (c == '\r')
	    continue;

	switch (state)
	{
	case NORMAL:
	    switch(c)
	    {
	    case '#':
		if (Lastc == '\n')
		{
		    state = CPP;
		    if (Cppflag)
		        PUTS("cpp");
		}
		else
		{
		    PUTCHAR(c);
		}
		break;
	    case '\'':
		state = CHARLITERAL;
		PUTS("CHARLITERAL");
		break;
	    case '"':
		state = STRINGLITERAL;
		PUTS("STRINGLITERAL");
		break;
	    case '/':
		c1 = getc(Input);
		switch(c1)
		{
		case '/':
		    state = CC_COMMENT;
		    break;
		case '*':
		    state = C_COMMENT;
		    break;
		case EOF:
		    break;
		default:
		    PUTCHAR(c);
		    ungetc(c1, Input);
		    break;
		}
		break;
	    default:
		PUTCHAR(c);
		break;
	    }
	    break;

	case C_COMMENT:		/* K&R C comment */
	    if (c == '\n')
	    {
		PUTCHAR(c);
	    }
	    else if (c == '*')
	    {
		c1 = getc(Input);
		if (c1 == '/')
		{
		    state = NORMAL;
		}
		else
		{
		    ungetc(c1, Input);
		}
	    }
	    break;

	case CPP:
	    switch(c)
	    {
	    case '\n':
		PUTCHAR(c);
		state = NORMAL;
		break;
	    case '\\':
		c1 = getc(Input);
		if (c1 == '\n')
		{
		    PUTCHAR(c1);
		    if (Cppflag)
		        PUTS("cpp");
		}
		break;
	    }
	    break;

	case CC_COMMENT:	/* C++ comment */
	    if (c == '\n')
	    {
		PUTCHAR(c);
		state = NORMAL;
	    }
	    break;

	case STRINGLITERAL:
	    switch(c)
	    {
	    case '\n':
		PUTCHAR(c);
		break;
	    case '"':
		state = NORMAL;
		break;
	    /* preserve embedded nulines */
	    case '\\':
		c1 = getc(Input);
		if (c1 == '\n')
		    PUTCHAR(c1);
		break;
	    }
	    break;

	case CHARLITERAL:
	    switch(c)
	    {
	    case '\n':
		PUTCHAR(c);
		break;
	    case '\'':
		state = NORMAL;
		break;
	    case '\\':
		getc(Input);
		break;
	    }
	    break;
	}
	Lastc = c;
    } while (state != NORMAL || Pipewrite - Piperead < SIZE / 2);

    return 0;
}


void
decomment_file(char *fname, FILE *f)
{
    int c;
    extern int Cppflag;

    Input = f;

    Cppflag = 1;

    while ((c = Getchar()) != EOF)
    {
	putc(c, stdout);
    }
}

int
decomment_files(int argc, char *argv[])
{
    int result = 0;
    if (argc == 1)
    {
	decomment_file("stdin", stdin);
    }
    else
    {
	while (argc > 1)
	{
	    FILE *f;

	    if ((f = fopen(argv[1], "r")) == NULL)
	    {
		result = 2;
		perror(argv[1]);
	    }
	    else
	    {
		decomment_file(argv[1], f);
		fclose(f);
	    }
	    SHIFT(1);
	}
    }

    return result;
}

void
ncss_Ungetc(int c)
{
    if (c == T_NCNULINE)
    {
	ncss_Line--;
    }
    Ungetc(c);
}

void
ncss_Ungets(char *s)
{
    Ungets(s);
}

int
ncss_Getchar()
{
    int c;
    static int blankline = 1;

    if ((c = Getchar()) != EOF)
    {
	if (blankline)
	{
	    if (!ISSPACE(c))
	    {
		blankline = 0;
	    }
	}
	else
	{
	    if (c == '\n')
	    {
		c = T_NCNULINE;
		blankline = 1;
	    }
	}
    }
    else
    {
	blankline = 1;
    }

    if (c == T_NCNULINE)
	ncss_Line++;

    return c;
}

void
ncss(FILE *in, int *linesp, int *nclinesp)
{
    int c;
    int lines = 0;
    int nclines = 0;
    int boline = 1;

    while ((c = Getchar()) != EOF)
    {
	if (c == '\n')
	{
	    lines++;
	    boline = 1;
	}
	else if (boline && !ISSPACE(c))
	{
	    boline = 0;
	    nclines++;
	}
    }

    *linesp = lines;
    *nclinesp = nclines;
}

void
ncss_file(char *fname, FILE *f, int *linesp, int *nclinesp)
{
    extern int Cppflag;
    int lines, nclines;

    Input = f;

    Cppflag = 1;
    ncss(f, &lines, &nclines);
    if (!Totalsonly)
    {
	if (lines != 0)
	{
	    int pct_csl = (int)(0.4999999 + 100.0 * (lines - nclines) / lines);

	    printf("%6d%4d%6d%4d%7d  %-s\n",
		lines - nclines,
		pct_csl,
		nclines,
		100 - pct_csl,
		lines,
		fname);
	}
	else
	{
	    printf("%6d n/a%6d n/a%7d  %-s\n",
		lines - nclines,
		nclines,
		lines,
		fname);
	}
    }

    *linesp += lines;
    *nclinesp += nclines;
}

int
ncss_files(int argc, char *argv[])
{
    int result = 0;
    int lines = 0, nclines = 0;
    int nfiles = argc - 1;

    if (Verbose)
        puts("   CSL PCT  NCSL PCT  TOTAL  FILENAME");

    if (argc == 1)
    {
	ncss_file("stdin", stdin, &lines, &nclines);
    }
    else
    {
	while (argc > 1)
	{
	    FILE *f;

	    if ((f = fopen(argv[1], "r")) == NULL)
	    {
		result = 2;
		perror(argv[1]);
	    }
	    else
	    {
		ncss_file(argv[1], f, &lines, &nclines);
		fclose(f);
	    }
	    SHIFT(1);
	}
    }

    if (Totals)
    {
	if (lines != 0)
	{
	    int pct_csl = (int)(0.4999999 + 100.0 * (lines - nclines) / lines);

	    printf("%6d%4d%6d%4d%7d  (total files: %d)\n",
		lines - nclines,
		pct_csl,
		nclines,
		100 - pct_csl,
		lines,
		nfiles);
	}
	else
	{
	    printf("%6d n/a%6d n/a%7d  (total files: %d)\n",
		lines - nclines,
		nclines,
		lines,
		nfiles);
	}
    }

    return result;
}
