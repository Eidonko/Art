
# line 2 "rl.y"
/*
 *
 *      File: rl.y
 *
 *      Description: parser of the RL language
 *
 *      Language: yacc
 *
 *      History:
 *      Version   1.1b 27-Aug-1997  Eidon@tutanota.com
 */
#define VERSION "v1.1b 27-Aug-1997"
#define EFTOS_ENV "EFTOS_HOME"
#define EFTOS_SRC ".eftosrc"
#define EFTOS_OUT ".eftosrcode"

/* input and output file names */
    char *ifname, *ofname;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "rl.h"

int i, goto_pc;
extern int list[], card_list, lines;
int default_action;
char verbose, debug;

FILE *f;  /* output file pointer */

typedef struct { int role; int id; } ident_t;

#include "rcode.c"

# line 40 "rl.y"
typedef union  {
	int   integer;
	ident_t  id; 
	struct { char name[64];
	         int  rcode;
	} string;
	int   status;
} YYSTYPE;
# define ROLE 257
# define NUMBER 258
# define GID 259
# define NID 260
# define TID 261
# define IF 262
# define ELSE 263
# define ELIF 264
# define FI 265
# define THEN 266
# define DEF 267
# define DA 268
# define KILL 269
# define RESTART 270
# define START 271
# define REBOOT 272
# define WARN 273
# define AND 274
# define OR 275
# define NOT 276
# define KILLED 277
# define REBOOTING 278
# define RESTARTED 279
# define PRESENT 280
# define ISOLATED 281
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
#include <stdio.h>
# define YYERRCODE 256
yytabelem yyexca[] ={
	-1, 1,
	0, -1,
	262, 23,
	-2, 0,
-1, 10,
	264, 27,
	-2, 26,
-1, 20,
	265, 31,
	-2, 32,
-1, 69,
	265, 34,
	-2, 23,
-1, 99,
	262, 23,
	-2, 25,
-1, 100,
	262, 23,
	-2, 29,
-1, 101,
	264, 27,
	-2, 26,
	};
# define YYNPROD 60
# define YYLAST 267
yytabelem yyact[]={

    43,    73,    64,    96,    54,    27,    28,    82,     4,    68,
    38,    31,    94,    95,    93,    92,    90,    91,    88,    89,
    87,    56,    57,    58,    39,    22,    36,    34,    18,    35,
    33,     2,    20,    61,    69,    40,    32,    55,    24,    25,
    19,    62,    41,    15,    26,    14,    13,    12,    74,    75,
     7,    72,    50,    30,   101,    23,    81,    21,    71,    11,
    37,    29,    10,    17,    16,     6,     5,     3,     1,    42,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    51,     0,     0,    59,    60,    67,     0,
    63,     0,     0,     0,     0,    65,    66,     0,    70,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    83,    84,
     0,     0,     0,     0,    85,    86,    97,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    98,     0,    99,   100,   102,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    52,    53,    52,    53,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    44,    45,    46,    47,
    49,    48,    52,    53,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    76,    78,    80,    79,    77,     8,     9 };
yytabelem yypact[]={

  -225,    -2,    37, -1000, -1000,    36,    35,    33,  -230,   -21,
 -1000,  -237, -1000, -1000, -1000, -1000,    -6,   -22,    -1,  -264,
 -1000,  -253,   -55,  -227,  -231,  -228,  -232, -1000, -1000,  -255,
  -239,   -56,   -40, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
   -40,   -89,  -238,   -40,   -40, -1000, -1000, -1000, -1000, -1000,
    31,   -91,   -40,   -40,    31, -1000, -1000, -1000, -1000,   -32,
 -1000, -1000, -1000, -1000,    31, -1000, -1000, -1000, -1000,    -9,
 -1000,  -259, -1000, -1000,    31,    31,  -238,  -238,  -241,  -244,
  -247,  -263,    31, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000, -1000,    31, -1000, -1000,    -9,
    -9, -1000, -1000 };
yytabelem yypgo[]={

     0,    69,    37,    68,    67,    66,    65,    48,    64,    63,
    33,    42,    62,    32,    61,    60,    59,    58,    34,    57,
    56,    54,    53,    52,    51,    49 };
yytabelem yyr1[]={

     0,     3,     3,     3,     4,     4,     4,     4,     5,     5,
     8,     8,     9,     6,     6,    10,    10,    11,    11,    11,
    11,    11,     7,    16,    17,    12,    13,    19,    20,    21,
    13,    14,    22,    23,    14,    15,     1,     1,     1,     1,
     1,     2,     2,     2,    18,    18,    24,    24,    24,    25,
    25,    25,    25,    25,    25,    25,    25,    25,    25,    25 };
