
struct term {
    struct rel *farg;         /* subterm list; used for complex only */
    int fpa_id;               /* for use by FPA indexing only */
    short sym_num;            /* used for names, complex, and sometimes vars */
    short varnum;             /* used for variables */
    unsigned char type;       /* NAME, VARIABLE, or COMPLEX */
    unsigned char bits;       /* bit flags (see macros.h) */
    };

struct rel {  /* list of subterms */
    struct term *argval;     /* subterm */
    struct rel *narg;        /* rest of subterm list */
    };

struct literal {
    int sign;
    struct term *atom;
    int weight;
    int from_parent;
    int into_parent;
    int bd_parent;
    struct gen_ptr *demod_parents;
    struct literal *next;
    struct list_pos *containers;
    char from_parent_extended;
    char into_parent_extended;

    int name;  /* <pid, lid> */
    int id;    /* <pid, birth_time> */
    int bd_count;  /* inference message back demodulated by strategy 4. */
    };

struct gen_ptr {     /* for constructing a list of pointers */
    struct gen_ptr *next;
    union {
	int i;
	struct term *t;
	struct rel *r;
	struct literal *l;
	void *v;
	} u;
    };

struct list {
    struct list_pos *first, *last;
    struct list *next;  /* for avail list */
    };

struct list_pos {
    struct list_pos *prev, *next, *nocc;
    struct list *container;
    struct literal *lit;
    };

