/*
 *     File: rl.h
 *
 *     Description: definitions common to the RL source files.
 *
 *     By  Eidon@tutanota.com
 */
#ifndef     __R_L_Hdr__
#define     __R_L_Hdr__

/* entities are identified by a couple
		( role , id )
   where  role  is a bit pattern and  id  is an integer.

   Entity:       Fulfilled condition:      Which entity:

   G{NUMBER}     isnormalgroup(role)       id == {NUMBER}
   N{NUMBER}     isnormalnode(role)        id == {NUMBER}
   T{NUMBER}     isnormalthread(role)      id == {NUMBER}
      *          isstar(role)                    -
     G*          isgroupstar(role)               -
     N*          isnodestar(role)                -
     T*          isthreadstar(role)              -
   G${NUMBER}    isgroupdollar(role)       id == {NUMBER}
   N${NUMBER}    isnodedollar(role)        id == {NUMBER}
   T${NUMBER}    isthreaddollar(role)      id == {NUMBER}
     G$$         isgroupdollars(role)            -
     N$$         isnodedollars(role)             -
     T$$         isthreaddollars(role)           -
 */
#define ID_GROUP        0001  /* 0-th bit is on */
#define ID_THREAD       0002  /* bit 1 */
#define ID_NODE         0004  /* bit 2 */
#define ID_ENTITY       0010  /* bit 3 */
#define ID_NORMAL       0020  /* bit 4 */
#define ID_STAR         0040  /* bit 5 */
#define ID_DOLLAR       0100  /* bit 6 */
#define ID_DOLLARS      0200  /* bit 7 */
#define ID_IDENTIFIER   0400  /* bit 8 */

#define isstar(e)    ( (e & ID_STAR) && (e & ID_ENTITY) )

#define isgroup(e)          ( e & ID_GROUP )
#define isthread(e)         ( e & ID_THREAD )
#define isnode(e)           ( e & ID_NODE )

#define isnormalgroup(e)          ( (e & ID_NORMAL) && (e & ID_GROUP) )
#define isnormalthread(e)         ( (e & ID_NORMAL) && (e & ID_THREAD) )
#define isnormalnode(e)           ( (e & ID_NORMAL) && (e & ID_NODE) )

#define isgroupstar(e)      ( (e & ID_STAR) && (e & ID_GROUP) )
#define isthreadstar(e)     ( (e & ID_STAR) && (e & ID_THREAD) )
#define isnodestar(e)       ( (e & ID_STAR) && (e & ID_NODE) )

#define isgroupdollar(e)    ( (e & ID_DOLLAR) && (e & ID_GROUP) )
#define isthreaddollar(e)   ( (e & ID_DOLLAR) && (e & ID_THREAD) )
#define isnodedollar(e)     ( (e & ID_DOLLAR) && (e & ID_NODE) )

#define isgroupdollars(e)   ( (e & ID_DOLLARS) && (e & ID_GROUP) )
#define isthreaddollars(e)  ( (e & ID_DOLLARS) && (e & ID_THREAD) )
#define isnodedollars(e)    ( (e & ID_DOLLARS) && (e & ID_NODE) )


/* max number of Boolean atoms in an IF statement */
#define BOOLE_ATOMS 32

#define MAX_NODES 8 /* max. number of nodes allowed is MAX_NODES */

#define MAX_TASKS 128  /* max number of tasks allowed to be identified by the Backbone */

#define MAX_TASKS_IN_LOGICAL 16  /* max number of tasks allowed in a logical */

#define MAX_LOGICALS 32  /* max number of task groups, a.k.a. logicals */

#define TASK_MAXNAME 128 /* max name of a task */

struct logical_t;
struct task_t;

typedef struct {
		int identifier;
		int node;
		int idf;
		int mbox;
		int alias;
		char *name;
	} task_t;

typedef struct {
		int *p_tasklist;
		int card;
		char *name;
	} logical_t;

typedef struct { float k, threshold;
		 char used;
	} alphacount_t;

#endif  /*  __R_L_Hdr__ */
