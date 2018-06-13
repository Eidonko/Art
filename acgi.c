/******************************************************************************/
/*                                                                            */
/*     assoc                                                                  */
/*                                                                            */
/*         a class of associative arrays and Web-oriented functions           */
/*                                                                            */
/*     conceived, designed, and implemented by                                */
/*     Eidon@tutanota.com                                                     */
/*     Copyright (c) Eidon@tutanota.com. All rights reserved.           */
/*                                                                            */
/*      acgi.c                                                                */
/*      the source file for the CGI functions in the class                    */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assoc.h"

int  _brickalloc(brick**);
void _brickfree(brick*);


/******************************************************************************/
/*                                                                            */
/*              this function tokenize stdin according to the CGI standard    */
/*    acgi      and returns a pointer to an associative array or NULL in      */
/*              case of errors. aerror is set or burned accordingly.          */
/*                                                                            */
/******************************************************************************/

void url_decode(char *str) {
  int h1, h2;
  int i,j;
  /* i: index to encoded string; j: index to decoded string */
  for (i=0,j=0; str[i]; i++,j++)
      switch (str[i]) {
	case '+': str[j]=' ';
		  break;
	case '%': h1=str[i+1], h2=str[i+2]; 
		  if (isdigit(h1)) h1 -= '0'; else h1 = 10 + h1 - 'A';
		  if (isdigit(h2)) h2 -= '0'; else h2 = 10 + h2 - 'A';
		  str[j]=h1*16+h2;
		  i+=2;
		  break;
	default : str[j]=str[i];
		  break;
	/* are there any other cases that must be treated? */
      }
  str[j]='\0';
}



ASSOC *acgi() {
  int cl;
  int i;
  char *name, *value, *q, strinput[MAX_CGI_INPUT];
  ASSOC *a;

  cl = atoi(getenv("CONTENT_LENGTH"));

  if (cl>MAX_CGI_INPUT) {
    strcpy(aerror, "MAX_CGI_INPUT reached");
    return NULL;
  }
	

  for (i=0; i<cl; i++)
      strinput[i]=getchar();
      
  
  a=aopen((int (*)(const void*, const void*))strcmp);
# ifdef DEBUG
  if(*aerror) { printf("%s<p>\n", aerror); exit(1); }
#endif

  for ( i=0, name=strtok(strinput, "&");
	name !=NULL;
	i++ , name=strtok(NULL, "&")
      ) {
    q=strchr(name, '=');
    *q='\0';
    value=q+1;
    url_decode(name);
    url_decode(value);
    awrite(a, name, value);
#   ifdef DEBUG
    printf("wrote (\"%s\", \"%s\")\n<p>\n", name, value);
    if(*aerror) { printf("%s<p>\n", aerror); exit(1); }
#   endif
  }


  return a;
}
/******************************************************************************/
/*                              end of file acgi.c                            */
/******************************************************************************/
