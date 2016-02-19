/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#ifdef __hpux
#define _XOPEN_SOURCE 1
#endif

#ifdef __unix
#include <unistd.h>
#endif

#ifdef WIN32
#include "getopt.h"
#endif

#ifdef NEED_OPTIND
extern int optind;
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static char id[] = "$Header: /gjd/tools/pmccabe/decomment.c 1.2 2001/01/26 23:00:35 bame Exp $";

static int G_removeBlankLines = 1;
static const char *G_lang;

static void
blankline(char *buf, char **bp)
{
    char *ptr;
    *(*bp) = '\0';

    if (G_removeBlankLines)
    {
	for (ptr = buf; *ptr != '\0'; ptr++)
	{
	    if (!isspace(*ptr))
	    {
		fputs(buf, stdout);
		break;
	    }
	}
    }
    else
    {
	fputs(buf, stdout);
    }

    *bp = buf;
}

static void
decomment_sh(const char *fname, FILE *in)
{
    register int c, c1;
    char outbuf[4 * 1024];
    char *op = outbuf;

    enum
    {
	NORMAL,
	SH_COMMENT,
	DOUBLEQUOTESTRING,
	SINGLEQUOTESTRING
    } state = NORMAL;

    while ((c = getc(in)) != EOF)
    {
	switch (state)
	{
	case NORMAL:
	    switch (c)
	    {
	    case '\'':
		state = SINGLEQUOTESTRING;
		*op++ = c;
		break;
	    case '"':
		state = DOUBLEQUOTESTRING;
		*op++ = c;
		break;
	    case '#':
		state = SH_COMMENT;
		break;
	    case '\\':
		*op++ = c;
		c1 = getc(in);
		*op++ = c1;
		break;
	    default:
		*op++ = c;
		break;
	    }
	    break;

	case SH_COMMENT:	/* C++ comment */
	    if (c == '\n')
	    {
		state = NORMAL;
	    }
	    break;

	case DOUBLEQUOTESTRING:
	    *op++ = c;
	    switch (c)
	    {
	    case '"':
		state = NORMAL;
		break;
	    case '\\':		/* handle quoted quotes */
		c1 = getc(in);
		*op++ = c1;
		break;
	    }
	    break;

	case SINGLEQUOTESTRING:
	    *op++ = c;
	    switch (c)
	    {
	    case '\'':
		state = NORMAL;
		break;
	    case '\\':
		getc(in);
		break;
	    }
	    break;
	}

	/* If we just stuffed a \n into the output buffer... */
	if (op > outbuf && op[-1] == '\n')
	{
	    blankline(outbuf, &op);
	}
    }
}

static void
decomment_asm(const char *fname, FILE *in)
{
    register int c, c1;
    char outbuf[4 * 1024];
    char *op = outbuf;

    enum
    {
	NORMAL,
	ASM_COMMENT,
	STRINGLITERAL,
	CHARLITERAL
    } state = NORMAL;

    while ((c = getc(in)) != EOF)
    {
	switch (state)
	{
	case NORMAL:
	    switch (c)
	    {
	    case '\'':
		state = CHARLITERAL;
		*op++ = c;
		break;
	    case '"':
		state = STRINGLITERAL;
		*op++ = c;
		break;
	    case ';':
		state = ASM_COMMENT;
		break;
	    default:
		*op++ = c;
		break;
	    }
	    break;

	case ASM_COMMENT:	/* C++ comment */
	    if (c == '\n')
	    {
		state = NORMAL;
	    }
	    break;

	case STRINGLITERAL:
	    *op++ = c;
	    switch (c)
	    {
	    case '"':
		state = NORMAL;
		break;
	    case '\\':		/* handle quoted quotes */
		c1 = getc(in);
		*op++ = c1;
		break;
	    }
	    break;

	case CHARLITERAL:
	    *op++ = c;
	    switch (c)
	    {
	    case '\'':
		state = NORMAL;
		break;
	    case '\\':
		getc(in);
		break;
	    }
	    break;
	}

	/* If we just stuffed a \n into the output buffer... */
	if (op > outbuf && op[-1] == '\n')
	{
	    blankline(outbuf, &op);
	}
    }
}

