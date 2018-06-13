/*
 *
 *     File: roles.c
 *
 *     Description: a function to set the role of nodes and a function
 *                  to know the default action within the DIR net.
 *
 *     By <a href=ihttps://github.com/Eidonko>Eidon@tutanota.com</a>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dirnet.h"
#include "rcode.h"

/* returns the number of roles assigned within the DIR net */
int DIR_Roles(char *roles)
{
	char *fname = RCODE_FILE;
	FILE *f;
	rcode_t  r;
	int   n;

	f = fopen(fname, "r+b");

	/* added v1.1 */
	if (f == NULL)
	{
		char fname2[80], *path;
		path = getenv("EFTOS_HOME");
		if (path == NULL)
			return(0);
		sprintf(fname2, "%s/%s", path, fname);
		f = fopen(fname2, "r+b");
		if (f == NULL)
			return(0);
	}
	/* end v1.1 */

	n = 0;
	while ( fread(&r, sizeof(r), 1, f) > 0 )
	{
		if (r[0] == R_SET_ROLE)
		{
			roles [ r[1] ] = (r[2]==R_AGENT)? DIR_AGENT :
					                  DIR_BACKUP;
			n++;
		}
		else
		if (r[0] == R_FALSE)   /* stop at the first FALSE statement */
			break;
	}


	fclose(f);
	return (n);
}
/* returns R_DA_IS_KILL or R_DA_IS_RESTART on success, and 0 in case of failure */
int DIR_Default_Action()
{
	char *fname = RCODE_FILE;
	FILE *f;
	rcode_t  r;
	int   n;

	f = fopen(fname, "r+b");

	n = 0;
	while ( fread(&r, sizeof(r), 1, f) > 0 )
	{
		if (r[0] == R_SET_DEF_ACT)
		{
			return (r[1]);
		}
	}


	fclose(f);
	return (0);
}
/* eof roles.c */
