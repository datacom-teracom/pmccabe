/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
/* $Id: dmain.h,v 1.8 2001/01/26 23:00:36 bame Exp $ */
#define EOINPUT 999
#define SIZE (1024 * 8)
extern short Pipe[SIZE];
extern short *Piperead;
extern short *Pipewrite;
extern short *Pipeend;

#define PUTCHAR(c)	{if (Pipewrite < Pipeend) *Pipewrite++ = (c);}
#define PUTS(s)		{char *a = s; while (*a != '\0') PUTCHAR(*a++);}

extern FILE *Input;
extern char Inputfile[1030];
