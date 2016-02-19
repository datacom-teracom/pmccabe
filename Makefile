# Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later
CFILES=\
	cparse.c      dmain.c	    gettoken.c	  nmain.c	pmccabe.c \
	getopt.c	    io.c

OFILES=$(CFILES:.c=.o)

INCLUDES=config.h dmain.h getopt.h pmccabe.h

###############{

# On HP-UX you will have to change this
INSTALL = install -o root -g root

DESTDIR = 

PROGS = codechanges pmccabe decomment vifn

MANPGS = codechanges.1 pmccabe.1 vifn.1 decomment.1

DOCS = TODO ChangeLog COPYING

PMOBJS	      = cparse.o \
		dmain.o \
		gettoken.o \
		io.o \
		nmain.o \
		pmccabe.o

all:		$(PROGS) test

test:		$(PROGS)
		./testsuite

pmccabe:	$(PMOBJS)
	$(CC) $(CFLAGS) -o pmccabe $(PMOBJS)

clean:
	rm -f *.[oa] pmccabe decomment *.out */*.out

install: $(PROGS) $(MANPGS) $(DOCS)
	$(INSTALL) -d $(DESTDIR)/usr/share/doc/pmccabe \
			$(DESTDIR)/usr/share/man/man1 \
			$(DESTDIR)/usr/bin
	$(INSTALL) -m 644 $(MANPGS) $(DESTDIR)/usr/share/man/man1
	#$(INSTALL) -m 644 $(DOCS)  $(DESTDIR)/usr/share/doc/pmccabe
	$(INSTALL) -m 755 $(PROGS) $(DESTDIR)/usr/bin

###############}

getopt.o :		config.h getopt.h

dmain.o \
io.o :		dmain.h pmccabe.h

cparse.o \
gettoken.o \
nmain.o :		pmccabe.h dmain.h

pmccabe.o :		pmccabe.h getopt.h
