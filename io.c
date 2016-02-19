/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#include <stdio.h>
#include "dmain.h"
#include "pmccabe.h"

int Line;

int Unbuf[256];
int *Unptr = Unbuf;

void
Ungetc(int c)
{
    if (c == '\n')
    {
	Line--;
    }
    *Unptr++ = c;
}

void
Ungets(char *s)
{
    int c;
    char *ptr;

    ptr = s + strlen(s);
    do
    {
	c = *--ptr;
	if (!ISSPACE(c))
	if (c == '\n')
	{
	    Line--;
	}
	*Unptr++ = c;
    } while (ptr != s);
}

int
Getchar()
{
    int c;
    if (Unptr == Unbuf)
    {
	if (Piperead >= Pipewrite)
	    decomment();

	c = *Piperead++;
    }
    else
    {
        c = *--Unptr;
    }

    if (c == '\n')
    	Line++;

    return c;
}
