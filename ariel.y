%{
/***************************************************************************
**
**     File: ariel.y
**
**     Description: parser of the Ariel language
**
**     Language: yacc
**
**     Version 3.0e   Eidon@tutanota.com
**      - Added functions for the management of `@' variables.
**      - ``reboot'' has been removed. A node can be restarted.
**      - added predicate STAGE.
**      - added option `-s'
***********************************************************************/

#define VERSION "v3.0e"
#define EFTOS_ENV "TIRAN_HOME"
#define EFTOS_SRC ".ariel"
#define HFNAME    "../trl.h"
#define TONAME    "../timeouts.h"
#define IDNAME    "../identifiers.h"
#define ALPHANAME "../alphacount.h"
#define STRNAME   "../arielstrings.h"
#define LOGICAL_FNAME "LogicalTable.csv"
#define TASK_FNAME    "TaskTable.csv"

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

int sflush(char*[], int);
int init_taskram(void);
int init_logicalram(void);
int kbwait (void);
int yyerror (char*);
int yylex (void);

/* debugging */
int phase;
#define DeBug /* {fprintf(stderr, ".   debug: phase %d\n", ++phase); fflush(stderr);} */

/* input and output file names */
char *ifname, *ofname;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include "assoc.h"
#include "ObjAlloc.h"
#include "rl.h"
#include "rcode.h"
/* Added on Jan 19 2000 by VdF */

int i, j, d, goto_pc;
extern int list[], card_list, lines;
int default_action;
int tasklist_nr;
int tasklist[MAX_TASKS_IN_LOGICAL];
char verbose, debug;
char static_version;
char notime; /* don't print time of day via external command `date' */
int errors;

int booleans;  /* cardinality of the expression list */
int boole_pc[BOOLE_ATOMS];  /* at most BOOLE_ATOMS can be found in one expr */

int booleanstack[RCODE_MAX_NEST]; /* a stack to save booleans */
int nestings;
int rec; /* a flag which records whether an IF has been encountered or not */

int nprocs = 3; /* number of nodes to be used */

int mia_send_timeout = 200000 ; /* timeout `Manager Is Alive' */
int taia_send_timeout = 200000 ; /* timeout `This Agent Is Alive' */
int imalive_clear_timeout = 120000 ; /* I'm Alive timeout */

int mia_recv_timeout = 250000 ; /* timeout `Manager Is Alive' -- backup side */
int taia_recv_timeout = 250000 ; /* `This Agent Is Alive' -- backup side */
int imalive_set_timeout = 300000 ; /* I'm Alive timeout -- IAT side */
int teif_timeout = 300000; /* this entity is faulty */
int mid_timeout = 500000; /* manager is down */

int request_db_timeout = 200000;
int reply_db_timeout = 500000;

int numtask[MAX_NODES];

alphacount_t alphas[MAX_TASKS];

static int var[26];  /* 26 pre-defined and pre-initialised variables */

FILE *f;  /* output file pointer */
FILE *h;  /* header file pointer */
ASSOC *a; /* associations found in header files */

typedef struct { 
	  unsigned
	  int role;   /* a bit pattern with one or more of these
			 bit positions turned on:
			   ID_GROUP, ID_THREAD, ID_NODE, ID_ENTITY,
			   ID_NORMAL, ID_STAR, ID_DOLLAR */
	  int id;     /* an integer identifying the entity */
	}             ident_t;


/************************** watchdogs *****************************/
typedef struct {
		int watching, watched, rate, unit, action,
                    target, running;
	}	watchdog_t;

#define    MAX_WDOGS	32
watchdog_t watchdog[MAX_WDOGS];
int	   w_sp = 0;
#define    INCWATCH	++w_sp;
#define    WATCHTOP	watchdog[w_sp]
#define    BADWATCH	(w_sp > MAX_WDOGS)
#define    MAX_WD_FNAME 80
#define    MAX_WD_NAME  40
int watchdog_flush(watchdog_t*, int);
/*********************** end watchdogs ****************************/


/************************* nversion ******************************/
typedef struct { int version, task, timeout, unit; } version_t;
#define    MAX_VERS	7
typedef struct {
		int running, task;
		version_t va[MAX_VERS]; int va_num;
		int voting; char *metric;
		int success, error;
		int versmin, versmax;
	}	nversion_t;
#define    MAX_NVERS	16
nversion_t nversion[MAX_NVERS];
int	   nv_sp = 0;
#define    INCNVERS	++nv_sp;
#define    NVERSTOP	nversion[nv_sp]
#define    BADNVERS	(nv_sp > MAX_NVERS)
#define    MAX_NV_FNAME 80
#define    MAX_NV_NAME  40
int nversion_flush(nversion_t*, int);
/*********************** end nversion ****************************/

#define GENERATE_RACTION(act,who) \
	if (who.role & ID_DOLLAR) \
	{                         \
	   rcode_raction(act, who.role, - boole_pc[ who.id -1 ]); \
	   d=dollarsign_check(who.role, boole_pc[ who.id -1]);     \
	   if (d == 1)    \
	   {              \
		errors++; \
		fprintf(stderr, "\tLine %d: semantical error: ", lines); \
		yyerror("Improper use of ``$'' sign.");                  \
	   }              \
	   if (who.id > booleans) \
	   {                      \
		errors++;         \
		fprintf(stderr, "\tLine %d: semantical error: ", lines); \
		yyerror("Too large an index in $var.");                  \
	   }                      \
	}                 \
	else              \
	if (who.role & ID_DOLLARS) \
	{                          \
	   rcode_raction(act, who.role, - boole_pc[ booleans-1 ]); \
	   d=dollarsign_check(who.role, boole_pc[ booleans -1]);   \
	   if (d == 1)     \
	   {               \
		errors++;  \
		fprintf(stderr, "\tLine %d: semantical error: ", lines); \
		yyerror("Improper use of ``$'' sign.");                  \
	   }               \
	}                  \
	else               \
	rcode_raction(act, who.role, who.id)

#define MANAGE_AT_OR_TILDE(act,n) \
{                             \
	int opcode, ro, id;   \
	if (n == -1)          \
	{                     \
		rcode_raction(act, -1, booleans -1);                              \
		query_rcode_array( boole_pc[ booleans -1 ], &opcode, &ro, &id);   \
	}                     \
	else                  \
	{                     \
		if (n > booleans)   \
		{                   \
			errors++;   \
			fprintf(stderr, "\tLine %d: semantical error: ", lines);  \
			yyerror("Too large an index in @- or ~-var.");                   \
		}                   \
		rcode_raction(act, -1, n -1);                                     \
		query_rcode_array( boole_pc[ n -1 ], &opcode, &ro, &id);          \
	}                           \
				    \
	if ( ! isclause(opcode) )   \
	{                           \
		fprintf(stderr, "\tLine %d: semantical error: ", lines);          \
		yyerror("Can only use `@' or `~' with clauses (-r, -i, -s, -f, -p, -t)");\
		errors++;                                                         \
	}                     \
}

char *today(void);

#include "rcode.c"

int read_associations(ASSOC*, char*);

int pushstring(char*);

%}

%union {
	float real;
	int   integer;
	ident_t  id; 
	struct { char name[64];
		 int  rcode;
	} string;
	char  quoted_string[64];
	int   status;
	int   which;
}

%token <string> ROLE
%token <integer> NUMBER VAR
%token <real> REAL 
%token <id> GID NID TID

