/*
 *
 *     File: rint.c
 *
 *     Description: Functions to interpret an R-code object file
 *                  (virtual machine running R-codes).
 *
 *     Version: 1.0, by
 *     <a href=ihttps://github.com/Eidonko>Eidon@tutanota.com</a> 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rcode.h"
#include "dirnet.h"
#include "rl.h"
#include "ObjAlloc.h"
#include "BACKBONE_db.h"
#include <sys/time.h>
int kbwait (void);

#ifdef    STATIC
typedef
struct {entity_t     *oa_satisfy[RCODE_MAX_NEST];
} RInt_static_object_t;
  RInt_static_object_t     RInt;
#endif

extern BACKBONE_recovery_t *ra;

#define PRESENT 1    /* the return value of the database routines when an entry is found */
#define ESTACK_SIZE 512
typedef int estack_t;
static estack_t estack[ESTACK_SIZE];
static int  esp;

#define MAX_WARN_ARGC 32

static FILE *f;   /* rcode file pointer */

/* oa_satisfy is a vector of pointers to ObjAlloc objects. They represent
   lists of ident_t objects. If not-NULL, oa_satisfy[i] is the list of
   entities (threads, groups, or nodes) that satisfy the i-th condition
   of the current Ariel IF statement.

   oa_satisfy objects are ``OA_opened'' as conditions are encountered
   during their run-time evaluation, and (possibly) filled in with the
   identities of those entities who satisfy the condition.
   A special rcode, R_OANEW, performs all OA_close's that are needed
   to re-initialise the state for the new IF to be managed.
 */

typedef struct { int role; int id; } entity_t;
ObjAlloc_t  *oa_satistack[RCODE_MAX_NEST][BOOLE_ATOMS];
ObjAlloc_t **oa_satisfy;
int          oa_satisp;
extern rcode_t *rcode;


#define R_INI_ASIZE 5    /* parameters used in OA_open() */
#define R_INC_ASIZE 5

/****** test main
main(int argc, char *argv[])
{
	int  i;
	for (i=1; i<argc; i++)
	{
		rcode(argv[i]);
	}
}
****************/


/**************************************************/
int R_NestIn(int dummy1, int dummy2, int dummy3)
/**************************************************/
{
	oa_satisfy = oa_satistack[oa_satisp++];
	if (oa_satisp < RCODE_MAX_NEST)
		return(1);
	return(0);
}
/**************************************************/
int R_NestOut(int dummy1, int dummy2, int dummy3)
/**************************************************/
{
	oa_satisfy = oa_satistack[--oa_satisp];
	if (oa_satisp < RCODE_MAX_NEST)
		return(1);
	return(0);
}

/**************************************************/
int R_Goto(int displacement, int dummy1, int dummy2)
/**************************************************/
{
	/* RCODE: R_GOTO
	   Ariel:    part of the IF statement */
	return fseek(f, displacement * sizeof(rcode_t), SEEK_CUR);
}

/***************************************************/
int R_False(int displacement, int dummy1, int dummy2)
/***************************************************/
{
	/* RCODE: R_FALSE
	   Ariel:    part of the IF statement */
	if (R_Pop() == FALSE)
		return fseek(f, displacement * sizeof(rcode_t), SEEK_CUR);
	return -1;
}

/***************************************/
int R_Not(int op, int dummy1, int dummy2)
/***************************************/
{
	/* RCODE: R_NOT
	   Ariel:    part of the expressions within IF statements */
	int t = R_Pop();
	R_Push( ! t );
	return t;
}

/***************************************/
int R_Or(int op1, int dummy1, int dummy2)
/***************************************/
{
	/* RCODE: R_OR
	   Ariel:    part of the expressions within IF statements */
	int t = R_Pop();
	int s = R_Pop();
	R_Push( t || s );
	return  t || s;
}

/****************************************/
int R_And(int op1, int dummy1, int dummy2)
/****************************************/
{
	/* RCODE: R_AND
	   Ariel:    part of the expressions within IF statements */
	int t = R_Pop();
	int s = R_Pop();
	R_Push( t && s );
	return  t && s;
}

/********************************************/
int R_Stop(int dummy1, int dummy2, int dummy3)
/********************************************/
{
	/* RCODE: R_STOP
	   Ariel:    end of file */
	/* cleans things up if necessary, then returns */

	/* close all OA objects */
	R_OArenew(BOOLE_ATOMS, 0, 0);
	return ra->recovering = 0;
}

/***************************************/
int R_Killed(int role, int id, int dummy)
/***************************************/
{
	/* RCODE: R_KILLED
	   Ariel:    expression '-k', IF statements */
	/* Checks whether entity `role', id `id' has been killed or not */
	/* There are seven cases: role may be '*', 'T*', 'G*', 'N*', 'T', 'G', 'N' */
	BACKBONE_ident_t *ident;
	BACKBONE_group_t *gptr;
	BACKBONE_node_t *nptr;
	int i, index, result;
	char FName[]="R_Killed";
	
        /* the top of the evaluation stack represents the position of this
           '-k' condition within the list of boolean atoms in the current
           IF statement. */

        int  ord = R_Pop();
        ObjAlloc_t *oa;
        int index2, index3;
	entity_t e;

        oa = oa_satisfy[ord] = OA_open(sizeof(entity_t), R_INI_ASIZE, 
#ifndef STATIC
	     R_INC_ASIZE);
#else
	     R_INC_ASIZE, RInt.oa_satisfy[ord]);
#endif

	if (isstar(role))             /* case  '*'  */
	{
		/* check whether anything has been killed;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->identifier_sem);
		
		ident = (BACKBONE_ident_t *) OA_array(ra->db->oa_identifier);
		e.role = ID_IDENTIFIER;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_identifier); i++, ident++)
			if (ident->status == BACKBONE_KILLED)
			{
				e.id = ident->identifier;
				OA_insert(oa, index++, &e);
			}

		Signal(ra->db->identifier_sem);

		if (index > 0)
			R_Push(1);
		else
			R_Push(0);

		Wait(ra->db->group_sem);

		gptr = (BACKBONE_group_t *) OA_array(ra->db->oa_group);

		e.role = ID_GROUP;

		index2 = index;
		for (i=0; i<OA_cardinality(ra->db->oa_group); i++, gptr++)
			if (gptr->status == BACKBONE_KILLED)
			{
				e.id = gptr->ID;
				OA_insert(oa, index2++, &e);
			}

		Signal(ra->db->group_sem);
		
		nptr = ra->db->node;
		
		e.role = ID_NODE;

		index3 = index2;
		for (i=0; i<ra->db->node_count; i++, nptr++)
		{
			Wait(nptr->sem);
			
			if (nptr->status == BACKBONE_KILLED)
			{
				e.id = i;
				OA_insert(oa, index3++, &e);
			}

			Signal(nptr->sem);
		}

		if (index3 > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isthreadstar(role))  /* case  'T*'  */
	{
		/* check whether any thread has been killed;
		   if so, R_Push (1) otherwise R_Push (0) */

		Wait(ra->db->identifier_sem);
		
		ident = (BACKBONE_ident_t *) OA_array(ra->db->oa_identifier);
		e.role = ID_IDENTIFIER;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_identifier); i++, ident++)
			if (ident->status == BACKBONE_KILLED)
			{
				e.id = ident->identifier;
				OA_insert(oa, index++, &e);
			}

		Signal(ra->db->identifier_sem);

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isgroupstar(role))   /* case  'G*'  */
	{
		/* check whether any group has been killed;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->group_sem);
		
		gptr = (BACKBONE_group_t *) OA_array(ra->db->oa_group);
		e.role = ID_GROUP;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_group); i++, gptr++)
			if (gptr->status == BACKBONE_KILLED)
			{
				e.id = gptr->ID;
				OA_insert(oa, index++, &e);
			}

		Signal(ra->db->group_sem);

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnodestar(role))    /* case  'N*'  */
	{
		/* check whether any node has been killed;
		   if so, R_Push (1) otherwise R_Push (0) */
		nptr = ra->db->node;
		
		e.role = ID_NODE;

		index = 0;
		for (i=0; i<ra->db->node_count; i++, nptr++)
		{
			Wait(nptr->sem);
			
			if (nptr->status == BACKBONE_KILLED)
			{
				e.id = i;
				OA_insert(oa, index++, &e);
			}

			Signal(nptr->sem);
		}
		
		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnormalthread(role))      /* case  'T<id>'  */
	{
		/* check whether thread <id> has been killed;
		   if so, R_Push (1) otherwise R_Push (0) */
		   
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			R_Push(0);
			return(-1);
		}

		if (ident->status == BACKBONE_KILLED)
		{
			Signal(ra->db->identifier_sem);
			e.role = ID_IDENTIFIER;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		Signal(ra->db->identifier_sem);

		R_Push(0);
		return(0);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* check whether group <id> has been killed;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->group_sem);
		   
		result = BACKBONE_Search_Group(ra->db, id, &gptr, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->group_sem);
			LogError(EC_ERROR, FName, "Group %d not present.", id);
			R_Push(0);
			return(-1);
		}

		if (gptr->status == BACKBONE_KILLED)
		{
			Signal(ra->db->group_sem);
			e.role = ID_GROUP;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		Signal(ra->db->group_sem);
		
		R_Push(0);
		return(0);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		/* check whether node <id> has been killed;
		   if so, R_Push (1) otherwise R_Push (0) */
		if (id < 0 || id >= ra->db->node_count)
		{
			LogError(EC_ERROR, FName, "Node %d not present.", id);
			R_Push(0);
			return(-1);
		}

		nptr = &((ra->db->node)[id]);

		Wait(nptr->sem);

		if (nptr->status == BACKBONE_KILLED)
		{
			Signal(nptr->sem);
			e.role = ID_NODE;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		Signal(nptr->sem);

		R_Push(0);
		return(0);
	}
	else
	{
		/* an error occurred */
		return -1;
	}
}