static
void
decomment_c(const char *fname, FILE *in)
{
    register int c, c1;
    char outbuf[4 * 1024];
    char *op = outbuf;

    enum
    {
	NORMAL,
	    C_COMMENT,
	    CC_COMMENT,
	    STRINGLITERAL,
	    CHARLITERAL
    } state = NORMAL;

    while ((c = getc(in)) != EOF)
    {
	switch (state)
	{
	case NORMAL:
	    switch (c)
	    {
	    case '\'':
		state = CHARLITERAL;
		*op++ = c;
		break;
	    case '"':
		state = STRINGLITERAL;
		*op++ = c;
		break;
	    case '/':
		c1 = getc(in);
		switch (c1)
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
		    *op++ = c;
		    ungetc(c1, in);
		    break;
		}
		break;
	    default:
		*op++ = c;
		break;
	    }
	    break;

	case C_COMMENT:		/* K&R C comment */
	    if (c == '*')
	    {
		c1 = getc(in);
		if (c1 == '/')
		{
		    state = NORMAL;
		}
		else
		{
		    ungetc(c1, in);
		}
	    }
	    break;

	case CC_COMMENT:	/* C++ comment */
	    if (c == '\n')
	    {
		state = NORMAL;
	    }
	    break;

	case STRINGLITERAL:
	    *op++ = c;
	    switch (c)
	    {
	    case '"':
		state = NORMAL;
		break;
	    case '\\':		/* handle quoted quotes */
		c1 = getc(in);
		*op++ = c1;
		break;
	    }
	    break;

	case CHARLITERAL:
	    *op++ = c;
	    switch (c)
	    {
	    case '\'':
		state = NORMAL;
		break;
	    case '\\':
		getc(in);
		break;
	    }
	    break;
	}

	/* If we just stuffed a \n into the output buffer... */
	if (op > outbuf && op[-1] == '\n')
	{
	    blankline(outbuf, &op);
	}
    };
}

static
void
doit(const char *fname, FILE *in)
{
    switch(*G_lang)
    {
    case 's':
	decomment_sh(fname, in);
	break;

    case 'c':
	decomment_c(fname, in);
	break;

    case 'a':
	decomment_asm(fname, in);
	break;

    default:
	abort();
	break;
    }
}

static
void
usage(const char *progname)
{
    fprintf(stderr,
	    "%s - Remove comments and blank lines from files\n\n"
	    "Usage: %s [options] [files]\n"
	    "	-b		Don't remove blank lines\n"
	    "	-l language 	Specify language of files\n"
	    "				c  = C and C++ (default)\n"
	    "				sh = Bourne/Posix shell-like\n"
	    "				asm = Assembler\n"
	    "			NOTE: Normally the language is derived from\n"
	    "			the file name.  '-l' overrides this entirely\n"
	    "			and is especially useful when %s is used as\n"
	    "			a filter.\n",
	    progname, progname, progname);

    exit(2);
}

int
main(int argc, char *argv[])
{
    char *langoverride = NULL;
    int c;

    while ((c = getopt(argc, argv, "bl:")) != EOF)
    {
	switch (c)
	{
	case 'b':
	    G_removeBlankLines = 0;
	    break;

	case 'l':
	    if (!(
			strcmp(optarg, "c") == 0 ||
			strcmp(optarg, "sh") == 0 ||
			strcmp(optarg, "asm") == 0
		    ))
	    {
		usage(argv[0]);
	    }
	    langoverride = optarg;
	    break;

	default:
	    usage(argv[0]);
	    break;
	}
    }

    if (optind == argc)
    {
	if (langoverride != NULL)
	{
	    G_lang = langoverride;
	}
	else
	{
	    G_lang = "c";
	}
	doit("stdin", stdin);
    }
    else
    {
	while (optind < argc)
	{
	    const char *fname = argv[optind];
	    G_lang = NULL;

	    if (langoverride != NULL)
	    {
		G_lang = langoverride;
	    }
	    else
	    {
		const char *basename;
		const char *ext;

		if ((basename = strrchr(fname, '/')) == NULL)
		{
		    basename = fname;
		}
		else
		{
		    basename++;
		}

		if ((ext = strrchr(basename, '.')) != NULL)
		{
		    ext++;
		    if (*ext != '\0')
		    {
			if (ext[1] == '\0')
			{
			    switch(ext[0])
			    {
			    case 'c':
			    case 'C':
			    case 'h':
			    case 'H':
				G_lang = "c";
				break;
			    case 's':
				G_lang = "asm";
				break;
			    }
			}
			else if (ext[0] == 'C' || ext[0] == 'c')
			{
			    G_lang = "c";
			}
			else if (strstr(ext, "sh") != NULL ||
				strstr(ext, "mak") != NULL ||
				strstr(ext, "mk") != NULL)
			{
			    G_lang = "sh";
			}
		    }
		}
		else
		{
		    if (strstr(basename, "akefile") != NULL)
		    {
			G_lang = "sh";
		    }
		}
	    }

	    if (G_lang == NULL)
	    {
		G_lang = "c";
	    }

	    if (strcmp(fname, "-") == 0)
	    {
		doit("stdin", stdin);
	    }
	    else
	    {
		FILE *f;
		if ((f = fopen(fname, "r")) == NULL)
		{
		    perror(fname);
		    continue;
		}
		doit(fname, f);
		fclose(f);
	    }

	    optind++;
	}
    }

    return 0;
}