%token IF ELSE ELIF FI THEN
%token DEF DA PHASE NPROCS
%token KILL RESTART START 
%token WARN ON ACTION ERROR ANY
%token CALL
%token MIA_TIMEOUT        MIA_TIMEOUT_B
%token TAIA_TIMEOUT       TAIA_TIMEOUT_B
%token ALIVE_TIMEOUT      ALIVE_TIMEOUT_B
%token REQUEST_DB_TIMEOUT REPLY_DB_TIMEOUT
%token TEIF_TIMEOUT       MID_TIMEOUT
%token NUMTASKS PAUSE SEND
%token INJECT BFAULT MFAULT NODE COMPONENT AFTER TICKS
%token KEYW_TASK KEYW_TASKID KEYW_NODE KEYW_IS KEYW_ALIAS KEYW_MBOX
%token KEYW_LOGICAL KEYW_ENDLOGICAL
%token WATCHDOG WATCHES HEARTBEATS EVERY
%token REBOOT KEYW_ENDWATCHDOG
%token NVERSION KEYW_VERSION TIMEOUT VOTING
%token MAJORITY ALGORITHM
%token METRIC SUCCESS KEYW_ENDNVERSION
%token THRESHOLD ALPHACOUNT FACTOR KEYW_ENDALPHA
%token WRITE LOG CLOCK CLEAR


%token <integer> LET VAL
%token <integer> EQ NEQ GT GE LT LE
%token <integer> MILLISEC MICROSEC
%token ERRN ERRT ERR ACT
%token REMOVE FROM ERRORLIST
%token ENABLE
%token <which> AT TILDE
%token INCLUDE 
%token <quoted_string> STRING OSTRING


%left AND OR
%left NOT

%token <status> KILLED RESTARTED PRESENT
%token <status> ISOLATED FAULTY DEADLOCKED
%token <status> REINTEGRATED FIRED

%type <status> status
%type <id> id
%type <integer> compare
%type <integer> seconds
%type <which> those
%type <integer> fault
%type <integer> expression linexp

/* precedences/associativity */
%left '+' '-'
%left '*' '/'
%left UMINUS 

%%
rlstats:
	| rlstats rlstat
	| error '\n'
		{
		fprintf(stderr, "\tLine %d: syntax error.\n", lines);
		errors++;
		/* yyerrok; */
		}
	;

rlstat:   '\n'
	| definition '\n'
		{
			if (rec)
			{
			fprintf(stderr, "\tLine %d: semantical error: ", lines);
			yyerror("Can't define roles in strategy section");
			errors++;
			}
		}
	| default_action '\n'
		{
		printf("\tDefault action is %s\n",
			(default_action==KILL)? "killing":"restarting");
		}
	| number_of_nodes '\n'
	| section '\n'
	| include '\n'
	| timeouts '\n'
	| definitions '\n'
	| identifiers '\n'
	| alphacounts '\n'
	| aliases '\n'
	| logicals '\n'
	| watchdog '\n'
	| nversiontask '\n'
	| injection '\n'
	;

aliases:        KEYW_TASK '[' NUMBER ',' NUMBER ']' KEYW_IS KEYW_MBOX '[' NUMBER ',' NUMBER ']' ',' KEYW_ALIAS '[' NUMBER ',' NUMBER ']'
		{
			task_t t;
			int storetaskII(task_t *);
			int i, tl, tu, ml, mu, al, au;

			tl = $3, tu = $5,
			ml = $10, mu = $12;
			al = $17, au = $19;

			for (i = tl; i <= tu; i++)
			{
				t.identifier = i;
				t.alias = al++;
				t.mbox = ml++;
				storetaskII(&t);
			}
			if (ml > mu + 1 || al > au + 1)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("MBOX/Aliases out of range.");
				errors++;
			}
		}
	|       KEYW_TASK NUMBER  KEYW_IS KEYW_MBOX NUMBER ',' KEYW_ALIAS NUMBER
		{
			task_t t;
			int storetaskII(task_t *);
			t.identifier = $2;
			t.mbox = $5;
			t.alias = $8;
			storetaskII(&t);
		}
	;

tasklist:       { tasklist_nr = 0; } KEYW_TASK NUMBER tasklist
		{
			tasklist[tasklist_nr++] = $3;
		}
	|       ',' KEYW_TASK NUMBER tasklist
		{
			tasklist[tasklist_nr++] = $3;
		}
	|       ',' NUMBER tasklist
		{
			tasklist[tasklist_nr++] = $2;
		}
	|
	;
	
		
logicals:       KEYW_LOGICAL NUMBER '=' STRING KEYW_IS tasklist KEYW_ENDLOGICAL
		{
			int i;
			int storelogical(int, int[], int, char*);
/*                      struct logicalid_t *pl1, *pl2, *pl3; */

			storelogical($2, tasklist, tasklist_nr, $4);

/*
			for (i=0; i<tasklist_nr; i++)
			{
				pl1 = malloc(sizeof(struct logicalid_t));       
				if (pl1 == NULL)
					yyerror("malloc");
				
				for (pl3 = pl2 = taskram[tasklist[i]].p_logicalid;
				     pl2 != NULL;
				     pl2 = pl2->plid_next;
				    )
					pl3 = pl2;

				pl1->plid_next = NULL;
				pl1->identifier = $2;

				if (pl3)
					pl3->plid_next = p1;
				else
					taskram[tasklist[i]].p_logicalid = p1;
			}
 */
		}
	;

nversiontask:	nversion_start nversion_args nversion_end
	;

nversion_start:	NVERSION KEYW_TASK NUMBER '\n'
		{
			NVERSTOP.running = 1; /* we're within a nversion task */
			NVERSTOP.task = $3;
			NVERSTOP.va_num = 0;
			NVERSTOP.versmin = 99999;
			NVERSTOP.versmax = -1;
		}
	;

nversion_args:	
	|	nv_version    nversion_args
	|	nv_voting     nversion_args
	|	nv_metric     nversion_args
	|	nv_on_error   nversion_args
	|	nv_on_success nversion_args
	;

nv_version:	KEYW_VERSION NUMBER KEYW_IS KEYW_TASK NUMBER '\n'
		{
			int this = NVERSTOP.va_num;
			NVERSTOP.va_num++;

			if ($2 > NVERSTOP.versmax)
				NVERSTOP.versmax = $2;
			if ($2 < NVERSTOP.versmin)
				NVERSTOP.versmin = $2;
			NVERSTOP.va[this].version = $2;
			NVERSTOP.va[this].task = $5;
			NVERSTOP.va[this].timeout = -1;
		}
	|	KEYW_VERSION NUMBER KEYW_IS KEYW_TASK NUMBER TIMEOUT NUMBER seconds '\n'
		{
			int this = NVERSTOP.va_num;
			NVERSTOP.va_num++;

			if ($2 > NVERSTOP.versmax)
				NVERSTOP.versmax = $2;
			if ($2 < NVERSTOP.versmin)
				NVERSTOP.versmin = $2;
			NVERSTOP.va[this].version = $2;
			NVERSTOP.va[this].task = $5;
			NVERSTOP.va[this].timeout = $7;
			NVERSTOP.va[this].unit = 
				($8 == MILLISEC)? 0:1;
		}
	;


nv_voting:	VOTING ALGORITHM KEYW_IS MAJORITY '\n'
		{
			NVERSTOP.voting = 1; /* majority voting */
		}
	;

nv_metric:	METRIC STRING '\n'
		{
			NVERSTOP.metric = strdup($2);
		}
	;

nv_on_error:	ON ERROR KEYW_TASK NUMBER '\n'
		{
			if (NVERSTOP.running)
			{
				NVERSTOP.error = $4;
			}
			else
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("bad use of ON ERROR.");
				errors++;
			}
		}
	;

nv_on_success:	ON SUCCESS KEYW_TASK NUMBER '\n'
		{
			if (NVERSTOP.running)
			{
				NVERSTOP.success = $4;
			}
			else
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("bad use of ON ERROR.");
				errors++;
			}
		}
	;

nversion_end:	KEYW_ENDNVERSION '\n'
		{
			NVERSTOP.running = 0;
			INCNVERS;
			if (BADNVERS)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Too many nversion tasks have been defined.");
				errors++;
			}
		}
	;


