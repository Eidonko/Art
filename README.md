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

(to be continued)
