/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pmccabe.h"
#include "dmain.h"

/* $Id: cparse.c,v 1.24 2001/01/26 23:00:30 bame Exp $ */

int
fancygettoken(char *buf, int classflag, int *line, int *nLine)
{
    int c;
    char tmpbuf[256];

    if ((c = gettoken(buf, line, nLine)) == T_IDENT)
    {
	if ((c = gettoken(tmpbuf, NULL, NULL)) == ':')
	{
	    if ((c = ncss_Getchar()) == ':')
	    {
		buf += strlen(buf); *buf++ = ':'; *buf++ = ':';
		*buf = '\0';
		switch (c = gettoken(tmpbuf, NULL, NULL))
		{
		case T_OPERATOR:
		    strcat(buf, tmpbuf);
		    buf += strlen(buf);
		    getoverloadedop(buf);
		    break;
		case T_IDENT:
		    /* ident::ident - could be recursive */
		    ncss_Ungets(tmpbuf);
		    if ((c = fancygettoken(buf, 1, NULL, NULL)) != T_IDENT)
			ncss_Ungetc(c);
		    break;
		case '~':	/* destructor, collect the identifier */
		    *buf++ = c;
		    gettoken(buf, NULL, NULL);
		    break;
		default:
		    ncss_Ungetc(c);
		    *buf++ = '\0';
		    break;
		}
	    }
	    else
	    {
		/* only got ':', who knows what this is */
		ncss_Ungets("::");
	    }
	}
	else
	{
	    ungettoken(c, tmpbuf);
	}

	c = T_IDENT;
    }
    else if (classflag && c == T_OPERATOR)
    {
	/* strcat(buf, tmpbuf); */
	buf += strlen(buf);
	getoverloadedop(buf);
    }
    else if (classflag && c == '~')
    {
        *buf++ = c;
	c = gettoken(buf, NULL, NULL);
	if (c != T_IDENT)
	{
	    fprintf(stderr, "fatal error file %s line %d\n",
	    	__FILE__, __LINE__);
	    exit(3);
	}
    }

    return c;
}

int
toplevelstatement(stats_t *stats)
/*
 *	At the top level of a C file, the statements are blocks of
 *	tokens ending in either ; or are function definitions which
 *	end in }.  Interesting types of statements include class
 *	and struct definitions - because they may contain inline functions,
 *	and function definitions.  All others are merely counted.
 */
{
    int endofstatement = FALSE;
    int c;
    char buf[1024];
    int functionFirstLine = -1;
    int functionFirstNLine = -1;
    int functionDefLine;
    int line, nLine;

    buf[0] = '\0';

    c = skipws();
    ncss_Ungetc(c);

    /* gettoken eats whitespace */
    while (!endofstatement &&
    	(c = fancygettoken(buf, stats->type == STATS_CLASS, &line, &nLine)) != EOF)
    {
	if (functionFirstLine == -1)
	{
	    functionFirstLine = line;
	    functionFirstNLine = nLine;
	}
	switch (c)
	{
	case T_CLASS:
	case T_STRUCT:
	case T_UNION:
	    if (maybeclass())
	    {
		stats->nsemicolons--;
		endofstatement = TRUE;
	    }
	    break;

	case T_NAMESPACE:
	    if (maybenamespace())
	    {
		/* no trailing semicolon for namespaces */
		endofstatement = TRUE;
	    }
	    break;

	case '(':
	    /* possible start of function */
	    functionDefLine = Line;
	    possiblefn(stats, buf, functionFirstLine, functionDefLine, functionFirstNLine);
	    endofstatement = TRUE;
	    break;

	case '}':
	case ')':
	    Exit = 2;
	    {
		char _buf[100];
		sprintf(_buf, "too many %c's", c);
		fileerror(_buf);
	    }
	    break;

	case '{':
	    c = matchcurly();
	    break;

	case ':':	/* This should catch C++ "class foo { public: } */
	case ';':
	    /* end of statement */
	    endofstatement = TRUE;
	    break;

	default:
	    break;
	}
    }

    return c;
}

int
findchar(char fc)
{
    int c;

    while ((c = ncss_Getchar()) != EOF && c != fc)
    {
    }

    return c;
}