yytabelem yyr2[]={

     0,     0,     4,     5,     2,     4,     5,     4,     9,     9,
     3,     7,     7,     7,     7,     2,     2,     5,     6,     7,
     7,     5,     9,     1,     1,    21,     0,     1,     1,     1,
    24,     0,     1,     1,    11,     3,     2,     2,     2,     2,
     2,     2,     2,     2,     0,     4,     2,     4,     4,     5,
     5,     5,     5,     5,     5,     5,     5,     5,     5,     5 };
yytabelem yychk[]={

 -1000,    -3,   256,    -4,    10,    -5,    -6,    -7,   267,   268,
   -12,   -16,    10,    10,    10,    10,    -8,    -9,   258,    61,
   -13,   -19,   262,    61,    44,    61,    45,   269,   270,   -14,
   -22,   264,    91,   257,   258,   257,   258,   -15,   265,   263,
    91,   -11,    -1,    40,   276,   277,   278,   279,   281,   280,
   -23,   -11,   274,   275,    93,    -2,   259,   260,   261,   -11,
   -11,   -10,    10,    59,    93,   -11,   -11,   -10,    41,   -18,
   -10,   -17,   -24,    10,    -7,   -25,   269,   273,   270,   272,
   271,   -20,   266,   -10,   -10,    -2,    -2,   261,   259,   260,
   260,   261,   259,   261,   259,   260,   266,   -10,   -10,   -18,
   -18,   -21,   -13 };
