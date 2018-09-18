#define SYM_TAB_SIZE   50
#define MAX_COMPLEX  1000  /* number of operators/terms in a sequence */

#define MAX_NAME 51   /* maximum # of chars in any symbol (including '\0') */
#define MAX_BUF 5000  /* maximum # of chars in input string (including '\0') */

#define XFX 1  /* special operator types */
#define XFY 2
#define YFX 3
#define FX  4
#define FY  5
#define XF  6
#define YF  7

struct sym_ent {  /* symbol table entry */
    struct sym_ent *next;
    int sym_num;           /* unique identifier */
    int arity;             /* 0 for constants and variables */
    char name[MAX_NAME];   /* the print symbol */

    int lex_val;           /* can be used to assign a lexical value */
    int lrpo_status;       /* status for lexicographic RPO */

    int special_op;  /* special operator */
    int op_type;     /* operator type */
    int op_prec;     /* operator precedence */

    int assoc_comm;  /* associative-commutative */
    int commutative; /* commutative or symmetric */
    };

struct symbol_table {
    struct sym_ent *table[SYM_TAB_SIZE];
    };

/* Following structure is to store data on symbol that might be special op. */
struct sequence_member {
    struct term *t;
    short  binary_type;
    short  binary_prec;
    short  unary_type;
    short  unary_prec;
    };