watchdog:	watchdog_start watchdog_args watchdog_end
	;

watchdog_args:	
	|	on_error watchdog_args
	|	heartbeats watchdog_args
	|	w_alphacount watchdog_args
	;

watchdog_start:      WATCHDOG KEYW_TASK NUMBER WATCHES KEYW_TASK NUMBER '\n'
		{
			WATCHTOP.running = 1; /* we're within a watchdog */
			WATCHTOP.watching = $3;
			WATCHTOP.watched = $6;
		}
	;

w_alphacount:    ALPHACOUNT KEYW_IS THRESHOLD '=' REAL ',' FACTOR '=' REAL KEYW_ENDALPHA '\n'
		{
			alphas[WATCHTOP.watching].used = 1;
			alphas[WATCHTOP.watching].threshold = $5;
			alphas[WATCHTOP.watching].k = $9;

		}
	;

seconds	:	MILLISEC | MICROSEC
	;

heartbeats:	HEARTBEATS EVERY NUMBER seconds '\n'
		{
			if (WATCHTOP.running)
			{
				WATCHTOP.rate = $3;
				WATCHTOP.unit = ($4 == MILLISEC)? 0:1;
			}
			else
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("bad use of ON ERROR.");
				errors++;
			}
		}
	;

on_error:	ON ERROR WARN KEYW_TASK NUMBER '\n'
		{
			if (WATCHTOP.running)
			{
				WATCHTOP.action = 0;
				WATCHTOP.target = $5;
			}
			else
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("bad use of ON ERROR.");
				errors++;
			}
		}
	|	ON ERROR REBOOT '\n'
		{
			if (WATCHTOP.running)
				WATCHTOP.action = 1;
			else
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("bad use of ON ERROR.");
				errors++;
			}
		}
	|	ON ERROR RESTART '\n'
		{
			if (WATCHTOP.running)
				WATCHTOP.action = 2;
			else
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("bad use of ON ERROR.");
				errors++;
			}
		}
	;


watchdog_end:	KEYW_ENDWATCHDOG '\n'
		{
			WATCHTOP.running = 0;
			INCWATCH;
			if (BADWATCH)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Too many watchdogs have been defined.");
				errors++;
			}
		}
	;

alphacounts:    ALPHACOUNT NUMBER KEYW_IS THRESHOLD '=' REAL ',' FACTOR '=' REAL KEYW_ENDALPHA
		{
			alphas[$2].used = 1;
			alphas[$2].threshold = $6;
			alphas[$2].k = $10;

		}
	;

identifiers:    KEYW_TASK NUMBER '=' STRING KEYW_IS KEYW_NODE NUMBER ',' KEYW_TASKID NUMBER
		{
			task_t t;
			int storetask(task_t *);
			t.identifier = $2;
			t.name = $4;
			t.node = $7;
			t.idf = $10;

			storetask(&t);
		}
	|
		KEYW_TASK '[' NUMBER ',' NUMBER ']' '=' STRING KEYW_IS KEYW_NODE NUMBER ',' KEYW_TASKID '[' NUMBER ',' NUMBER ']'
		{
			task_t t;
			int storetask(task_t *);
			int i, l1, u1, l2, u2;
			char name[TASK_MAXNAME];

			l1 = $3, u1 = $5,
			l2 = $15, u2 = $17;

			t.node = $11;
			t.name = name;
			for (i = l1; i<=u1; i++)
			{
				t.identifier = i;
				t.idf = l2++;
				sprintf(name, "%s.%d", $8, i);
				storetask(&t);
			}
			if (l2 > u2 + 1)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("IDF's out of range.");
				errors++;
			}
		}
	;

definitions:    NUMTASKS NUMBER '=' NUMBER
		{
			numtask[$2] = $4;
		}
	;

timeouts:       MIA_TIMEOUT '=' NUMBER
		{
			mia_send_timeout = $3;
		}
	|       TAIA_TIMEOUT '=' NUMBER
		{
			taia_send_timeout = $3;
		}
	|       ALIVE_TIMEOUT '=' NUMBER
		{
			imalive_clear_timeout = $3;
		}
	|       MIA_TIMEOUT_B '=' NUMBER
		{
			mia_recv_timeout = $3;
		}
	|       TAIA_TIMEOUT_B '=' NUMBER
		{
			taia_recv_timeout = $3;
		}
	|       ALIVE_TIMEOUT_B '=' NUMBER
		{
			imalive_set_timeout = $3;
		}
	|       TEIF_TIMEOUT '=' NUMBER
		{
			teif_timeout = $3;
		}
	|       REQUEST_DB_TIMEOUT '=' NUMBER
		{
			request_db_timeout = $3;
		}
	|       REPLY_DB_TIMEOUT '=' NUMBER
		{
			reply_db_timeout = $3;
		}
	|       MID_TIMEOUT '=' NUMBER
		{
			mid_timeout = $3;
		}
	;

fault:  BFAULT
		{
			return BFAULT;
		}
	| MFAULT
		{
			return BFAULT;
		}
	;

what:   NODE | COMPONENT
	;

ticks:
	| TICKS
	;

injection:      INJECT fault ON what NUMBER AFTER NUMBER ticks
		{
			if ($2 == BFAULT)
			{
			}
			else /* MFAULT */
			{
			}
		}
	;

definition:     DEF list '=' ROLE
			{
				for (i=0; i<card_list; i++)
				     {
				     rcode_set_role((char) list[i], $4.rcode);
				     }
			}
	|
		DEF interval '=' ROLE
			{ int i;
				if (rec)
				{
					fprintf(stderr, "\tLine %d: semantical error: ", lines);
					yyerror("Can't define roles in strategy section");
					errors++;
				}
				for (i=list[0]; i<=list[1]; i++)
				 {
				     rcode_set_role((char) i, $4.rcode);
				 }
			}
	;

list:   NUMBER 
		{
		list[card_list++] = $1;
		}
	|
	list ',' NUMBER
		{
		list[card_list++] = $3;
		}
	;

interval:       NUMBER '-' NUMBER
			{
			list[card_list++] = $1;
			list[card_list++] = $3;
			}
	;

number_of_nodes:        NPROCS '=' NUMBER
			{
			nprocs = $3;
			}
	;

default_action: DA '=' KILL
			{
				if (rec)
				{
					fprintf(stderr, "\tLine %d: semantical error: ", lines);
					yyerror("Can't set default action in strategy section");
					errors++;
				}
			default_action = KILL;
			rcode_set_defaction(R_DA_IS_KILL);
			}
	|
		DA '=' RESTART
			{
				if (rec)
				{
					fprintf(stderr, "\tLine %d: semantical error: ", lines);
					yyerror("Can't set default action in strategy section");
					errors++;
				}
			default_action = RESTART;
			rcode_set_defaction(R_DA_IS_RESTART);
			}
	;

Sepp:   '\n'  |  ';'
	;

those   :       AT | TILDE
	;