yytabelem yydef[]={

     1,    -2,     0,     2,     4,     0,     0,     0,     0,     0,
    -2,     0,     3,     5,     6,     7,     0,     0,    10,     0,
    -2,     0,     0,     0,     0,     0,     0,    13,    14,     0,
     0,     0,     0,     8,    11,     9,    12,    22,    35,    33,
     0,     0,     0,     0,     0,    36,    37,    38,    39,    40,
     0,     0,     0,     0,     0,    17,    41,    42,    43,     0,
    21,    44,    15,    16,     0,    19,    20,    24,    18,    -2,
    28,     0,    45,    46,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    47,    48,    49,    50,    51,    52,    53,
    54,    55,    56,    57,    58,    59,     0,    44,    44,    -2,
    -2,    -2,    30 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

char * yyreds[] =
{
	"-no such reduction-",
      "rlstats : /* empty */",
      "rlstats : rlstats rlstat",
      "rlstats : error '\\n'",
      "rlstat : '\\n'",
      "rlstat : definition '\\n'",
      "rlstat : default_action '\\n'",
      "rlstat : if_then_else '\\n'",
      "definition : DEF list '=' ROLE",
      "definition : DEF interval '=' ROLE",
      "list : NUMBER",
      "list : list ',' NUMBER",
      "interval : NUMBER '-' NUMBER",
      "default_action : DA '=' KILL",
      "default_action : DA '=' RESTART",
      "sep : '\\n'",
      "sep : ';'",
      "expr : status id",
      "expr : '(' expr ')'",
      "expr : expr AND expr",
      "expr : expr OR expr",
      "expr : NOT expr",
      "if_then_else : if elif else fi",
      "if : /* empty */",
      "if : IF '[' expr ']' sep",
      "if : IF '[' expr ']' sep THEN sep actions",
      "elif : /* empty */",
      "elif : /* empty */",
      "elif : ELIF '[' expr ']' sep",
      "elif : ELIF '[' expr ']' sep THEN sep actions",
      "elif : ELIF '[' expr ']' sep THEN sep actions elif",
      "else : /* empty */",
      "else : /* empty */",
      "else : ELSE",
      "else : ELSE sep actions",
      "fi : FI",
      "status : KILLED",
      "status : REBOOTING",
      "status : RESTARTED",
      "status : ISOLATED",
      "status : PRESENT",
      "id : GID",
      "id : NID",
      "id : TID",
      "actions : /* empty */",
      "actions : actions action",
      "action : '\\n'",
      "action : if_then_else sep",
      "action : recovery_action sep",
      "recovery_action : KILL id",
      "recovery_action : WARN id",
      "recovery_action : RESTART TID",
      "recovery_action : RESTART GID",
      "recovery_action : RESTART NID",
      "recovery_action : REBOOT NID",
      "recovery_action : REBOOT TID",
      "recovery_action : REBOOT GID",
      "recovery_action : START TID",
      "recovery_action : START GID",
      "recovery_action : START NID",
};
yytoktype yytoks[] =
{
	"ROLE",	257,
	"NUMBER",	258,
	"GID",	259,
	"NID",	260,
	"TID",	261,
	"IF",	262,
	"ELSE",	263,
	"ELIF",	264,
	"FI",	265,
	"THEN",	266,
	"DEF",	267,
	"DA",	268,
	"KILL",	269,
	"RESTART",	270,
	"START",	271,
	"REBOOT",	272,
	"WARN",	273,
	"AND",	274,
	"OR",	275,
	"NOT",	276,
	"KILLED",	277,
	"REBOOTING",	278,
	"RESTARTED",	279,
	"PRESENT",	280,
	"ISOLATED",	281,
	"'\\n'",	10,
	"'='",	61,
	"','",	44,
	"'-'",	45,
	"';'",	59,
	"'('",	40,
	"')'",	41,
	"'['",	91,
	"']'",	93,
	"-unknown-",	-1	/* ends search */
};
#endif /* YYDEBUG */

/* @(#)27       1.7.1.3  src/bos/usr/ccs/bin/yacc/yaccpar, cmdlang, bos411, 9432B411a 8/10/94 14:01:53 */
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: yyparse
 * ORIGINS: 3
 */
/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#ifdef YYSPLIT
#   define YYERROR      return(-2)
#else
#   define YYERROR      goto yyerrlab
#endif
#ifdef YACC_MSG
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <nl_types.h>
nl_catd yyusercatd;
#endif
#define YYACCEPT        return(0)
#define YYABORT         return(1)
#ifndef YACC_MSG
#define YYBACKUP( newtoken, newvalue )\
{\
        if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
        {\
                yyerror( "syntax error - cannot backup" );\
                YYERROR;\
        }\
        yychar = newtoken;\
        yystate = *yyps;\
        yylval = newvalue;\
        goto yynewstate;\
}
#else
#define YYBACKUP( newtoken, newvalue )\
{\
        if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
        {\
                yyusercatd=catopen("yacc_user.cat", NL_CAT_LOCALE);\
                yyerror(catgets(yyusercatd,1,1,"syntax error - cannot backup" ));\
                YYERROR;\
        }\
        yychar = newtoken;\
        yystate = *yyps;\
        yylval = newvalue;\
        goto yynewstate;\
}
#endif
#define YYRECOVERING()  (!!yyerrflag)
#ifndef YYDEBUG
#       define YYDEBUG  1       /* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;                    /* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG          (-1000)

#ifdef YYSPLIT
#   define YYSCODE { \
                        extern int (*_yyf[])(); \
                        register int yyret; \
                        if (_yyf[yytmp]) \
                            if ((yyret=(*_yyf[yytmp])()) == -2) \
                                    goto yyerrlab; \
                                else if (yyret>=0) return(yyret); \
                   }
#endif

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];      /* value stack */
int yys[ YYMAXDEPTH ];          /* state stack */

YYSTYPE *yypv;                  /* top of value stack */
YYSTYPE *yypvt;                 /* top of value stack for $vars */
int *yyps;                      /* top of state stack */

int yystate;                    /* current state */
int yytmp;                      /* extra var (lasts between blocks) */

int yynerrs;                    /* number of errors */
int yyerrflag;                  /* error recovery flag */
int yychar;                     /* current input token number */

#ifdef __cplusplus
 #ifdef _CPP_IOSTREAMS
  #include <iostream.h>
  extern void yyerror (char *); /* error message routine -- iostream version */
 #else
  #include <stdio.h>
  extern "C" void yyerror (char *); /* error message routine -- stdio version */
 #endif /* _CPP_IOSTREAMS */
 extern "C" int yylex(void);        /* return the next token */
#endif /* __cplusplus */


