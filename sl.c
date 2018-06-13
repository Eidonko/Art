#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/root.h>
#include <sys/logerror.h>
#include <sys/link.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/thread.h>

int i;

int sender(LinkCB_t *link)
{
	for (i=0; i<4; i++)
	{
	SendLink(link, &i, sizeof(int));
	TimeWait((unsigned int) (TimeNow()+1*CLOCK_TICK));
	}
	StopLink(link);
	for (i=0; i<4; i++)
	{
	SendLink(link, &i, sizeof(int));
	TimeWait((unsigned int) (TimeNow()+1*CLOCK_TICK));
	}
}

int receiver(LinkCB_t *link)
{
	int j=1;

	while ( RecvLink(link, &i, sizeof(int)) >= 0 )
	{
	LogError(EC_MESS, "receiver", "ok");
	}
	perror("RecvLink has failed!");
}

main() {
int Error;
LinkCB_t *pipe[2];

LocalLink(pipe);
CreateThread(NULL, 0, (int (*)()) sender, &Error, pipe[0]);
CreateThread(NULL, 0, (int (*)()) receiver, &Error, pipe[1]);

sleep(40);
}
