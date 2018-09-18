/******** Type of function symbol, types of unification ********/

#define COMMUTE       1
#define ASSOC_COMMUTE 2

/******** AC Unification ********/

#define MAX_COEF    250
#define MAX_BASIS   100   /* must be <= MAX_VARS, because rows are indexed. */
#define MAX_COMBOS  200   /* for superset-restricted AC unif. */
#define MAX_AC_ARGS 500   /* 2500 is enough for ALU problem */

/******** dereference a variable ********/

#define DEREFERENCE(t, c) { int i; \
    while (c && t->type == VARIABLE && c->terms[i = t->varnum]) \
    { t = c->terms[i]; c = c->contexts[i]; } } 

/******** bind a variable, record binding in either trail or bt_node ********/

#define BIND_TR(i, c1, t2, c2, trp) { struct trail *tr; \
    c1->terms[i] = t2; c1->contexts[i] = c2; \
    tr = get_trail(); tr->varnum = i; tr->context = c1; \
    tr->next = *trp; *trp = tr; }

#define BIND_BT(i, c1, t2, c2, bt) {  \
    c1->terms[i] = t2; c1->contexts[i] = c2; \
    bt->varnum = i; bt->cb = c1; }

/******** context -- substitution table ********/

struct context {
    struct term *terms[MAX_VARS];
    struct context *contexts[MAX_VARS];
    int multiplier;  /* needed for apply, not for unify or match */
    int built_in_multiplier;  /* the use of this is optional */
    struct term *partial_term;
    };

/******** trail -- to record binding, so that it can be undone ********/

struct trail {
    struct trail *next;
    struct context *context;
    int varnum;
    };

/******** bt_node -- (backtrack node) for unification and matching ********/

/* In general, backtracking is handled by constructing and traversing     */
/* a tree built from nodes like this, rather than by recursively passing  */
/* the alternatives as an argument.                                       */

struct bt_node {

    struct bt_node *parent, *next, *prev, *first_child, *last_child;

    struct term *t1, *t2;         /* terms being unified or matched */
    struct context *c1, *c2;      /* respective contexts for variables */
                                  /* (for match, t1 is pattern, c2 is NULL) */

    int varnum;                   /* for unbinding when backtracking */
    struct context *cb;           /* for unbinding when backtracking */

    int alternative;              /* type of alternative (position) */

    /* for commutative unification */
    int flipped;
    struct bt_node *position_bt;  /* in sequence of alternatives */

    /* for AC unification */
    struct ac_position *ac;       /* position in sequence of AC unifiers */

    /* for AC matching */
    struct ac_match_pos *acm;     /* position in sequence of AC matchers */
    int partial;                  /* partial match for this pair */
   
    };

/******** ac_position -- position in sequence of AC unifiers ********/

struct ac_position {
    int m, n, num_basis;              /* # of coefficients and size of basis */
    int basis[MAX_BASIS][MAX_COEF];
    int constraints[MAX_COEF];        /* 0 for vars, else symbol number */
    struct term *args[MAX_COEF];
    struct context *arg_contexts[MAX_COEF];
    struct term *new_terms[MAX_COEF]; /* substitution terms */
    int combo[MAX_BASIS];             /* current subset of basis solutions */
    int sum[MAX_COEF];                /* solution corresponding to combo */
    struct term *basis_terms[MAX_BASIS][MAX_COEF];
    struct context *c3;               /* table for new variables */
    struct bt_node *sub_position;     /* position in sub-unification problem */
    int superset_limit;               /* for superset-restricted AC unif. */
    int combos[MAX_COMBOS][MAX_BASIS];/* for superset-restricted AC unif. */
    int combos_remaining;             /* for superset-restricted AC unif. */
    struct ac_position *next;         /* for avail list only */
    };

/******** ac_match_pos -- position in sequence of AC matchers ********/

struct ac_match_pos {
    struct term *t1, *t2;  /* t1 is pattern, t2 is subject */
    struct context *c1;    /* context for variables in t1  */
    int n1;                /* number of arguments in t1 */ 
    int n2;                /* size of set of set of args in t2 */
    struct term *args1[MAX_AC_ARGS], *args2[MAX_AC_ARGS];  /* the arguments */
           /* position in sequence of matches for complex args of args2 */
    struct bt_node *bt1[MAX_AC_ARGS];
           /* flags indicating which of args1 have been matched */
    int match1[MAX_AC_ARGS];
           /* integer indicating how many of each of args2 have been matched */
    int match2[MAX_AC_ARGS];
    int mults2[MAX_AC_ARGS];  /* multiplicities for args2 */
           /* indicates which of args2 are matched by bound vars in args1 */
    int bound_matches[MAX_AC_ARGS], bound_count;
    int last_a1_functor;   /* position of last non-variable arg in args1 */
           /* list of backtrack positions for free variables of args1 */
    struct ac_match_free_vars_pos *free_first, *free_last;
           /* # args of unmatched term---used for partial match */
    int partial_term_size;
    struct ac_match_pos *next;  /* for avail list only */
    };

/******* backtrack node for free variables of args1 ********/

struct ac_match_free_vars_pos {
    int varnum;                 /* the index of the free variable */
    int coef;                   /* # of occurrences of the var in args1 */
    int targets[MAX_AC_ARGS];   /* terms in args2 that can go with variable */
    int n;                      /* number of tragets*/
    int combo[MAX_AC_ARGS];     /* current subset of the targets */
    struct ac_match_free_vars_pos *prev, *next;
    };

    
    
