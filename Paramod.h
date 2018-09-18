/* The following is used to build a list of pairs for paramodulation. */

struct para_pair {
    int weight;
    struct literal *c1;
    struct literal *c2;
    struct para_pair *prev, *next;
    };

#define PP_INDEX_SIZE 100

/* positions into which to paramodulate */

#define ALL         0
#define TOP_ONLY    1
#define ALL_BUT_TOP 2

/****************** Global variables for paramod code ************/

#ifdef OWNER_OF_PARA_GLOBALS
#define PARA_GLOBAL         /* empty string if included by owner */
#else
#define PARA_GLOBAL extern  /* extern if included by anything else */
#endif

PARA_GLOBAL struct para_pair *Para_pairs_first, *Para_pairs_last;
PARA_GLOBAL struct para_pair *Para_pairs_index[PP_INDEX_SIZE];
PARA_GLOBAL int Para_pairs_pending[PP_INDEX_SIZE];    /* for statistics only */
PARA_GLOBAL int Para_pairs_used[PP_INDEX_SIZE];       /* for statistics only */