/*****************************************/
int R_Rebooted(int role, int id, int dummy)
/*****************************************/
{
	/* RCODE: R_KILLED
	   Ariel:    expression '-r', IF statements */
	/* checks whether entity `role', id `id' has been rebooted or not */
	/* there are three cases: role may be '*', 'N*', 'N' */
	BACKBONE_node_t *nptr;
	int i;
	char FName[]="R_Rebooted";
	
        /* the top of the evaluation stack represents the position of this
           '-k' condition within the list of boolean atoms in the current
           IF statement. */

        int  ord = R_Pop();
        ObjAlloc_t *oa;
        int  index, index2, index3;
	entity_t e;

        oa = oa_satisfy[ord] = OA_open(sizeof(entity_t), R_INI_ASIZE, 
#ifndef STATIC
	     R_INC_ASIZE);
#else
	     R_INC_ASIZE, RInt.oa_satisfy[ord]);
#endif

	if (isstar(role) || isnodestar(role))  /* case  '*' and 'N*'  */
	{
		/* check whether any node has been rebooted;
		   if so, R_Push (1) otherwise R_Push (0) */
		
		nptr = ra->db->node;

		
		e.role = ID_NODE;

		index = 0;

		for (i=0; i<ra->db->node_count; i++, nptr++)
		{
			Wait(nptr->sem);
			
			if (nptr->reboot_count > 0)
			{
				e.id = i;
				OA_insert(oa, index++, &e);
			}

			Signal(nptr->sem);
		}

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnormalnode(role))                 /* case  N<id>  */
	{
		/* check whether node <id> has been rebooted;
		   if so, R_Push (1) otherwise R_Push (0) */
	
		if (id < 0 || id >= ra->db->node_count)
		{
			LogError(EC_ERROR, FName, "Node %d not present.", id);
			R_Push(0);
			return(-1);
		}

		nptr = &((ra->db->node)[id]);

		Wait(nptr->sem);
		
		e.role = ID_NODE;

		if (nptr->reboot_count > 0)
		{
			Signal(nptr->sem);
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		Signal(nptr->sem);

		R_Push(0);
		return(0);
	}
	else
	{
		/* an error occurred */
		return (-1);
	}
}

/******************************************/
int R_Restarted(int role, int id, int dummy)
/******************************************/
{
	/* RCODE: R_RESTARTED
	   Ariel:    expression '-s', IF statements */
	/* checks whether entity `role', id `id' has been restarted or not */
	/* there are three cases: role may be '*', 'T*', 'T' */
	BACKBONE_ident_t *ident;
	int i, result, index;
	char FName[]="R_Restarted";
	
        /* the top of the evaluation stack represents the position of this
           '-k' condition within the list of boolean atoms in the current
           IF statement. */

        int  ord = R_Pop();
        ObjAlloc_t *oa;
        int  index2, index3;
	entity_t e;

        oa = oa_satisfy[ord] = OA_open(sizeof(entity_t), R_INI_ASIZE, 
#ifndef STATIC
	     R_INC_ASIZE);
#else
	     R_INC_ASIZE, RInt.oa_satisfy[ord]);
#endif
	
	if (isstar(role) || isthreadstar(role))  /* case  '*' and 'T*' */
	{
		/* check whether any thread has been restarted;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->identifier_sem);
		
		ident = (BACKBONE_ident_t *) OA_array(ra->db->oa_identifier);
		e.role = ID_IDENTIFIER;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_identifier); i++, ident++)
			if (ident->restart_count > 0)
			{
				e.id = ident->identifier;
				OA_insert(oa, index++, &e);
			}

		Signal(ra->db->identifier_sem);
	
		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnormalthread(role))                 /* case  T<id>  */
	{
		/* check whether thread <id> has been restarted;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->identifier_sem);
		   
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			R_Push(0);
			return(-1);
		}

		if (ident->restart_count > 0)
		{
			Signal(ra->db->identifier_sem);
			e.role = ID_IDENTIFIER;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		Signal(ra->db->identifier_sem);

		R_Push(0);
		return(0);
	}
	else
	{
		/* an error occurred */
		return (-1);
	}
}

/****************************************/
int R_Present(int role, int id, int dummy)
/****************************************/
{
	/* RCODE: R_PRESENT
	   Ariel:    expression '-p', IF statements */
	/* checks whether entity `role', id `id' is present or not */
	/* there are 4 cases: role may be 'T*', 'G*', 'T', 'G', */
	BACKBONE_ident_t *ident;
	BACKBONE_group_t *gptr;
	int i, result, index;

        /* the top of the evaluation stack represents the position of this
           '-k' condition within the list of boolean atoms in the current
           IF statement. */

        int  ord = R_Pop();
        ObjAlloc_t *oa;
        int  index2, index3;
	entity_t e;

        oa = oa_satisfy[ord] = OA_open(sizeof(entity_t), R_INI_ASIZE, 
#ifndef STATIC
	     R_INC_ASIZE);
#else
	     R_INC_ASIZE, RInt.oa_satisfy[ord]);
