/******************************************************************************/
/*                                                                            */
/*     assoc                                                                  */
/*                                                                            */
/*         a class of associative arrays and Web-oriented functions           */
/*                                                                            */
/*     conceived, designed, and implemented by                                */
/*     <a href=ihttps://github.com/Eidonko>Eidon@tutanota.com</a>             */
/*                                                                            */
/*      assoc.c                                                               */
/*      main source file                                                      */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assoc.h"

int  _brickalloc(brick**);
void _brickfree(brick*);


/******************************************************************************/
/*                                                                            */
/*              this function opens a new associative array and returns       */
/*    aopen     a pointer to it. It returns NULL in case of error, !NULL      */
/*              otherwise. aerror is set to the error or burned accordingly   */
/*                                                                            */
/******************************************************************************/

ASSOC *aopen(int (*acmp)(const void*,const void*)) {
  ASSOC *a;

  a = (ASSOC*) malloc(sizeof(ASSOC));
  if (!a) { strcpy(aerror, "alloc"); return NULL; }

  a->current = & a->root;
  a->last = a->root.l  = a->root.r = a->root.next = NULL;
  a->root.i = a->root.o = NULL;

  a->p = NULL;
  a->acmp = acmp;
  *aerror = '\0';
  return a;
}



/******************************************************************************/
/*                                                                            */
/*              this function writes a new couple (or substitute an existing  */
/*    awrite    couple) in the associative array that is its first argument.  */
/*              In the first case a new brick is allocated. aerror is set or  */
/*              burned accordingly.                                           */
/*                                                                            */
/******************************************************************************/

int awrite(ASSOC *a, void *i, void *o) {
  brick *n, *this;
  int    cmp; 

  if (!a) { strcpy(aerror, "not aopen()'d"); return 0; }

  n = & a->root;

  if (n->i == NULL && n->o == NULL) {
     n->i = i; n->o = o;
     n->l = n->r = n->next = NULL;
     n->status = A_OK;
     a->last = n;
#    ifdef DEBUG
     printf("wrote: (\"%s\", \"%s\")\n", i, o);
#    endif
     return A_OK;
  }


  a->p = NULL;
  while ( n ) {
	if ((cmp = a->acmp(i, n->i)) == 0) {
	   n->o = o; 
	   n->status = A_OK;
	   strcpy(aerror, "overwritten");
#          ifdef DEBUG
           printf("overwrote: (\"%s\", \"%s\")\n", i, o);
#          endif

	   return A_OK;
	}

#       ifdef DEBUG
        printf("comparing \"%s\" with \"%s\"...%s\n", 
		i, n->i, (cmp<0)?"(l)":"(r)");
#       endif

	a->p = n;
	if (cmp<0) n = n->l ; else n = n->r;
  }


 /* a final leaf's been reached */
     if (_brickalloc(&this)) return A_ALLOC;

     this->l = this->r  = NULL;
     this->i = i; this->o = o;
     this->status = A_OK;
#    ifdef DEBUG
     printf("wrote: (\"%s\", \"%s\")\n", i, o);
#    endif

     if (cmp<0) a->p->l = this; else a->p->r = this;
     a->last->next = this;
     this->next = NULL;
     a->last = this;

     return A_OK;
}



/******************************************************************************/
/*                                                                            */
/*              this function returns the domain value corresponding to next  */
/*    anext     couple in the associative array, regarded as a linked list    */
/*              ordered after the succession of awrite() calls. As usual for  */
/*              aerror. Deleted couples are not considered.                   */
/*                                                                            */
/******************************************************************************/

void *anext(ASSOC* a) {
  void *i;

  while (a->current) {
     if (a->current->status != A_DELETED) {
	i = a->current->i;
	a->current = a->current->next;
	*aerror = '\0'; /* added! */
	return i;
     }
     if (a->current != NULL)
     {
	     if (a->current->next == NULL)
		     break;
	     a->current = a->current->next;
     }
  }

  strcpy(aerror, "no more couples");
  return NULL;
}



/******************************************************************************/
/*                                                                            */
/*              this function queries an associative for a value previously   */
/*    aread     coupled with a awrite. It returns NULL if such value does not */
/*              exist, or a value otherwise. If the value was previously      */
/*              deleted with adel(), it is returned and aerror is set to      */
/*              "deleted"; otherwise it's burned                              */
/*                                                                            */
/******************************************************************************/


void *aread(ASSOC *a, void *i) {
  brick *n = & a->root;
  int cmp;

  a->p = NULL;
  *aerror = '\0';
  while ( n  &&  (cmp = a->acmp(i, n->i)) ) {
	a->p = n;
	if (cmp<0) n=n->l; else n=n->r;
  }

  if (n==NULL) return NULL;

  a->p = n;
  if (n->status == A_DELETED)
     strcpy(aerror, "deleted");

  return n->o;
}



/******************************************************************************/
/*                                                                            */
/*              resets the pointer to next domain value to be read to the     */
/*    arewind   root of the associative array; aerror is burned.              */
/*                                                                            */
/******************************************************************************/

void arewind(ASSOC *a) {
  a->current = & a->root;
  *aerror = '\0';
}


int asetpos (ASSOC *a, apos_t *Position) {
  if (!a) { strcpy(aerror, "not aopen()'d"); return 0; }
  if (!Position) { strcpy(aerror, "invalid apos_t"); return 0; }

  a->current = Position;
  *aerror = '\0';
  return 1;
}
  

int agetpos (ASSOC *a, apos_t *Position) {
  if (!a) { strcpy(aerror, "not aopen()'d"); return 0; }
  Position = a->current;
  *aerror = '\0';
  return 1;
}




/******************************************************************************/
/*                                                                            */
/*              turns the status flag to the A_DELETED value. Memory is not   */
/*    adel      freed---only, from now on anext() will ignore the couple.     */
/*              Warning: dirty trick used: aread() is executed, then a side   */
/*              effect is exploited.                                          */
/*                                                                            */
/******************************************************************************/

void adel(ASSOC *a, void *i) {
  aread(a, i);
  a->p->status = A_DELETED;
                 /* aerror is not "touched": it's the same at exit of aread() */
}



/******************************************************************************/
/*                                                                            */
/*              close the associative array and free the memory for its       */
/*    aclose    pointer after having freed all the bricks.                    */
/*                                                                            */
/******************************************************************************/

void aclose(ASSOC *a) {
  *aerror = '\0';

   arewind(a);
   for (; a->current; a->current = a->current->next)
	 _brickfree(a->current);

   free(a);
}




/******************************************************************************/
/*                                                                            */
/*   _brick      functions for allocating/deallocating bricks                 */
/* free/alloc                                                                 */
/*                                                                            */
/******************************************************************************/

int  _brickalloc(brick** b) {
  *b = (brick*) malloc(sizeof(brick));
  *aerror = '\0';
  if (*b==NULL) { strcpy(aerror, "alloc"); return 1; }
  return 0;
}
void _brickfree(brick* b) { free(b); }


/******************************************************************************/
/*                             end of file assoc.c                            */
/******************************************************************************/