expr:   status id
		{
			if (debug)
				printf("\t\t +expr(pc==%d)\n", pc);

			if ($2.role & ID_DOLLAR || $2.role & ID_DOLLARS)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use $var in expressions");
				errors++;
			}

			if ($1 == R_PRESENT)
			{
				if (isstar($2.role) || isnode($2.role))
				{
					fprintf(stderr, "\tLine %d: semantical error: ", lines);
					yyerror("Can't use  -p  with  *, N, or N*");
					errors++;
				}
			}

			if ($1 == R_REINTEGRATED)
			{
				if (!isnode($2.role))
				{
					fprintf(stderr, "\tLine %d: semantical error: ", lines);
					yyerror("clause `-reintegrated' requires a node as an argument.");
					errors++;
				}
			}

			boole_pc[booleans] = rcode_status ($1, $2, booleans) -1;
			booleans++;

			if (debug)
				printf("\t\t -expr(pc==%d)\n", pc);
		}
	| status those
		{
			fprintf(stderr, "\tLine %d: semantical error: ", lines);
			yyerror("Can't use @- or ~-vars in expressions");
			errors++;
		}
	| '(' expr ')'
	| expr AND expr
		{
			if (debug)
				printf("\t\t +exprAND(pc==%d)\n", pc);

			rcode_single_op (R_AND);

			if (debug)
				printf("\t\t -exprAND(pc==%d)\n", pc);
		}
	| expr OR expr
		{
			if (debug)
				printf("\t\t +exprOR(pc==%d)\n", pc);

			rcode_single_op (R_OR);

			if (debug)
				printf("\t\t -exprOR(pc==%d)\n", pc);
		}
	| NOT expr
		{
			if (debug)
				printf("\t\t +exprNOT(pc==%d)\n", pc);

			rcode_single_op (R_NOT);

			if (debug)
				printf("\t\t -exprNOT(pc==%d)\n", pc);
		}
	| ERRN     '(' id  ')'  compare   NUMBER 
		{
			if ($3.role & ID_DOLLAR || $3.role & ID_DOLLARS)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use $var in expressions");
				errors++;
			}
			if ($3.role & ID_STAR)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use `*' with ERRN");
				errors++;
			}
			/* rcode_err advances pc by 2 units */
			boole_pc[booleans] = rcode_errn($3, $5, $6) -2;
			booleans++;
		}
	| ERRT     '(' id  ')'  compare   NUMBER 
		{
			if ($3.role & ID_DOLLAR || $3.role & ID_DOLLARS)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use $var in expressions");
				errors++;
			}
			if ($3.role & ID_STAR)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use `*' with ERRT");
				errors++;
			}
			/* rcode_err advances pc by 2 units */
			boole_pc[booleans] = rcode_errt($3, $5, $6) -2;
			booleans++;
		}
	| PHASE     '(' id  ')'  compare   NUMBER 
		{
			if ($3.role & ID_DOLLAR || $3.role & ID_DOLLARS)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use $var in expressions");
				errors++;
			}
			if ($3.role & ID_STAR)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use `*' with PHASE");
				errors++;
			}
			if (! ($3.role & ID_THREAD) )
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can only use PHASE with threads");
				errors++;
			}
			/* rcode_phase advances pc by 2 units */
			boole_pc[booleans] = rcode_phase($3, $5, $6) -2;
			booleans++;
		}
	| DEADLOCKED id id
		{
			if ($2.role & ID_DOLLAR || $2.role & ID_DOLLARS || 
			    $3.role & ID_DOLLAR || $3.role & ID_DOLLARS)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use $var in expressions");
				errors++;
			}

			if ($2.role & ID_THREAD   &&   $3.role & ID_THREAD)
			{
				rcode_deadlocked($2.id, $3.id);
			}
			else
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can only use thread identifiers with -d clause.");
				errors++;
			}
			/* should be added here */
		}
	| VAL     '(' VAR  ')'  compare   NUMBER 
		{
			/* rcode_compare_val advances pc by 2 units */
			boole_pc[booleans] = rcode_compare_val($3, $5, $6) -2;
			booleans++;
		}
	;

compare:        EQ | NEQ | GT | GE | LT | LE
	;

include:        INCLUDE STRING 
		{
			printf("[ Including file `%s' ...", $2);
			printf("%d associations have been stored. ]\n", read_associations(a, $2));
		}
	;

section:        if elif else fi
			{
			rec = 1;
			printf("\tif-then-else: ok\n");
			}
	;
		
if:                     {
				if (debug)
					printf("\t if starts...(pc==%d)\n", pc);


				/* each IF implies incrementing the nesting counter */
				rcode_single_op(R_INC_NEST);

				booleans = 0;

			}
		IF '[' expr  ']'     Sepp 
			{
				if (debug)
					printf("\t\t +if expr(pc==%d)\n", pc);

				rcode_if();

				if (debug)
					printf("\t\t -if expr(pc==%d)\n", pc);

				if (debug)
				{ int i;
				  for (i=0; i<booleans; i++)
				    printf("atom %d at line %d\n", i+1, boole_pc[i]);
				}

				booleanstack[nestings++] = booleans;
			}
		THEN Sepp actions
			{
				int i;

				if (debug)
					printf("\t\t then actions(pc==%d)\n", pc);
			}
	;

elif:   |
			{
				if (debug)
					printf("\t\t elif starts...(pc==%d)\n", pc);
				goto_pc = rcode_goto(0);

				rcode_oanew(booleanstack[nestings-1]);
				booleans = 0;
			}
		ELIF '[' expr ']'    Sepp

			{
				if (debug)
					printf("\t\t +elif expr(pc==%d)\n", pc);

				rcode_elif(goto_pc);

				if (debug)
					printf("\t\t -elif expr(pc==%d)\n", pc);

				if (debug)
				{ int i;
				  for (i=0; i<booleans; i++)
				    printf("atom %d at line %d\n", i+1, boole_pc[i]);
				}

				booleanstack[nestings-1] = booleans;
			}
		THEN Sepp actions
			{
				if (debug)
					printf("\t\t then actions(pc==%d)\n", pc);
			}
		elif
	;

else:   |
			{
				if (debug)
					printf("\t\t else starts...(pc==%d)\n", pc);
				goto_pc = rcode_goto(0);
			}
		ELSE 
			{
				if (debug)
					printf("\t\t +else starts...(pc==%d)\n", pc);

				rcode_else(goto_pc);

				if (debug)
					printf("\t\t -else starts...(pc==%d)\n", pc);
			}
		Sepp actions
			{
				if (debug)
					printf("\t\t actions(pc==%d)\n", pc);
			}
	;

fi:             FI 
			{
				if (debug)
					printf("\t\t +fi(pc==%d)\n", pc);

				rcode_fi();

				/* each FI implies decrementing the nesting counter */
				rcode_single_op(R_DEC_NEST);
				rcode_oanew(booleanstack[--nestings]);
				booleanstack[nestings] = 0;

				if (debug)
					printf("\t\t -fi(pc==%d)\n", pc);
			}
	;

status:         KILLED|RESTARTED|ISOLATED|PRESENT|FAULTY|REINTEGRATED|FIRED
	;

id:      GID |NID |TID
	;

actions:
	| actions action
	;

action: '\n' 
	|
	section       Sepp
	|
	recovery_action    Sepp
	;

