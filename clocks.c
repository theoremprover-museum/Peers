#include "Header.h"

/*************
 *
 *    init_clocks() - Initialize all clocks.
 *
 *************/

void init_clocks()
{
    int i;

    for (i=0; i<MAX_CLOCKS; i++)
	clock_reset(i);
}  /* init_clocks */

/*
 *
 *    CPU_TIME(msec) - It has been msec milliseconds (UNIX user time)
 *    since the start of this process.
 *
 */

/* This routine has been made into a macro. */

/*
 *
 *    CLOCK_START(clock_num) - Start or continue timing.
 *
 *    If the clock is already running, a warning message is printed.
 *
 */

/* This routine has been made into a macro. */

/*
 *
 *    CLOCK_STOP(clock_num) - Stop timing and add to accumulated total.
 *
 *    If the clock not running, a warning message is printed.
 *
 */

/* This routine has been made into a macro. */

/*************
 *
 *    long clock_val(clock_num) - Returns accumulated time in milliseconds.
 *
 *    Clock need not be stopped.
 *
 *************/

long clock_val(c)
int c;
{
    long msec, i, j;

    i = Clocks[c].accum_msec;
    if (Clocks[c].curr_msec == -1)
	return(i);
    else {
	CPU_TIME(msec)
	j = msec - Clocks[c].curr_msec;
	return(i+j);
	}
}  /* clock_val */

/*************
 *
 *    clock_reset(clock_num) - Clocks must be reset before being used.
 *
 *************/

void clock_reset(c)
int c;
{
    Clocks[c].accum_msec = 0;
    Clocks[c].curr_msec = -1;
}  /* clock_reset */

/*************
 *
 *   char *get_time() - get a string representation of current date and time
 *
 *************/

char *get_time()
{
    long i;

    i = time((long *) NULL);
    return(asctime(localtime(&i)));
}  /* get_time */

/*************
 *
 *    long system_time() - Return system time in milliseconds.
 *
 *************/

long system_time()
{
#ifdef TP_RUSAGE
    struct rusage r;
    long sec, usec;

    getrusage(RUSAGE_SELF, &r);
    sec = r.ru_stime.tv_sec;
    usec = r.ru_stime.tv_usec;

    return((sec * 1000) + (usec / 1000));
#else
    return(0);
#endif
}  /* system_time */

/*************
 *
 *    long run_time() - Return run time in milliseconds.
 *
 *    This is used instead of the normal clock routines in case
 *    program is compiled with NO_CLOCK.
 *
 *************/

long run_time()
{
#ifdef TP_RUSAGE
    struct rusage r;
    long sec, usec;

    getrusage(RUSAGE_SELF, &r);
    sec = r.ru_utime.tv_sec;
    usec = r.ru_utime.tv_usec;

    return((sec * 1000) + (usec / 1000));
#else
    long ticks;
    long sec;

    ticks = clock();
    sec = ((double) ticks / CLOCKS_PER_SEC) * 1000;
    return(sec);
#endif
}  /* run_time */

/*************
 *
 *     wall_seconds()
 *
 *************/

long wall_seconds()
{
    long i;

    i = time((long *) NULL);
    return(i);
}  /* wall_seconds */