#endif

	if (isthreadstar(role))  /* case  'T*'  */
	{
		/* check whether any thread is present;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->identifier_sem);
		   
		ident = (BACKBONE_ident_t *) OA_array(ra->db->oa_identifier);
		e.role = ID_IDENTIFIER;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_identifier); i++, ident ++)
			if (ident->status == BACKBONE_RUNNING)
			{
				e.id = ident->identifier;
				OA_insert(oa, index++, &e);
			}

		Signal(ra->db->identifier_sem);

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isgroupstar(role))   /* case  'G*'  */
	{
		/* check whether any group is present;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->group_sem);
		
		if (OA_cardinality(ra->db->oa_group) > 0)
		{
			e.role = ID_GROUP;
			gptr = (BACKBONE_group_t *) OA_array(ra->db->oa_group);

			for (i=0; i<OA_cardinality(ra->db->oa_group); i++, gptr++)
			{
				e.id = gptr->ID;
				OA_insert(oa, i, &e);
			}
			Signal(ra->db->group_sem);
			
			R_Push(1);
			return(1);
		}
		Signal(ra->db->group_sem);
		
		R_Push(0);
		return(0);
	}
	else if (isnormalthread(role))      /* case  'T<id>'  */
	{
		/* check whether thread <id> is present;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->identifier_sem);
		   
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			R_Push(0);
			return(0);
		}

		if (ident->status == BACKBONE_RUNNING)
		{
			Signal(ra->db->identifier_sem);
			e.role = ID_IDENTIFIER;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}
		Signal(ra->db->identifier_sem);
		
		R_Push(0);
		return(0);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* check whether group <id> is present;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->group_sem);
		   
		result = BACKBONE_Search_Group(ra->db, id, &gptr, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->group_sem);
			R_Push(0);
			return(0);
		}

		if (gptr->status != BACKBONE_KILLED)
		{
			Signal(ra->db->group_sem);
			e.role = ID_GROUP;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}
		Signal(ra->db->group_sem);
		
		R_Push(0);
		return(0);
	}
	else
	{
		/* an error occurred */
	}
}

/*****************************************/
int R_Isolated(int role, int id, int dummy)
/*****************************************/
{
	/* RCODE: R_ISOLATED
	   Ariel:    expression '-i', IF statements */
	/* checks whether entity `role', id `id' is isolated or not */
	/* there are seven cases: role may be '*', 'T*', 'G*', 'N*', 'T', 'G', 'N' */
	BACKBONE_node_t *nptr;
	BACKBONE_thread_t *tptr;
	BACKBONE_group_t *gptr;
	BACKBONE_ident_t *ident;
	int i, j, result, index;
	char FName[]="R_Isolated";

        /* the top of the evaluation stack represents the position of this
           '-k' condition within the list of boolean atoms in the current
           IF statement. */

        int  ord = R_Pop();
        ObjAlloc_t *oa;
        int  index2, index3;
	entity_t e;

        oa = oa_satisfy[ord] = OA_open(sizeof(entity_t), R_INI_ASIZE, 
#ifndef STATIC
	     R_INC_ASIZE);
#else
	     R_INC_ASIZE, RInt.oa_satisfy[ord]);
#endif

	if (isstar(role))             /* case  '*'  */
	{
		/* check whether anything has been isolated;
		   if so, R_Push (1) otherwise R_Push (0) */
		nptr = ra->db->node;

		index = 0;
		for (i=0; i<ra->db->node_count; i++, nptr++)
		{
			Wait(nptr->sem);
		
			if (nptr->status == BACKBONE_ISOLATED)
			{
				e.role = ID_NODE;
				e.id = i;
				OA_insert(oa, index++, &e);
			}

			tptr = (BACKBONE_thread_t *) OA_array(nptr->oa_thread);
			
			e.role = ID_IDENTIFIER;
			for (j=0; j<OA_cardinality(nptr->oa_thread); j++, tptr++)
				if (tptr->status == BACKBONE_ISOLATED)
				{
					e.id = tptr->identifier;
					OA_insert(oa, index++, &e);
				}

			Signal(nptr->sem);
		}

		Wait(ra->db->group_sem);

		gptr = (BACKBONE_group_t *) OA_array(ra->db->oa_group);

		e.role = ID_GROUP;

		index2 = index;
		for (i=0; i<OA_cardinality(ra->db->oa_group); i++, gptr++)
			if (gptr->status == BACKBONE_ISOLATED)
			{
				e.id = gptr->ID;
				OA_insert(oa, index2++, &e);
			}

		Signal(ra->db->group_sem);

		if (index2 > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isthreadstar(role))  /* case  'T*'  */
	{
		/* check whether any thread has been isolated;
		   if so, R_Push (1) otherwise R_Push (0) */
		nptr = ra->db->node;

		index = 0;
		for (i=0; i<ra->db->node_count; i++, nptr++)
		{
			Wait(nptr->sem);
		
			tptr = (BACKBONE_thread_t *) OA_array(nptr->oa_thread);
			e.role = ID_IDENTIFIER;

			for (j=0; j<OA_cardinality(nptr->oa_thread); j++, tptr++)
				if (tptr->status == BACKBONE_ISOLATED)
				{
					e.id = tptr->identifier;
					OA_insert(oa, index++, &e);
				}
			
			Signal(nptr->sem);
		}

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isgroupstar(role))   /* case  'G*'  */
	{
		/* check whether any group has been isolated;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->group_sem);
		   
		gptr = (BACKBONE_group_t *) OA_array(ra->db->oa_group);
		e.role = ID_GROUP;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_group); i++, gptr++)
			if (gptr->status == BACKBONE_ISOLATED)
			{
				e.id = gptr->ID;
				OA_insert(oa, index++, &e);
			}

		Signal(ra->db->group_sem);

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnodestar(role))    /* case  'N*'  */
	{
		/* check whether any node has been isolated;
		   if so, R_Push (1) otherwise R_Push (0) */
		nptr = ra->db->node;
		e.role = ID_NODE;

		index = 0;
		for (i=0; i<ra->db->node_count; i++, nptr++)
		{
			Wait(nptr->sem);
			
			if (nptr->status == BACKBONE_ISOLATED)
			{
				e.id = i;
				OA_insert(oa, index++, &e);
			}

			Signal(nptr->sem);
		}

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnormalthread(role))      /* case  'T<id>'  */
	{
		/* check whether thread <id> has been isolated;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->identifier_sem);

		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			R_Push(0);
			return(-1);
		}

		if (ident->status != BACKBONE_RUNNING)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not running.", id);
			R_Push(0);
			return(-1);
		}

		nptr = &((ra->db->node)[ident->node_nr]);

		Wait(nptr->sem);

		BACKBONE_Search_Thread(ra->db, ident->node_nr, ident->threadID, &tptr, &index, 0);

		if (tptr->status == BACKBONE_ISOLATED)
		{
			Signal(ra->db->identifier_sem);
			Signal(nptr->sem);
			e.role = ID_IDENTIFIER;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		Signal(nptr->sem);
		Signal(ra->db->identifier_sem);

		R_Push(0);
		return(0);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* check whether group <id> has been isolated;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->group_sem);   
		   
		result = BACKBONE_Search_Group(ra->db, id, &gptr, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->group_sem);
			LogError(EC_ERROR, FName, "Group %d not present.", id);
			R_Push(0);
			return(-1);
		}

		if (gptr->status == BACKBONE_ISOLATED)
		{
			Signal(ra->db->group_sem);
			e.role = ID_GROUP;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}
		Signal(ra->db->group_sem);

		R_Push(0);
		return(0);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		/* check whether node <id> has been isolated;
		   if so, R_Push (1) otherwise R_Push (0) */
		if (id < 0 || id >= ra->db->node_count)
		{
			LogError(EC_ERROR, FName, "Node %d not present.", id);
			R_Push(0);
			return(-1);
		}

		nptr = &((ra->db->node)[id]);

		Wait(nptr->sem);

		if (nptr->status == BACKBONE_ISOLATED)
		{
			Signal(nptr->sem);
			e.role = ID_NODE;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}
		Signal(nptr->sem);

		R_Push(0);
		return(0);
	}
	else
	{
		/* an error occurred */
	}
}

/***************************************/
int R_Faulty(int role, int id, int dummy)
/***************************************/
{
	/* RCODE: R_FAULTY
	   Ariel:    expression '-f', IF statements */
	/* checks whether entity `role', id `id' is faulty or not */
	/* there are seven cases: role may be '*', 'T*', 'G*', 'N*', 'T', 'G', 'N' */
	BACKBONE_error_t *eptr;
	BACKBONE_ident_t *ident;
	BACKBONE_group_t *gptr;
	BACKBONE_node_t *nptr;
	BACKBONE_thread_t *tptr;
	int i, index, index2, index3, result;
        int  ord = R_Pop();
        ObjAlloc_t *oa;
	entity_t e;
	char FName[]="R_Faulty";
	
        /* the top of the evaluation stack represents the position of this
           '-k' condition within the list of boolean atoms in the current
           IF statement. */


        oa = oa_satisfy[ord] = OA_open(sizeof(entity_t), R_INI_ASIZE, 
#ifndef STATIC
	     R_INC_ASIZE);
#else
	     R_INC_ASIZE, RInt.oa_satisfy[ord]);
#endif

	if (isstar(role))             /* case  '*'  */
	{
		/* check whether anything is faulty;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->error_sem);
		   
		if (OA_cardinality(ra->db->oa_error) > 0)
		{
			index = 0;
			for (i=0; i<OA_cardinality(ra->db->oa_error); i++, eptr++)
			{
				if (eptr->groupID != -1)
				/* a group */
				{
					e.role = ID_GROUP;
					e.id = eptr->groupID;
					OA_insert(oa, index++, &e);
				}
				else
				if (eptr->node_nr != -1 && eptr->threadID == -1 && eptr->groupID == -1)
				/* a node */
				{
					e.role = ID_NODE;
					e.id = eptr->node_nr;
					OA_insert(oa, index++, &e);
				}
				else
				if (eptr->node_nr != -1 && eptr->threadID != -1)
				/* a thread */
				{
					e.role = ID_IDENTIFIER;
					result = BACKBONE_Search_Thread(ra->db, eptr->node_nr, eptr->threadID, &tptr, &index3, 1);
					if (result == PRESENT)
					{
						e.id = tptr->identifier;
						OA_insert(oa, index++, &e);
					}
				}
			}
			Signal(ra->db->error_sem);
			
			if (index > 0)
			{
				R_Push(1);
				return(1);
			}
			else
			{
				R_Push(0);
				return(0);
			}
		}
		Signal(ra->db->error_sem);
		
		R_Push(0);
		return(0);
	}
	else if (isthreadstar(role))  /* case  'T*'  */
	{
		/* check whether any thread is faulty;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->error_sem);
		   
		eptr = (BACKBONE_error_t *) OA_array(ra->db->oa_error);
		e.role = ID_IDENTIFIER;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_error); i++, eptr++)
			if (eptr->node_nr != -1 && eptr->threadID != -1)
			{
				result = BACKBONE_Search_Thread(ra->db, eptr->node_nr, eptr->threadID, &tptr, &index3, 1);
				if (result == PRESENT)
				{
					e.id = tptr->identifier;
					OA_insert(oa, index++, &e);
				}
			}
		Signal(ra->db->error_sem);

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isgroupstar(role))   /* case  'G*'  */
	{
		/* check whether any group is faulty;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->error_sem);
		   
		eptr = (BACKBONE_error_t *) OA_array(ra->db->oa_error);
		e.role = ID_GROUP;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_error); i++, eptr++)
			if (eptr->groupID != -1)
			{
				e.id = eptr->groupID;
				OA_insert(oa, index++, &e);
			}
		Signal(ra->db->error_sem);

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnodestar(role))    /* case  'N*'  */
	{
		/* check whether any node is faulty;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->error_sem);
		   
		eptr = (BACKBONE_error_t *) OA_array(ra->db->oa_error);
		e.role = ID_NODE;

		index = 0;
		for (i=0; i<OA_cardinality(ra->db->oa_error); i++, eptr++)
			if (eptr->node_nr != -1 && eptr->threadID == -1 && eptr->groupID == -1)
			{
				e.id = eptr->node_nr;
				OA_insert(oa, index++, &e);
			}
		Signal(ra->db->error_sem);

		if (index > 0)
		{
			R_Push(1);
			return(1);
		}
		else
		{
			R_Push(0);
			return(0);
		}
	}
	else if (isnormalthread(role))      /* case  'T<id>'  */
	{
		/* check whether thread <id> is faulty;
		   if so, R_Push (1) otherwise R_Push (0) */
		Wait(ra->db->identifier_sem);
		   
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			R_Push(0);
			return(-1);
		}

		index = -1;
		result = BACKBONE_Search_Error(ra->db, ident->node_nr, ident->threadID, -1, -1, BACKBONE_ERROR_NODE | BACKBONE_ERROR_THREAD, &eptr, &index, 1);

		Signal(ra->db->identifier_sem);

		if (result == PRESENT)
		{
			e.role = ID_IDENTIFIER;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* check whether group <id> is faulty;
		   if so, R_Push (1) otherwise R_Push (0) */
		index = -1;   
		result = BACKBONE_Search_Error(ra->db, -1, -1, id, -1, BACKBONE_ERROR_GROUP, &eptr, &index, 1);

		if (result == PRESENT)
		{
			e.role = ID_GROUP;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		/* check whether node <id> is faulty;
		   if so, R_Push (1) otherwise R_Push (0) */
		index = -1;   
		result = BACKBONE_Search_Error(ra->db, id, -1, -1, -1, BACKBONE_ERROR_NODE, &eptr, &index, 1);

		if (result == PRESENT)
		{
			e.role = ID_NODE;
			e.id = id;
			OA_insert(oa, 0, &e);
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else
	{
		/* an error occurred */
	}
}

/******************************************/
int R_StrErrNum(int role, int id, int dummy)
/******************************************/
{
	/* pushes the number of errors which affected the referred entity. */
        /* RCODE: R_STRERRN
           Ariel:    ERRN() function, IF statements
         */

	/* there are three cases: role may be 'T', 'G', 'N' */
	BACKBONE_ident_t *ident;
	BACKBONE_group_t *gptr;
	BACKBONE_node_t *nptr;
	int result, index, errn;
	char FName[]="R_StrErrNum";
	
	
	if (isnormalthread(role))           /* case  'T<id>'  */
	{
		/* queries the database about errn, the number of
		   errors which affected thread <id>; */
		Wait(ra->db->identifier_sem);

		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);
		
		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			R_Push(0);
			return(-1);
		}
		
		errn = ident->error_count; 

		Signal(ra->db->identifier_sem);

		R_Push(errn);
		return(errn);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* queries the database about errn, the number of
		   errors which affected group <id>; */
		Wait(ra->db->group_sem);

		result = BACKBONE_Search_Group(ra->db, id, &gptr, &index, 0);
		
		if (result != PRESENT)
		{
			Signal(ra->db->group_sem);
			LogError(EC_ERROR, FName, "Group %d not present.", id);
			R_Push(0);
			return(-1);
		}
		
		errn = gptr->error_count; 

		Signal(ra->db->group_sem);

		R_Push(errn);
		return(errn);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		/* queries the database about errn, the number of
		   errors which affected node <id>; */
		if (id < 0 || id >= ra->db->node_count)
		{
			LogError(EC_ERROR, FName, "Node %d not present.", id);
			R_Push(0);
			return(-1);
		}

		nptr = &((ra->db->node)[id]);

		Wait(nptr->sem);

		errn = nptr->error_count;

		Signal(nptr->sem);

		R_Push(errn);
		return(errn);
	}
	else
	{
		/* an error occurred */
	}
}

/*******************************************/
int R_StrErrType(int role, int id, int dummy)
/*******************************************/
{
	/* pushes the type of errors which affected the referred entity. */
        /* RCODE: R_STRERRT
           Ariel:    ERRT() function, IF statements
         */
	/* there are three cases: role may be 'T', 'G', 'N' */
	BACKBONE_ident_t *ident;
	BACKBONE_error_t *eptr;
	BACKBONE_group_t *gptr;
	int result, index, errt;
	char FName[]="R_StrErrType";
	
	
	if (isnormalthread(role))           /* case  'T<id>'  */
	{
		/* queries the database about errt, the type of
		   error which affected thread <id>; */
		Wait(ra->db->identifier_sem);
		Wait(ra->db->error_sem);
		   
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			Signal(ra->db->error_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			R_Push(0);
			return(-1);
		}

		index = -1;
		result = BACKBONE_Search_Error(ra->db, ident->node_nr, ident->threadID, -1, -1, BACKBONE_ERROR_NODE | BACKBONE_ERROR_THREAD, &eptr, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			Signal(ra->db->error_sem);
			R_Push(0);
			return(-1);
		}

		errt = eptr->type;

		Signal(ra->db->identifier_sem);
		Signal(ra->db->error_sem);

		R_Push(errt);
		return(errt);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* queries the database about errt, the type of
		   error which affected group <id>; */
		Wait(ra->db->error_sem);
		   
		index = -1;  
		result = BACKBONE_Search_Error(ra->db, -1, -1, id, -1, BACKBONE_ERROR_GROUP, &eptr, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->error_sem);
			LogError(EC_ERROR, FName, "Group %d not present.", id);
			R_Push(0);
			return(-1);
		}

		errt = eptr->type;

		Signal(ra->db->error_sem);

		R_Push(errt);
		return(errt);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		/* queries the database about errt, the type of
		   error which affected node <id>; */
		Wait(ra->db->error_sem);
		   
		index = -1;  
		result = BACKBONE_Search_Error(ra->db, id, -1, -1, -1, BACKBONE_ERROR_NODE, &eptr, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->error_sem);
			LogError(EC_ERROR, FName, "Node %d not present.", id);
			R_Push(0);
			return(-1);
		}

		errt = eptr->type;
	
		Signal(ra->db->error_sem);
		
		R_Push(errt);
		return(errt);
	}
	else
	{
		/* an error occurred */
	}
}

/***********************************************************************/
int equal(int a, int b) { return a == b; }
int notequal(int a, int b) { return a != b; }
int greater(int a, int b) { return a > b; }
int greaterorequal(int a, int b) { return a >= b; }
int less(int a, int b) { return a < b; }
int lessorequal(int a, int b) { return a <= b; }

int (*arith_op[])(int,int) = {
	NULL,
	equal, notequal, greater, greaterorequal, less, lessorequal
	};
/***********************************************************************/

/*******************************************/
int R_Compare(int op_id, int this, int dummy)
/*******************************************/
{
	int that = R_Pop();

	R_Push( arith_op[op_id] (this, that) );
}

/* decomposes an action into a number of basic steps, also dealing
   with the `$' metacharacter. */
/***********************************************************/
int Perform_RAction(int role, int id, int (*action)(int,int))
/***********************************************************/
{
        /* RCODE: R_KILL, R_START, R_RESTART, R_REBOOT
           Ariel:    KILL, START, RESTART, REBOOT
         */

	BACKBONE_ident_t *ident;
	BACKBONE_thread_t *tptr;
	BACKBONE_group_t *gptr;
	BACKBONE_bthread_t *bptr;
	int i, j, role2, id2, opcode, result, index, *gid;
	char FName[]="Perform_Raction";
	

	if (isthread(role))           /* case  'T<id>'  or  T$ @line (-id) */
	{
		if (id > 0)
		{
			return (*action)(role, id);
		}

		query_rcode_array(-id, &opcode, &role2, &id2);

		if (isthread(role2))
		{
			return (*action)(role2, id2);
		}

		/* impossible combination */
		/* return failure */
	}
	else if (isgroup(role))       /* case  'G<id>'  or  G$ @line (-id) */
	{
		if (id > 0)
		{
			return (*action)(role, id);
		}

		query_rcode_array(-id, &opcode, &role2, &id2);

		if (isthread(role2))
		{
			Wait(ra->db->identifier_sem);
		
			result = BACKBONE_Search_Identifier(ra->db, id2, &ident, &index, 0);
	
			if (result != PRESENT)
			{
				Signal(ra->db->identifier_sem);
				LogError(EC_ERROR, FName, "Thread %d not present.", id2);
				return(1);
			}

			if (ident->status != BACKBONE_RUNNING)
			{
				Signal(ra->db->identifier_sem);
				LogError(EC_ERROR, FName, "Thread %d not running.", id2);
				return(1);
			}
		
			BACKBONE_Search_Thread(ra->db, ident->node_nr, ident->threadID, &tptr, &index, 1); 
			
			Signal(ra->db->identifier_sem);
			
			gid = (int *) OA_array(tptr->oa_groupID);

			for (i=0; i<OA_cardinality(tptr->oa_groupID); i++, gid++)
				(*action)(ID_GROUP, *gid);
				
			/* return success if all (*action)'s were OK,
			   or failure otherwise */
		}
		if (isgroup(role2))
		{
			return (*action)(role2, id2);
		}
		else /* isnode() or anything else */
		{
			/* impossible combination */
			/* return failure */
		}
	}
	else if (isnode(role))        /* case  'N<id>'  or  N$ @line (-id) */
	{
		if (id > 0)
		{
			return (*action)(role, id);
		}
		/* else */
		query_rcode_array(-id, &opcode, &role2, &id2);

		if (isthread(role2))
		{
			/* kill the node on which thread <id2> is (or was) running */
			result = BACKBONE_Search_Identifier(ra->db, id2, &ident, &index, 1);

			if (result != PRESENT) 
			{
				LogError(EC_ERROR, FName, "Thread %d not present.", id2);
				return(1);
			}
			
			/* let $n be the node on which thread <id2> is (or was) running */
			return (*action)(ID_NODE, ident->node_nr);
		}
		if (isgroup(role2))
		{
			/* kill all nodes on which at least one member of group <id2>
			   (was ?) is running */
			result = BACKBONE_Search_Group(ra->db, id2, &gptr, &index, 1);

			if (result != PRESENT) 
			{
				LogError(EC_ERROR, FName, "Group %d not present.", id2);
				return(1);
			}
			
			for(i=0; i<ra->db->node_count; i++) 
			{
				bptr = (BACKBONE_bthread_t *) OA_array(gptr->oa_list);
				
				for(j=0; j<OA_cardinality(gptr->oa_list); j++, bptr++) 
					if (bptr->node_nr == i)
					{
						(*action)(ID_NODE, i);
						break;
					}
			}
			/* return success if all actions went ok, failure otherwise */
		}
		if (isnode(role2))
		{
			return (*action)(role2, id2);
		}
		else
		{
			/* impossible combination */
			/* return failure */
		}
	}
	else
	{
		/* an error occurred */
		/* return failure */
	}
}

/**************************************************************************/
/*                       Recovery action functions.                       */
/*  for all these functions we have to check whether  id  is positive or  */
/*  not. If it's positive, then it is a plain id, otherwise it points to  */
/*  an rcode entry  describing an entity --- in other words, it resulted  */
/*  from a                   <ACTION> <entity>$                statement. */
/**************************************************************************/

/******************************/
int entitykill(int role, int id)
/******************************/
{
	BACKBONE_ident_t *ident;
	int result, index, identifier;
	char FName[]="entitykill";

	
	if (role == ID_IDENTIFIER)
	{
		Wait(ra->db->identifier_sem);
	
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			return(-1);
		}

		if (ident->status != BACKBONE_RUNNING)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not running.", id);
			return(-1);
		}

		Signal(ra->db->identifier_sem);

		result = BACKBONE_Kill_Thread(ident->node_nr, ident->threadID, &identifier);

		return(result);
	}

	if (role == ID_GROUP)
	{
		result = BACKBONE_Kill_Group(id);

		return(result);
	}

	if (role == ID_NODE)
	{
		result = BACKBONE_Kill_Node(id);

		return(result);
	}

	return(-1);
}


