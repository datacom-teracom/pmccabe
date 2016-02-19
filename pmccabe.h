/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#include <string.h>
#ifdef __hpux
#include <strings.h>
#endif
#include <ctype.h>

/* Returned by gettoken() */
#define T_BASE 256
#define T_ASSIGN (T_BASE + 0)
#define T_LOGICAL (T_BASE + 1)
/* non-commented nuline returned by ncss_Getchar() */
#define T_NCNULINE (T_BASE + 1)

#define T_WORDS (T_BASE + 20)
#define T_IDENT (T_WORDS + 2)
#define T_IF (T_WORDS + 3)
#define T_WHILE (T_WORDS + 4)
#define T_CASE (T_WORDS + 5)
#define T_SWITCH (T_WORDS + 6)
#define T_FOR (T_WORDS + 7)
#define T_UNION (T_WORDS + 8)
#define T_STRUCT (T_WORDS + 9)
#define T_CLASS (T_WORDS + 10)
#define T_OPERATOR (T_WORDS + 11)
#define T_CONST (T_WORDS + 12)
#define T_NAMESPACE (T_WORDS + 13)

#define STREQUAL(a, b)	(strcmp((a),(b)) == 0)
#define ZERO(x)	memset(&x, 0, sizeof x)

#define SHIFT(n)	argc -= (n); argv += (n)

/* values for stats_t.type */
#define STATS_TOTAL	0
#define STATS_FILE	1
#define STATS_FUNCTION	2
#define STATS_CLASS	3
#define STATS_NAMESPACE	4

struct stats_t
{
    char *name;
    int nstatements;
    int nfunctions;
    int firstline;
    int lastline;
    int defline;
    int nLines;
    int nfor, nwhile, nswitch, ncase, nif;
    int nand, nor, nq;
    int nsemicolons;
    struct stats_t *prev;
    char type;
};

typedef struct stats_t stats_t;

/* can only nest this many names, including Total and file name */
#define MAXDEPTH 100

extern int Line, Linetokens, ncss_Line;
extern int Exit;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* pmccabe.c - command-line options */
extern int Cyco;
extern int Softbuild;
extern int Verbose;
extern int Pass1;
extern int Totals;
extern int Totalsonly;
extern int Files;
extern int Filesonly;
extern int Line;
extern int Ncss;
extern int Ncssfunction;
extern int Unbuf[256];
extern int *Unptr;

/* cparse.c */
int fancygettoken(char *buf, int classflag, int *line, int *nLine);
int toplevelstatement(stats_t *stats);
int findchar(char fc);
int maybeclass(void);
void findsemicolon();
int getoverloadedop(char *buf);
int fancyfunction(char *buf, stats_t *fs, stats_t *fn);
void possiblefn(stats_t *stats, const char *name, int line1, int defline, int nLine1);
int prefunction(int *nstatements);
int countfunction(stats_t *fn);
void countword(stats_t *fn, int id);

/* dmain.c */
int decomment(void);
int decomment_files(int argc, char *argv []);
int ncss_files(int argc, char *argv []);

/* gettoken.c */
int matchcurly();
int matchparen(void);
int skipws(void);
int getsimpleident(char *buf);
int gettoken(char *buf, int *line, int *nLine);
int gettoken2(char *buf, int *line, int *nLine);
void operatorident(char *s, int c);
int identify(const char *ident);

void Ungetc(int c);
void ncss_Ungetc(int c);

void Ungets(char *s);
void ncss_Ungets(char *s);

int Getchar(void);
int ncss_Getchar(void);

/* nmain.c */
void file(char *fname, FILE *f);
void cycoprintstats(stats_t *fs, stats_t *fn);
void softbuildprintstats(stats_t *fs, stats_t *fn);
void printstats(stats_t *sp);
void fileerror(const char *s);

/* pmccabe.c */
int main(int argc, char *argv []);
stats_t *stats_push(const char *name, int type);
stats_t *stats_current(void);
stats_t *stats_pop(stats_t *sp);
void stats_accumulate(stats_t *sp);

#define ISSPACE(c) ((c) == T_NCNULINE || (c) == '\n' \
			|| (c) == '\t' || (c) == ' ')

#define ISIDENT1(c) (((c) >= 'a' && (c) <= 'z') \
			|| ((c) >= 'A' && (c) <= 'Z') \
			|| ((c) == '_'))

#define ISIDENT(c)  ((ISIDENT1(c)) || ((c) >= '0' && (c) <= '9'))
