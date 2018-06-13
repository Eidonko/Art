/*******************************************************************************
**
** Created By   :    Eidon@tutanota.com
**
********************************************************************************
** Description :
** definition of the R-codes (pseudocodes of the Ariel language)
**
*******************************************************************************/

#ifndef     R_CODE_H
#define     R_CODE_H

/* max number of RCode's */
#ifndef RCODE_MAX_CARD
#define RCODE_MAX_CARD   4096
#endif

/* max nesting level for if's */
#define RCODE_MAX_NEST     32

/* name of the file with the textual representation of the output rcodes */
#define OUTPUT_RCODE       "output.rcode"

#define R_AGENT	        1000
#define R_BACKUP        2000
#define R_MANAGER       3000

#define R_DA_IS_KILL	666
#define R_DA_IS_RESTART 999




/* The R-opcodes */

#define R_STOP		000000
#define isstoprcode(x)     (! x)



#define R_SET_ROLE	1
#define R_SET_DEF_ACT	2

#define issetrcode(x)   (x>=R_SET_ROLE && x<=R_SET_DEF_ACT)


#define R_AND		3
#define R_OR		4
#define R_NOT		5
#define R_INC_NEST      6
#define R_DEC_NEST      7
#define R_STRVAL        8
#define R_STRPHASE      9

#define is1operandrcode(x)   (x>=R_AND && x<=R_STRPHASE)


#define R_FALSE		10
#define R_GOTO	        11
#define R_PUSH          12
#define R_FUNCTION_CALL 13
#define R_OANEW	        14
#define R_CLEAR         15
#define R_PAUSE	        16

#define is2operandrcode(x)   (x>=R_FALSE && x<=R_PAUSE)


#define R_KILLED       17
#define R_RESTARTED    18
#define R_PRESENT      19
#define R_ISOLATED     20
#define R_FAULTY       21
#define R_FIRED        22
#define R_REINTEGRATED 23

#define R_STRERRN      24
#define R_STRERRT      25
#define R_COMPARE      26

#define isclause(x)   (x>=R_KILLED && x<=R_REINTEGRATED)
#define istestrcode(x)   (x>=R_KILLED && x<=R_COMPARE)



#define R_KILL	       27
#define R_WARN	       28
#define R_ANDWARN      29
#define R_START	       30
#define R_RESTART      31
#define R_REBOOT       32
#define R_CHKERR       33
#define R_ENABLE       34
#define R_REMOVE       35
#define R_REMOVE_ALL   36
#define R_SEND         37
#define R_GET          38 // VdF, Sept 2000
#define R_SET          39 // VdF, Sept 2000
#define R_CONST        40 // VdF, Sept 2000
#define R_ADD          41 // VdF, Oct 2000
#define R_SUBTRACT     42 // VdF, Oct 2000
#define R_MULTIPLY     43 // VdF, Oct 2000
#define R_DIVIDE       44 // VdF, Oct 2000
#define R_COMPLEMENT   45 // VdF, Oct 2000
#define R_LOGS         46 // VdF, Oct 2000
#define R_LOGI         47 // VdF, Oct 2000
#define R_LOGC         48 // VdF, Oct 2000
#define R_LOGV         49 // VdF, Oct 2000

#define isactionrcode(x)   (x>=R_KILL && x<=R_REMOVE_ALL)

#define R_DEADLOCKED   50
#define LAST_RCODE     R_DEADLOCKED

#define  DEST   1	/* field # for destination in R_FALSE statements */

typedef int rcode_t[3];

#ifdef COMPILETIME
static rcode_t compile_time_rcode[RCODE_MAX_CARD];
#endif

struct goto_t {
   int pc;
   struct goto_t *next;
};

typedef struct {
        int pc;
	struct goto_t *gotos;
	} if_t;

static if_t ifs[RCODE_MAX_NEST], *iftop;
static int  ifp;

#define RCODE_FILE     ".rcode"
#define RINT_DUMP_FILE "trace.rint"



#ifndef COMPILETIME

int   R_Stop(int,int,int);
int   R_And(int,int,int);
int   R_Or(int,int,int);
int   R_Not(int,int,int);
int   R_OArenew(int,int,int);
int   R_Clear(int,int,int);
int   R_NestIn(int,int,int);
int   R_NestOut(int,int,int);
int   R_StorePhase(int,int,int);
int   R_StoreVal(int,int,int); // VdF Sep 2000 */
int   R_False(int,int,int);
int   R_Goto(int,int,int);
int   R_Pause(int,int,int);
int   R_Killed(int,int,int);
int   R_Rebooted(int,int,int);
int   R_Restarted(int,int,int);
int   R_Present(int,int,int);
int   R_Isolated(int,int,int);
int   R_Faulty(int,int,int);
int   R_Fired(int,int,int);
int   R_Reintegrated(int,int,int);
int   R_StrErrNum(int,int,int);
int   R_StrErrType(int,int,int);
int   R_Compare(int,int,int);
int   R_Kill(int,int,int);
int   R_DotsWarn(int,int,int);
int   R_AndWarn(int,int,int);
int   R_Start(int,int,int);
int   R_Restart(int,int,int);
int   R_Send(int,int,int);
int   R_Reboot(int,int,int);
int   R_CheckErr(int,int,int);
int   R_Enable(int,int,int);
int   R_Call(int,int,int);
int   R_Remove(int,int,int);
int   R_RemoveAll(int,int,int);
int   R_PushArg(int,int,int);
int   R_Get(int,int,int);
int   R_Set(int,int,int);
int   R_Const(int,int,int);
int   R_Add(int,int,int);
int   R_Subtract(int,int,int);
int   R_Multiply(int,int,int);
int   R_Divide(int,int,int);
int   R_Complement(int,int,int);
int   R_Logs(int,int,int);
int   R_Logc(int,int,int);
int   R_Logi(int,int,int);
int   R_Logv(int,int,int);
int   R_Deadlocked(int,int,int);

