/* kbhit.c - routine to check for keyboard char present */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>

extern int tcgetattr (int fd, struct termios *t);
extern int tcsetattr (int fd, int action, const struct termios *t);

struct termios save_termios;

int kbwait (void);
int kbhit (void);
void init_term (void);
void restore_term (void);

int
kbwait (void) {

	setbuf (stdout, 0);             /* unbuffer stdout */
	init_term ();

	//return kbhit ();

	restore_term ();

	return 0;

}

/* *************************************************************************
*/

void
init_term (void) {
        struct termios ios;

if (tcgetattr (STDIN_FILENO, &save_termios) < 0)
        exit errno;

ios = save_termios;
ios.c_lflag &= ~(ICANON | ECHO);
ios.c_cc[VMIN] = 1;
ios.c_cc[VTIME] = 0;

if (tcsetattr (STDIN_FILENO, TCSAFLUSH, &ios) < 0)
        exit errno;

}

/* *************************************************************************
*/

void
restore_term (void) {

if (tcsetattr (STDIN_FILENO, TCSAFLUSH, &save_termios) < 0)
        exit errno;

}

/* *************************************************************************
*/

int
kbhit (void) {
        fd_set rfds;
        struct timeval tv;

FD_ZERO (&rfds);
FD_SET (0, &rfds);
tv.tv_sec = 0;
tv.tv_usec = 0;

select (1, &rfds, 0, 0, &tv);

if (FD_ISSET (0, &rfds)) {
        return getc (stdin);
}
return 0;

}

/* *************************************************************************
*/