recovery_action:  KILL    id
		{
			if (debug)
				printf("adding recovery action: kill %d %d\n", $2.role, $2.id);
			if ($2.role & ID_STAR)
			{
				fprintf(stderr, "\tLine %d: semantical error: ", lines);
				yyerror("Can't use `*' with KILL");
				errors++;
			}
			GENERATE_RACTION(R_KILL, $2);
		}
	|         KILL those
		{
			MANAGE_AT_OR_TILDE(R_KILL, $2);
		}
	|         ERR NUMBER id WARN id
		{
			rcode_twoargs(R_PUSH, 0);  /* no arguments */
			rcode_twoargs(R_PUSH, $2); 
			GENERATE_RACTION(R_CHKERR, $3);
			rcode_twoargs(R_PUSH, pc-1); 
			GENERATE_RACTION(R_WARN, $5);
		}
	|         ERR NUMBER id WARN id '(' list ')'
		{
			for (i=card_list-1; i>=0; i--)
			{
				rcode_twoargs(R_PUSH, list[i]);
			}
			rcode_twoargs(R_PUSH, card_list);

			rcode_twoargs(R_PUSH, $2); 
			GENERATE_RACTION(R_CHKERR, $3);
			rcode_twoargs(R_PUSH, pc-1); 
			GENERATE_RACTION(R_WARN, $5);

			if (debug) {
				printf("\tWARN has %d arguments, (", card_list);
				for (i=0; i<card_list-1; i++)
				{
				     printf("%d,", list[i]);
				}
				printf("%d), to be ignored for the time being.\n", 
				     list[card_list-1]);
			}
		}
	|         ERR NUMBER id WARN those           /* version w/o arguments */
		{
			rcode_twoargs(R_PUSH, 0);  /* no arguments */
			rcode_twoargs(R_PUSH, $2); 
			GENERATE_RACTION(R_CHKERR, $3);
			rcode_twoargs(R_PUSH, pc-1); 
			MANAGE_AT_OR_TILDE(R_WARN, $5);
		}
	|         ERR NUMBER id WARN those '(' list ')'
		{
			for (i=card_list-1; i>=0; i--)
			{
				rcode_twoargs(R_PUSH, list[i]);
			}
			rcode_twoargs(R_PUSH, card_list);

			rcode_twoargs(R_PUSH, $2); 
			GENERATE_RACTION(R_CHKERR, $3);
			rcode_twoargs(R_PUSH, pc-1); 
			MANAGE_AT_OR_TILDE(R_WARN, $5);

			if (debug) {
				printf("\tWARN has %d arguments, (", card_list);
				for (i=0; i<card_list-1; i++)
				{
				     printf("%d,", list[i]);
				}
				printf("%d), to be ignored for the time being.\n", 
				     list[card_list-1]);
			}
		}
	|         recovery_action AND WARN id  /* version w/o arguments */
		{
			rcode_twoargs(R_PUSH, 0);  /* no arguments */
			/* save the program counter on the stack */
			rcode_twoargs(R_PUSH, pc-2); 
			GENERATE_RACTION(R_ANDWARN, $4);
		}
	|         recovery_action AND WARN id '(' list ')'
		{
			for (i=card_list-1; i>=0; i--)
			{
				rcode_twoargs(R_PUSH, list[i]);
			}
			rcode_twoargs(R_PUSH, card_list);

			/* save the program counter on the stack */
			rcode_twoargs(R_PUSH, pc-2-card_list); 
			GENERATE_RACTION(R_ANDWARN, $4);

			if (debug) {
				printf("\tWARN has %d arguments, (", card_list);
				for (i=0; i<card_list-1; i++)
				{
				     printf("%d,", list[i]);
				}
				printf("%d), to be ignored for the time being.\n", 
				     list[card_list-1]);
			}
		}
	|         recovery_action AND WARN those                     /* version w/o arguments */
		{
			rcode_twoargs(R_PUSH, 0);  /* no arguments */
			/* save the program counter on the stack */
			rcode_twoargs(R_PUSH, pc-2); 
			MANAGE_AT_OR_TILDE(R_ANDWARN, $4);
		}
	|         recovery_action AND WARN those '(' list ')'
		{
			for (i=card_list-1; i>=0; i--)
			{
				rcode_twoargs(R_PUSH, list[i]);
			}
			rcode_twoargs(R_PUSH, card_list);

			/* save the program counter on the stack */
			rcode_twoargs(R_PUSH, pc-2-card_list); 
			MANAGE_AT_OR_TILDE(R_ANDWARN, $4);

			if (debug) {
				printf("\tWARN has %d arguments, (", card_list);
				for (i=0; i<card_list-1; i++)
				{
				     printf("%d,", list[i]);
				}
				printf("%d), to be ignored for the time being.\n", 
				     list[card_list-1]);
			}
		}
	|         RESTART TID
		{
			if (debug)
				printf("adding recovery action: restart %d %d\n", $2.role, $2.id);
			GENERATE_RACTION(R_RESTART, $2);
		}
	|         RESTART GID
		{
			if (debug)
				printf("adding recovery action: restart %d %d\n", $2.role, $2.id);
			GENERATE_RACTION(R_RESTART, $2);
		}
	|         RESTART NID      /* formerly known as REBOOT NID */
		{
			if (debug)
				printf("adding recovery action: restart %d %d\n", $2.role, $2.id);
			GENERATE_RACTION(R_REBOOT, $2);
		}
	|         RESTART those
		{
			MANAGE_AT_OR_TILDE(R_RESTART, $2);
		}
	|       START   TID
		{
			if (debug)
				printf("adding recovery action: start %d %d\n", $2.role, $2.id);
			GENERATE_RACTION(R_START, $2);
		}
	|       START   GID      /* an error */
		{
			fprintf(stderr, "\tLine %d: semantical error: ", lines);
			yyerror("Can't start groups");
			errors++;
		}
	|       START   NID      /* an error */
		{
			fprintf(stderr, "uh?\n");
			fprintf(stderr, "\tLine %d: semantical error: ", lines);
			yyerror("can't start nodes");
			errors++;
		}
	|         START those
		{
			MANAGE_AT_OR_TILDE(R_START, $2);
		}
	|       REMOVE NUMBER id FROM ERRORLIST
		{
			rcode_twoargs(R_PUSH, $2);
			GENERATE_RACTION(R_REMOVE, $3);
		}
	|       REMOVE ANY id FROM ERRORLIST
		{
			GENERATE_RACTION(R_REMOVE_ALL, $3);
		}
	|       SEND NUMBER TID
		{
			rcode_twoargs(R_PUSH, $2);
			GENERATE_RACTION(R_SEND, $3);
		}
	|       ENABLE TID
		{
			if (debug)
				printf("adding recovery action: enable %d %d\n", $2.role, $2.id);
			GENERATE_RACTION(R_ENABLE, $2);
		}
	|       ENABLE GID
		{
			if (debug)
				printf("adding recovery action: enable %d %d\n", $2.role, $2.id);
			GENERATE_RACTION(R_ENABLE, $2);
		}
	|       ENABLE  NID      /* an error */
		{
			fprintf(stderr, "\tLine %d: semantical error: ", lines);
			yyerror("Can't enable nodes");
			errors++;
		}
	|       ENABLE those
		{
			MANAGE_AT_OR_TILDE(R_ENABLE, $2);
		}
	| CALL NUMBER
		{
			rcode_twoargs(R_PUSH, 0);  /* no arguments */
			rcode_twoargs(R_FUNCTION_CALL, $2); 
		}
	| CALL NUMBER '(' list ')'
		{
			for (i=card_list-1; i>=0; i--)
			{
				rcode_twoargs(R_PUSH, list[i]);
			}
			rcode_twoargs(R_PUSH, card_list);

			rcode_twoargs(R_FUNCTION_CALL, $2); 
		}
	| PAUSE NUMBER
		{
			rcode_twoargs(R_PAUSE, $2);
		}
	| LET VAR '=' linexp
		{
			rcode_twoargs(R_SET, $2);
		}
	| LOG STRING
		{
			rcode_twoargs(R_LOGS, pushstring($2));
		}
	| LOG NUMBER 
		{
			rcode_twoargs(R_LOGI, $2);
		}
	| LOG CLOCK 
		{
			rcode_single_op(R_LOGC);
		}
	| LOG VAR 
		{
			rcode_twoargs(R_LOGV, $2);
		}
	|         CLEAR TID
		{
			GENERATE_RACTION(R_CLEAR, $2);
		}
	;

linexp:	expression    ';'
                {
                	$$ = $1;
                }
        |       error   '\n'
                { yyclearin; yyerrok; }
                linexp
                { return 0; }
        ;


