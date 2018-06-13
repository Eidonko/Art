#ifndef _ASSOC_HEADER
#define _ASSOC_HEADER


/******************************************************************************/
/*                                                                            */
/*     assoc                                                                  */
/*                                                                            */
/*         a class of associative arrays and Web-oriented functions           */
/*                                                                            */
/*     conceived, designed, and implemented by                                */
/*     <a href=http://www.esat.kuleuven.ac.be/~deflorio>Eidon@tutanota.com</a>*/
/*     Copyright (c) 1995, 2001, Eidon@tutanota.com. All rights reserved.     */
/*                                                                            */
/*     Modified in March 1996 by Cesare Antifora,                             */
/*     to fix a bug in the decoding procedure.                                */
/*                                                                            */
/*      Rel. date: Feb 27, 2001.  Version 1.1                                 */
/*                                                                            */
/******************************************************************************/

/* the library mimes the fopen() class of functions.
 */

/* The class defines two main types:
 */
typedef struct brick {
	       struct brick *l, *r, *next;
	       void  *i, *o;
	       int   status;
	} brick;
typedef struct {
	       brick root, *current, *p, *last;
	       int (*acmp)(const void*,const void*);
	} ASSOC;

typedef brick apos_t;




/* A few defines are needed */
#define A_OK	  1                              /* the couple is not deleted */
#define A_DELETED 2                                  /* the couple is deleted */


#define A_ALLOC   (-1)            /* error code from an unsuccessful malloc() */





/* A global variable, aerror, keeps track of the last error */
char aerror[512];


/* max value for the CONTENT_LENGTH environment variable */
#define MAX_CGI_INPUT 4096




/* The functions' prototypes: */
ASSOC *aopen( int (*)(const void*,const void*) );
void   aclose(ASSOC*);
int    awrite(ASSOC*, void*, void*);
void  *aread(ASSOC*, void*);
void  *anext(ASSOC*);
void   arewind(ASSOC*);
void   adel(ASSOC*, void*);

ASSOC *acgi(void);
ASSOC *ascgi(char*);
ASSOC *aargcgi(char**, int);

ASSOC *aenv();

void   asave(ASSOC*);
void   aload(ASSOC*);

int acmp(const void*, const void*);
#endif