/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#ifdef __cplusplus
extern "C"
#endif /* __cplusplus */
int
yyparse()
{
        /*
        ** Initialize externals - yyparse may be called more than once
        */
        yypv = &yyv[-1];
        yyps = &yys[-1];
        yystate = 0;
        yytmp = 0;
        yynerrs = 0;
        yyerrflag = 0;
        yychar = -1;
#ifdef YACC_MSG
        yyusercatd=catopen("yacc_user.cat", NL_CAT_LOCALE);
#endif
        goto yystack;
        {
                register YYSTYPE *yy_pv;        /* top of value stack */
                register int *yy_ps;            /* top of state stack */
                register int yy_state;          /* current state */
                register int  yy_n;             /* internal state number info */

                /*
                ** get globals into registers.
                ** branch to here only if YYBACKUP was called.
                */
        yynewstate:
                yy_pv = yypv;
                yy_ps = yyps;
                yy_state = yystate;
                goto yy_newstate;

                /*
                ** get globals into registers.
                ** either we just started, or we just finished a reduction
                */
        yystack:
                yy_pv = yypv;
                yy_ps = yyps;
                yy_state = yystate;

                /*
                ** top of for (;;) loop while no reductions done
                */
        yy_stack:
                /*
                ** put a state and value onto the stacks
                */
#if YYDEBUG
                /*
                ** if debugging, look up token value in list of value vs.
                ** name pairs.  0 and negative (-1) are special values.
                ** Note: linear search is used since time is not a real
                ** consideration while debugging.
                */
                if ( yydebug )
                {
                        register int yy_i;

#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                        cout << "State " << yy_state << " token ";
                        if ( yychar == 0 )
                                cout << "end-of-file" << endl;
                        else if ( yychar < 0 )
                                cout << "-none-" << endl;
#else
                        printf( "State %d, token ", yy_state );
                        if ( yychar == 0 )
                                printf( "end-of-file\n" );
                        else if ( yychar < 0 )
                                printf( "-none-\n" );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                        else
                        {
                                for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
                                        yy_i++ )
                                {
                                        if ( yytoks[yy_i].t_val == yychar )
                                                break;
                                }
#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                                cout << yytoks[yy_i].t_name << endl;
#else
                                printf( "%s\n", yytoks[yy_i].t_name );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                        }
                }
#endif /* YYDEBUG */
                if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )    /* room on stack? */
                {
#ifndef YACC_MSG
                        yyerror( "yacc stack overflow" );
#else
                        yyerror(catgets(yyusercatd,1,2,"yacc stack overflow" ));
#endif
                        YYABORT;
                }
                *yy_ps = yy_state;
                *++yy_pv = yyval;

                /*
                ** we have a new state - find out what to do
                */
        yy_newstate:
                if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
                        goto yydefault;         /* simple state */
#if YYDEBUG
                /*
                ** if debugging, need to mark whether new token grabbed
                */
                yytmp = yychar < 0;
#endif
                if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
                        yychar = 0;             /* reached EOF */
#if YYDEBUG
                if ( yydebug && yytmp )
                {
                        register int yy_i;

#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                        cout << "Received token " << endl;
                        if ( yychar == 0 )
                                cout << "end-of-file" << endl;
                        else if ( yychar < 0 )
                                cout << "-none-" << endl;
#else
                        printf( "Received token " );
                        if ( yychar == 0 )
                                printf( "end-of-file\n" );
                        else if ( yychar < 0 )
                                printf( "-none-\n" );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                        else
                        {
                                for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
                                        yy_i++ )
                                {
                                        if ( yytoks[yy_i].t_val == yychar )
                                                break;
                                }
#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                                cout << yytoks[yy_i].t_name << endl;
#else
                                printf( "%s\n", yytoks[yy_i].t_name );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                        }
                }