expression :	'(' expression ')'
		{
		$$ = $2;
		}
	|	expression '*' expression       %prec '*'
		{
		rcode_single_op(R_MULTIPLY);
		}
	|	expression '/' expression       %prec '*'
		{
		rcode_single_op(R_DIVIDE);
		}
	|	expression '+' expression       %prec '+'
		{
		rcode_single_op(R_ADD);
		}
	|	expression '-' expression       %prec '+'
		{
		rcode_single_op(R_SUBTRACT);
		}
	|	'-' expression                  %prec UMINUS
		{
		rcode_single_op(R_COMPLEMENT);
		}
	|	VAR
		{
		/* variables are pointed to by negative */
		/* integers in -25..0 */
		rcode_twoargs(R_GET, $1);
		}
	|	NUMBER
		{
		rcode_twoargs(R_PUSH, $1); 
		}
	;
%%
#include "lex.yy.c"

int main(int argc, char *argv[])
{ 
    char  filename[255];
    char *path; 
    int   fd, old_fd;
    clock_t t1, t2;
    void options(void);
    extern FILE *f;  /* output file pointer */
    char *today(void);
    char *codomain, *domain;
    extern int       maxlogical;
    extern int       maxtask, maxtaskII;

    printf("Content-type: text/plain\n\n"); fflush(stdout);

    ifname = EFTOS_SRC;
    ofname = RCODE_FILE; 
    verbose = 1, debug = notime = static_version = 0; /* set default values */

    fprintf(stderr, "Ariel translator, %s, (c) K.U.Leuven 1998, 1999, 2000.\n", 
	VERSION);
    for (i=1; i<argc; i++)
	 if (argv[i][0] == '-')
	     switch (argv[i][1])
	     {
	     case 'i':  ifname = argv[++i];
			break;
	     case 'o':  ofname = argv[++i];
			break;
	     case 'V':  verbose= 1;
			break;
	     case 'v':  verbose= 0;
			break;
	     case 'D':  debug= 1;
			break;
	     case 'd':  debug= 0;
			break;
	     case 'h':
	     case 'H':
	     case '?':
			options();
			return(0);
	     case 's':  static_version = 1;
			break;
	     case 't':  notime = 1;
			break;
	     default :  fprintf(stderr, "Invalid option.\n");
			exit(1);
	     }
	 else
	 {
	     fprintf(stderr, "Invalid option --- aborting.\n");
	     options();
	     exit(1);
	 }

/*
For the CGI version: output is /dev/null and stderr = stdout
ofname = "/dev/null";
dup2(1,2);
*/

    /* opening associative array `a' */
    a = aopen( (int(*)(const void*, const void*)) strcmp );

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

    old_fd = dup(0);

    dup2(fd,0);

    f = fopen(ofname, "wb");
    if (f==NULL)
    {
	fprintf(stderr, "Couldn't open %s for writing --- exiting.\n", ofname);
	exit(1);
    }

    init_taskram(); /* this must be executed *before* yyparse() */
    init_logicalram();

    fprintf(stderr, "Parsing file %s...\n", ifname);

    DeBug; /* 1 */

    t1 = clock();
    yyparse();

    DeBug; /* 2 */

    rcode_stop();

    DeBug; /* 3 */

    rflush();

    DeBug; /* 4 */

    t2 = clock();

    lines--;

    if (notime)
	    fprintf(stderr, "...done.\n");
    else
    {
        fprintf(stderr, "...done (%d lines", lines,
	           (double)(t2-t1)/CLOCKS_PER_SEC);
        if (t2-t1 != 0)
	    fprintf(stderr, " in %lf CPU secs, or %.3lf lines per CPU sec.)\n",
		(double)(t2-t1)/CLOCKS_PER_SEC,
		(double) lines * CLOCKS_PER_SEC / (t2-t1));
        else
	    fprintf(stderr, ".)\n");
    }

    fclose(f);

    DeBug; /* 5 */

    if (errors)
    {
	fprintf(stderr, "%d error%s detected --- output rejected.\n", 
	  errors, (errors>1)? "s":"");
	unlink(ofname);
    }
    else
    {
	    fprintf(stderr, "Output written in file %s.\n", ofname);
    }


    DeBug; /* 6 */

    if (w_sp > 0)
    {
	    watchdog_flush(watchdog, w_sp);
	    fprintf(stderr, "Watchdogs configured.\n");
    }

    DeBug; /* 7 */

    if (nv_sp > 0)
    {
	    nversion_flush(nversion, nv_sp);
	    fprintf(stderr, "N-version tasks configured.\n");
    }

    DeBug; /* 8 */

    if (maxlogical > 0)
    {
	    h = fopen(LOGICAL_FNAME, "w");
	    bsl_logical_flush(h);
	    fclose(h);
	    fprintf(stderr, "Logicals written in file %s.\n", LOGICAL_FNAME);
    }

    DeBug; /* 9 */

    if (maxtask > 0 || maxtaskII > 0)
    {
	    h = fopen(TASK_FNAME, "w");
	    bsl_task_flush(h);
	    fclose(h);
	    fprintf(stderr, "Tasks written in file %s.\n", TASK_FNAME);
    }


    DeBug; /* 10 */

    if (static_version)
    {
	fprintf(stderr, "static version\n"); fflush(stderr);

	h = fopen(HFNAME, "w");
	if (h==NULL)
	{
	    fprintf(stderr, "Couldn't open %s for writing --- exiting.\n", HFNAME);
	    exit(1);
	}

    DeBug; /* 11 */

	hflush(h, ifname);
	fclose(h);
	fprintf(stderr, "Preloaded r-codes written in file %s.\n", HFNAME);

	h = fopen(TONAME, "w");
	if (h==NULL)
	{
	    fprintf(stderr, "Couldn't open %s for writing --- exiting.\n", TONAME);
	    exit(1);
	}

    DeBug; /* 12 */

	timeout_flush(h, ifname);
	fclose(h);
	fprintf(stderr, "Time-outs written in file %s.\n", TONAME);

	h = fopen(IDNAME, "w");
	if (h==NULL)
	{
	    fprintf(stderr, "Couldn't open %s for writing --- exiting.\n", IDNAME);
	    exit(1);
	}

    DeBug; /* 11 o 13 */

	ident_flush(h, ifname);
	fclose(h);
	fprintf(stderr, "Identifiers written in file %s.\n", IDNAME);
    }

    DeBug; /* 12 o 14 */

    h = fopen(ALPHANAME, "w");
    if (h==NULL)
    {
	fprintf(stderr, "Couldn't open %s for writing --- exiting.\n",
		ALPHANAME);
	exit(1);
    }
    alpha_flush(h);
    fclose(h);

    fprintf(stderr, "Alpha-count parameters written in file %s.\n", ALPHANAME);

    /* close the associative array `a' */
#ifdef FREE_ASSOC
    for (arewind(a); domain=anext(a); )
    {
	codomain = aread(a, domain);
	printf("domain=%s, codomain=%s\n", domain, codomain);
	free(domain), free(codomain);
    }
    aclose(a);
#endif


    DeBug; /* 15 */

    pushstring(NULL);

    DeBug; /* 16 */

    close(fd);
    printf("Press any key to finish processing...\n");
    dup2(old_fd, 0);
    while ( kbwait() == 0) ;

	return 0;
}
void options()
{
fprintf(stderr, "Valid options are\n");
fprintf(stderr, "  -i <filename>    : set input to <filename>\n");
fprintf(stderr, "  -o <filename>    : set output to <filename>\n");
fprintf(stderr, "  -s               : write a preloaded version of the r-codes in file `trl.h'.\n");
fprintf(stderr, "  -V / -v          : turn verbose mode on / off (default: on)\n");
fprintf(stderr, "  -D / -d          : turn debug mode on / off (default: off)\n");
fprintf(stderr, "  -t               : do not use external command DATE.EXE\n");
fprintf(stderr, "  -H / -h / -?     : show this message.\n");
}
char *today()
{ 
  FILE *popen(const char *, const char *);
  /* FILE *f = popen("./date", "r"); */
  FILE *f = fopen("date", "r");
  static char td[80];
  if (f==NULL)
	return NULL;

  fgets(td, 80, f);
  *strchr(td, '\n') = '\0';
  pclose(f);
  return td;
}