/*************************************/
int R_Kill(int role, int id, int dummy)
/*************************************/
{
	/* Kills the referred entity */
        /* RCODE: R_KILL
           Ariel:    action KILL
         */
	return Perform_RAction(role, id, entitykill);
}
			
/********************************/
int entityreboot(int role, int id)
/********************************/
{
	int result;


	if (role == ID_NODE)
	{
		result = BACKBONE_Reboot_Node(id);

		return(result);
	}

	return(-1);
}

/***************************************/
int R_Reboot(int role, int id, int dummy)
/***************************************/
{
	/* Reboots the referred entity */
	return Perform_RAction(role, id, entityreboot);
}


/*********************************/
int entityrestart(int role, int id)
/*********************************/
{
	BACKBONE_ident_t *ident;
	int result, index;
	char FName[]="entityrestart";

	
	if (role == ID_IDENTIFIER)
	{
		Wait(ra->db->identifier_sem);
		
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			return(-1);
		}

		if (ident->status != BACKBONE_RUNNING)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not running.", id);
			return(-1);
		}

		Signal(ra->db->identifier_sem);

		result = BACKBONE_Restart_Thread(ident->node_nr, ident->threadID);

		return(result);
	}

	if (role == ID_GROUP)
	{
		result = BACKBONE_Restart_Group(id);

		return(result);
	}

	return(-1);
}