#endif /* YYDEBUG */
                if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
                        goto yydefault;
                if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )  /*valid shift*/
                {
                        yychar = -1;
                        yyval = yylval;
                        yy_state = yy_n;
                        if ( yyerrflag > 0 )
                                yyerrflag--;
                        goto yy_stack;
                }

        yydefault:
                if ( ( yy_n = yydef[ yy_state ] ) == -2 )
                {
#if YYDEBUG
                        yytmp = yychar < 0;
#endif
                        if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
                                yychar = 0;             /* reached EOF */
#if YYDEBUG
                        if ( yydebug && yytmp )
                        {
                                register int yy_i;

#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                                cout << "Received token " << endl;
                                if ( yychar == 0 )
                                        cout << "end-of-file" << endl;
                                else if ( yychar < 0 )
                                        cout << "-none-" << endl;
#else
                                printf( "Received token " );
                                if ( yychar == 0 )
                                        printf( "end-of-file\n" );
                                else if ( yychar < 0 )
                                        printf( "-none-\n" );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                                else
                                {
                                        for ( yy_i = 0;
                                                yytoks[yy_i].t_val >= 0;
                                                yy_i++ )
                                        {
                                                if ( yytoks[yy_i].t_val
                                                        == yychar )
                                                {
                                                        break;
                                                }
                                        }
#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                                        cout << yytoks[yy_i].t_name << endl;
#else
                                        printf( "%s\n", yytoks[yy_i].t_name );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                                }
                        }
#endif /* YYDEBUG */
                        /*
                        ** look through exception table
                        */
                        {
                                register int *yyxi = yyexca;

                                while ( ( *yyxi != -1 ) ||
                                        ( yyxi[1] != yy_state ) )
                                {
                                        yyxi += 2;
                                }
                                while ( ( *(yyxi += 2) >= 0 ) &&
                                        ( *yyxi != yychar ) )
                                        ;
                                if ( ( yy_n = yyxi[1] ) < 0 )
                                        YYACCEPT;
                        }
                }

                /*
                ** check for syntax error
                */
                if ( yy_n == 0 )        /* have an error */
                {
                        /* no worry about speed here! */
                        switch ( yyerrflag )
                        {
                        case 0:         /* new error */
#ifndef YACC_MSG
                                yyerror( "syntax error" );
#else
                                yyerror(catgets(yyusercatd,1,3,"syntax error" ));
#endif
                                goto skip_init;
                        yyerrlab:
                                /*
                                ** get globals into registers.
                                ** we have a user generated syntax type error
                                */
                                yy_pv = yypv;
                                yy_ps = yyps;
                                yy_state = yystate;
                                yynerrs++;
                        skip_init:
                        case 1:
                        case 2:         /* incompletely recovered error */
                                        /* try again... */
                                yyerrflag = 3;
                                /*
                                ** find state where "error" is a legal
                                ** shift action
                                */
                                while ( yy_ps >= yys )
                                {
                                        yy_n = yypact[ *yy_ps ] + YYERRCODE;
                                        if ( yy_n >= 0 && yy_n < YYLAST &&
                                                yychk[yyact[yy_n]] == YYERRCODE)                                        {
                                                /*
                                                ** simulate shift of "error"
                                                */
                                                yy_state = yyact[ yy_n ];
                                                goto yy_stack;
                                        }
                                        /*
                                        ** current state has no shift on
                                        ** "error", pop stack
                                        */
#if YYDEBUG
                                        if ( yydebug )
#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                                            cout << "Error recovery pops state "
                                                 << (*yy_ps)
                                                 << ", uncovers state "
                                                 << yy_ps[-1] << endl;
#else
#       define _POP_ "Error recovery pops state %d, uncovers state %d\n"
                                                printf( _POP_, *yy_ps,
                                                        yy_ps[-1] );
#       undef _POP_
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
#endif
                                        yy_ps--;
                                        yy_pv--;
                                }
                                /*
                                ** there is no state on stack with "error" as
                                ** a valid shift.  give up.
                                */
                                YYABORT;
                        case 3:         /* no shift yet; eat a token */
#if YYDEBUG
                                /*
                                ** if debugging, look up token in list of
                                ** pairs.  0 and negative shouldn't occur,
                                ** but since timing doesn't matter when
                                ** debugging, it doesn't hurt to leave the
                                ** tests here.
                                */
                                if ( yydebug )
                                {
                                        register int yy_i;

#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                                        cout << "Error recovery discards ";
                                        if ( yychar == 0 )
                                            cout << "token end-of-file" << endl;
                                        else if ( yychar < 0 )
                                            cout << "token -none-" << endl;
#else
                                        printf( "Error recovery discards " );
                                        if ( yychar == 0 )
                                                printf( "token end-of-file\n" );
                                        else if ( yychar < 0 )
                                                printf( "token -none-\n" );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                                        else
                                        {
                                                for ( yy_i = 0;
                                                        yytoks[yy_i].t_val >= 0;
                                                        yy_i++ )
                                                {
                                                        if ( yytoks[yy_i].t_val
                                                                == yychar )
                                                        {
                                                                break;
                                                        }
                                                }
#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                                                cout << "token " <<
                                                    yytoks[yy_i].t_name <<
                                                    endl;
#else
                                                printf( "token %s\n",
                                                        yytoks[yy_i].t_name );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
                                        }
                                }
#endif /* YYDEBUG */
                                if ( yychar == 0 )      /* reached EOF. quit */
                                        YYABORT;
                                yychar = -1;
                                goto yy_newstate;
                        }
                }/* end if ( yy_n == 0 ) */
                /*
                ** reduction by production yy_n
                ** put stack tops, etc. so things right after switch
                */
