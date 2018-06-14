#include "../TIRAN_API.h"
/* Task 2 of NVersion Task 1
   Version 1 / 3
 */

int TIRAN_task_2(void) {
	TIRAN_Voting_t *dv;
	size_t size;
	double nvp_comp(const void*, const void*);

	dv = TIRAN_VotingOpen(nvp_comp);
	if (dv == NULL) {
		RaiseEvent(TIRAN_ERROR_VOTING_CANTOPEN);
		TIRAN_exit(TIRAN_ERROR_VOTING_CANTOPEN);
	}

	/* voting task description: which tasks and which versions */
	/* constitute the n-version task */
	TIRAN_VotingDescribe(dv, 2, 1, 1);
	TIRAN_VotingDescribe(dv, 4, 2, 0);
	TIRAN_VotingDescribe(dv, 8, 3, 0);

	TIRAN_VotingRun(dv);

	/* output should be sent to task 5 */
	TIRAN_VotingOutput(dv, 5);
	TIRAN_VotingOption(dv, TIRAN_VOTING_IS_MAJORITY);

	/* redirect stdout into a pipe input stream */
	TIRAN_pipework();

	/* execute the version */
	/* Nota Bene: time-out management to be added */
	task_2();

	size = read(0, buff, MAX_BUFF);
	if (size > 0) {
		/* forward the input buffer to the local voter of this version */
		TIRAN_VotingInput(dv, buff, size);
	} else {
		/* signal there's no input */
		TIRAN_VotingInput(dv, NULL, 0);
		RaiseEvent(TIRAN_ERROR_VOTING_NOINPUT, 2);
		TIRAN_NotifyTask(12, TIRAN_ERROR_VOTING_NOINPUT);
	}
}
/* EOF file TIRAN_task_2.c */