/****************************************/
int R_Restart(int role, int id, int dummy)
/****************************************/
{
	/* Restarts the referred entity */
	return Perform_RAction(role, id, entityrestart);
}


/*******************************/
int entitystart(int role, int id)
/*******************************/
{
	int result;
	

	if (role == ID_IDENTIFIER)
	{
		result = BACKBONE_Start_Thread(id);
	
		return(result);
	}

	return(-1);
}


/**************************************/
int R_Start(int role, int id, int dummy)
/**************************************/
{
	/* Starts the referred entity */
	return Perform_RAction(role, id, entitystart);
}


/* CheckErr is part of the Ariel statement:
 *
 * ERR NUMBER id WARN id2
 * 
 *   i      PUSH <NUMBER>
 >   i+1    CHECK_ERR <id>  ; pops <NUMBER>, pushes (error <NUMBER> affected <id>)
 *   i+2    PUSH  i+1
 *   i+3    DOTSWARN <id2>  ; pops i+1, pops truthvalue, and if truthvalue is true
 *                          ; then    . retrieves <NUMBER> and <id> by means of  i+1
 *                          ;         . warn <id2> that error <NUMBER> affected <id>
 *
 * R_CheckErr pops NUMBER off the stack, then checks whether
 * error NUMBER did affect entity  id  , then pushes the answer
 * onto the stack. Next function to be executed will be R_DotsWarn()
 * which pops the answer and the location of id off the stack 
 * and warns  id2  in case it was a positive answer.
 */