/* input: a,     already opened associative array,
 *        fname, include file to be searched for definitions
 *
 *  process: fname is read through looking for definitions
 *           of the form    #define <STRING> <INTEGER>
 *           For each definition, a new awrite(a, ...) stores
 *           the association <STRING> -> <INTEGER> into the
 *           associative array.
 *
 *  output:  a possibly modified associative array.
 *
 *  return values: number of associations added to a.
 *
 *  Creation date: 28 Oct. 1997.
 */
#define  SEPAR   "\t\n "
int read_associations(ASSOC *a, char *fname)
{
  FILE *f;
  char string[256];
  char *domain, *codomain;
  char *tmp;
  int   n = 0;
  int   l;
  char *x, *y;
  char *strdup(const char *);
  
      /*
  printf("within read_associations\n");
      */
  if (!a || !f) return 0;
  f = fopen(fname, "r");
  if (!f) return 0;

  while (fgets(string, 256, f) != NULL)
  {
      if (string[0]!='#') continue;

      /*
      printf("found a '#'-record in file\n");
      printf("string == %s", string);
      printf("string+1 == %s", string+1);
      */
      tmp = strtok(string+1, SEPAR);

      /*
      printf("tmp == %s\n", tmp);
      */

      if (strcmp("define", tmp) != 0) continue;

      /*
      printf("found a '#define'-record in file\n");
      */

      x = strtok(NULL, SEPAR);
      if (x == NULL)
	continue;
      y = strtok(NULL, SEPAR);
      if (y == NULL)
	continue;

      domain = strdup(x);
      codomain = strdup(y);

      l = strlen(codomain);
      for (i=0, l=strlen(codomain);  i<l;  i++)
	   if (! isdigit(codomain[i])  && codomain[i] != '-' && codomain[i] == '\0')
		 break;

      if (i<l) continue;

      /*
      printf("about to add association `%s' -> `%s'\n", domain, codomain);
      */
      if (domain && codomain)
      {
	  awrite(a, domain, codomain);
	  n++;
      }
  }

  fclose(f);
      /*
  printf("out of read_associations\n");
      */
  return n;
}

int yyerror(char *s)
{
fprintf(stderr, "%s\n", s);
return 0;
}

/* as a global ( => static) area, this is set to zero by the system */
task_t taskram[MAX_TASKS];
int    maxtask, maxtaskII;

int storetask(task_t *t)
{
	if (t == NULL || t->identifier < 0 || t->identifier >= MAX_TASKS)
		return -1;

	taskram[t->identifier].identifier = t->identifier;
	taskram[t->identifier].node = t->node;
	taskram[t->identifier].idf = t->idf;
	taskram[t->identifier].name = strdup(t->name);

	if (t->identifier > maxtask)
		maxtask = t->identifier;
	return maxtask;
}

logical_t logicalram[MAX_LOGICALS];
int       maxlogical;
extern alphacount_t alphas[];

int storelogical(int identifier, int tasklist[], int n, char *name)
{
	if (identifier < 0 || identifier >= MAX_LOGICALS)
	{
		yyerror("storelogical: group id not allowed");
		return -1;
	}
	if (n<0 || n>= MAX_TASKS_IN_LOGICAL)
	{
		yyerror("storelogical: more tasks than maximum allowed");
		return -2;
	}
	if (! tasklist)
	{
		yyerror("storelogical: invalid NULL argument");
		return -3;
	}

	logicalram[identifier].card = n;
	logicalram[identifier].p_tasklist = malloc((n)*sizeof(int));
	if ( ! logicalram[identifier].p_tasklist )
	{
		yyerror("storelogical:malloc");
		return -4;
	}
	memcpy(logicalram[identifier].p_tasklist, tasklist, (n)*sizeof(int));
	
	logicalram[identifier].name = strdup(name);

	if (identifier > maxlogical)
		maxlogical = identifier;
	return maxlogical;
}

int init_taskram()
{
	int i;

	for (i=0; i<MAX_TASKS; i++)
	{
		taskram[i].identifier = -1;
		taskram[i].idf = -1;
		taskram[i].alias = -1;
		taskram[i].mbox = -1;
		taskram[i].name = '\0';
/*
		taskram[i].p_logicalid = NULL;
 */
		alphas[i].k = alphas[i].threshold = 0.0;
		alphas[i].used = 0;
	}
	return 0;
}
int init_logicalram()
{
	int i;
	for (i=0; i<MAX_LOGICALS; i++)
	{
		logicalram[i].p_tasklist = NULL;
		logicalram[i].name = '\0';
		logicalram[i].card = -1;
	}
	return 0;
}
int storetaskII(task_t *t)
{
	if (t == NULL || t->identifier < 0 || t->identifier >= MAX_TASKS)
		return -1;

	taskram[t->identifier].identifier = t->identifier;
	taskram[t->identifier].mbox = t->mbox;
	taskram[t->identifier].alias = t->alias;
	/* taskram[t->identifier].name = '\0'; /* strdup(t->name); */

	if (t->identifier > maxtaskII)
		maxtaskII = t->identifier;
	return maxtaskII;
}

int watchdog_flush(watchdog_t *wp, int n)
{
	FILE *f;
	char fname[MAX_WD_FNAME];
	char name[MAX_WD_NAME];
	char *action[] = { "warn", "reboot", "restart" } ;
	int standard_headers(FILE*);
	int t, node;

/*
int watching, watched, rate, unit, action,
    target, running;
 */
	for (; n--; wp++)
	{
		sprintf(name, "TIRAN_Watchdog_%d", wp->watching);
		sprintf(fname, "TIRAN_task_%d.c", wp->watching);
		// TIRAN_GetTaskNodeId(wp->watching, &t, &node);

		f = fopen(fname, "w");
		if (f==NULL)
		{
			fprintf(stderr,
                   "Couldn't open %s for writing --- skipping.\n", fname);
			exit(1);
		}

		fprintf(f, "/*\n *   Watchdog task no. %d -- \n *   created by the ariel translator on %s\n */\n\n", wp->watching, today());
		standard_headers(f);
		fprintf(f, "#include \"../timeouts.h\"\n");
		fprintf(f, "#include \"../TYPES.h\"\n");
		fprintf(f, "#include \"../ACTORS.h\"\n");
		fprintf(f, "#include \"../EVENTS.h\"\n");
		fprintf(f, "#include \"../RaiseEvent.h\"\n");
		fprintf(f, "#include \"../identifiers.h\"\n\n");


		fprintf(f, 
		"\t/* Definition of task %d as a watchdog, watching task %d,\n",
			wp->watching, wp->watched);
		fprintf(f, "\t   with a heartbeat rate of %d %s,\n",
			wp->rate, (wp->unit==0)?"milliseconds":"microseconds");
		fprintf(f, "\t   and with an on error action equal to \"%s",
			action[wp->action]);
		if (wp->action == 0) /* WARN */
		{
			fprintf(f, "task %d", wp->target);
		}
		fprintf(f, "\"\n\t */\n\n\n");


		fprintf(f, "void main(int argc,char *argv[]) {\n");
		fprintf(f, "int node, task, r;\nchar buff[256];\n\n");

		fprintf(f, "/* initialise global variables */\n");
		fprintf(f, "TIRAN_ThisNodeId = %d;\n", 
			taskram[wp->watching].node);
		fprintf(f, "TIRAN_ThisTaskId = %d;\n\n",
			taskram[wp->watching].idf);

		fprintf(f, "/* initialise TIRAN */\n");
		fprintf(f, "TIRAN_SocketStartup();\nTIRAN_InitLibrary();\n\n");

		fprintf(f, "/* create this task's mailbox */\n");
		fprintf(f, "TIRAN_CreateMailBox((TIRAN_MBOX_ID)%d);\n\n",
			taskram[wp->watching].idf);
			//wp->watching);

		fprintf(f, "/* wait for an activation message */\n");
		fprintf(f, "\nTIRAN_ReceiveMailBoxMessage((TIRAN_MBOX_ID) %d, ",
			taskram[wp->watching].idf);
			//wp->watching);
		fprintf(f, "buff, 256, 10000000);\n\n");
		fprintf(f, "printf(\"Watchdog %d activated\\n\");\n",
			wp->watching);

		fprintf(f, "\nwhile (1) {\n");
		fprintf(f, "\tr=TIRAN_ReceiveMailBoxMessage((TIRAN_MBOX_ID) %d,",
			taskram[wp->watching].idf);
			//wp->watching);
		fprintf(f, "buff, 256, %d);\n", wp->rate);
		fprintf(f, "\tif (strncmp(buff, \"end\", 3) == 0) break;\n");
		fprintf(f, "\tif (r==TIRAN_ERROR) {\n");
		fprintf(f,"\t\tRaiseEvent(TIRAN_E_WD_FIRE, TIRAN_WD, %d, 0);\n",
			wp->watching);
		fprintf(f,"\t\tbreak;\n");
		fprintf(f,"\t\t}\n");

		fprintf(f, "\t}\n\n");

		fprintf(f, "TIRAN_CloseMailBox((TIRAN_MBOX_ID) %d);\n",
			taskram[wp->watching].idf);
			//wp->watching);
		fprintf(f, "TIRAN_CloseLibrary();\n");
		fprintf(f, "TIRAN_SocketCleanup();\n\n");

		fprintf(f, "}\n/* EOF file %s */\n", fname);
		fclose(f);

		fprintf(stderr, "Watchdog set up in file %s\n", fname);
	}

	return 0;
}
int standard_headers(FILE *f)
{
	if (f != NULL)
	{
		fprintf(f, "#include \"../TIRAN_API.h\"\n");
		return 0;
	}
	return 1;
}