#if YYDEBUG
                /*
                ** if debugging, print the string that is the user's
                ** specification of the reduction which is just about
                ** to be done.
                */
                if ( yydebug )
#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)
                        cout << "Reduce by (" << yy_n << ") \"" <<
                            yyreds[ yy_n ] << "\"\n";
#else
                        printf( "Reduce by (%d) \"%s\"\n",
                                yy_n, yyreds[ yy_n ] );
#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */
#endif
                yytmp = yy_n;                   /* value to switch over */
                yypvt = yy_pv;                  /* $vars top of value stack */
                /*
                ** Look in goto table for next state
                ** Sorry about using yy_state here as temporary
                ** register variable, but why not, if it works...
                ** If yyr2[ yy_n ] doesn't have the low order bit
                ** set, then there is no action to be done for
                ** this reduction.  So, no saving & unsaving of
                ** registers done.  The only difference between the
                ** code just after the if and the body of the if is
                ** the goto yy_stack in the body.  This way the test
                ** can be made before the choice of what to do is needed.
                */
                {
                        /* length of production doubled with extra bit */
                        register int yy_len = yyr2[ yy_n ];

                        if ( !( yy_len & 01 ) )
                        {
                                yy_len >>= 1;
                                yyval = ( yy_pv -= yy_len )[1]; /* $$ = $1 */
                                yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
                                        *( yy_ps -= yy_len ) + 1;
                                if ( yy_state >= YYLAST ||
                                        yychk[ yy_state =
                                        yyact[ yy_state ] ] != -yy_n )
                                {
                                        yy_state = yyact[ yypgo[ yy_n ] ];
                                }
                                goto yy_stack;
                        }
                        yy_len >>= 1;
                        yyval = ( yy_pv -= yy_len )[1]; /* $$ = $1 */
                        yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
                                *( yy_ps -= yy_len ) + 1;
                        if ( yy_state >= YYLAST ||
                                yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
                        {
                                yy_state = yyact[ yypgo[ yy_n ] ];
                        }
                }
                                        /* save until reenter driver code */
                yystate = yy_state;
                yyps = yy_ps;
                yypv = yy_pv;
        }
        /*
        ** code supplied by user is placed in this switch
        */

                switch(yytmp){

case 3:
# line 68 "rl.y"
{
		fprintf(stderr, "\tLine %d: syntax error.\n", lines);
		} /*NOTREACHED*/ break;
case 6:
# line 76 "rl.y"
{
		printf("\tDefault action is %s\n",
			(default_action==KILL)? "killing":"restarting");
		} /*NOTREACHED*/ break;
case 8:
# line 84 "rl.y"
{
			for (i=0; i<card_list; i++)
			     {
			     rcode_set_role((char) list[i], yypvt[-0].string.rcode);
			     }
			} /*NOTREACHED*/ break;
case 9:
# line 92 "rl.y"
{ int i;
			for (i=list[0]; i<=list[1]; i++)
			 {
			     rcode_set_role((char) i, yypvt[-0].string.rcode);
			 }
			} /*NOTREACHED*/ break;
case 10:
# line 101 "rl.y"
{
		list[card_list++] = yypvt[-0].integer;
		} /*NOTREACHED*/ break;
case 11:
# line 106 "rl.y"
{
		list[card_list++] = yypvt[-0].integer;
		} /*NOTREACHED*/ break;
case 12:
# line 112 "rl.y"
{
			list[card_list++] = yypvt[-2].integer;
			list[card_list++] = yypvt[-0].integer;
			} /*NOTREACHED*/ break;
case 13:
# line 119 "rl.y"
{
			default_action = KILL;
			rcode_set_defaction(R_DA_IS_KILL);
			} /*NOTREACHED*/ break;
case 14:
# line 125 "rl.y"
{
			default_action = RESTART;
			rcode_set_defaction(R_DA_IS_RESTART);
			} /*NOTREACHED*/ break;
case 17:
# line 136 "rl.y"
{
			if (debug)
				printf("\t\t +expr(pc==%d)\n", pc);

			rcode_status (yypvt[-1].status, yypvt[-0].id);

			if (debug)
				printf("\t\t -expr(pc==%d)\n", pc);
		} /*NOTREACHED*/ break;
case 19:
# line 147 "rl.y"
{
			if (debug)
				printf("\t\t +exprAND(pc==%d)\n", pc);

			rcode_boolean (R_AND);

			if (debug)
				printf("\t\t -exprAND(pc==%d)\n", pc);
		} /*NOTREACHED*/ break;
case 20:
# line 157 "rl.y"
{
			if (debug)
				printf("\t\t +exprOR(pc==%d)\n", pc);

			rcode_boolean (R_OR);

			if (debug)
				printf("\t\t -exprOR(pc==%d)\n", pc);
		} /*NOTREACHED*/ break;
case 21:
# line 167 "rl.y"
{
			if (debug)
				printf("\t\t +exprNOT(pc==%d)\n", pc);

			rcode_boolean (R_NOT);

			if (debug)
				printf("\t\t -exprNOT(pc==%d)\n", pc);
		} /*NOTREACHED*/ break;
case 22:
# line 179 "rl.y"
{
			printf("\tif-then-else: ok\n");
			} /*NOTREACHED*/ break;
case 23:
# line 184 "rl.y"
{
				if (debug)
					printf("\t if starts...(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 24:
# line 189 "rl.y"
{
				if (debug)
					printf("\t\t +if expr(pc==%d)\n", pc);

				rcode_if();

				if (debug)
					printf("\t\t -if expr(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 25:
# line 199 "rl.y"
{
				if (debug)
					printf("\t\t then actions(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 27:
# line 206 "rl.y"
{
				if (debug)
					printf("\t\t elif starts...(pc==%d)\n", pc);
				goto_pc = rcode_goto(0);
			} /*NOTREACHED*/ break;
case 28:
# line 213 "rl.y"
{
				if (debug)
					printf("\t\t +elif expr(pc==%d)\n", pc);

				rcode_elif(goto_pc);

				if (debug)
					printf("\t\t -elif expr(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 29:
# line 223 "rl.y"
{
				if (debug)
					printf("\t\t then actions(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 32:
# line 231 "rl.y"
{
				if (debug)
					printf("\t\t else starts...(pc==%d)\n", pc);
				goto_pc = rcode_goto(0);
			} /*NOTREACHED*/ break;
case 33:
# line 237 "rl.y"
{
				if (debug)
					printf("\t\t +else starts...(pc==%d)\n", pc);

				rcode_else(goto_pc);

				if (debug)
					printf("\t\t -else starts...(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 34:
# line 247 "rl.y"
{
				if (debug)
					printf("\t\t actions(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 35:
# line 254 "rl.y"
{
				if (debug)
					printf("\t\t +fi(pc==%d)\n", pc);

				rcode_fi();

				if (debug)
					printf("\t\t -fi(pc==%d)\n", pc);
			} /*NOTREACHED*/ break;
case 49:
# line 283 "rl.y"
{
			if (debug)
				printf("adding recovery action: kill %d %d\n", yypvt[-0].id.role, yypvt[-0].id.id);
			rcode_raction(R_KILL, yypvt[-0].id.role, yypvt[-0].id.id);
		} /*NOTREACHED*/ break;
case 50:
# line 289 "rl.y"
{
			if (debug)
				printf("adding recovery action: warn %d %d\n", yypvt[-0].id.role, yypvt[-0].id.id);
			rcode_raction(R_WARN, yypvt[-0].id.role, yypvt[-0].id.id);
		} /*NOTREACHED*/ break;
case 51:
# line 295 "rl.y"
{
			if (debug)
				printf("adding recovery action: restart %d %d\n", yypvt[-0].id.role, yypvt[-0].id.id);
			rcode_raction(R_RESTART, yypvt[-0].id.role, yypvt[-0].id.id);
		} /*NOTREACHED*/ break;
case 52:
# line 301 "rl.y"
{
			if (debug)
				printf("adding recovery action: restart %d %d\n", yypvt[-0].id.role, yypvt[-0].id.id);
			rcode_raction(R_RESTART, yypvt[-0].id.role, yypvt[-0].id.id);
		} /*NOTREACHED*/ break;
case 53:
# line 307 "rl.y"
{
			fprintf(stderr, "\tLine %d: semantical error.\n", lines);
			yyerror("Can't restart nodes");
		} /*NOTREACHED*/ break;
case 54:
# line 312 "rl.y"
{
			if (debug)
				printf("adding recovery action: reboot %d %d\n", yypvt[-0].id.role, yypvt[-0].id.id);
			rcode_raction(R_REBOOT, yypvt[-0].id.role, yypvt[-0].id.id);
		} /*NOTREACHED*/ break;
case 55:
# line 318 "rl.y"
{
			fprintf(stderr, "\tLine %d: semantical error.\n", lines);
			yyerror("Can't reboot threads");
		} /*NOTREACHED*/ break;
case 56:
# line 323 "rl.y"
{
			fprintf(stderr, "\tLine %d: semantical error.\n", lines);
			yyerror("Can't reboot groups");
		} /*NOTREACHED*/ break;
case 57:
# line 328 "rl.y"
{
			if (debug)
				printf("adding recovery action: start %d %d\n", yypvt[-0].id.role, yypvt[-0].id.id);
			rcode_raction(R_KILL, yypvt[-0].id.role, yypvt[-0].id.id);
		} /*NOTREACHED*/ break;
case 58:
# line 334 "rl.y"
{
			fprintf(stderr, "\tLine %d: semantical error.\n", lines);
			yyerror("Can't start groups");
		} /*NOTREACHED*/ break;
case 59:
# line 339 "rl.y"
{
			fprintf(stderr, "\tLine %d: semantical error.\n", lines);
			yyerror("Can't start nodes");
		} /*NOTREACHED*/ break;
}


        goto yystack;           /* reset registers in driver code */
}

# line 345 "rl.y"

#include "lex.yy.c"

main(int argc, char *argv[])
{ 
    char  filename[255];
    char *path; 
    int   fd;
    clock_t t1, t2;
    void options(void);
    extern FILE *f;  /* output file pointer */

    ifname = EFTOS_SRC;
    ofname = EFTOS_OUT;

    fprintf(stderr, "RL translator, %s, (c) K.U.Leuven 1997.\n", VERSION);
    for (i=1; i<argc; i++)
         if (argv[i][0] == '-')
             switch (argv[i][1])
             {
	     case 'i':  ifname = argv[++i];
			break;
	     case 'o':  ofname = argv[++i];
			break;
	     case 'v':  verbose= 1;
			break;
	     case 'd':  debug= 1;
			break;
	     default :  fprintf(stderr, "Invalid option.\n");
	                options();
			exit(1);
             }
         else
	 {
	     fprintf(stderr, "Invalid option --- aborting.\n");
	     options();
	     exit(1);
	 }

    fd = open(ifname,O_RDONLY);
    if(fd<0)
    {
	 fprintf(stderr, "Couldn't open file %s", ifname);
         if( (path = getenv(EFTOS_ENV)) )
	 {
	      sprintf(filename, "%s/%s", path, ifname);
	      ifname = filename;
	      if ( (fd = open(filename, O_RDONLY)) < 0 )     
	      {
	         fprintf(stderr, " nor file %s --- exiting.\n", filename);
		 exit(1);
	      }
	      fprintf(stderr, ". Switching...\n");
	 }
	 else
	 {
	      fprintf(stderr, " --- exiting.\n");
	      exit(1);
	 }
    }
    dup2(fd,0);

    f = fopen(ofname, "wb");
    if (f==NULL)
    {
        fprintf(stderr, "Couldn't open %s for writing --- exiting.\n", ofname);
	exit(1);
    }

    fprintf(stderr, "Parsing file %s...\n", ifname);

    t1 = clock();
    yyparse();
    rcode_stop();
    rflush();
    t2 = clock();

    fprintf(stderr, "...done (%d lines in %lf CPU secs", lines,
           (double)(t2-t1)/CLOCKS_PER_SEC);
    if (t2-t1 != 0)
        fprintf(stderr, ", or %.3lf lines per CPU sec.)\n",
                (double) lines * CLOCKS_PER_SEC / (t2-t1));
    else
        fprintf(stderr, ".)\n");
}
void options()
{
fprintf(stderr, "Valid options are\n");
fprintf(stderr, "  -i <filename>    : set input to <filename>\n");
fprintf(stderr, "  -o <filename>    : set output to <filename>\n");
fprintf(stderr, "  -v               : set verbose mode\n");
}
int ident2int(ident_t who)
{
return who.role + who.id;
}
ident_t int2ident(int n)
{
	ident_t i;

	if (! (ID_GROUP < ID_NODE && ID_NODE < ID_THREAD) )
	{
		fprintf(stderr, "Inconsistency on [GNT]_OFFSET's\n");
	}
	if (n >= ID_GROUP && n<ID_NODE)
	{
		i.role = ID_GROUP;
		i.id   = n - ID_GROUP;
		return i;
	}
	if (n >= ID_NODE && n < ID_THREAD)
	{
		i.role = ID_NODE;
		i.id   = n - ID_NODE;
		return i;
	}
	i.role = ID_THREAD;
	i.id   = n - ID_THREAD;
	return i;
}
