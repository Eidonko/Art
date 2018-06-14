# Art
Translator of the Ariel recovery language

ARIEL is a declarative language with a syntax somewhat similar to that of the UNIX shells. One
instruction per line is allowed. Comments are like in the C shell (“#” starts a comment which
ends at next new line or at end-of-file). Names are not case-sensitive. ARIEL deals with five basic
types: “nodes”, “tasks”, “logicals”, integers, and real numbers. A node is a uniquely identifiable
processing node of the system, e.g., a processor of a MIMD supercomputer. A task is a uniquely
identifiable process or thread in the system. A logical is a uniquely identifiable collection of
tasks, possibly running on different nodes. Nodes, tasks, and logicals are generically called
entities. Entities are uniquely identified via non-negative integers; for instance, NODE3 or N3
refer to processing node currently configured as number 3.

Integer symbolic constants can be “imported” from C language header files through the statement
INCLUDE. For instance, if the C language header file "vf.h" contains a define statement
such as:

    #define PROC NUM 4,

then it is possible to use that symbolic constant wherever an integer is expected in the language.
To de-reference a symbolic constant imported via INCLUDEs a “brace-operator” has
been defined—for instance, under the above assumptions the following valid ARIEL statement:

    NPROCS = {PROC_NUM}

(described later on) is equivalent to

    NPROCS = 4.

An ARIEL script basically consists of two parts:
- A part dedicated to configuration.
- A part containing the guarded actions which constitute the user-defined error recovery
strategy.

## Configuration
The ARIEL configuration language can be used to define and configure the target system and
application entities, e.g., nodes, tasks, and group of tasks. For instance

    TASK 3 = "TMR.EXE" IS NODE 0, TASKID 8

declares that task 8, local to node 0, is to be globally referred to as “task 3”. String 
“TMR.EXE” may also be used to refer symbolically to task3. More complex rules are possible—for instance,

    TASK [1,3] = "Triple" IS NODE 0, TASKID [6,8]

is equivalent to

    TASK 1 = "Triple1" IS NODE 0, TASKID 6,
    TASK 2 = "Triple2" IS NODE 0, TASKID 7,
    TASK 3 = "Triple3" IS NODE 0, TASKID 8.

Line

    LOGICAL 10 = "TMR" IS TASK 5, TASK 6, TASK 7 END LOGICAL

defines a group of tasks to be globally referred to as “logical 10”, symbolically known as TMR, and corresponding
to the three tasks whose unique-id are 5, 6, and 7.

Let us assume the following lines have been written in a file called “test1.ariel”:

    TASK 5 = "TMR_LEADER" IS NODE 0, TASKID 8
    TASK 6 = "TMR2" IS NODE 1, TASKID 8
    TASK 7 = "TMR3" IS NODE 2, TASKID 8
    LOGICAL 10 = "TMR" IS TASK 5, TASK 6, TASK 7 END LOGICAL

File test1.ariel can be translated by executing the following command:

    $ art -i test1.ariel
    Ariel translator, v3.0e, by Eidon@tutanota.com.
    Parsing file test1.ariel...
    ...done (4 lines.)
    Output written in file .rcode.
    Logicals written in file outputs/LogicalTable.csv.
    Tasks written in file outputs/TaskTable.csv.
    Alpha-count parameters written in file outputs/alphacount.h.

Extra files are created by using the "-s" option.

    $ art -s -i test1.ariel
    Ariel translator, v3.0e, by Eidon@tutanota.com.
    Parsing file test1.ariel...
    ...done (4 lines.)
    Output written in file .rcode.
    Logicals written in file outputs/LogicalTable.csv.
    Tasks written in file outputs/TaskTable.csv.
    static version
    Preloaded r-codes written in file outputs/trl.h.
    Time-outs written in file outputs/timeouts.h.
    Identifiers written in file outputs/identifiers.h.
    Alpha-count parameters written in file outputs/alphacount.h.

The “-s” option, for “static”, requests the writing of a number of header
files. The produced header files contain definitions like the following one, from file “timeouts.h”:

ARIEL can be used to configure statically the TIRAN tools. The current prototypic version can
configure only one tool, the TIRAN watchdog. The following syntax is recognised by art to
configure it:

    WATCHDOG 10 WATCHES TASK 14
        HEARTBEATS EVERY 100 MS
        ON ERROR WARN TASK 18
    END WATCHDOG.

The output in this case is a C file that corresponds to a configured instance of a watchdog.

