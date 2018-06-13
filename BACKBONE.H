/*******************************************************************************
**
** Copyright (c)1999, 2000  K.U.Leuven/ESAT/ACCA, Leuven, 2000
** All Rights Reserved.
**
********************************************************************************
** Project      :    TIRAN
** Task			:    TIRAN Backbone
** Module       :    TIRAN backbome main module
** File Name    :    BACKBONE.H
**
** Date Created :    Dec 1999
** Created By   :    V De Florio <VDF>
** Organization :    K.U.Leuven
**
** Version      :    1.0
** Last Changed :	 14 Feb 2000
** Author       :	 VDF
** Organization :	 K.U.Leuven
** Saved at     :	 
**
** Revisions    :
** Rev ID Who   Date         What                            Approved/Date
** --- -- ----- ------------ ------------------------------- -------------------
**   1     AS   <dd/MMM/yyyy>                                <ini>/<dd/MMM/yyyy>
**
********************************************************************************
** Description :
** HEADER FOR THE TIME-OUTS, MAILBOXES, LOGICALS OF THE TIRAN BACKBONE
**
*******************************************************************************/
#ifndef __BACKBONE_H_
#define __BACKBONE_H_

#include "TIRAN_API.h"

#define IA_FLAG_TIMEOUT 10
#define IA_FLAG_CYCLIC TOM_CYCLIC
#define IA_FLAG_DEADLINE IMALIVE_CLEAR_TIMEOUT

#define MIA_TIMEOUT 15
#define MIA_CYCLIC TOM_CYCLIC
#define MIA_DEADLINE MIA_SEND_TIMEOUT

#define TAIA_TIMEOUT 20
#define TAIA_CYCLIC TOM_CYCLIC
#define TAIA_DEADLINE TAIA_RECV_TIMEOUT

#define TEIF_TIMEOUT_A 30
#define TEIF_CYCLIC TOM_NON_CYCLIC
#define TEIF_DEADLINE TEIF_TIMEOUT

#define IA_FLAG_TIMEOUT_B 50
#define IA_FLAG_CYCLIC_B TOM_CYCLIC
#define IA_FLAG_DEADLINE_B IMALIVE_CLEAR_TIMEOUT

#define MIA_TIMEOUT_B 55
#define MIA_CYCLIC_B TOM_CYCLIC
#define MIA_DEADLINE_B MIA_RECV_TIMEOUT

#define TAIA_TIMEOUT_B 60
#define TAIA_CYCLIC_B TOM_CYCLIC
#define TAIA_DEADLINE_B TAIA_SEND_TIMEOUT

#define TEIF_TIMEOUT_B 70
#define TEIF_CYCLIC_B TOM_NON_CYCLIC
#define TEIF_DEADLINE_B TEIF_TIMEOUT

#define IAT_TIMEOUT 40
#define IAT_CYCLIC TOM_CYCLIC
#define IAT_DEADLINE IMALIVE_SET_TIMEOUT

#define WADB_TIMEOUT 42
#define WADB_DEADLINE 1000000

#define INJECT_FAULT_TIMEOUT 6
#define INJECT_FAULT_DEADLINE 6000000

#define MID_TIMEOUT 56
#define MID_CYCLIC TOM_NON_CYCLIC
#define MID_DEADLINE MANAGER_IS_DOWN_TIMEOUT

void TIRAN_BackboneGenericComponent(void);
void RestartManager(void);
int GetNumTasks(void);
void StopTask(void);
void BACKBONERestartTask(void);

char* role2ascii(int);
char* BACKBONEPrintTimeout(int);
char* BACKBONEPrintMessage(int);
char* BACKBONEPrintCode(int);
char* PrintStatus(void);

#define KillTask(a) TIRAN_KillTask(local_to_global[TIRAN_ThisNodeId][a])
#define RestartTask(a) TIRAN_RestartTask(local_to_global[TIRAN_ThisNodeId][a])
#define StartTask(a) TIRAN_CreateTask(local_to_global[TIRAN_ThisNodeId][a],NULL)

/* local task-id's for the IAT, RINT and backbone generic component */
#define IAT_TASKID			5
#define RINT_TASKID			9
#define BACKBONE_TASKID		1

#define BACKBONE_UNIQUEID	(0 + TIRAN_ThisNodeId)  // 1--4
#define IAT_UNIQUEID		(4 + TIRAN_ThisNodeId)  // 5--8
#define RINT_UNIQUEID		(8 + TIRAN_ThisNodeId)  // 9--12

#define Backbone_Mbox		BACKBONE_TASKID
#define IAT_Mbox			IAT_TASKID
#define RINT_Mbox			RINT_TASKID

#define B_LOGICAL(i)		(0 + i)
#define I_LOGICAL(i)		(4 + i)
#define R_LOGICAL(i)		(8 + i)

#  ifdef FIRST_NODE_IS_ZERO
#   define FRST  0
#   define CMPR  <
#   define ADJUST 0
#  else
#   define FRST   1
#   define CMPR  <=
#   define ADJUST 1
#  endif

#endif