/*****************************************/
int R_CheckErr(int role, int id, int dummy)
/*****************************************/
{
	/* checks whether the error on the top of the evaluation
	   stack has affected id or not. Pushes the result on the 
	   evaluation stack. */
	int result, errortype, index;
	BACKBONE_error_t *eptr;
	BACKBONE_ident_t *ident;
	char FName[]="R_CheckErr";
	
	errortype = R_Pop();

	/* there are seven cases: role may be '*', 'T*', 'G*', 'N*', 'T', 'G', 'N' */

	if (isstar(role)) /* case '*' */ 
	{ 
		index = -1;
		result = BACKBONE_Search_Error(ra->db, -1, -1, -1, errortype, BACKBONE_ERROR_TYPE, &eptr, &index, 1);
		 
		if (result == PRESENT)
		{
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isnodestar(role)) /* case 'N*' */
	{
		index = -1;

		do
		{
			result = BACKBONE_Search_Error(ra->db, -1, -1, -1, errortype, BACKBONE_ERROR_TYPE, &eptr, &index, 1);
		}
		while ((result == PRESENT) && (eptr->node_nr == -1 || eptr->threadID != -1 || eptr->groupID != -1));

		if (result == PRESENT)
		{
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isthreadstar(role)) /* case 'T*' */
	{
		index = -1;

		do
		{
			result = BACKBONE_Search_Error(ra->db, -1, -1, -1, errortype, BACKBONE_ERROR_TYPE, &eptr, &index, 1);
		}
		while ((result == PRESENT) && (eptr->node_nr == -1 || eptr->threadID == -1));

		if (result == PRESENT)
		{
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isgroupstar(role))   /* case  'G*'  */
	{
		index = -1;

		do
		{
			result = BACKBONE_Search_Error(ra->db, -1, -1, -1, errortype, BACKBONE_ERROR_TYPE, &eptr, &index, 1);
		}
		while ((result == PRESENT) && (eptr->groupID != -1));

		if (result == PRESENT)
		{
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isnormalthread(role))      /* case  'T<id>'  */
	{
		Wait(ra->db->identifier_sem);
	
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result != PRESENT)
		{
			Signal(ra->db->identifier_sem);
			LogError(EC_ERROR, FName, "Thread %d not present.", id);
			R_Push(0);
			return(-1);
		}

		index = -1;
		result = BACKBONE_Search_Error(ra->db, ident->node_nr, ident->threadID, -1, errortype, BACKBONE_ERROR_NODE | BACKBONE_ERROR_THREAD | BACKBONE_ERROR_TYPE, &eptr, &index, 1);

		Signal(ra->db->identifier_sem);

		if (result == PRESENT)
		{
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		index = -1;
		result = BACKBONE_Search_Error(ra->db, -1, -1, id, errortype, BACKBONE_ERROR_TYPE | BACKBONE_ERROR_GROUP, &eptr, &index, 1);
		{
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		index = -1;
		result = BACKBONE_Search_Error(ra->db, id, -1, -1, errortype, BACKBONE_ERROR_TYPE | BACKBONE_ERROR_NODE, &eptr, &index, 1);

		if (result == PRESENT)
		{
			R_Push(1);
			return(1);
		}

		R_Push(0);
		return(0);
	}
	else
	{
		/* an error occurred */
		/* R_push(1); (stg has to be pushed anyway on the stack) */
	}

	/* return success */
}

/* DotsWarn is part of the Ariel statement:
 *
 * ERR NUMBER id2 WARN id
 * 
 *   i      PUSH <NUMBER>
 *   i+1    CHECK_ERR <id2>  ; pops <NUMBER>, pushes (error <NUMBER> affected <id2>)
 *   i+2    PUSH  i+1
 >   i+3    DOTSWARN <id>  ; pops i+1, pops truthvalue, and if truthvalue is true
 *                          ; then    . retrieves <NUMBER> and <id2> by means of  i+1
 *                          ;         . warn <id> that error <NUMBER> affected <id2>
 *
 */

/*******************************************/
int R_DotsWarn (int role, int id, int action)
/*******************************************/
{
	int which_entity; /* ...pops i+1... */
	int result;       /* ...pops truthvalue... */
	int argc;         /* ...pops number of arguments... */
	int i, j, opcode, role2, id2, errornum, index;
	BACKBONE_warning_t warning;
	BACKBONE_thread_t *tptr;
	BACKBONE_bthread_t *btptr;
	BACKBONE_ident_t *ident;
	BACKBONE_group_t *gptr;
	int argv[MAX_WARN_ARGC];

	which_entity = R_Pop(); /* Do not change the order of */
	result       = R_Pop(); /* these three lines please!! */
	argc         = R_Pop();

	/* This is to be done anyway, 'cause the stack needs to be 
	   cleaned up...
	 */
	for (i=0; i<argc && i<MAX_WARN_ARGC; i++)
		argv[i] = R_Pop();

	if (result == 0)
		return 0;

	/* ...and if truthvalue is true... */

	/* ...retrieves <NUMBER>... */
	query_rcode_array(which_entity-1, &opcode, &errornum, &id2);
	/* ...and <id> by means of i+1... */
	query_rcode_array(which_entity, &opcode, &role2, &id2);


	/* ...and warn <id> (role,id) that error <NUMBER> (errornum) 
	      affected <id2> (role2,id2) */
	warning.node_nr = warning.threadID = warning.groupID = -1;
	warning.type = errornum;
	      
	if (role2 == ID_IDENTIFIER)
		warning.threadID = id2;
	else if (role2 == ID_GROUP)
		warning.groupID = id2; 
	else if (role2 == ID_NODE)
		warning.node_nr = id2;

	/* there are seven cases: role may be '*', 'T*', 'G*', 'N*', 'T', 'G', 'N' */

	if (isstar(role) || isnodestar(role) || isthreadstar(role))  
	{                             /* cases  '*', 'N*', and 'T*' */
		/* warn all threads in a row */
		Wait(ra->db->identifier_sem);
		
		ident = (BACKBONE_ident_t *) OA_array(ra->db->oa_identifier);

		for (i=0; i<OA_cardinality(ra->db->oa_identifier); i++, ident++)
			if (ident->type == BACKBONE_USER_TYPE)
				BACKBONE_Warn_Thread(ident->node_nr, ident->threadID, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ERROR_DETECTED);					
	
		Signal(ra->db->identifier_sem);
			
		return(0);		
	}
	else if (isgroupstar(role))   /* case  'G*'  */
	{
		/* for each $g group:
		       warn each thread in $g */
		Wait(ra->db->group_sem);
		       
		gptr = (BACKBONE_group_t *) OA_array(ra->db->oa_group);

		for (i=0; i<OA_cardinality(ra->db->oa_group); i++, gptr++)
			BACKBONE_Warn_Group(gptr->ID, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ERROR_DETECTED);

		Signal(ra->db->group_sem);
		
		return(0);
	}
	else if (isnormalthread(role))      /* case  'T<id>'  */
	{
		/* warn thread <id> */
		Wait(ra->db->identifier_sem);
		
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result == NOT_PRESENT)
		{
			Signal(ra->db->identifier_sem);
			return(0);
		}

		BACKBONE_Warn_Thread(ident->node_nr, ident->threadID, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ERROR_DETECTED);

		Signal(ra->db->identifier_sem);
		
		return(0);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* warn all threads in group <id> */
		BACKBONE_Warn_Group(id, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ERROR_DETECTED);
		
		return(0);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		/* warn all threads in node <id> */
		BACKBONE_Warn_Node(id, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ERROR_DETECTED);

		return(0);
	}
	else
	{
		/* an error occurred */
	}

	return(0);
}


/* AND_WARN <role,id> */
/*******************************************/
int R_AndWarn (int role, int id, int line)
/*******************************************/
{
        /* RCODE: R_ANDWARN
           Ariel:    ``action  AND  WARN  id''

           Description: Ariel allows you to attach a warning to a specific
           recovery action by means of the AND keyword. The information
           concerning which action has taken place is pushed onto the
           evaluation stack as an integer stack pointer.
         */
	int i, opcode, role2, id2, result, index;
	int pc;
	int argc;
	BACKBONE_warning_t warning;
	BACKBONE_ident_t *ident;
	BACKBONE_group_t *gptr;
	int argv[MAX_WARN_ARGC];

	pc = R_Pop(); /* Do not swap these two lines please */
	argc = R_Pop();

	/* This is to be done anyway, 'cause the stack needs to be 
	   cleaned up...
	 */
	for (i=0; i<argc && i<MAX_WARN_ARGC; i++)
		argv[i] = R_Pop();
	
	/* pops a program counter (pc) value off the stack; warns
	   <role,id> of action pointed by pc. */

	query_rcode_array(pc, &opcode, &role2, &id2);
	
	warning.node_nr = warning.threadID = warning.groupID = -1;
	warning.type = opcode;
	      
	if (role2 == ID_IDENTIFIER)
		warning.threadID = id2;
	else if (role2 == ID_GROUP)
		warning.groupID = id2; 
	else if (role2 == ID_NODE)
		warning.node_nr = id2;

	/* there are seven cases: role may be '*', 'T*', 'G*', 'N*', 'T', 'G', 'N' */

	if (isstar(role) || isnodestar(role) || isthreadstar(role))  
	{                             /* cases  '*', 'N*', and 'T*' */
		/* warn all threads in a row */
		/* that action <opcode> has been executed on <role2,id2> */
		Wait(ra->db->oa_identifier);
		
		ident = (BACKBONE_ident_t *) OA_array(ra->db->oa_identifier);

		for (i=0; i<OA_cardinality(ra->db->oa_identifier); i++, ident++)
			if (ident->type == BACKBONE_USER_TYPE)
				BACKBONE_Warn_Thread(ident->node_nr, ident->threadID, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ACTION_PERFORMED);					
			
		Signal(ra->db->oa_identifier);

		return(0);		
	}
	else if (isgroupstar(role))   /* case  'G*'  */
	{
		/* for each $g group:
		       warn each thread in $g */
		/* that action <opcode> has been executed on <role2,id2> */
		Wait(ra->db->oa_group);
		
		gptr = (BACKBONE_group_t *) OA_array(ra->db->oa_group);

		for (i=0; i<OA_cardinality(ra->db->oa_group); i++, gptr++)
			BACKBONE_Warn_Group(gptr->ID, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ACTION_PERFORMED);
		Signal(ra->db->oa_group);

		return(0);
	}
	else if (isnormalthread(role))      /* case  'T<id>'  */
	{
		/* warn thread <id> */
		/* that action <opcode> has been executed on <role2,id2> */
		Wait(ra->db->oa_identifier);
		
		result = BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);

		if (result == NOT_PRESENT)
		{
			Signal(ra->db->oa_identifier);
			return(0);
		}

		BACKBONE_Warn_Thread(ident->node_nr, ident->threadID, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ACTION_PERFORMED);
		
		Signal(ra->db->oa_identifier);
		
		return(0);
	}
	else if (isnormalgroup(role))       /* case  'G<id>'  */
	{
		/* warn all threads in group <id> */
		/* that action <opcode> has been executed on <role2,id2> */
		BACKBONE_Warn_Group(id, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ACTION_PERFORMED);
		
		return(0);
	}
	else if (isnormalnode(role))        /* case  'N<id>'  */
	{
		/* warn all threads in node <id> */
		/* that action <opcode> has been executed on <role2,id2> */
		BACKBONE_Warn_Node(id, &warning, sizeof(BACKBONE_warning_t), BACKBONE_ACTION_PERFORMED);
		
		return(0);
	}
	else
	{
		/* an error occurred */
	}
}

/********************************************/
int R_PushArg(int arg, int dummy1, int dummy2)
/********************************************/
{
	R_Push(arg);
}

/***************************************/
int R_Remove(int role, int id, int dummy)
/***************************************/
{
	int error_type = R_Pop();
	int index, i;
	BACKBONE_error_t *eptr;
	BACKBONE_ident_t *ident;

	/* purge all occurrences of <error_type> that have been
	   ascribed to entity <role,id> and are in the errorlist
	 */
	if (isstar(role))
	{
		BACKBONE_Remove_Error(ra->db, -1, -1,-1,-1, error_type, BACKBONE_ERROR_TYPE, 1);
		return 0;
	}
	if (isthreadstar(role))
	{
		Wait(ra->db->error_sem);
	
		for ( index = OA_cardinality(ra->db->oa_error)-1,
		      eptr = ((BACKBONE_error_t*)OA_array(ra->db->oa_error))+index,
		      i = index
		    ; i >= 0
		    ; i--, eptr--
		    )
		{
                	if (eptr->threadID != -1 && eptr->type == error_type)
				OA_remove(ra->db->oa_error, i);
		}

		Signal(ra->db->error_sem);
		
		return 0;
	}
	if (isgroupstar(role))
	{
		Wait(ra->db->error_sem);
		
		for ( index = OA_cardinality(ra->db->oa_error)-1,
		      eptr = ((BACKBONE_error_t*)OA_array(ra->db->oa_error))+index,
		      i = index
		    ; i >= 0
		    ; i--, eptr--
		    )
		{
                	if (eptr->groupID != -1 && eptr->type == error_type)
				OA_remove(ra->db->oa_error, i);
		}
		
		Signal(ra->db->error_sem);
		
		return 0;
	}
	if (isnodestar(role))
	{
		Wait(ra->db->error_sem);
		
		for ( index = OA_cardinality(ra->db->oa_error)-1,
		      eptr = ((BACKBONE_error_t*)OA_array(ra->db->oa_error))+index,
		      i = index
		    ; i >= 0
		    ; i--, eptr--
		    )
		{
                	if (eptr->node_nr != -1 && eptr->threadID == -1 && eptr->type == error_type)
				OA_remove(ra->db->oa_error, i);
		}
		
		Signal(ra->db->error_sem);
		
		return 0;
	}
	if (isnormalthread(role))
	{
		Wait(ra->db->identifier_sem);
	
		BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);
		
		BACKBONE_Remove_Error(ra->db, -1, ident->node_nr, ident->threadID, -1, error_type, BACKBONE_ERROR_TYPE | BACKBONE_ERROR_THREAD | BACKBONE_ERROR_NODE, 1);
		
		Signal(ra->db->identifier_sem);
		
		return 0;
	}
	if (isnormalgroup(role))
	{
		BACKBONE_Remove_Error(ra->db, -1, -1, -1, id, error_type, BACKBONE_ERROR_TYPE | BACKBONE_ERROR_GROUP, 1);
		return 0;
	}
	if (isnormalnode(role))
	{
		BACKBONE_Remove_Error(ra->db, -1, id, -1, -1, error_type, BACKBONE_ERROR_TYPE | BACKBONE_ERROR_NODE, 1);
		return 0;
	}
}

/******************************************/
int R_RemoveAll(int role, int id, int dummy)
/******************************************/
{
	int error_type = R_Pop();
	int index, i;
	BACKBONE_error_t *eptr;
	BACKBONE_ident_t *ident;
	/* purge all errors of any type that have been
	   ascribed to entity <role,id> and are in the errorlist
	 */
	if (isstar(role))
	{
		Wait(ra->db->error_sem);
	
		for ( i = OA_cardinality(ra->db->oa_error)-1 ; i >= 0 ; i--)
			OA_remove(ra->db->oa_error, i);
		
		Signal(ra->db->error_sem);
		
		return 0;
	}
	if (isthreadstar(role))
	{
		Wait(ra->db->error_sem);
		
		for ( index = OA_cardinality(ra->db->oa_error)-1,
		      eptr = ((BACKBONE_error_t*)OA_array(ra->db->oa_error))+index,
		      i = index
		    ; i >= 0
		    ; i--, eptr--
		    )
		{
                	if (eptr->threadID != -1)
				OA_remove(ra->db->oa_error, i);
		}
		
		Signal(ra->db->error_sem);
		
		return 0;
	}
	if (isgroupstar(role))
	{
		Wait(ra->db->error_sem);
		
		for ( index = OA_cardinality(ra->db->oa_error)-1,
		      eptr = ((BACKBONE_error_t*)OA_array(ra->db->oa_error))+index,
		      i = index
		    ; i >= 0
		    ; i--, eptr--
		    )
		{
                	if (eptr->groupID != -1)
				OA_remove(ra->db->oa_error, i);
		}
		
		Signal(ra->db->error_sem);
		
		return 0;
	}
	if (isnodestar(role))
	{
		Wait(ra->db->error_sem);
		
		for ( index = OA_cardinality(ra->db->oa_error)-1,
		      eptr = ((BACKBONE_error_t*)OA_array(ra->db->oa_error))+index,
		      i = index
		    ; i >= 0
		    ; i--, eptr--
		    )
		{
                	if (eptr->node_nr != -1 && eptr->threadID == -1)
				OA_remove(ra->db->oa_error, i);
		}
		
		Signal(ra->db->error_sem);
		
		return 0;
	}
	if (isnormalthread(role))
	{
		Wait(ra->db->identifier_sem);
		
		BACKBONE_Search_Identifier(ra->db, id, &ident, &index, 0);
		
		BACKBONE_Remove_Error(ra->db, -1, ident->node_nr, ident->threadID, -1, -1, BACKBONE_ERROR_THREAD | BACKBONE_ERROR_NODE, 1);
		
		Signal(ra->db->identifier_sem);
		
		return 0;
	}
	if (isnormalgroup(role))
	{
		BACKBONE_Remove_Error(ra->db, -1, -1, -1, id, -1, BACKBONE_ERROR_GROUP, 1);
		return 0;
	}
	if (isnormalnode(role))
	{
		BACKBONE_Remove_Error(ra->db, -1, id, -1, -1, -1, BACKBONE_ERROR_NODE, 1);
		return 0;
	}
}

/* pushes `boolean' on top of the evaluation stack. Returns the current
   length of the stack on success and -1 on failure.
 */
int R_Push (int boolean)
{
	if (esp < ESTACK_SIZE)
	{
		estack[esp++] = boolean;
		return esp;
	}
	return -1;
}
/* pops the top of the evaluation stack. Returns such element
   on success and -1 on failure.
 */
int R_Pop (void)
{
	if (esp > 0)
	{
		return estack[--esp];
	}
	return -1;
}

/****************************/
int BACKBONE_rint_size(char *fname)
/****************************/
{
	struct stat buf;
	int count;


	stat(fname, &buf);
	
	count = buf.st_size/(3*sizeof(int));
	
	return(count);
}

/*******************************************************/
int BACKBONE_rint_read(char *fname, rcode_t *rcode, int count)
/*******************************************************/
{
	int rv;
	FILE *f;
	

	f = fopen(fname, "r+b");

	rv = fread(rcode, sizeof(rcode_t), count, f);

	fclose(f);
	
	return (rv);
}

/*********************************************/
int BACKBONE_Rint_Execute(rcode_t *rcode, int count)
/*********************************************/
{
	int line;


	for (line=0; line<count; line++)
	{
		if (rcode[line][0]>=0)
		    rfunc [ rcode[line][0] ] (rcode[line][1], rcode[line][2], line);
	}

	return (0);
}


int BACKBONE_read_default_action(char *fname)
{
	FILE *f;
	int line;
	int count;
	rcode_t r;

	f = fopen(fname, "r+b");

	for (count=0; fread(&r, sizeof(rcode_t), 1, f); count++)
	{
		if (r[0] == R_SET_DEF_ACT)
		{
			fclose(f);
			return r[1];
		}
	}

	fclose(f);
	return (0);
}

int BACKBONE_rcode(char *fname)
{
	FILE *f;
	int line;
	int count;

	f = fopen(fname, "r+b");

	for (count=0; fread(&rcode[count][0], sizeof(rcode_t), 1, f); count++)
		;

	for (line=0; line<count; line++)
	{
		if (rcode[line][0]>=0)
		    rfunc [ rcode[line][0] ] (rcode[line][1], rcode[line][2], line);
	}

	fclose(f);
	return (0);
}
/* functions to backup / restore the evaluation stack */
int R_Save_Stack(char *fname)
{
        FILE *ef;
        int   items;
        ef = fopen( fname? fname:".estack", "w+b" );
        if (ef == NULL)
                return -1;

        items = fwrite( estack, sizeof(estack_t),  esp, ef);
        fclose( ef );
        return (items);
}
int R_Load_Stack(char *fname)
{
        FILE *ef;
        ef = fopen( fname? fname:".estack", "r+b" );
        if (ef == NULL)
                return -1;

        esp = fread( estack, sizeof(estack_t),  ESTACK_SIZE, ef);
        fclose( ef );
        return (esp);
}

/* Renews card OA objects pointed to by oa_satisfy */
/**********************************************/
int R_OArenew(int card, int dummy2, int dummy3)
/**********************************************/
{
	int i;
	for (i=0; i<card; i++)
		if (oa_satisfy[i])
		{
			OA_close(oa_satisfy[i]);
			oa_satisfy[i] = NULL;
		}
}

/****************************************************/
int R_StorePhase(int threadID, int dummy1, int dummy2)
/****************************************************/
{
 	/* pushes onto the run-time stack the running phase of thread threadID */
	int result, phase, index;
	BACKBONE_ident_t *ident;
	BACKBONE_thread_t *tptr;
	BACKBONE_node_t *nptr;


	Wait(ra->db->identifier_sem);
		
	result = BACKBONE_Search_Identifier(ra->db, threadID, &ident, &index, 0);

	if (result == NOT_PRESENT)
	{
		Signal(ra->db->identifier_sem);
		R_Push(0);
		return(-1);
	}

	nptr = &((ra->db->node)[ident->node_nr]);

	Wait(nptr->sem);

	result = BACKBONE_Search_Thread(ra->db, ident->node_nr, ident->threadID, &tptr, &index, 0);

	Signal(ra->db->identifier_sem);
	
	if (result == PRESENT)
	{
		phase = tptr->phase;
		R_Push(phase);
		Signal(nptr->sem);
		return(0);
	}
	 
	Signal(nptr->sem);
	R_Push(0);
	return(-1);
}

int R_Deadlocked(int dummy1, int dummy2, int dummy3)
{
}
/* this code is in common between the run-time part and the translation-time part */
int query_rcode_array(int line, int *opcode, int *op1, int *op2)
{
	*opcode = rcode[line][0],
	*op1    = rcode[line][1],
	*op2    = rcode[line][2];
	return 0;
}
int R_Reintegrated(int role, int id, int dummy)
{}
int R_Enable(int role, int id, int dummy)
{}
int R_Pause(int seconds, int dummy1, int dummy2)
{
	TimeWait(TimeNow() + seconds * CLOCK_TICK);
}
int R_Call(int function_id, int dummy1, int dummy2)
{
	int argc;
	int argv[MAX_WARN_ARGC];
	int i;

	argc = R_Pop();

	/* This is to be done anyway, 'cause the stack needs to be 
	   cleaned up...
	 */
	for (i=0; i<argc && i<MAX_WARN_ARGC; i++)
		argv[i] = R_Pop();

	/* At this point function <function_id> needs to
	   be called with (argc, argv[]) as arguments.

	   To be added.
	 */
}
/* eof rint.c */
