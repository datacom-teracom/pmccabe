/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#include <stdio.h>
#include "pmccabe.h"

#include "dmain.h"

int
matchcurly()
{
    int c;
    int nest = 1;

    while (nest > 0 && (c = ncss_Getchar()) != EOF)
    {
	switch (c)
	{
	case '{':
	    nest++;
	    break;

	case '}':
	    nest--;
	    break;
	}
    }

    if (nest > 0 /* && c == EOF */ )
    {
	Exit = 7;
	fileerror("not enough }'s");
    }

   return c;
}

int
matchparen()
{
    int c;
    int nest = 1;

    while (nest > 0 && (c = ncss_Getchar()) != EOF)
    {
	switch (c)
	{
	case '(':
	    nest++;
	    break;

	case ')':
	    nest--;
	    break;
	}
    }

    if (nest > 0 /* && c == EOF */ )
    {
	Exit = 8;
	fileerror("not enough )'s");
    }

    return c;
}

int
skipws()
{
    int c;

    /* skip whitespace */
    while ((c = ncss_Getchar()) != EOF && ISSPACE(c))
    {
    }

    return c;
}

int
getsimpleident(char *buf)
{
    int c = 0;

    while (c != EOF)
    {
	c = ncss_Getchar();

	if (ISIDENT(c))
	{
	    *buf++ = c;
	}
	else
	{
	    *buf++ = '\0';
	    break;
	}
    }

    return c;
}

int
identify(const char *ident)
{
    int r = T_IDENT;

    switch(*ident)
    {
    case 'i':
        if (STREQUAL(ident, "if"))
	    r = T_IF;
	break;
    case 'w':
        if (STREQUAL(ident, "while"))
	    r = T_WHILE;
	break;
    case 'c':
        if (STREQUAL(ident, "case"))
	    r = T_CASE;
	else if (STREQUAL(ident, "class"))
	    r = T_CLASS;
	else if (STREQUAL(ident, "const"))
	    r = T_CONST;
	break;
    case 's':
        if (STREQUAL(ident, "switch"))
	    r = T_SWITCH;
	else if (STREQUAL(ident, "struct"))
	    r = T_STRUCT;
	break;
    case 'f':
        if (STREQUAL(ident, "for"))
	    r = T_FOR;
	break;
    case 'u':
        if (STREQUAL(ident, "union"))
	    r = T_UNION;
	break;
    case 'o':
        if (STREQUAL(ident, "operator"))
	    r = T_OPERATOR;
	break;
    case 'n':
        if (STREQUAL(ident, "namespace"))
	    r = T_NAMESPACE;
	break;
    }

    return r;
}

void
ungettoken(int c, char *s)
{
    if (c >= T_WORDS)
    {
	ncss_Ungets(s);
    }
    else
    {
	ncss_Ungetc(c);
    }
}

int
gettoken(char *buf, int *line, int *nLine)
/*
 * Callers depend on the fact that gettoken() doesn't modify buf except
 * when T_IDENT is parsed.
 */
{
    int c;
    int colon = FALSE;
    char *startbuf = buf;

    /* skip whitespace */
    c = skipws();

    if (line != NULL)
    	*line = Line;

    if (nLine != NULL)
	*nLine = ncss_Line;

    if (ISIDENT1(c))
    {
	*buf++ = c;
	ncss_Ungetc(getsimpleident(buf));
	c = identify(buf - 1);
    }

    return c;
}

int
gettoken2(char *buf, int *line, int *nLine)
/*
 * This one can additionally return T_ASSIGN and T_LOGICAL.  But note
 * that the caller isn't given enough data to know what specifically
 * was parsed.
 */
{
    int c, c1, c2;
    int colon = FALSE;
    char *startbuf = buf;

    /* skip whitespace */
    c = skipws();

    if (line != NULL)
    	*line = Line;

    if (nLine != NULL)
	*nLine = ncss_Line;

    switch(c)
    {
    case '*':	/* OP= */
    case '/':
    case '%':
    case '^':
	c1 = ncss_Getchar();
	if (c1 == '=')
	{
	    c = T_ASSIGN;
	}
	else
	{
	    ncss_Ungetc(c1);
	}
	break;

    case '+':	/* +=, ++ */
    case '-':	/* -=, -- */
	c1 = ncss_Getchar();
	if (c1 == '=' || c1 == c)
	{
	    c = T_ASSIGN;
	}
	else
	{
	    ncss_Ungetc(c1);
	}
	break;

    case '&':	/* &=, && */
    case '|':	/* |=, || */
	c1 = ncss_Getchar();
	if (c1 == '=')
	{
	    c = T_ASSIGN;
	}
	else if (c1 == c)
	{
	    c = T_LOGICAL;
	}
	else
	{
	    ncss_Ungetc(c1);
	}
	break;

    case '<':	/* >>= */
    case '>':	/* <<= */
	c1 = ncss_Getchar();
	if (c1 == c)
	{
	    c2 = ncss_Getchar();
	    if (c2 == '=')
	    {
		c = T_ASSIGN;
	    }
	    else
	    {
		ncss_Ungetc(c2);
	    }
	}
	else
	{
	    ncss_Ungetc(c1);
	}
	break;
    }

    if (ISIDENT1(c))
    {
	*buf++ = c;
	ncss_Ungetc(getsimpleident(buf));
	c = identify(buf - 1);
    }

    return c;
}

void
operatorident(char *s, int c)
/*
 * We're in an operator-overloaded C++ identifier.  This isn't
 * perfect but we read until either ( or ; to guess the identifier's
 * name.  In this pass we also replace whitespace with _ for printing.
 */
{
    while (c != EOF)
    {
	if (ISSPACE(c))
	{
	    ncss_Ungetc(skipws());
	    *s++ = '_';
	}
	else if (c == '(' || c == ';')
	{
	    ncss_Ungetc(c);
	    break;
	}
	else
	{
	    *s++ = c;
	}
	c = ncss_Getchar();
    }

    if (s[-1] == '_')
	s[-1] = '\0';
    else
	s[0] = '\0';
}
