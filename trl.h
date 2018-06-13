 /********************************************************************************
  *                                                                              *
  *  Header file trl.h                                                           *
  *                                                                              *
  *  This file contains a preloaded set of r-codes for Ariel file        .ariel  *
  *                                                                              *
  ********************************************************************************/

#ifndef _T_R_L__H_
#define _T_R_L__H_

#include "rcode.h"

#define RCODE_CARD 22 /* number of rcodes that have been produced */


rcode_t rcodes[] = {

/*line#*/	   /* opcode */     /* operand 1 */  /* operand 2 */

/*0*/		{ R_SET_ROLE,              1,            3000 },
/*1*/		{ R_SET_ROLE,              2,            2000 },
/*2*/		{ R_SET_DEF_ACT,         666,              -1 },
/*3*/		{ R_INC_NEST,             -1,              -1 },
/*4*/		{ R_STRPHASE,              0,              -1 },
/*5*/		{ R_COMPARE,               1,            9999 },
/*6*/		{ R_STRPHASE,              2,              -1 },
/*7*/		{ R_COMPARE,               1,               1 },
/*8*/		{ R_AND,                  -1,              -1 },
/*9*/		{ R_FALSE,                10,              -1 },
/*10*/		{ R_KILL,                 66,              -4 },
/*11*/		{ R_PUSH,                 66,              -1 },
/*12*/		{ R_SEND,                 18,               3 },
/*13*/		{ R_PUSH,                  0,              -1 },
/*14*/		{ R_SEND,                 18,               3 },
/*15*/		{ R_PUSH,                  3,              -1 },
/*16*/		{ R_SEND,                 18,               1 },
/*17*/		{ R_PUSH,                  3,              -1 },
/*18*/		{ R_SEND,                 18,               2 },
/*19*/		{ R_DEC_NEST,             -1,              -1 },
/*20*/		{ R_OANEW,                 2,              -1 },
/*21*/		{ R_STOP,                 -1,              -1 },
};

int  num_tasks[] = {

	/* node 0 is to be loaded with... */ 0, /* tasks */
	/* node 1 is to be loaded with... */ 11, /* tasks */
	/* node 2 is to be loaded with... */ 10, /* tasks */
	/* node 3 is to be loaded with... */ 0, /* tasks */
	/* node 4 is to be loaded with... */ 0, /* tasks */
	/* node 5 is to be loaded with... */ 0, /* tasks */
	/* node 6 is to be loaded with... */ 0, /* tasks */
	/* node 7 is to be loaded with... */ 0, /* tasks */
};

#endif /* _T_R_L__H_ */
