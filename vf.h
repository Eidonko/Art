#ifndef  _vF_HDr_
#define  _vF_HDr_
/***************************************************************************
 *                                                                         *
 * Voting farm library v1.7                                                *
 *                                                                         *
 * Module name   vf.h                          Purpose   voting algorithms *
 * Author name   Eidon@tutanota.com                Official version    1.7 *
 *                                                                         *
 ***************************************************************************/


/* global definitions */
#define VF_MAX_NTS        16 /* size of stacks $\equiv$ max value for |N| */
#define VOTING_FARMS_MAX  64 /* max number of simultaneous active voting farms */
#define VF_MAX_INPUT_MSG 512 /* max size of an input message */
#define VF_MAX_MSGS       10 /* max size of the message buffer */
#define VF_EVENT_TIMEOUT   6 /* |Select| time-out is 6 seconds */
#define NO                 0
#define YES                1


/* error conditions */
#define E_VF_OVERFLOW     1 
#define E_VF_CANT_ALLOC   2
#define E_VF_UNDEFINED_VF 3
#define E_VF_WRONG_NODE   4
#define E_VF_GETGLOBID    5
#define E_VF_CANT_SPAWN   6   /* |CreateThread| error */
#define E_VF_CANT_CONNECT 7   /* |ConnectLink| error */
#define E_VF_RECVLINK     8   /* |RecvLink| error */
#define E_VF_BROADCAST    9   /* Invalid input message - can't broadcast */
#define E_VF_DELIVER      10  /* Invalid output |LinkCB_t| - can't deliver */
#define E_VF_BUSY_SLOT    11  /* Duplicated input message */
#define E_VF_WRONG_VFID   12
#define E_VF_WRONG_DISTANCE 13
#define E_VF_INVALID_VF   14
#define E_VF_NO_LVOTER    15  /* exactly one voter is mandatorily needed */
#define E_VF_TOO_MANY_LVOTERS 16  /* exactly one voter is mandatorily needed */
#define E_VF_WRONG_MSG_NB 17  /* wrong number of messages */
#define E_VF_SENDLINK     18  /* |SendLink| error */
#define E_VF_INPUT_SIZE   19  /* inconsistency in the size of the input */
#define E_VF_UNDESCRIBED  20  /* undescribed |vf| object */
#define E_VF_INACTIVE     21  /* inactive |vf| object */
#define E_VF_UNKNOWN_SENDER 22 /* inconsistency---sender unknown */
#define E_VF_EVENT_TIMEOUT 23 /* a |Select| reached time-out */
#define E_VF_SELECT       24 /* a |Select| returned an index out of range */
#define E_VF_WRONG_ALGID  25 /* AlgorithmID out of range */
#define E_VF_NULLPTR      26 /* A pointer parameter held |NULL| */
#define E_VF_TOO_MANY     27 /* Too many opened voting farms */
#define E_VF_OA_INSERT    28 /* OA_insert failed */


#define VF_ERROR_NB	     29 /* number of errors, plus one */

 /* voting algorithms */
/* #define VFA_EXACT_CONCENSUS  0 */
#define VFA_MAJORITY         1
#define VFA_MEDIAN           2
#define VFA_PLURALITY        3
#define VFA_WEIGHTED_AVG     4
#define VFA_SIMPLE_MAJORITY  5
#define VFA_SIMPLE_AVERAGE   6


#define VF_NB_ALGS 7

#include <stdio.h>
#include <stdlib.h>
#include <sys/root.h>
#include <sys/logerror.h>
#include <sys/link.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/thread.h>

#ifdef UseServerNet
#include "server.h"
#endif


/* global variables */
int VF_error;     /* global variable for storing the error condition */
char *VFN;        /* function name of the last function entered */


/* new types */
typedef unsigned char flag;
typedef struct { int       vf_id;
		 int      *vf_node_stack;
		 int      *vf_tid_stack;
		 LinkCB_t *pipe[2];
		 int       N;
		 int       user_thread;
		 int       this_voter;
		 double   (*distance) (void*,void*);
                 flag      broadcast_done;
                 flag      inp_msg_got; 
                 flag      destroy_requested;
        } VotingFarm_t;
typedef struct { int code;
                 void *msg;
                 int msglen;
        } VF_msg_t;



/* function prototypes */
int VF_add(VotingFarm_t*,int,int);
void VF_perror(void);
VotingFarm_t *VF_open(int,double (*)(void*,void*));

# ifdef     UseServerNet
int  VF_run(LinkCB_t*, VotingFarm_t*);
# else
int  VF_run(VotingFarm_t*);
# endif  /* UseServerNet */

int VF_control_list(VotingFarm_t*,VF_msg_t*,int);
int VF_control(VotingFarm_t*,VF_msg_t*);
int VF_send(VotingFarm_t*,int,...);
int VF_close(VotingFarm_t*);
VF_msg_t *VF_get(VotingFarm_t*);
VF_msg_t *VFO_Set_Input_Message(void *, size_t);
VF_msg_t *VFO_Set_Scaling_Factor (double*);
VF_msg_t *VFO_Set_Algorithm(int);

double dstrcmp(void*,void*);

/* events */
#define VF_INP_MSG	100
#define VF_OUT_LCB   101
#define VF_SELECT_ALG 102
#define VF_DESTROY 103
#define VF_NOP 104
#define VF_RESET 105
#define VF_REFUSED 106
#define VF_QUIT 107
#define VF_DONE 108
#define VF_EPSILON 109
#define VF_ERROR 110
#define VF_SCALING_FACTOR 111

#define VF_SUCCESS 1
#define VF_FAILURE 0
#endif  /* _vF_HDr_ */