ARIEL supports N-version programming [Avi85]. The following is an example that shows how it is
possible to define an “N-version task” with ARIEL:

    N-VERSION TASK 1
        VERSION 1 IS TASK2
        VERSION 2 IS TASK4
        VERSION 3 IS TASK8
        METRIC "nvp_comp"
        ON SUCCESS TASK5
        ON ERROR TASK12
        VOTING ALGORITHM IS MAJORITY
    END N-VERSION

If we invoke art with as input the above excerpt (see file test2.ariel), a number of C codes are automatically created:

    $ art -s -i test2.ariel
    Ariel translator, v3.0e, by Eidon@tutanota.com.
    ...
    
    $ ls -lt | head -5
    total 1255
    -rwxrwx---+ 1 DEFLORIV Domain Users    221 Jun 14 22:17 output.rcode
    -rw-rw-r--+ 1 DEFLORIV Domain Users   1196 Jun 14 22:17 TIRAN_task_8.c
    -rw-rw-r--+ 1 DEFLORIV Domain Users   1196 Jun 14 22:17 TIRAN_task_4.c
    -rw-rw-r--+ 1 DEFLORIV Domain Users   1196 Jun 14 22:17 TIRAN_task_2.c

    $ less TIRAN_task_8.c
    #include "../TIRAN_API.h"
    /* Task 8 of NVersion Task 1
       Version 3 / 3
     */

    int TIRAN_task_8(void) {
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
            TIRAN_VotingDescribe(dv, 2, 1, 0);
            TIRAN_VotingDescribe(dv, 4, 2, 0);
            TIRAN_VotingDescribe(dv, 8, 3, 1);

            TIRAN_VotingRun(dv);

            /* output should be sent to task 5 */
            TIRAN_VotingOutput(dv, 5);
            TIRAN_VotingOption(dv, TIRAN_VOTING_IS_MAJORITY);
    ...

The above is one of the three files produced by the ARIEL translator when it parses test2.ariel.
Said file configures an instance of the TIRAN DV tool (distributed voting tool), which I also designed and developed.
Note also how all technicalities concerning:

- the API of the tool,
- input replication,
- the adopted voting strategy,
- output communication,

and so forth are fully made transparent to the designer, who needs only be concerned with the
functional service. This allows the fault-tolerance designer to modify all the above mentioned
technicalities with no repercussions on the tasks of the application designer, and even to deploy
different strategies depending on the particular target platform.

## ARIEL as a recovery language
Recovery strategies are collections of sections with the following syntax:

    section : if elif else fi ;
    if : IF ’[’ guard ’]’ THEN actions ;
    elif :
         | ELIF ’[’ guard ’]’
           THEN actions elif ;
    else :
         | ELSE actions ;
    fi : FI ;

where non-terminals 'guard' and 'actions' are respectively a condition testing the status of a task or a node
and an action affecting a task, a node, or a group of tasks.

An excerpt of the context-free grammar rules for guards follows:

    status :FAULTY | RUNNING | REBOOTED | STARTED | ISOLATED | RESTARTED | TRANSIENT ;
    entity :GROUP | NODE | TASK ;
    expr : status entity
         |  ’(’ expr ’)’
         | expr AND expr
         | expr OR expr
         | NOT expr
         | ERRN ’(’ entity ’)’ comp NUMBER
         | PHASE ’(’ entity ’)’ comp NUMBER ;
    comp :EQ j NEQ j GT j GE j LT j LE ;

The following conditions and values have been foreseen:

- Faulty. This is true when an error notification related to a processing node, a group of tasks, or a single task, can be found in the TIRAN DB.
- Running. True when the corresponding entity is active and no error has been detected that regards it.
- Rebooted (only applicable to nodes). This means that a node has been rebooted at least once during the run-time of the application.
- Started (not applicable to nodes). This checks whether a waiting task or group of task has been started.
- Isolated. This clause is true when its argument has been isolated from the rest of the application through a deliberate action.
- Phase (only applicable to tasks). It returns the current value of an attribute set by any task via the public function RaiseEvent. This value is forwarded to the BB to represent its current “phase” or state (e.g., an identifier referring to its current algorithmic step, or the outcome of a test or of an assertion). For instance, a voting task could inform the BB that it has completed a given algorithmic step by setting a given integer value after each step (this approach is transparently adopted in the EFTOS voting tool and is described in more detail in [DFDL98c]). Recovery block tests can take advantage of this facility to switch back and try an alternate task when a primary one sets a “failure” phase or when a guarding watchdog expires because a watched task sent it no signs of life. This condition returns an integer symbol that can be compared via C-like arithmetic operators.
- Restarted (not applicable to nodes). This returns the number of times a given task or group has been restarted. It implies started.
- Transient is true when an entity has been detected as faulty and the current assessment of the alpha-count fault identification mechanism is “transient”. It implies faulty.

Furthermore, it is possible to query the number of errors that have been detected and pertain
to a given entity.

Complex guards can be built via the standard logic connectives and parentheses.
As an example, the following guard:

    FAULTY TASK{MASTER} AND ERRN(TASK{MASTER}) > 10 AND RESTARTED TASK{MASTER}.

checks whether the three conditions:

- the task, the unique-id of which is the value of the symbolic constant MASTER, has been detected as faulty;
- more than 10 errors have been associated to that task;
- that task has been restarted,

are all true.

“Actions” can be attached to the THEN or ELSE parts of a section. In the
current implementation of the language, these actions allow to start, isolate, restart, terminate a
task or a group of tasks, to isolate or reboot a node, to invoke a local function. Moreover, it is
possible to multicast messages to groups of tasks and to purge events from the DB.

An excerpt of the context-free grammar for ARIEL’s actions follows:

    actions :
            | actions action ;
    action :
           | section
           | recovery action ;

    recovery action
           : STOP entity
           | ISOLATE entity
           | START entity
           | REBOOT entity
           | RESTART entity
           | ENABLE entity
           | SEND NUMBER TASK
           | SEND NUMBER GROUP
           | WARN entity ( condition )
           | REMOVE PHASE entity FROM ERRORL
           | CALL NUMBER
           | CALL NUMBER ’(’ list ’)’
    condition : ERR NUMBER entity ;

A special case of action is a section, i.e., another guarded action.
This allows to specify hierarchies (trees) of sections such that, during the run-time evaluation of
the recovery strategies, a branch is only visited when its parent clause has been evaluated as true.
In the current, prototypic implementation, the following actions have been foreseen

- Stop terminates a task or a group of tasks, or initiates the shutdown procedure of a node8.
- Isolate prevents an entity to communicate with the rest of the system9.
- Reboot reboots a node (via the TIRAN Reboot Node BT).
- Start spawns (or, in static environments, awakes) a task or a group.
- Restart is reverting a task or group of tasks to their initial state or, if no other means are available,
- stopping that entity and spawning a clone of it.
- Enable awakes a task or group, or boots a node.
- Send multicasts (or sends) signals to groups of tasks (or single tasks).
- Warn informs a task or group of tasks that an error regarding an entity has been detected. Action “WARN x” is equivalent to action “SEND {WARN} x”
- Remove purges records from the section of the DB collecting the errors or the phases of a given entity.

Custom actions and conditions may be easily added to the grammar of ARIEL.

## ARIEL in action
Once fed with a recovery script, the art translator produces a binary pseudo-code, called the
r-code. In the current version, this r-code is written in a binary file and in a C header file as a
statically defined C array. The r-code is made of a set of “triplets” of integers,
given by an opcode and two operands. These are called “r-codes”.
This header file needs to be compiled with the application. Run-time error recovery is carried
out by the RINT module, which basically is an r-code interpreter. This module and its recovery
algorithm will be part of another repository. 

I now describe how to translate an ARIEL script into the r-code. The simple script test3.ariel
will be used as an example:

    INCLUDE "my_definitions.h"
    IF [ PHASE (T{VOTER1}) == {HAS FAILED} ]
    THEN
        STOP T{VOTER1}
        SEND {WAKEUP} T{SPARE}
        SEND {VOTER1} T{SPARE}
        SEND {SPARE} T{VOTER2}
        SEND {SPARE} T{VOTER3}
    FI

where my_definitions.h is as follows:

    #define ALARM 999
    #define SPARE 3
    #define VOTER1  0
    #define VOTER2  1
    #define VOTER3  2
    #define NODE1  1
    #define NODE2  2
    #define NODE3  3
    #define NODE4  4
    #define HAS_FAILED 9999

The following scenario is assumed: a triple modular redundancy (TMR) system consisting of
three voting tasks, identified by integers {VOTER1}, {VOTER2}, and {VOTER3} is operating.
A fourth task, identified as T{SPARE}, is available and waiting. It is ready to take over one
of the voting tasks should the latter fail. The failed voter signals its state to the backbone by
entering phase HAS_FAILED through some self-diagnostic module (e.g., assertions or control flow
monitoring). The spare is enabled when it receives a {WAKEUP} message and it requires
the identity of the voter it has to take over. Finally, it is assumed that once a voter receives a
control message with the identity of the spare, it has to initiate a reconfiguration of the TMR
such that the failed voter is switched out of and the spare is switched in the system.

The following command processes test3.ariel

    $ art -s -i test3.ariel    
    Ariel translator, v3.0e, by Eidon@tutanota.com.
    Parsing file test3.ariel...
    [ Including file `my_definitions.h' ...about to add association `ALARM' -> `999'
    about to add association `SPARE' -> `3'
    about to add association `VOTER1' -> `0'
    about to add association `VOTER2' -> `1'
    about to add association `VOTER3' -> `2'
    about to add association `NODE1' -> `1'
    about to add association `NODE2' -> `2'
    about to add association `NODE3' -> `3'
    about to add association `NODE4' -> `4'
    about to add association `HAS_FAILED' -> `9999'
    10 associations have been stored. ]
        substituting T{VOTER1} with T0
        substituting {HAS FAILED} with 1
        substituting T{VOTER1} with T0
        substituting {WAKEUP} with 18
        substituting T{SPARE} with T3
        substituting {VOTER1} with 0
        substituting T{SPARE} with T3
        substituting {SPARE} with 3
        substituting T{VOTER2} with T1
        substituting {SPARE} with 3
        substituting T{VOTER3} with T2
        if-then-else: ok
    ...done (9 lines.)
    Output written in file .rcode.
    static version
    Preloaded r-codes written in file outputs/trl.h.
    Time-outs written in file outputs/timeouts.h.
    Identifiers written in file outputs/identifiers.h.
    Alpha-count parameters written in file outputs/alphacount.h.

File outputs/trl.h containes the the r-codes:

    /********************************************************************************
      *                                                                              *
      *  Header file trl.h                                                           *
      *                                                                              *
      *  This file contains a preloaded set of r-codes for Ariel file   test3.ariel  *
      *  Written by art (           v3.0e) on                               (null)  *
      *  (c) Eidon@tutanota.com (https://github.com/Eidonko)    .                    *
      *                                                                              *
      ********************************************************************************/

    #ifndef _T_R_L__H_
    #define _T_R_L__H_

    #include "rcode.h"

    #define RCODE_CARD 16 /* number of rcodes that have been produced */


    static rcode_t rcodes[] = {

    /*line#*/      /* opcode */     /* operand 1 */  /* operand 2 */

    /*0*/       { R_INC_NEST,             -1,              -1 },
    /*1*/       { R_STRPHASE,              0,              -1 },
    /*2*/       { R_COMPARE,               1,               1 },
    /*3*/       { R_FALSE,                10,              -1 },
    /*4*/       { R_KILL,                 18,               0 },
    /*5*/       { R_PUSH,                 18,              -1 },
    /*6*/       { R_SEND,                 18,               3 },
    /*7*/       { R_PUSH,                  0,              -1 },
    /*8*/       { R_SEND,                 18,               3 },
    /*9*/       { R_PUSH,                  3,              -1 },
    /*10*/      { R_SEND,                 18,               1 },
    /*11*/      { R_PUSH,                  3,              -1 },
    /*12*/      { R_SEND,                 18,               2 },
    /*13*/      { R_DEC_NEST,             -1,              -1 },
    /*14*/      { R_OANEW,                 1,              -1 },
    /*15*/      { R_STOP,                 -1,              -1 },
    };

    #endif /* _T_R_L__H_ */

A textual representation of the r-codes is also produced in file output.rcode:

    Art translated Ariel strategy file: .... test3.ariel
    into rcode object file : ............... .rcode

     line                rcode     opn1   opn2
    -----------------------------------------------
    00000                   IF
    00001       STORE_PHASE...   Thread      0
    00002           ...COMPARE       ==      1
    00003                FALSE     10
    00004                 KILL   Thread      0
    00005              PUSH...     18
    00006              ...SEND   Thread      3
    00007              PUSH...      0
    00008              ...SEND   Thread      3
    00009              PUSH...      3
    00010              ...SEND   Thread      1
    00011              PUSH...      3
    00012              ...SEND   Thread      2
    00013                   FI
    00014      ANEW_OA_OBJECTS      1
    00015                 STOP
