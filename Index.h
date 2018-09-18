/******** Discrimination Tree Indexing, more general terms. ******/


#define AC_ARG_TYPE     10  /* WARNING---these are used along with NAME, */
#define AC_NV_ARG_TYPE   9  /* COMPLEX, and VARIABLE in Header.h, so     */
                            /* they must be different from those.        */

struct discrim {
    struct discrim *next;  /* sibling */
    union {
        struct discrim *kids;    /* for internal nodes */
        struct gen_ptr *data;    /* for leaves */
        } u;
    short lab;    /* variable number or symbol number */
    unsigned char type;    /* VARIABLE, NAME, or COMPLEX */
    };

struct flat {
    struct term *t;
    struct flat *prev, *next, *last;
    struct discrim *alternatives;
    int bound;
    int varnum;
    int place_holder;
    int num_ac_args;
    int num_ac_nv_args;
    };

struct discrim_pos {         /* to save position in set of subsuming terms */
    struct gen_ptr *data;    /* ident. terms from leaf of discrim tree */
    struct flat *f;          /* stack of states for backtracking */
    };

/********** FPA/Path indexing ***********/

/* types of retrieval */
#define MORE_GEN   1
#define INSTANCE   2
#define UNIFY      3

#define FPA_SIZE 500  /* size of FPA hash tables */

struct fpa_index {
    int depth;       /* maximum term depth to which indexing is applied */
    struct fpa_head *table[FPA_SIZE];
    };

struct fpa_head {            /* head of an FPA list */
    struct gen_ptr *terms;       /* list of terms with path */
    struct fpa_head *next;        /* next FPA list */
    int *path;
    };

struct fpa_tree {     /* for constructing FPA/path lookup tree */
    struct gen_ptr *terms;    /* for leaves only */
    struct fpa_tree *left;    /* for INTERSECT and UNION nodes */
    struct fpa_tree *right;   /* for INTERSECT and UNION nodes */
    struct term *left_term;   /* for UNION nodes only */
    struct term *right_term;  /* for UNION nodes only */
    int type;                 /* INTERSECT, UNION, LEAF */
    int *path;                /* for debugging only */
    };

struct fpa_pos {
    struct term *query_term;
    struct term *found_term;
    int type;
    struct fpa_tree *tree;         /* position in FPA query */
    struct context *subst_query;
    struct context *subst_found;
    struct trail *tr;              /* trail for unbinding variables */
    struct bt_node *bt_position;   /* for backtracking unify/match */
    struct fpa_pos *next;          /* for avail list only */
    };
