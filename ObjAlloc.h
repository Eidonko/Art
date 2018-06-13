/*
 *     File: ObjAlloc.h
 *
 *     Dynamically growing arrays: header file
 *
 *     By Eidon@tutanota.com
 */

#ifndef _OBJ__ALLOCATION_MANAGER
#define _OBJ__ALLOCATION_MANAGER
/**@name ObjAlloc.c
   
   @memo This describes the code for manipulating dynamically growing arrays
   */

#include <stdio.h>
#include <stdlib.h>

/*@{*/
/**@name Error codes
   */
/*@{*/
/**A null pointer was given.*/
#define E_OA_NULL_PTRS            -201
/**Could not reallocate some memory.*/
#define E_OA_COULD_NOT_REALLOC    -202
/**Indexes where inconsistent.*/
#define E_OA_INDEX_INCONSISTENCY  -203
/**There is an inconsitency problem*/
#define E_OA_INCONSISTENCY        -204
/**A Malloc failed*/
#define E_OA_MALLOC               -205
/*@}*/

typedef struct 
{
	size_t  nelem;
	size_t  elem_sz;
	size_t  inc_asize;
  	size_t  allocated;
	void    *array;
} ObjAlloc_t;


ObjAlloc_t *OA_open(size_t, size_t, size_t);

int OA_close(ObjAlloc_t *);

/*void *OA_array(ObjAlloc_t *);

int OA_cardinality(ObjAlloc_t *);*/

void *OA_insert(ObjAlloc_t *, size_t, void *);

int OA_remove(ObjAlloc_t *, size_t);

ObjAlloc_t *OA_import(size_t,size_t,size_t,void*,size_t);

char *OA_error_description(void);

int OA_bsearch (ObjAlloc_t*,void*,int(*)(void*,void*),int*);
int OA_lsearch(ObjAlloc_t*,void*,int(*)(void*,void*),int*);
/**@name Defined Macros*/
/*@{*/
/**Return the included data array of the object.*/
#define OA_array(oa)         oa->array
/**Return the number of elements currently in the data structure.*/
#define OA_cardinality(oa)   oa->nelem
/**Return the size of the elements making up the data structure.*/
#define OA_size(oa)          oa->elem_sz
/*@}*/
#endif /*_OBJ_ALLOCATION_MANAGER*/
#ifndef _OBJ__ALLOCATION_MANAGER
/**@name Type definitions*/
/*@{*/
/**@name The ObjAlloc\_t type
   
   This is the allocation object type.
\begin{verbatim}
typedef struct 
{
        size_t  nelem;
        size_t  elem_sz;
        size_t  inc_asize;
        size_t  allocated;
        void    *array;
} ObjAlloc_t;
\end{verbatim}
\begin{description}
\item [nelem] The amount of elements in the data list.
\item [elem\_sz] The size of the elements.
\item [inc\_asize] The incrementor size.
\item [allocated] The currently allocated space.
\item [*array] The data list. 
\end{description}
*/
/*@}*/

//@Include: ObjAlloc.c
#endif/*_OBJ_ALLOCATION_MANAGER*/
/*@}*/