int
maybeclass()
/*
 * We've just seen "class" at the top level in a file so
 * we may be entering a definition of same.  If so, we want to be
 * on the lookout for inline functions.  Return 1 if this is a
 * class definition else restore the function name and return 0.
 */
{
    char classname[256], dummy[256];
    int isclass = 0;
    int c;

    if ((c =gettoken(classname, NULL, NULL)) == T_IDENT)
    {
	/* "class name" */
	switch(c = gettoken(dummy, NULL, NULL))
	{
	case '{':	/* "class name {" */
	    break;

	case ':':	/* "class name :" */
            c = gettoken(dummy, NULL, NULL);
	    if (c == ':')
	    {
	        /* "class name :: */
		/* this is a namespace-qualified declaration since cannot
		   have a definition like this. */
		break;
	    }
	    else
	    {
		/* "clas name : [one or more initializers]" */
	        while (c != '{')
		{
		    c = gettoken(dummy, NULL, NULL);
		}
	    }
	    break;

	default:
	    /* if we fail to get "class name [:.*] {" */
	    ungettoken(c, dummy);
	}
    }
    else if (c == '{')	/* Unnamed class */
    {
	/* "class {" */
	strcpy(classname, "unnamed");
    }
    else
    {
	/* "class BOGUS" -- perhaps this is C code using a variable "class" */
	ungettoken(c, dummy);
    }

    if (isclass = (c == '{'))
    {
	stats_t *class = stats_push(classname, STATS_CLASS);

	while ((c = gettoken(dummy, NULL, NULL)) != '}')
	{
	    if (c == EOF)
	    {
		fileerror("unexpected EOF");
		break;
	    }
	    else
		ungettoken(c, dummy);

	    toplevelstatement(class);
	}

	stats_pop(class);
    }

    return isclass;
}

int
maybenamespace()
/*
 * We've just seen "namespace" at the top level in a file so
 * we may be entering a definition of same (if we next find "token {").
 * Return 1 if this is a
 * namespace definition else restore the name and return 0.
 */
{
    char nsname[256], dummy[256];
    int isns = 0;
    int c;

    if ((c = gettoken(nsname, NULL, NULL)) == T_IDENT)
    {
	/* "namespace name" */
	switch(c = gettoken(dummy, NULL, NULL))
	{
	case '{':	/* "namespace name {" */
	    break;

	default:
	    /* if we fail to get "namespace name {" */
	    ungettoken(c, dummy);
	}
    }
    else if (c == '{')	/* Unnamed namespace */
    {
	/* "namespace {" */
	strcpy(nsname, "anonymous_namespace");
    }
    else
    {
	/* "namespace BOGUS" -- is C code using a variable "namespace"? */
	ungettoken(c, dummy);
    }

    if (isns = (c == '{'))
    {
	stats_t *ns = stats_push(nsname, STATS_NAMESPACE);

	while ((c = gettoken(dummy, NULL, NULL)) != '}')
	{
	    if (c == EOF)
	    {
		fileerror("unexpected EOF");
		break;
	    }
	    else
		ungettoken(c, dummy);

	    toplevelstatement(ns);
	}

	stats_pop(ns);
    }

    return isns;
}


void
findsemicolon()
{
    int c;

    while ((c = ncss_Getchar()) != EOF && c != ';')
    {
	switch (c)
	{
	case '(':
	    c = matchparen();
	    break;
	case '{':
	    c = matchcurly();
	    break;
	}
    }

    if (c == EOF)
    {
	Exit = 5;
	fileerror("expected ';' got EOF");
    }
}

int
getoverloadedop(char *buf)
/*
 * Having just read ident::operator, try to read the operator into buf.
 * If the first non-WS character is a '(', the overloaded thing is a
 * function call.  Otherwise it's some type of real operator and we
 * terminate normally on '('.  If we read ; or { we probably should
 * print a warning and bail out.
 */
{
    char *savebuf = buf;
    char tmpbuf[256];
    int c = gettoken(tmpbuf, NULL, NULL);

    if (c == '(')
    {
	/* overloaded function call syntax */
	*buf++ = c;
	/* Match the paren */
	while(c != ')')
	{
	    if ((c = skipws()) == EOF)
		break;

	    *buf++ = c;
	}
    }
    else if (c == T_IDENT)
    {
	/* class::operator int() */
	/* Overloaded typecast */
	*buf++ = '_';
	*buf = '\0';
	strcat(buf, tmpbuf);
	buf += strlen(buf);
	*buf++ = '(';
	*buf++ = ')';
	*buf = '\0';
    }
    else if (c != EOF)
    {
	*buf++ = c;
	while ((c = ncss_Getchar()) != EOF)
	{
	    if (!ISSPACE(c))
	    {
		if (c == '(' || c == ';')
		{
		    ncss_Ungetc(c);
		    break;
		}
		else
		    *buf++ = c;
	    }
	}
    }

    *buf = '\0';

    return c;
}

