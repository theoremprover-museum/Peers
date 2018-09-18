/************ BASIC INCLUDES ************/

#include <stdio.h>
#ifndef TP_NO_STDLIB
#include <stdlib.h>  /* doesn't exist on some UNIXes */
#endif

/*********** INCLUDES FOR TIMES AND DATES ************/

#include <sys/time.h>
#include <sys/resource.h>  /* p4 includes this */

/* #include <time.h>  */
/* added to compute run-time on HP's before finding what follows */

#ifdef HP_UX
#include <sys/syscall.h>
#define getrusage(a, b)  syscall(SYS_GETRUSAGE, a, b)
#endif /* HP_UX */

/*********** SIZES OF INTEGERS ***************/

#define MAX_LONG_INT 2147483647
#define MAX_INT      2147483647

/******** TYPES OF TERM *********/

#define NAME 1        /* basic types of term */
#define VARIABLE 2
#define COMPLEX 3

#define MAX_VARS 300   /* maximum # of distinct variables in clause */

/******** TERM ORDERING *********/

#define LESS_THAN        1
#define GREATER_THAN     2
#define SAME_AS          3
#define NOT_COMPARABLE   4
#define NOT_GREATER_THAN 5
#define NOT_LESS_THAN    6

#define LRPO_MULTISET_STATUS  0
#define LRPO_LR_STATUS        1

/******** Types of exit ********/

#define PROOF_EXIT           10
#define ABEND_EXIT           11
#define SOS_EMPTY_EXIT       12
#define MAX_GIVEN_EXIT       13
#define INTERRUPT_EXIT       14
#define INPUT_ERROR_EXIT     15
#define MAX_SECONDS_EXIT     16

#include "Cos.h"        /* flag, parameter, statistic, and clock names */

/************* END OF ALL GLOBAL CONSTANT DEFINITIONS ****************/

#include "Macros.h"  /* preprocessor (#define) macros */

#include "Proto.h"   /* function prototypes */

#include "Types.h"   /* the basic type declarations */

/******** Global Variables *********/

#ifdef IN_MAIN
#define GLOBAL         /* empty string if included by main program */
#else
#define GLOBAL extern  /* extern if included by anything else */
#endif

GLOBAL struct flag {  /* Flags are boolean valued options */
    char *name;
    int val;
    } Flags[MAX_FLAGS];

GLOBAL struct parm {  /* Parms are integer valued options */
    char *name;
    int val;
    int min, max;  /* minimum and maximum permissible values */
    } Parms[MAX_PARMS];

/* statistics */

GLOBAL long Stats[MAX_STATS];

/* clocks */

GLOBAL struct clock {  /* for timing, see cos.h, macros.h, and clocks.c */
    long accum_msec;   /* accumulated time */
    long curr_msec;    /* time since clock has been turned on */
    } Clocks[MAX_CLOCKS];

/* Output files: each peer has its own output file: peer1, ..., peern. */

GLOBAL FILE *Peer_fp;

/***********************************************************************/
