#ifndef   __STA__GES__
#define   __STA__GES__

#define _MAX_STAGES   32
#define _MAX_SECTIONS 32

int stages[_MAX_STAGES][_MAX_SECTIONS];
int stagep[_MAX_STAGES];
int stagen;

int stagepush(int which, int progcounter)
{
	if (which >= _MAX_STAGES)
		return -1;

	if (stagep[ which ] >= _MAX_SECTIONS)
		return -2;

	if (which > stagen)
		stagen = which;

	return stages[which][ stagep[ which ]++ ] = progcounter;
}

int whichstagecard(int which) { return stagep [ which ]; }

#endif /* __STA__GES__ */