void
possiblefn(stats_t *stats, const char *name, int line1, int defline, int nLine1)
/*
 *	We've just read an open parenthesis.  If there's a legal identifier
 *	in name we may be within a function definition.
 */
{
    char dummy[257];
    int nstatements = 0;	/* in case there's code prior to the { */
    int c;

    if (strlen(name) == 0)
    {
	/* no function name - must not be a function - return */
	findsemicolon();
    }
    else
    {
	if ((c = matchparen()) != EOF)
	{
	    c = gettoken(dummy, NULL, NULL);

	    switch (c)
	    {
	    case T_CONST:
		if (strchr(name, ':') != NULL || stats->type == STATS_CLASS)
		{
		    /* foo::foo() const ^ [;] { */
		    /* This'll either be a ; for a declaration or a { */
		    c = gettoken(dummy, NULL, NULL);
		    break;
		}
		/* foo() const ^ char *a; { */
		/*** FALL THROUGH ***/

	    case T_IDENT:
	    case T_STRUCT:
	    case T_UNION:
		/* if (strchr(name, ':') == NULL && stats->type != STATS_CLASS) */
		{
		    /* K&R function, T_IDENT is part of first parm defn */
		    /* Read up to that first '{' */
		    /* function foo(a, b, c) int a; */
		    /*                          ^   */
		    while ((c = ncss_Getchar()) != EOF && c != '{')
		    {
		    }
		}
		break;
	    case '{':
		/* open { of the function */
		break;
	    case '(':
		/* wierd possibility in C++ - what we thought was the */
		/* parameter list was really part of an overloaded */
		/* operator overloading of an odd typecast or something. */
		/* The function name will be wrong but who cares :-> */

		c = matchparen();
		if (c != EOF)
		    c = gettoken(dummy, NULL, NULL);
		break;
	    case ':':
		/* Another C++-ism:	main(args): ident(args), ident(args) */
		c = prefunction(&nstatements);
		break;
	    }

	    if (c == '{')
	    {
		/* This really is a function */
		stats_t *fn = stats_push(name, STATS_FUNCTION);
		fn->nfunctions = 1;
		fn->firstline = line1;
		fn->defline = defline;
		fn->nsemicolons = nstatements;

		c = countfunction(fn);
		fn->nLines = ncss_Line - nLine1;
		stats->nLines -= fn->nLines;
		if (!Totalsonly && !Filesonly)
		    printstats(fn);
		stats_pop(fn);
	    }
	}
    }
}

int
prefunction(int *nstatements)
/*
 * Handle C++ ident(args) : ident(args), ident(args).  Count each
 * ident(args) after function declaration as a statement.
 */
{
    int c;

    (*nstatements)++;

    while((c = ncss_Getchar()) != EOF)
    {
	switch(c)
	{
	case '(':
	    c = matchparen();
	    break;
	case ',':
	    (*nstatements)++;
	    break;
	case '{':
	    return c;
	}
    }
    Exit = 9;
    fileerror("expected { got EOF");

    return c;
}

int
countfunction(stats_t *fn)
{
    int nest = 1;
    int c;
    char id[257];

    while (nest > 0 && (c = gettoken2(id, NULL, NULL)) != EOF)
    {
	switch (c)
	{
	case ';':
	    fn->nsemicolons++;
	    break;

	case '{':
	    nest++;
	    break;

	case '}':
	    nest--;
	    break;

	case '?':
	    fn->nq++;
	    break;

	case T_LOGICAL:
	    fn->nor++;
	    break;

	case '^':
	    fn->nor++;	 /* This is XOR but I'm lazy */
	    break;

	case T_CLASS:
	case T_UNION:
	case T_STRUCT:
	    if (maybeclass())
		fn->nsemicolons--;
	    break;

	default:
	    countword(fn, c);
	    break;
	}
    }

    fn->lastline = Line;

    if (nest > 0 /* && c == EOF */ )
    {
	Exit = 6;
	fileerror("not enough }'s");
    }

    return c;
}

void
countword(stats_t * fn, int id)
{
    switch (id)
    {
    case T_IF:
	fn->nif++;
	break;
    case T_FOR:
	fn->nfor++;
	break;
    case T_WHILE:
	fn->nwhile++;
	break;
    case T_SWITCH:
	fn->nswitch++;
	break;
    case T_CASE:
	fn->ncase++;
	break;
    }
}
