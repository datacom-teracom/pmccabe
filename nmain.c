/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#include <stdio.h>
#if defined MSC6 || defined WIN32
#define MAXPATHLEN 1024
#else
#include <sys/param.h>	/* for MAXPATHLEN */
#endif
#include <stdlib.h>
#include <assert.h>
#include "pmccabe.h"
#include "dmain.h"

/* $Id: nmain.c,v 1.23 2001/01/26 23:00:37 bame Exp $ */

int Exit = 0;

void
file(char *fname, FILE *f)
{
    stats_t *filestats;
    extern int Unbuf[], *Unptr;
    extern int Pass1;

    Input = f;

    if (Pass1)
    {
	int c;
	extern int Cppflag;

	Cppflag = 1;

	while ((c = Getchar()) != EOF)
	{
	    putc(c, stdout);
	}
	return;
    }

    Unptr = Unbuf;
    Line = 1;
    ncss_Line = 0;

    ZERO(filestats);
    filestats = stats_push(fname, STATS_FILE);
    filestats->firstline = 1;

    while (toplevelstatement(filestats) != EOF)
    {
	filestats->nsemicolons++;
    }

    filestats->lastline = Line - 1;
    /* Not 100% sure why I need to subtract 1 here */
    filestats->nLines += ncss_Line - 1;

    if (Files)
    	printstats(filestats);
    stats_pop(filestats);
}

void
cycoprintstats(stats_t *fs, stats_t *fn)
{
    int basic, cycloswitch, cyclocase;

    basic = fn->nfor + fn->nwhile + fn->nif + fn->nand + fn->nor + fn->nq;

    cycloswitch = 1 + basic + fn->nswitch;
    cyclocase = 1 + basic + fn->ncase;

    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
	cycloswitch,
	cyclocase,
	fn->nstatements,
	fn->firstline,
	fn->defline,
	fn->lastline,
	fn->lastline - fn->firstline + 1,
	fs->name,
	fn->name);
}

void
softbuildprintstats(stats_t *fs, stats_t *fn)
{
    int basic, cycloswitch, cyclocase;

    basic = fn->nfor + fn->nwhile + fn->nif + fn->nand + fn->nor + fn->nq;

    cycloswitch = 1 + basic + fn->nswitch;
    cyclocase = 1 + basic + fn->ncase;

    printf("\"%s\", line %d: %s%%\t%d\t%d\t%d\t%d\t%d\n",
	fs->name, fn->defline,
	fn->name,
	cycloswitch,
	cyclocase,
	fn->nstatements,
	fn->firstline,
	fn->lastline - fn->firstline + 1);
}

static void
printname(stats_t *sp)
{
    if (sp != NULL)
    {
	printname(sp->prev);
        switch(sp->type)
	{
	case STATS_TOTAL:
	case STATS_FILE:
	    break;

	case STATS_FUNCTION:
	    printf("%s", sp->name);
	    break;

	case STATS_CLASS:
	    if (sp->prev != NULL && sp->prev->type == STATS_FUNCTION)
	        printf("/");
	    printf("%s::", sp->name);
	    break;

	case STATS_NAMESPACE:
	    printf("%s::", sp->name);
	    break;
	}
    }
}

void
printstats(stats_t *sp)
{
    int basic, cycloswitch, cyclocase;
    static int t_cswitch = 0;
    static int t_ccase = 0;
    static int t_statements = 0;
    static int t_lines = 0;
    int snlines;
    stats_t *fsp;

    basic = sp->nfor + sp->nwhile + sp->nif + sp->nand + sp->nor + sp->nq;

    sp->nstatements = basic - sp->nand - sp->nor + sp->nsemicolons;

    cycloswitch = sp->nfunctions + basic + sp->nswitch;
    cyclocase = sp->nfunctions + basic + sp->ncase;

    for (fsp = sp; fsp != NULL && fsp->type != STATS_FILE; fsp = fsp->prev)
    {
    }

    if (Ncssfunction)
    {
	snlines = sp->nLines + 1;
    }
    else
    {
	snlines = sp->lastline - sp->firstline + 1;
    }

    switch(sp->type)
    {
    case STATS_TOTAL:
	if (Softbuild)
	{
	    printf("\"n/a\", line n/a: %s", sp->name);
	    printf("%%\t%d\t%d\t%d\tn/a\t%d\n",
		    cycloswitch,
		    cyclocase,
		    sp->nstatements,
		    snlines);
	}
	else if (Cyco)
	{
	    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
		cycloswitch,
		cyclocase,
		sp->nstatements,
		sp->firstline,
		sp->defline,
		sp->lastline,
		snlines,
		"Total");
	}
	else
	{
	    printf("%d\t%d\t%d\tn/a\t%d\t",
		    cycloswitch,
		    cyclocase,
		    sp->nstatements,
		    snlines);

	    printf("Total\n");
	}
	break;
    case STATS_FILE:
	assert(fsp != NULL);
	if (Softbuild)
	{
	    printf("\"%s\", line 1: n/a", fsp->name);
	    printf("%%\t%d\t%d\t%d\t%d\t%d\n",
		    cycloswitch,
		    cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    snlines);
	}
	else if (Cyco)
	{
	    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
		cycloswitch,
		cyclocase,
		sp->nstatements,
		sp->firstline,
		sp->defline,
		sp->lastline,
		snlines,
		fsp->name);
	}
	else
	{
	    printf("%d\t%d\t%d\t%d\t%d\t",
		    cycloswitch,
		    cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    snlines);

	    printf("%s\n", fsp->name);
	}
	break;
    case STATS_FUNCTION:
	assert(fsp != NULL);
	if (Softbuild)
	{
	    printf("\"%s\", line %d: ", fsp->name, sp->defline);
	    printname(sp);
	    printf("%%\t%d\t%d\t%d\t%d\t%d\n",
		    cycloswitch,
		    cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    snlines);
	}
	else if (Cyco)
	{
	    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t",
		cycloswitch,
		cyclocase,
		sp->nstatements,
		sp->firstline,
		sp->defline,
		sp->lastline,
		snlines,
		fsp->name);
	    printname(sp);
	    putchar('\n');
	}
	else
	{
	    printf("%d\t%d\t%d\t%d\t%d\t",
		    cycloswitch,
		    cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    snlines);

	    printf("%s(%d): ", fsp->name, sp->defline);

	    printname(sp);
	    printf("\n");
	}
	break;
    case STATS_CLASS:
	abort();
	if (Softbuild)
	{
	}
	else if (Cyco)
	{
	}
	else
	{
	}
	break;
    }
}
