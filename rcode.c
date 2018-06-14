/*******************************************************************************
**
********************************************************************************
** Description  :    Functions to generate the rcode-equivalent of an 
**                   Ariel strategy file.
** Created By   :    Eidon@tutanota.com
**
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rcode.h"
#include "rl.h"
#define MAX_TASKS_IN_SYSTEM 30 

#define Equal                 1
#define NotEqual              2
#define GreaterThan           3
#define GreaterThanOrEqualTo  4
#define LessThan              5
#define LessThanOrEqualTo     6


extern char verbose;

#define  DEST   1

static char   *rmsgs[RCODE_MAX_CARD];
static int     pc;
char *rcode2ascii(int);

#   define FRST   1
#   define CMPR  <=
#   define ADJUST 1

/*****************************************************************************/
/*      generates the R_STOP R-code, which closes an R-code object file      */
/*****************************************************************************/
int rcode_stop()
{
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = R_STOP;
     compile_time_rcode[pc][1] = -1;
     compile_time_rcode[pc][2] = -1;
     pc++;
     return pc;
  }
  return 0;
}


/*****************************************************************************/
/*    generates the R_SET_ROLE R-code, to set the role of a specific node    */
/*****************************************************************************/
int rcode_set_role(int who, int what)
{
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = R_SET_ROLE;
     compile_time_rcode[pc][1] = who;
     compile_time_rcode[pc][2] = what;
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*    generates the R_SET_DEF_ACT R-code, to set the default action either   */
/*    to R_DA_IS_KILL or to R_DA_IS_RESTART.                                 */
/*****************************************************************************/
int rcode_set_defaction(int what)
{
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = R_SET_DEF_ACT;
     compile_time_rcode[pc][1] = what;
     compile_time_rcode[pc][2] = -1;
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*      generates the R_FALSE R-code, corresponding to the beginning of      */
/*      `if/then/elif/else/fi' statements.                                   */
/*****************************************************************************/
int rcode_if()
{

  if (pc < RCODE_MAX_CARD)
  {
     if (iftop == NULL)        /* top-level if  */
     {
	 iftop = &ifs[0];
	 ifp = 0;
     }
     else                      /* non top-level if */
     {
	 ifp++;
	 iftop = &ifs[ifp];
     }

     iftop->gotos = NULL;
     iftop->pc = pc;

     compile_time_rcode[pc][0]     = R_FALSE;   /* new R_FALSE statement */
     compile_time_rcode[pc][DEST]  = 0;         /* goto value initially set to 0 */
     compile_time_rcode[pc][2]     = -1;        /* unused */

     if (debug)
	     printf("\t\t\t (FALSE added, (pc==%d))\n", pc);
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*      generates the R_GOTO R-code (unconditioned branch). It is used to    */
/*      jump to the next `elif' or the `else' part of an `if' statement.     */
/*****************************************************************************/
int rcode_goto(int where)
{
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = R_GOTO;
     compile_time_rcode[pc][1] = where;
     compile_time_rcode[pc][2] = -1;
     if (debug)
	     printf("\t\t\t (GOTO added, (pc==%d))\n", pc);
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*   generates the R_GOTO R-code statement which closes the current section  */
/*   of an `if' statement, together with the R_FALSE R-code which starts a   */
/*   new `elif' section.                                                     */
/*****************************************************************************/
int rcode_elif(int goto_address)
{
/* when we encounter an `elif' statement, three things have to be managed:

   1. first, a goto_t block has to be allocated and linked to the list, and
      an incomplete goto statement has to be added to rcode list;
   2. secondly, compile_time_rcode[iftop->pc][3] i.e., the running value, has to be updated
      with the current value of the pc register;
   3. third, a new `false' statement has to added, and iftop->pc should be 
      updated with the current value of the pc register.
 */

   struct goto_t *gt;

/* 1. */

   gt = (struct goto_t *)malloc(sizeof(struct goto_t));

   gt->pc     = goto_address-1;/* points to the current statement (goto) */

   gt->next   = iftop->gotos;  /* the goto block is added to the list */
   iftop->gotos = gt;

/* 2. & 3. */
				  /* complete the previous R_FALSE statement */
   if (debug)
	   printf("\t\t\t FALSE at line %d updated to %d-%d==%d\n", 
		iftop->pc, goto_address, iftop->pc, goto_address-iftop->pc);

   compile_time_rcode[iftop->pc][DEST]  = goto_address - iftop->pc;
   iftop->pc               = pc;  /* update it */ 

   if (pc < RCODE_MAX_CARD)    /* add another R_FALSE */
   {
      compile_time_rcode[pc][0] = R_FALSE;
      compile_time_rcode[pc][DEST] = 0;     /* running value */
      compile_time_rcode[pc][2] = -1;        /* unused */
      if (debug)
	      printf("\t\t\t (FALSE added, (pc==%d))\n", pc);

      pc++;
      return pc;
   }

   return 0;
}


/*****************************************************************************/
/*      generates the R_GOTO R-code, corresponding to the beginning of       */
/*      an `else' section of an `if' statement. Ends the R_FALSE R-code      */
/*      R-code corresponding to previous section.                            */
/*****************************************************************************/
int rcode_else(int goto_address)
{
/* when we encounter an `else' statement, two things have to be managed:

   1. first, a goto_t block has to be allocated and linked to the list, and
      an incomplete goto statement has to be added to rcode list;
   2. secondly, compile_time_rcode[iftop->pc][3] i.e., the running value, has to be updated
      with the current value of the pc register;

   No new `false' statement has to be added, nor iftop->pc should be 
   updated with the current value of the pc register, like it was with 
   rcode_elif().
 */

   struct goto_t *gt;


/* 1. */

   gt = (struct goto_t *)malloc(sizeof(struct goto_t));

   gt->pc     = goto_address-1;/* points to the current statement (goto) */

   gt->next   = iftop->gotos;  /* the goto block is added to the list */
   iftop->gotos = gt;

/* 2. & 3. */
			       /* complete the previous R_FALSE statement */
   if (debug)
	   printf("\t\t\t FALSE at line %d updated to %d-%d==%d\n", 
		iftop->pc, goto_address, iftop->pc, goto_address-iftop->pc);

   compile_time_rcode[iftop->pc][DEST]  = goto_address - iftop->pc;

   /* compile_time_rcode[iftop->pc][DEST] = pc - iftop->pc; */
   iftop->pc = pc;             /* update it */ 

   return pc;
}

/*****************************************************************************/
/*      ends the pending R_GOTO's, corresponding to all sections of the      */
/*      current `if'. Ends also the top R_FALSE.                             */
/*****************************************************************************/
int rcode_fi()
{
/* Once we encounter a `fi' it obviously means that a whole `if' statement 
   is over; when this happens we still have to adjust the whole list of goto's
   so to point to this statement.

   When everything's done for each and every goto, then it's time to pop 
   the `if' statement off the stack. When we reach the bottom we also have 
   to reset iftop to NULL.
 */
   struct goto_t *gt;
   int once_at_least = 0;

   while ( (gt = iftop->gotos) )
   {
      once_at_least = 1;
      if (debug)
      {
	      printf("\t\t\tCompleting goto at address no.%d\n", gt->pc);
	      printf("\t\t\t goto %d - %d == %d\n", pc, gt->pc, pc - gt->pc);
      }
      compile_time_rcode[ gt->pc ][1] = pc - gt->pc;
      iftop->gotos = gt->next;    /* advance to next item in the list */
      free(gt);
   }


   /* complete the previous R_FALSE statement */

   if ( compile_time_rcode[iftop->pc][0] == R_FALSE && compile_time_rcode[iftop->pc][DEST] == 0 )
   {
	   if (debug)
	   {
		   printf("\t\t\tCompleting IF at address no.%d\n", iftop->pc);
		   printf("\t\t\t IF %d - %d == %d \n", pc, iftop->pc, pc-iftop->pc);
	   }
	   compile_time_rcode[iftop->pc][DEST] = pc     -iftop->pc;
   }

   /* anew iftop and the ifp stack pointer */
   if (ifp>0)
   {
       ifp--;
       iftop = &ifs[ifp];
   }
   else
   {
       iftop = &ifs[0];
       ifp = 0;
   }

   return pc;
}

/*****************************************************************************/
/*    flushes the R-codes to the object file, resets counters accordingly    */
/*****************************************************************************/
int rflush()
{
   int i, n;
   extern FILE *f;
   extern char *ifname, *ofname;
   static FILE *v;
   static int line;

   n = fwrite( (void*) &compile_time_rcode[0][0], sizeof(rcode_t), pc, f);
   fflush(f);
   /*
   if (!verbose) 
   {
	pc = 0;         / * reset the program counter * /
   printf("within rflush, not in verbose mode.\n");
	return (n);
   }
   */

   if (v==NULL)
   {
	 v = fopen(OUTPUT_RCODE, "w");
	 if (v==NULL)
	 {
	     fprintf(stderr, "can't open file %s --- switching to stderr\n",
			      OUTPUT_RCODE);
	     v = stderr;
	 }

	 fprintf(v, "Art translated Ariel strategy file: .... %s\n", ifname);
	 fprintf(v, "into rcode object file : ............... %s\n\n", ofname);
	 fprintf(v, "%5s %20s %8s %6s\n", "line", "rcode", 
		    "opn1", "opn2");
	 fprintf(v, "-----------------------------------------------\n");
   }


   for (i=0; i<pc; i++)
   {
       int rc, role;

       fprintf(v, "%05d %20s ", 
	 line++, rcode2ascii(compile_time_rcode[i][0]));

       rc = compile_time_rcode[i][0], role = compile_time_rcode[i][1];

       /*
       if (rc == R_SEND)
       {
	if ( isnormalthread(role) )
	     fprintf(v, "Thread   %6d\tmsg(%d)", compile_time_rcode[i][1], role);
	else if ( isnormalnode(role) )
	     fprintf(v, "Node     %6d\tmsg(%d)", compile_time_rcode[i][1], role);
	else if ( isthreaddollar(role) )
	     fprintf(v, "%8s %6s(%d)\tmsg(%d)", " Thread", "@line", -compile_time_rcode[i][2], role);
	else if ( isnodedollar(role) )
	     fprintf(v, "%8s %6s(%d)\tmsg(%d)", " Node", "@line", -compile_time_rcode[i][2], role);
	else if ( isthreaddollars(role) )
	     fprintf(v, "%8s %6s(%d)\tmsg(%d)", " Thread", "@line", -compile_time_rcode[i][2], role);
	else if ( isnodedollars(role) )
	     fprintf(v, "%8s %6s(%d)\tmsg(%d)", " Node", "@line", -compile_time_rcode[i][2], role);

	fprintf(v, "\n");
	     continue;
       }
       */
       if (rc == R_CONST)
	{
		fprintf(v, "%6d\n", compile_time_rcode[i][1]);
		continue;
	}
       else
       if (rc == R_SET)
	{
		fprintf(v, "     %c\n", 'A' + compile_time_rcode[i][1]);
		continue;
	}
       else
       if (rc == R_GET)
	{
		fprintf(v, "     %c\n", 'A' + compile_time_rcode[i][1]);
		continue;
	}
       else
       if (rc == R_STRVAL)
	{
		fprintf(v, "     %c\n", 'A' + compile_time_rcode[i][1]);
		continue;
	}
       else
       if (rc == R_STRPHASE)
       {
	     fprintf(v, "%8s %6d\n", " Thread", compile_time_rcode[i][1]);
	     continue;
       }
       else
       if (rc == R_DEADLOCKED)
       {
	     fprintf(v, "  Threads (%d, %d)\n", role, compile_time_rcode[i][2]);
	     continue;
       }
       else
       if (rc == R_COMPARE)
       {
	switch (role)
	{
	  case Equal :
		fprintf(v, "%8s ", " =="); break;
	  case NotEqual:
		fprintf(v, "%8s ", " !="); break;
	  case GreaterThan :
		fprintf(v, "%8s ", "  >"); break;
	  case GreaterThanOrEqualTo :
		fprintf(v, "%8s ", " >="); break;
	  case LessThan :
		fprintf(v, "%8s ", "  <"); break;
	  case LessThanOrEqualTo :
		fprintf(v, "%8s ", " <="); break;
	}
       }
       else if ( is1operandrcode(rc) )
       {
		fprintf(v, "\n", role);
		continue;
       }
       else if ( issetrcode(rc) || is2operandrcode(rc) )
       {
		fprintf(v, "%6d ", role);
		if (rc==R_SET_ROLE)
		   switch (compile_time_rcode[i][2])
		   {
		   case R_AGENT: fprintf(v, "   Agent\n");break;
		   case R_BACKUP: fprintf(v, "Assistant\n");break;
		   default: fprintf(v, "   Manager\n");
		   }
		else
		   fprintf(v, "\n");
		continue;
       }
       else if ( istestrcode(rc) ||  isactionrcode(rc) || rc == R_SEND)
       {
	if      (role == -1)
	     fprintf(v, "%8s %6s(%d)", " those", "fulfilling", compile_time_rcode[i][2]+1);
	else if ( isnormalgroup(role) )
	     fprintf(v, "%8s %6d", " Group", compile_time_rcode[i][2]);
	else if ( isnormalthread(role) )
	     fprintf(v, "%8s %6d", " Thread", compile_time_rcode[i][2]);
	else if ( isnormalnode(role) )
	     fprintf(v, "%8s %6d", " Node", compile_time_rcode[i][2]);
	else if ( isstar(role) )
	     fprintf(v, "%8s ", "  *");
	else if ( isgroupstar(role) )
	     fprintf(v, "%8s %6s", " Group", "*");
	else if ( isthreadstar(role) )
	     fprintf(v, "%8s %6s", " Thread", "*");
	else if ( isnodestar(role) )
	     fprintf(v, "%8s %6s", " Node", "*");
	else if ( isgroupdollar(role) )
	     fprintf(v, "%8s %6s(%d)", " Group", "@line", -compile_time_rcode[i][2]);
	else if ( isthreaddollar(role) )
	     fprintf(v, "%8s %6s(%d)", " Thread", "@line", -compile_time_rcode[i][2]);
	else if ( isnodedollar(role) )
	     fprintf(v, "%8s %6s(%d)", " Node", "@line", -compile_time_rcode[i][2]);
	else if ( isgroupdollars(role) )
	     fprintf(v, "%8s %6s(%d)", " Group", "@line", -compile_time_rcode[i][2]);
	else if ( isthreaddollars(role) )
	     fprintf(v, "%8s %6s(%d)", " Thread", "@line", -compile_time_rcode[i][2]);
	else if ( isnodedollars(role) )
	     fprintf(v, "%8s %6s(%d)", " Node", "@line", -compile_time_rcode[i][2]);

	fprintf(v, "\n");
	continue;
       }

       if (compile_time_rcode[i][2] > 0)
	 fprintf(v, "%6d", compile_time_rcode[i][2]);

       fprintf(v, "\n");
   }
   fflush(v);

   return (n);
}

/*****************************************************************************/
/*          creates the header file for the static version of rint           */
/*****************************************************************************/
int hflush(FILE *h, char *rlfile)
{
   int i, n, m;
   char *rcode2define(int);
   char *p;
   char line[80];
   extern int numtask[];


   fprintf(h, " %s\n %s\n %s\n %s\n %s%13s%s\n %s%16s%s%36s%s\n %s\n %s\n %s\n\n"
    ,"/********************************************************************************"
    ," *                                                                              *"
    ," *  Header file trl.h                                                           *"
    ," *                                                                              *"
    ," *  This file contains a preloaded set of r-codes for Ariel file ",  rlfile,    "  *"
    ," *  Written by art (", VERSION, ") on ",               today(),              "  *"
    ," *  (c) Eidon@tutanota.com (https://github.com/Eidonko)    .                    *"
    ," *                                                                              *"
    ," ********************************************************************************/"
   );

   fprintf(h, "#ifndef _T_R_L__H_\n");
   fprintf(h, "#define _T_R_L__H_\n\n");


   fprintf(h, "#include \"rcode.h\"\n\n");
   fprintf(h, "#define RCODE_CARD %d /* number of rcodes that have been produced */\n\n", pc);

   fprintf(h, "\nstatic rcode_t rcodes[] = {\n\n");
   fprintf(h, "/*line#*/\t   /* opcode */     /* operand 1 */  /* operand 2 */\n\n");

   for (i=0; i<pc; i++)
   {
    p = rcode2define(compile_time_rcode[i][0]);
    n = strlen(p);
    m = 20-n;
    fprintf(h, "/*%d*/\t\t{ %s, ", i, p);
    while (m-- > 0)    fprintf(h, " ");
    /* if (compile_time_rcode[i][1]>0) fprintf(h, " "); */
    fprintf(h, "%4d, ", compile_time_rcode[i][1]);
    /* if (compile_time_rcode[i][2]>0) fprintf(h, " "); */
    fprintf(h, "           %4d },\n", compile_time_rcode[i][2]);
   }
   fprintf(h, "};\n\n");

   fprintf(h, "static int  num_tasks[] = {\n\n");
   for (i=0; i<MAX_NODES; i++)
	fprintf(h, "\t/* node %d is to be loaded with... */ %d, /* tasks */\n", i, numtask[i]);
   fprintf(h, "};\n\n");

   fprintf(h, "#endif /* _T_R_L__H_ */\n");
}

/*****************************************************************************/
/*          creates the header file with the timeout values
/*****************************************************************************/
int timeout_flush(FILE *h, char *rlfile)
{
   int i, n, m;
   char *rcode2define(int);
   char *p;
   char line[80];
   extern int nprocs;
   extern int mia_send_timeout;
   extern int taia_send_timeout;
   extern int imalive_clear_timeout;
   extern int mia_recv_timeout;
   extern int taia_recv_timeout;
   extern int imalive_set_timeout;
   extern int request_db_timeout;
   extern int reply_db_timeout;
   extern int teif_timeout;
   extern int mid_timeout;


   fprintf(h, " %s\n %s\n %s\n %s\n %s\n %s%16s%s%37s%s\n %s\n %s\n %s\n\n"
    ,"/********************************************************************************"
    ," *                                                                              *"
    ," *  Header file timeouts.h                                                      *"
    ," *                                                                              *"
    ," *  This file contains a set of timeout values for the TIRAN Backbone           *"
    ," *  Written by art (", VERSION, ") on ",               today(),               " *"
    ," *  (c) Eidon@tutanota.com (https://github.com/Eidonko)    .                    *"
    ," *                                                                              *"
    ," ********************************************************************************/"
   );

   fprintf(h, "#ifndef _T_M_O__H_\n");
   fprintf(h, "#define _T_M_O__H_\n\n");


   fprintf(h, "\n\n/* Number of available nodes\n */\n");
   fprintf(h, "#define MAX_PROCS                 %d\n", nprocs);

   fprintf(h, "\nchar crashed[MAX_PROCS+2]; /* initially all nodes are considered as non-crashed */\n");

   fprintf(h, "\n\n/* maximum period between two clear(IA-flag)'s\n */\n");
   fprintf(h, "#define IMALIVE_CLEAR_TIMEOUT     %d\n", imalive_clear_timeout);

   fprintf(h, "\n\n/* maximum period between sending two MIA's (manager-to-assistants)\n */\n");
   fprintf(h, "#define MIA_SEND_TIMEOUT          %d\n", mia_send_timeout);

   fprintf(h, "\n\n/* maximum period between sending two TAIA's (assistant-to-manager)\n */\n");
   fprintf(h, "#define TAIA_SEND_TIMEOUT         %d\n", taia_send_timeout);

   fprintf(h, "\n\n\n/* maximum suspicion period\n */\n");
   fprintf(h, "#define IMALIVE_SET_TIMEOUT       %d\n", imalive_set_timeout);

   fprintf(h, "\n\n/* maximum period between two receiving two MIA's (manager-to-assistants)\n */\n");
   fprintf(h, "#define MIA_RECV_TIMEOUT          %d\n", mia_recv_timeout);

   fprintf(h, "\n\n/* maximum period between two receiving two TAIA's (assistant-to-manager)\n */\n");
   fprintf(h, "#define TAIA_RECV_TIMEOUT         %d\n", taia_recv_timeout);

   fprintf(h, "\n\n/* after this time a suspected node is assumed to be faulty\n */\n");
   fprintf(h, "#define TEIF_TIMEOUT              %d\n", teif_timeout);

   fprintf(h, "\n\n/* if a new manager is not spawned within this time, no more\n");
   fprintf(h, "   managers can be spawned on that node, so that node has to be\n");
   fprintf(h, "   removed from the list of those active.\n */\n");
   fprintf(h, "#define MANAGER_IS_DOWN_TIMEOUT   %d\n", mid_timeout);

   fprintf(h, "\n\n/* maximum time to send a REQUEST_DB message\n */\n");
   fprintf(h, "#define REQUEST_DB_TIMEOUT        %d\n", request_db_timeout);

   fprintf(h, "\n\n/* maximum delay for a response to a REPLY_DB message\n */\n");
   fprintf(h, "#define REPLY_DB_TIMEOUT          %d\n", reply_db_timeout);

   fprintf(h, "\n#endif /* _T_M_O__H_ */\n");
}

/*****************************************************************************/
/*      creates the header file with the mapping identifier <-> node, IDF    */
/*****************************************************************************/
int ident_flush(FILE *h, char *rlfile)
{
   int i, node, n, m, z, w;
   char *rcode2define(int);
   char *p;
   char line[80];
   extern int    maxtask, maxtaskII;
   extern task_t taskram[];
   extern int nprocs;
   int idcmp(const void*, const void*);
   int texcmp(const void*, const void*);


   fprintf(h, " %s\n %s\n %s\n %s\n %s\n %s\n %s%16s%s%37s%s\n %s\n %s\n %s\n\n"
    ,"/********************************************************************************"
    ," *                                                                              *"
    ," *  Header file identifiers.h                                                   *"
    ," *                                                                              *"
    ," *  This file contains a set of lookup tables to convert a backbone identifier  *"
    ," *  to a TEX couple (node, IDF) and vice-versa.                                 *"
    ," *  Written by art (", VERSION, ") on ",               today(),               " *"
    ," *  (c) Eidon@tutanota.com (https://github.com/Eidonko)    .                    *"
    ," *                                                                              *"
    ," ********************************************************************************/"
   );

   fprintf(h, "#ifndef _I_D_F__H_\n");
   fprintf(h, "#define _I_D_F__H_\n\n");

   /* taskram holds a full set of task_t objects, ranging from 0 to task_sp-1 */

   /* first, we order taskram according to field "identifier": */
   // qsort( (void*)taskram, maxtask, sizeof(task_t), idcmp );
   fprintf(h, "typedef struct { int node; int idf; int mbox; int alias; } couple_t;\n\n");
   fprintf(h, "static couple_t global_to_local[%d] = {\n", maxtask+1);

   for (i=0; i<=maxtask; i++)
   {
	if (taskram[i].identifier == -1)
	{
		fprintf(h, "\t/* %3d */\t{ %d, %d, %d, %d },\t\t/* unused */\n",
			i,-1,-1,-1,-1);
	}
	else
	{
		fprintf(h, "\t/* %3d */\t{ %d, %d, %d, %d },\n", 
		    i, taskram[i].node, taskram[i].idf, taskram[i].mbox,
		    taskram[i].alias );
	}
   }

   fprintf(h, "};\t\t/* global to local */\n");

   if(i==MAX_TASKS)
   {
	fprintf(stderr, "too large values for global identifiers -- should be less than MAX_TASKS.\n");
   }

   /* then, we order taskram according to fields "node" (primary key) and
      "idf" (secondary key) */
   qsort( (void*)taskram, maxtask, sizeof(task_t), texcmp );

   for (n=i=0; i<MAX_NODES; i++)
        if (n < numtask[i])
                n = numtask[i];

   fprintf(h, "\nstatic int local_to_global[MAX_PROCS+1][%d] = {\n", n);

	for (i=0; i<n; i++)
		if (taskram[i].identifier != -1)
			break;

	for (node=0; i<=nprocs; i++)
	{
		while (taskram[i].node > node)
		{
			fprintf(f, "\t{ ");
			for (j=0; j<n; j++)
				fprintf(f, " -1,");
			fprintf(f, "},\n");
			node++;
		}

	redo:
		/* node == taskram[i].node */
		fprintf(f, "\t{ ");

		for (j=0; taskram[i].node == node && j<n; j++)
			if (taskram[i].idf == j)
			{
				fprintf(f, "%3d,", taskram[i].identifier);
				i++;
			}
			else
				fprintf(f, " -1,");

		for (; j<n; j++)
			fprintf(f, " -1,");


/*
		printf("\n\t\ttaskram[%d].node == %d, node == %d, t == %d, u == %d\n",
			i, taskram[i].node, node, taskram[i].idf, taskram[i].identifier);
 */
 

		fprintf(f, "}, \n");
		node++;

		if (taskram[i].node == node)
			goto redo;
	}

   fprintf(h, "};\t\t/* local_to_global */\n");
   fprintf(h, "\nstatic int TEXTaskNum() { return %d; }\n", maxtask+1);
   fprintf(h, "\n#endif /* _I_D_F__H_ */\n");
}


/*****************************************************************************/
/*  creates the basic services library file with the definition of logicals
/*****************************************************************************/
int bsl_logical_flush(FILE *h)
{
   int i, j, k, n, *p;
   extern int    maxlogical;
   extern logical_t logicalram[];
   int intcmp(const void*, const void*);


   fprintf(h, "0,Dummy\n");
   for (i=0; i<MAX_TASKS_IN_SYSTEM; i++)
	fprintf(h, "0,");
   fprintf(h, "\n");
   for (i=1; i<=maxlogical; i++)
   {
	n = logicalram[i].card;

	if (n != -1)
	{
		fprintf(h, "%d,%s\n", i, logicalram[i].name);
		free(logicalram[i].name);

		qsort( (void*)logicalram[i].p_tasklist,
                       n, sizeof(int), intcmp );

		if (n<MAX_TASKS_IN_SYSTEM)
		    n=MAX_TASKS_IN_SYSTEM;

		for (j=k=0, p=logicalram[i].p_tasklist; 
		     j<n; j++)
		{
			if (j == p[k])
			{
				fprintf(h, "1,");
				k++;
			}
			else
			{
				fprintf(h, "0,");
			}
		}
   
		free(logicalram[i].p_tasklist);
		fprintf(h, "\n");
	}
   }
   return 0;
}


/*****************************************************************************/
/*    creates the basic services library file with the definition of tasks   */
/*****************************************************************************/
int bsl_task_flush(FILE *h)
{
   int i, max;
   extern int    maxtask, maxtaskII;
   extern task_t taskram[];
   int idcmp(const void*, const void*);


   max = (maxtask > maxtaskII)? maxtask : maxtaskII;

   /* first, we order taskram according to field "identifier": */
   qsort( (void*)taskram, max+1, sizeof(task_t), idcmp );

   fprintf(h, "0,Dummy,99,1\n"); /* required by the bsl */
   for (i=1; i<=max; i++)
   {
	if (taskram[i].identifier == -1) continue;
	fprintf(h, "%d,%s,%d,%d\n", taskram[i].identifier,
		taskram[i].name, taskram[i].idf, taskram[i].node);
	if (taskram[i].name != '\0')
		free(taskram[i].name);
   }
   return 0;
}

/*****************************************************************************/
/*  creates the header file defining the parameters of the alphacount filter */
/*****************************************************************************/
int alpha_flush(FILE *h)
{
   int i;
   extern alphacount_t alphas[];


   fprintf(h, " %s\n %s\n %s\n %s\n %s\n %s\n %s%16s%s%37s%s\n %s\n %s\n %s\n\n"
    ,"/********************************************************************************"
    ," *                                                                              *"
    ," *  Header file alphacount.h                                                    *"
    ," *                                                                              *"
    ," *  This file contains a preloaded array with the parameters of the alphacount  *"
    ," *  filter (factor and threshold )                                              *"
    ," *  Written by art (", VERSION, ") on ",               today(),               " *"
    ," *  (c) Eidon@tutanota.com (https://github.com/Eidonko)    .                    *"
    ," *                                                                              *"
    ," ********************************************************************************/"
   );

   fprintf(h, "#ifndef     __ALPHA_COUNT__\n");
   fprintf(h, "#define     __ALPHA_COUNT__\n");
   fprintf(h, "\n#include \"DB.h\"\n\n");
   fprintf(h, "alphacount_t alphas[%d] = {\n", MAX_TASKS);

   for (i=0; i<MAX_TASKS; i++)
   {
	fprintf(h, "\t\t{ %f, %f, %d }, /* entry %d */\n",
		alphas[i].k, alphas[i].threshold, alphas[i].used, i);
   }
   fprintf(h, "\t};\n");
   fprintf(h, "#endif  /*  __ALPHA_COUNT__  */\n");
   return 0;
}

int rcode_twoargs (int opcode, int arg)
{
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = opcode;
     compile_time_rcode[pc][1] = arg;
     compile_time_rcode[pc][2] = -1;
     pc++;
     return pc;
  }
  return 0;
}
/*****************************************************************************/
/*      generates an R-code amongst R_KILLED, R_RESTARTED,                   */
/*      R_PRESENT, R_FAULTY, R_FIRED, and R_ISOLATED.                        */
/*****************************************************************************/
int rcode_status(int status, ident_t id, int ord)
{
  /* ord is the position of `status' in the boolean atoms list */
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = R_PUSH;
     compile_time_rcode[pc][1] = ord;
     compile_time_rcode[pc][2] = -1;
     pc++;
  }

  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = status;
     compile_time_rcode[pc][1] = (int) id.role;
     compile_time_rcode[pc][2] = id.id;
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*           generates an R-code amongst R_AND, R_OR, and R_NOT.             */
/*****************************************************************************/
int rcode_single_op(int code)
{
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = code;
     compile_time_rcode[pc][1] = -1;
     compile_time_rcode[pc][2] = -1;
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*         generates the R-code corresponding to a recovery action.          */
/*****************************************************************************/
int rcode_raction(int action, unsigned int role, int id)
{
  if (pc < RCODE_MAX_CARD)
  {
     if (debug)
	     printf("\t\t\t(rcode_action: %d %d %d)\n", action, role, id);

     compile_time_rcode[pc][0] = action;
     compile_time_rcode[pc][1] = (int) role;
     compile_time_rcode[pc][2] = id;
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*  generates the R-code corresponding to checking whether a certain action  */
/*  did affect a certain entity.                                             */
/*****************************************************************************/
int rcode_check_action(int action, unsigned int role, int id)
{
  /* actually no r-code is generated; on the contrary, a negative
     value equal to    -action -1  is put in the place where we expect
     r-codes. This `tricky' way of doing has been adopted for the sake
     of keeping the column-format of an r-code to 3.
   */
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = - action - 1;
     compile_time_rcode[pc][1] = (int) role;
     compile_time_rcode[pc][2] = id;
     pc++;
     return pc;
  }
  return 0;
}

/*****************************************************************************/
/*  R-code to ascii conversion routine. Returns NULL if the code is unknown. */
/*****************************************************************************/
char *rcode2ascii(int r)
{
  static char string[80];

  switch(r)
  {
   case R_SET_ROLE:         return "SET_ROLE";
   case R_SET_DEF_ACT:      return "SET_DEFAULT_ACTION";

   case R_KILLED:           return "...KILLED";
   case R_RESTARTED:        return "...RESTARTED";
   case R_PRESENT:          return "...PRESENT";
   case R_ISOLATED:         return "...ISOLATED";
   case R_FAULTY:           return "...FAULTY";
   case R_FIRED:            return "...FIRED";
   case R_REINTEGRATED:     return "...REINTEGRATED";

   case R_CLEAR:            return "CLEAR";
   case R_KILL:             return "KILL";
   case R_WARN:             return "...WARN";
   case R_ANDWARN:          return "...AND_WARN";
   case R_START:            return "START";
   case R_RESTART:          return "RESTART";
   case R_CHKERR:           return "IF_ERRORS_ON...";
   case R_PUSH:             return "PUSH...";
   case R_REMOVE:           return "...REMOVE_FROM_LIST";
   case R_REMOVE_ALL:       return "REMOVE_FROM_LIST";
   case R_REBOOT:           return "REBOOT";
   case R_ENABLE:           return "ENABLE...";
   case R_FUNCTION_CALL:    return "CALL_FUNCTION#";

   case R_AND:              return "AND";
   case R_OR:               return "OR";
   case R_NOT:              return "NOT";
   case R_OANEW:            return "ANEW_OA_OBJECTS";
   case R_INC_NEST:         return "IF";
   case R_DEC_NEST:         return "FI";

   case R_FALSE:            return "FALSE";

   case R_GOTO:             return "GOTO";
   case R_PAUSE:            return "PAUSE";

   case R_SEND:             return "...SEND";
   case R_STOP:             return "STOP";

   case R_STRERRN:          return "STORE_ERR_NUM...";
   case R_STRERRT:          return "STORE_ERR_TYPE...";
   case R_STRPHASE:         return "STORE_PHASE...";
   case R_COMPARE:          return "...COMPARE";
   case R_DEADLOCKED:       return "DEADLOCK_BETWEEN";
   case R_STRVAL:           return "STORE_VAL";
   case R_CONST:       	    return "CONST";
   case R_GET:       	    return "GET";
   case R_SET:       	    return "SET";
   case R_ADD:       	    return "ADD";
   case R_SUBTRACT:       	    return "SUBTRACT";
   case R_MULTIPLY:       	    return "MULTIPLY";
   case R_DIVIDE:       	    return "DIVIDE";
   case R_COMPLEMENT:       	    return "COMPLEMENT";
   case R_LOGS:             return "LOG_STRING";
   case R_LOGI:             return "LOG_INT";
   case R_LOGC:             return "LOG_CLOCK";
   case R_LOGV:             return "LOG_VARIABLE";

   default:                 return  NULL;
  }
}
/*****************************************************************************/
/*  R-code to ascii conversion routine. Returns NULL if the code is unknown. */
/*****************************************************************************/
char *rcode2define(int r)
{
  static char string[80];

  switch(r)
  {
   case R_SET_ROLE:         return "R_SET_ROLE";
   case R_SET_DEF_ACT:      return "R_SET_DEF_ACT";

   case R_KILLED:           return "R_KILLED";
   case R_RESTARTED:        return "R_RESTARTED";
   case R_PRESENT:          return "R_PRESENT";
   case R_ISOLATED:         return "R_ISOLATED";
   case R_FAULTY:           return "R_FAULTY";
   case R_FIRED:            return "R_FIRED";
   case R_REINTEGRATED:     return "...REINTEGRATED";

   case R_CLEAR:            return "R_CLEAR";
   case R_KILL:             return "R_KILL";
   case R_WARN:             return "R_WARN";
   case R_ANDWARN:          return "R_ANDWARN";
   case R_START:            return "R_START";
   case R_RESTART:          return "R_RESTART";
   case R_CHKERR:           return "R_CHKERR";
   case R_ENABLE:           return "R_ENABLE";
   case R_FUNCTION_CALL:    return "R_FUNCTION_CALL";
   case R_PUSH:             return "R_PUSH";
   case R_REMOVE:           return "R_REMOVE";
   case R_REBOOT:           return "R_REBOOT";
   case R_REMOVE_ALL:       return "R_REMOVE_ALL";
   case R_PAUSE:            return "R_PAUSE";

   case R_AND:              return "R_AND";
   case R_OR:               return "R_OR";
   case R_NOT:              return "R_NOT";
   case R_OANEW:            return "R_OANEW";
   case R_INC_NEST:         return "R_INC_NEST";
   case R_DEC_NEST:         return "R_DEC_NEST";

   case R_FALSE:            return "R_FALSE";

   case R_GOTO:             return "R_GOTO";
   case R_SEND:             return "R_SEND";

   case R_STOP:             return "R_STOP";

   case R_STRERRN:          return "R_STRERRN";
   case R_STRERRT:          return "R_STRERRT";
   case R_STRPHASE:         return "R_STRPHASE";
   case R_COMPARE:          return "R_COMPARE";
   case R_ADD:              return "R_ADD";
   case R_SUBTRACT:         return "R_SUBTRACT";
   case R_MULTIPLY:         return "R_MULTIPLY";
   case R_DIVIDE:           return "R_DIVIDE";
   case R_COMPLEMENT:       return "R_COMPLEMENT";
   case R_DEADLOCKED:       return "R_DEADLOCKED";
   case R_SET:              return "R_SET";
   case R_GET:              return "R_GET";
   case R_CONST:            return "R_CONST";
   case R_STRVAL:           return "R_STRVAL";
   case R_LOGS:             return "R_LOGS";
   case R_LOGI:             return "R_LOGI";
   case R_LOGC:             return "R_LOGC";
   case R_LOGV:             return "R_LOGV";

   default:                 return  NULL;
  }
}

/*****************************************************************************/
/*           generates the two R-codes  R_STRERRN  and  R_COMPARE.           */
/*****************************************************************************/
int rcode_errn(ident_t operand1, int arithm_op, int operand2)
{
/* Translates the ERRN instruction into *two* Rcode's, namely: 
		   STORE_ERRN operand1.role operand1.id
		   COMPARE    arithm_op     operand2
   The first one pushes on the evaluation stack the number of errors
   occurred on  operand1.
   The second rcode pops that number, compares it with operand2
   by operator arithm_pop, and pushes the booolean result onto
   the evaluation stack.
 */
  if (pc < RCODE_MAX_CARD -1)
  {
     compile_time_rcode[pc][0] = R_STRERRN;
     compile_time_rcode[pc][1] = (int) operand1.role;
     compile_time_rcode[pc][2] = operand1.id;
     pc++;

     compile_time_rcode[pc][0] = R_COMPARE;
     compile_time_rcode[pc][1] = arithm_op;
     compile_time_rcode[pc][2] = operand2;
     pc++;
     
     return pc;
  }
  return 0;
}
/*****************************************************************************/
/*           generates the two R-codes  R_STRERRT  and  R_COMPARE.           */
/*****************************************************************************/
int rcode_errt(ident_t operand1, int arithm_op, int operand2)
{
/* Translates the ERRT instruction into *two* Rcode's, namely: 
		   STORE_ERRT operand1.role operand1.id
		   COMPARE    arithm_op     operand2
   The first one pushes on the evaluation stack the type of the error
   occurred on  operand1.
   The second rcode pops that number, compares it with operand2
   by operator arithm_pop, and pushes the booolean result onto
   the evaluation stack.
 */
  if (pc < RCODE_MAX_CARD -1)
  {
     compile_time_rcode[pc][0] = R_STRERRT;
     compile_time_rcode[pc][1] = (int) operand1.role;
     compile_time_rcode[pc][2] = operand1.id;
     pc++;

     compile_time_rcode[pc][0] = R_COMPARE;
     compile_time_rcode[pc][1] = arithm_op;
     compile_time_rcode[pc][2] = operand2;
     pc++;
     
     return pc;
  }
  return 0;
}
/*****************************************************************************/
/*           generates the two R-codes  R_STRERRT  and  R_COMPARE.           */
/*****************************************************************************/
int rcode_phase(ident_t operand1, int arithm_op, int operand2)
{
/* Translates the ERRT instruction into *two* Rcode's, namely: 
		   STORE_PHASE operand1.id
		   COMPARE     arithm_op     operand2
   The first one pushes on the evaluation stack the type of the error
   occurred on  operand1.
   The second rcode pops that number, compares it with operand2
   by operator arithm_pop, and pushes the booolean result onto
   the evaluation stack.
 */
  if (pc < RCODE_MAX_CARD -1)
  {
     compile_time_rcode[pc][0] = R_STRPHASE;
     compile_time_rcode[pc][1] = operand1.id;
     compile_time_rcode[pc][2] = -1;
     pc++;

     compile_time_rcode[pc][0] = R_COMPARE;
     compile_time_rcode[pc][1] = arithm_op;
     compile_time_rcode[pc][2] = operand2;
     pc++;
     
     return pc;
  }
  return 0;
}
int dollarsign_check(unsigned int role_dollar, int old_pc)
{
  unsigned int role_expr = (unsigned) compile_time_rcode[old_pc][1];

  if (role_expr & ID_GROUP && role_dollar & ID_THREAD)
      return 1;
  if (role_expr & ID_NODE  && ! (role_dollar & ID_NODE) )
      return 1;
  return 0;
}
int rcode_oanew(int card)
{
  if (pc < RCODE_MAX_CARD)
  {
     compile_time_rcode[pc][0] = R_OANEW;
     compile_time_rcode[pc][1] = card;
     compile_time_rcode[pc][2] = -1;
     pc++;
     return pc;
  }
  return 0;
}
/*****************************************************************************/
/*****************************************************************************/
int rcode_deadlocked(int id1, int id2)
{
  if (pc < RCODE_MAX_CARD)
  {
     if (debug)
	     printf("\t\t\t(rcode_deadlocked: %d %d)\n", id1, id2);
     compile_time_rcode[pc][0] = R_DEADLOCKED;
     compile_time_rcode[pc][1] = id1;
     compile_time_rcode[pc][2] = id2;
     pc++;
     return pc;
  }
  return 0;
}
int idcmp(const void* v1, const void* v2)
{
	task_t *t1, *t2;
	t1 = (task_t*)v1, t2 = (task_t*)v2;
	if (t1->identifier == -1 && t2->identifier == -1)
		return 0;
	if (t1->identifier == -1)
		return 1;
	if (t2->identifier == -1)
		return -1;
		
	return t1->identifier - t2->identifier;
}
int texcmp(const void* v1, const void* v2)
{
	task_t *t1, *t2;
	t1 = (task_t*)v1, t2 = (task_t*)v2;
	if (t1->node != t2->node)
		return t1->node - t2->node;
	return t1->idf - t2->idf;
}
int intcmp(const void* v1, const void* v2)
{
	int *t1, *t2;
	t1 = (int*)v1, t2 = (int*)v2;
	return *t1 - *t2;
}
/*****************************************************************************/
/*           generates the two R-codes  R_STRERRT  and  R_COMPARE.           */
/*****************************************************************************/
int rcode_compare_val(int operand1, int arithm_op, int operand2)
{
/* Translates the VAL instruction into *two* Rcode's, namely: 
		   STORE_VAL operand1
		   COMPARE     arithm_op     operand2
   The first one pushes on the evaluation stack the value of 
   variable val[operand1].
   The second rcode pops that number, compares it with operand2
   by operator arithm_pop, and pushes the booolean result onto
   the evaluation stack.
 */
  if (pc < RCODE_MAX_CARD -1)
  {
     compile_time_rcode[pc][0] = R_STRVAL;
     compile_time_rcode[pc][1] = operand1;
     compile_time_rcode[pc][2] = -1;
     pc++;

     compile_time_rcode[pc][0] = R_COMPARE;
     compile_time_rcode[pc][1] = arithm_op;
     compile_time_rcode[pc][2] = operand2;
     pc++;
     
     return pc;
  }
  return 0;
}

int exprcode(int operand)
{
}
/* eof rcode.c */
