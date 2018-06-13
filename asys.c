/******************************************************************************/
/*                                                                            */
/*     assoc                                                                  */
/*                                                                            */
/*         a class of associative arrays and Web-oriented functions           */
/*                                                                            */
/*     conceived, designed, and implemented by                                */
/*     <a href=ihttps://github.com/Eidonko>Eidon@tutanota.com</a>             */
/*                                                                            */
/*      asys.c                                                                */
/*      the source file for "system" functions in the class		              */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "assoc.h"

int  _brickalloc(brick**);
void _brickfree(brick*);


/******************************************************************************/
/*                                                                            */
/*              this function returns an associative array which holds        */
/*    aenv      the system environmental informations that the user may       */
/*              access with getenv(). returns NULL in case of errors,         */
/*              in which case, aerror contains an error message.              */
/*                                                                            */
/******************************************************************************/

ASSOC *aenv() {
  int i, l;
  char *tmp;
  char *name, *value;
  ASSOC *a;

  if (*environ == NULL) { strcpy(aerror, "can't read environ"); return NULL; }

  a=aopen((int (*)(const void*, const void*))strcmp);
  if (!a) return NULL;

  for (i=0; environ[i]; i++) {
    l=strlen(environ[i]);
    tmp=(char*)malloc(l);
    if(!tmp) { strcpy(aerror, "alloc"); return NULL; }
    strcpy(tmp, environ[i]);

    printf("Environment string no. %d = \"%s\"\n", i+1, tmp);

    name=strchr(tmp, '=');
    *name='\0';
    value=name+1;
    name = tmp;

#   ifdef DEBUG
    printf("wrote (\"%s\", \"%s\")\n", name, value);
#   endif

    if (A_OK != awrite(a, name, value)) return NULL;
  }

  return a;
}

/******************************************************************************/
/*                              end of file asys.c                            */
/******************************************************************************/
