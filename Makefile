LEX=lex
YACC=yacc -d
CC=gcc #-Xlinker -Bstatic # -ansi
AR=ar -ruv
#AR=ar r
CFLAGS= -L. -I../include -I..
.SUFFIXES: .c .h .y .l

.y.h:
	$(YACC) -d $<

.y.c:
	$(YACC) $<

.l.c:
	$(LEX) $<

all:	art

compile-time:	art

art:	rcode.h rcode.c kbhit.c lex.yy.c y.tab.c rl.h y.tab.h libassoc.a 
	$(CC) $(CFLAGS) -DCOMPILETIME -o art y.tab.c kbhit.c -lfl -lassoc 

lex.yy.c:	ariel.l
	$(LEX) ariel.l

y.tab.c:	ariel.y
	$(YACC) -d $<

assoc.o:	assoc.c assoc.h
	$(CC) -O -c assoc.c
	
acgi.o:		acgi.c assoc.h
	$(CC) -O -c acgi.c
	
asys.o:		asys.c assoc.h
	$(CC) -O -c asys.c

libassoc.a:	assoc.o acgi.o asys.o
	$(AR) libassoc.a assoc.o acgi.o asys.o
	ranlib libassoc.a
