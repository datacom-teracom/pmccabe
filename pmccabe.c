/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#ifdef __hpux
/* Required for access to 'optind' for getopt() on HP-UX 9 */
#define _HPUX_SOURCE 1
#endif
#include <unistd.h>
#include <strings.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pmccabe.h"
#ifdef WIN32
#include "getopt.h"
#endif

#ifdef NEED_OPTIND
extern int optind;
#endif

static char _RcsVersion[] = "@(#)REV: $Header: /gjd/tools/pmccabe/pmccabe.c 1.23 2001/01/26 23:00:34 bame Exp $";
static const char _Version[] = "@(#) pmccabe 2.6";

int Cyco = 0;
int Softbuild = 0;
int Verbose = 0;
int Pass1 = 0;
int Totals = 0;
int Totalsonly = 0;
int Files = 0;
int Filesonly = 0;
int Ncss = 0;
int Ncssfunction = 0;

static char Normalheader[] = "Modified McCabe Cyclomatic Complexity\n"
"|   Traditional McCabe Cyclomatic Complexity\n"
"|       |    # Statements in function\n"
"|       |        |   First line of function\n"
"|       |        |       |   # lines in function\n"
"|       |        |       |       |  filename(definition line number):function\n"
"|       |        |       |       |           |\n";

static char NCSSheader[] = "Modified McCabe Cyclomatic Complexity\n"
"|   Traditional McCabe Cyclomatic Complexity\n"
"|       |    # Statements in function\n"
"|       |        |   First line of function\n"
"|       |        |       |   # uncommented nonblank lines in function\n"
"|       |        |       |       |  filename(definition line number):function\n"
"|       |        |       |       |           |\n";

static char Usage[] =
	"Usage: %s [-vdCbtTfFVn]\n"
	"\t-V\tPrint pmccabe version information\n"
	"\t-v\tVerbose - print column headers (nonsense with -b or -C)\n"
	"\t-t\tPrint totals\n"
	"\t-T\tPrint totals only\n"
	"\tMajor Modes:\n"
	"\t    -d\tDe-comment only - can be used to count non-commented\n"
	"\t    \tsource lines for example.\n"
	"\t    -n\tCount non-commented source lines\n"
	"\t    *\tDefault mode: count complexity, #statements, etc...\n"
	"\t\t-C\tA custom output format\n"
	"\t\t-c\tCount noncommented lines/function instead of\n"
	"\t\t\traw lines/function\n"
	"\t\t-b\tAn output format compatible with softbuild and other\n"
	"\t\t\ttools which understand traditional compiler errors\n"
	"\t\t-f\tPrint per-file complexity totals\n"
	"\t\t-F\tPrint per-file complexity totals only\n"
	;

int
main(int argc, char *argv[])
{
    int result = 0;
    int c;
    char *progname = argv[0];

    /* grab command-line options */

    while ((c = getopt(argc, argv, "CvbdTtfFVnc")) != EOF)
    {
	switch(c)
	{
	case 'c':
	    Ncssfunction = 1;
	    break;
	case 'C':
	    Cyco = 1;
	    break;
	case 'v':
	    Verbose = 1;
	    break;
	case 'b':
	    Softbuild = 1;
	    break;
	case 'd':
	    Pass1 = 1;
	    break;
	case 'T':
	    Totalsonly = 1;
	    Totals = 1;
	    break;
	case 't':
	    Totals = 1;
	    break;
	case 'f':
	    Files = 1;
	    break;
	case 'F':
	    Filesonly = 1;
	    Files = 1;
	    break;
	case 'V':
	    puts(_Version);
	    return 0;
	    break;
	case 'n':
	    Ncss = 1;
	    break;
	case '?':
	default:
	    fprintf(stderr, Usage, progname);
	    exit(3);
	    break;
	}
    }

    SHIFT(optind - 1);

    if (Pass1)
        result = decomment_files(argc, argv);
    else if (Ncss)
    	result = ncss_files(argc, argv);
    else
    {
	stats_t *total = stats_push("Total", STATS_TOTAL);
	total->firstline = 1;

	if (Verbose && !Cyco && !Softbuild)
	{
	    fputs(Ncssfunction ? NCSSheader : Normalheader, stdout);
	}

	if (argc == 1)
	{
	    file("stdin", stdin);
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
		    file(argv[1], f);
		    fclose(f);
		}
		SHIFT(1);
	    }
	}

	if (Totals)
	{
	    printstats(total);
	}
    }

    return result;
}

static stats_t Stats[MAXDEPTH];
int Nstats = 0;

/* These are cheating */
#define STOTAL Stats[0]
#define SFILE Stats[1]

stats_t *
stats_push(const char *name, int type)
{
    stats_t *sp;

    if (Nstats == MAXDEPTH)
    {
        fprintf(stderr, "Maximum name nesting depth (%d) exceed - exit\n",
				MAXDEPTH);
	exit(3);
    }

    sp = &Stats[Nstats++];
    ZERO(*sp);
    if (Nstats > 1)
    	sp->prev = sp - 1;
    sp->name = strdup(name);
    sp->type = type;

    return sp;
}

stats_t *
stats_current()
{
    if (Nstats < 1)
    {
	fprintf(stderr, "stats_current() called with Nstats < 1 -- exit\n");
	exit(3);
    }

    return Stats + Nstats - 1;
}

stats_t *
stats_pop(stats_t *sp)
{
    assert(sp != NULL);

    if (sp != NULL)
    {
	if (sp != stats_current())
	{
	    fprintf(stderr, "stats_pop() popped value not current value - exit\n");
	    exit(3);
	}
    }

    if (Nstats == 0)
    {
	fprintf(stderr, "stats_pop() can't pop zero-length stack - exit\n");
	exit(3);
    }

    stats_accumulate(sp);
    free(sp->name);
    Nstats--;

    return stats_current();
}

static
void
stats_add(stats_t *result, stats_t *sp)
{
    result->nfor += sp->nfor;
    result->nwhile += sp->nwhile;
    result->nif += sp->nif;
    result->nand += sp->nand;
    result->nor += sp->nor;
    result->nq += sp->nq;
    result->nsemicolons += sp->nsemicolons;
    result->nswitch += sp->nswitch;
    result->ncase += sp->ncase;
    result->nstatements += sp->nstatements;
    result->nfunctions += sp->nfunctions;
    result->lastline += sp->lastline - sp->firstline + 1;
    result->nLines += sp->nLines;
}

void
stats_accumulate(stats_t *sp)
{
    stats_t *result;
    for (result = sp->prev; result != NULL; result = result->prev)
    {
        if (result->type == STATS_FILE || result->type == STATS_TOTAL)
	    break;
    }

    if (result == NULL)
    {
        fprintf(stderr, "Error in stats_accumulate() - exit\n");
	exit(3);
    }
    stats_add(result, sp);
}

void
fileerror(const char *error)
{
    fprintf(stderr, "\"%s\", line %d: %s\n",
    	SFILE.name,
	Line,
	error);
}
