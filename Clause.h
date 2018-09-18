#define CLAUSE_TAB_SIZE 1000

#ifdef OWNER_OF_CLAUSE_GLOBALS
#define CLAUSE_GLOBAL         /* empty string if included by owner */
#else
#define CLAUSE_GLOBAL extern  /* extern if included by anything else */
#endif

CLAUSE_GLOBAL struct gen_ptr *Id_table[CLAUSE_TAB_SIZE];
CLAUSE_GLOBAL struct gen_ptr *Name_table[CLAUSE_TAB_SIZE];
CLAUSE_GLOBAL struct gen_ptr *Already_broadcast_table[CLAUSE_TAB_SIZE];
