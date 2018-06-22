#include <stdio.h>
#include <string.h>
// First include the assoc.h header
#include "assoc.h"

// Then define a function taking two const void * and returning an integer
// Note: the function must work as strcmp and compare two domain objects
int cmp(const void*a, const void*b) {
	return strcmp(a, b);
}

int main(int argc, char *argv[], char *arge[]) {
	// Then you declare your associative arrays.
	// This is the same as declaring FILE* pointers: these are containers waiting
	// to receive a reference to an ASSOC object!
	ASSOC *a, *b;
	char *s;


	// Then you "open" your first ASSOC. This is like using fopen(),
	// but you pass your object compare function instead of a filename
	a=aopen(cmp);
	if(*aerror) printf("aerror=%s\n", aerror);

	// awrite associates keys to values. This one is the same as
	// a['paris'] = 'france' # in Python
	awrite(a, "paris", "france");
	if(*aerror) printf("aerror=%s\n", aerror);

	// a['dublin'] = 'ireland' # in Python
	awrite(a, "dublin", "ireland");
	if(*aerror) printf("aerror=%s\n", aerror);

	// a['roma'] = 'italia' # in Python
	awrite(a, "roma", "italia");
	if(*aerror) printf("aerror=%s\n", aerror);

    // anext returns the next key in the array
	printf("anexts...\n");
	while (s = (char*)anext(a) ) {
		// aread returns the value associated with a key, if any
		// for s in a:           # In Python:
		//     print(a[s])
		printf("domain %s: codomain = %s\n", s, aread(a, s));
		if(*aerror) printf("aerror=%s\n", aerror);
	}

	// etc etc
	printf("awrites...\n");
	awrite(a, "london", "uk");
	if(*aerror) printf("aerror=%s\n", aerror);

	printf("anexts...\n");
	while (s = (char*)anext(a) ) {
		printf("domain %s: codomain = %s\n", s, aread(a, s));
		if(*aerror) printf("aerror=%s\n", aerror);
	}

	// arewind "rewinds" the keys of an array. No need to do that in Python...
	printf("\narewind\n\n");
	arewind(a);
	if(*aerror) printf("aerror=%s\n", aerror);

	printf("anexts...\n");
	while (s = (char*)anext(a) ) {
		printf("domain %s: codomain = %s\n", s, aread(a, s));
		if(*aerror) printf("aerror=%s\n", aerror);
	}

	// adel removes a key -> value association
	printf("adel...\n");
	adel(a, "roma");

	printf("\narewind\n\n");
	arewind(a);
	if(*aerror) printf("aerror=%s\n", aerror);

	printf("anexts...\n");
	while (s = (char*)anext(a) ) {
		printf("domain %s: codomain = %s\n", s, aread(a, s));
		if(*aerror) printf("aerror=%s\n", aerror);
	}

	printf("aread(\"roma\")=%s, aerror=\"%s\"\n", aread(a, "roma"), aerror);

    // aenv constructs an ASSOC object with all environment variables
	printf("aenv()...\n");
	b=aenv();
	printf("...done\n");
	if(*aerror) printf("aerror=%s\n", aerror);
	while (s = (char*)anext(b) ) {
		printf("domain %s: codomain = %s\n", s, aread(b, s));
		if(*aerror) printf("aerror=%s\n", aerror);
	}

	// this is destructor! It's like fclose(), but closes the array instead of the file
	aclose(a);
}
