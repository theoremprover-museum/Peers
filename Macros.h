/*
 *         Copyright (C) Argonne National Laboratory
 *
 *   Argonne does not guarantee this software in any manner and is
 *   not responsible for any damages that may result from its use.
 *   Furthermore, Argonne does not provide any formal support for this
 *   software.  This is an experimental program.  This software
 *   or any part of it may be freely copied and redistributed,
 *   provided that this paragraph is included in each source file.
 *
 */

/*
 *  macros.h -- This file contains some #define preprocessor macros
 *
 */

/*************
 *
 *    CPU_TIME(msec) - It has been msec milliseconds  (UNIX user time)
 *        since the start of this process.
 *
 *************/

#ifdef TP_RUSAGE
#define CPU_TIME(msec)  \
{  \
    struct rusage r;  \
    getrusage(RUSAGE_SELF, &r);  \
    msec = r.ru_utime.tv_sec * 1000 + r.ru_utime.tv_usec / 1000;  \
}  /* CPU_TIME */
#else
#ifdef HP_UX
#define CPU_TIME(msec)  \
{  \
    struct rusage r;  \
    getrusage(RUSAGE_SELF, &r);  \
    msec = r.ru_utime.tv_sec * 1000 + r.ru_utime.tv_usec / 1000;  \
}  /* CPU_TIME */
#else
#define CPU_TIME(msec)  \
{ \
    long ticks; \
    long sec; \
    ticks = clock(); \
    msec = (ticks * 1000.) / CLOCKS_PER_SEC; \
}	/* CPU_TIME */
#endif
#endif

/*************
 *
 *    CLOCK_START(clock_num) - Start or continue timing.
 *
 *        If the clock is already running, a warning message is printed.
 *
 *************/

#ifdef NO_CLOCK
#define CLOCK_START(c)   /* empty string */
#else
#define CLOCK_START(c)  \
{  \
    struct clock *cp;  \
  \
    cp = &Clocks[c];  \
    if (cp->curr_msec != -1) {  \
	fprintf(stderr, "WARNING, CLOCK_START: clock %d already on.\n", c);  \
	printf("WARNING, CLOCK_START: clock %d already on.\n", c);  \
	}  \
    else  \
	CPU_TIME(cp->curr_msec) \
}  /* CLOCK_START */
#endif

/*************
 *
 *    CLOCK_STOP(clock_num) - Stop timing and add to accumulated total.
 *
 *        If the clock not running, a warning message is printed.
 *
 *************/

#ifdef NO_CLOCK
#define CLOCK_STOP(c)   /* empty string */
#else
#define CLOCK_STOP(c)  \
{  \
    long msec;  \
    struct clock *cp;  \
  \
    cp = &Clocks[c];  \
    if (cp->curr_msec == -1) {  \
	fprintf(stderr, "WARNING, CLOCK_STOP: clock %d already off.\n", c);  \
	printf("WARNING, CLOCK_STOP: clock %d already off.\n", c);  \
	}  \
    else {  \
	CPU_TIME(msec)  \
	cp->accum_msec += msec - cp->curr_msec;  \
	cp->curr_msec = -1;  \
	}  \
}  /* CLOCK_STOP */
#endif