int   R_Nop(int a,int b,int c); // { return a+b+c; }



static int (*rfunc[])(int,int,int) = 
 { R_Stop,		// 0 
   R_Nop,		// 1 
   R_Nop,		// 2 

   R_And,		// 3 
   R_Or,		// 4 
   R_Not,		// 5 
   R_NestIn,	// 6
   R_NestOut,	// 7
   R_StoreVal,  // 8
   R_StorePhase,// 9

   R_False,		// 10
   R_Goto,		// 11
   R_PushArg,		// 12
   R_Call,		// 13
   R_OArenew,		// 14
   R_Clear,		// 15
   R_Pause,		// 16

   R_Killed,	// 17 
   R_Restarted,	// 18
   R_Present,	// 19
   R_Isolated,	// 20
   R_Faulty,	// 21
   R_Fired,	// 22
   R_Reintegrated,// 23

   R_StrErrNum,	// 24
   R_StrErrType,// 25
   R_Compare,	// 26

   R_Kill,		// 27
   R_DotsWarn,	// 28
   R_AndWarn,	// 29
   R_Start,		// 30
   R_Restart,	// 31
   R_Reboot,	// 32
   R_CheckErr,	// 33
   R_Enable,	// 34
   R_Remove,	// 35
   R_RemoveAll,	// 36
   R_Send,    	// 37
   R_Get,    	// 38
   R_Set,    	// 39
   R_Const,    	// 40
   R_Add,    	// 41
   R_Subtract, 	// 42
   R_Multiply, 	// 43
   R_Divide,   	// 44
   R_Complement,// 45
   R_Logs,	// 46
   R_Logi,	// 47
   R_Logc,	// 48
   R_Logv,	// 49
   R_Deadlocked,// 50
 };

#endif


static char *r2a[] =
 { "R_Stop",		/* 0 */
   "R_Nop",		/* 1 */
   "R_Nop",		/* 2 */

   "R_And",		/* 3 */
   "R_Or",		/* 4 */
   "R_Not",		/* 5 */
   "R_NestIn",		/* 6 */
   "R_NestOut",		/* 7 */
   "R_StoreVal",	/* 8 */
   "R_StorePhase",	/* 9 */

   "R_False",		/* 10 */
   "R_Goto",		/* 11 */
   "R_PushArg",		/* 12 */
   "R_Call",		/* 13 */
   "R_OArenew",		/* 14 */
   "R_Clear",		/* 15 */
   "R_Pause",		/* 16 */

   "R_Killed",		/* 17 */
   "R_Restarted",	/* 18 */
   "R_Present",		/* 19 */
   "R_Isolated",	/* 20 */
   "R_Faulty",		/* 21 */
   "R_Fired",		/* 22 */
   "R_Reintegrated",	/* 23 */

   "R_StrErrNum",	/* 24 */
   "R_StrErrType",	/* 25 */
   "R_Compare",		/* 26 */

   "R_Kill",		/* 27 */
   "R_DotsWarn",	/* 28 */
   "R_AndWarn",		/* 29 */
   "R_Start",		/* 30 */
   "R_Restart",		/* 31 */
   "R_Reboot",		/* 32 */
   "R_CheckErr",	/* 33 */
   "R_Enable",		/* 34 */
   "R_Remove",		/* 35 */
   "R_RemoveAll",	/* 36 */
   "R_Send",            /* 37 */
   "R_Get",             /* 38 */
   "R_Set",             /* 39 */
   "R_Const",           /* 40 */
   "R_Add",             /* 41 */
   "R_Subtract",        /* 42 */
   "R_Multiply",        /* 43 */
   "R_Divide",          /* 44 */
   "R_Complement",      /* 45 */
   "R_Logs",      	/* 46 */
   "R_Logi",      	/* 47 */
   "R_Logc",      	/* 48 */
   "R_Logv",      	/* 49 */
   "R_Deadlocked",	/* 50 */
 };

char *PrintRcode(int);
int DIR_read_default_action(char*);

#ifdef COMPILETIME
/* this code is in common between the run-time part and the translation-time part */
int query_rcode_array(int, int *, int *, int *);
#endif
#endif  /*  R_CODE_H */