int nversion_flush(nversion_t *nvp, int n)
{
	FILE *f;
	char fname[MAX_NV_FNAME];
	char name[MAX_NV_NAME];
	int standard_headers(FILE*);
	int verscomp(const void *, const void *);
	version_t *this;
	int storelogical(int, int[], int, char*);
	int iv, i;
	int s=0;

	for (; n--; nvp++)
	{
	    /* order versions */
	    qsort(nvp->va, nvp->va_num, sizeof(version_t), verscomp);

	    sprintf(name, "TIRAN_NVersion_Task%d", nvp->task);
	    tasklist_nr = 0;

	    this = nvp->va;

	    for (iv = 0; iv<nvp->va_num; iv++, this++)
	    {
		/* creates a logical consisting of the va_num versions */
		tasklist[tasklist_nr++] = this->task;

		/* name of the logical */
		sprintf(name, "TIRAN_NVersion_Task%d", nvp->task);

		/* source name for version iv */
		sprintf(fname, "TIRAN_task_%d.c", this->task);
		f = fopen(fname, "w");
		if (f==NULL)
		{
			fprintf(stderr,
                   "Couldn't open %s for writing --- skipping.\n", fname);
			exit(1);
		}
		standard_headers(f);
		fprintf(f, "/* Task %d of NVersion Task %d\n",
			this->task, nvp->task);
		fprintf(f, "   Version %d / %d\n */\n\n",
			this->version, nvp->va_num);

		fprintf(f, "int TIRAN_task_%d(void) {\n", this->task);
		fprintf(f, "\tTIRAN_Voting_t *dv;\n");
		fprintf(f, "\tsize_t size;\n");
		fprintf(f, "\tdouble %s(const void*, const void*);\n\n",
			nvp->metric);
		fprintf(f, "\tdv = TIRAN_VotingOpen(%s);\n", nvp->metric);
		fprintf(f, "\tif (dv == NULL) {\n");
		fprintf(f, "\t\tRaiseEvent(TIRAN_ERROR_VOTING_CANTOPEN);\n");
		fprintf(f, "\t\tTIRAN_exit(TIRAN_ERROR_VOTING_CANTOPEN);\n");
		fprintf(f, "\t}\n\n");

		fprintf(f, "\t/* voting task description: which tasks and which versions */\n");
		fprintf(f, "\t/* constitute the n-version task */\n");

		for (i=0; i<nvp->va_num; i++)
		    fprintf(f, "\tTIRAN_VotingDescribe(dv, %d, %d, %s);\n",
			nvp->va[i].task, nvp->va[i].version, 
			(nvp->va[i].version == this->version)? "1":"0");

		fprintf(f, "\n\tTIRAN_VotingRun(dv);\n\n");
		if (nvp->success > 0)
		{
		    fprintf(f, "\t/* output should be sent to task %d */\n",
			nvp->success);
		    fprintf(f, "\tTIRAN_VotingOutput(dv, %d);\n", nvp->success);
		}
		if (nvp->voting == 1) /* Majority voting */
		    fprintf(f, "\tTIRAN_VotingOption(dv, TIRAN_VOTING_IS_MAJORITY);\n\n");
		fprintf(f, "\t/* redirect stdout into a pipe input stream */\n");
		fprintf(f, "\tTIRAN_pipework();\n\n");

		fprintf(f, "\t/* execute the version */\n");
		fprintf(f, "\t/* Nota Bene: time-out management to be added */\n");
		fprintf(f, "\ttask_%d();\n\n", this->task);

		fprintf(f, "\tsize = read(0, buff, MAX_BUFF);\n");
		fprintf(f, "\tif (size > 0) {\n");
		fprintf(f, "\t\t/* forward the input buffer to the local voter of this version */\n");
		fprintf(f, "\t\tTIRAN_VotingInput(dv, buff, size);\n");
		fprintf(f, "\t} else {\n");
		fprintf(f, "\t\t/* signal there's no input */\n");
		fprintf(f, "\t\tTIRAN_VotingInput(dv, NULL, 0);\n");
		fprintf(f, "\t\tRaiseEvent(TIRAN_ERROR_VOTING_NOINPUT, %d);\n",
				this->task);
		fprintf(f, "\t\tTIRAN_NotifyTask(%d, TIRAN_ERROR_VOTING_NOINPUT);\n",
				nvp->error);
		fprintf(f, "\t}\n");

		fprintf(f, "}\n/* EOF file %s */\n", fname);
		fclose(f);
	    }

	    storelogical(nvp->task, tasklist, tasklist_nr, name);
	}

	return 0;
}
int verscomp(const void *a, const void *b)
{
	return ((version_t*)a)->version - ((version_t*)b)->version;
}
int query_rcode_array(int line, int *opcode, int *op1, int *op2)
{
	*opcode = compile_time_rcode[line][0],
	*op1    = compile_time_rcode[line][1],
	*op2    = compile_time_rcode[line][2];
	return 0;
}



#define MAXSTRINGS 256
int pushstring(char *s)
{
	static int n;
	static char *strings[MAXSTRINGS];

	if (s == NULL)
	{
		sflush(strings, n);
		if (n>0)
			while (n--) free(strings[n]);
		return 0;
	}

	/* else */
	strings[n] = strdup(s);
	return n++;
}

int sflush(char*s[], int n)
{
	FILE* f;
	int i;
	
	f = fopen(STRNAME, "w");
	if (f==NULL) return 0;

	if (n==0)
	{
		fclose(f);
		return 1;
	}

	fprintf(f, "static char *arstrings[%d] = {\n", n);
	for (i=0; i<n; i++)
		fprintf(f, "\t \"%s\", /* %d */\n", s[i], i);
	fprintf(f, "};\n");
	fclose(f);
	return 1;
}
/* EOF ariel.y */
