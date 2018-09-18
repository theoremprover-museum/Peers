#include "Header.h"
#include "Unify.h"
#include "Io.h"
#include "Index.h"
#include "Paramod.h"

/* Size of chunk allocated by malloc */

#define TP_ALLOC_SIZE 300000
#define MALLOC_ARG_TYPE unsigned

static char *Alloc_block;    /* location returned by most recent malloc */
static char *Alloc_pos;      /* current position in block */

/*  a list of available nodes for each type of strucure */

static struct term *term_avail;
static struct rel *rel_avail;
static struct sym_ent *sym_ent_avail;
static struct gen_ptr *gen_ptr_avail;
static struct context *context_avail;
static struct trail *trail_avail;
static struct bt_node *bt_node_avail;
static struct ac_position *ac_position_avail;
static struct discrim *discrim_avail;
static struct flat *flat_avail;
static struct discrim_pos *discrim_pos_avail;
static struct fpa_head *fpa_head_avail;
static struct fpa_tree *fpa_tree_avail;
static struct fpa_pos *fpa_pos_avail;
static struct ac_match_pos *ac_match_pos_avail;
static struct ac_match_free_vars_pos *ac_match_free_vars_pos_avail;
static struct literal *literal_avail;
static struct list *list_avail;
static struct list_pos *list_pos_avail;
static struct para_pair *para_pair_avail;
/* Insert new avail here. */

static int Malloc_calls;  /* number of calls to malloc */

/* # of gets, frees, and size of avail list for each type of structure */

static long term_gets, term_frees, term_avails;
static long rel_gets, rel_frees, rel_avails;
static long sym_ent_gets, sym_ent_frees, sym_ent_avails;
static long gen_ptr_gets, gen_ptr_frees, gen_ptr_avails;
static long context_gets, context_frees, context_avails;
static long trail_gets, trail_frees, trail_avails;
static long bt_node_gets, bt_node_frees, bt_node_avails;
static long ac_position_gets, ac_position_frees, ac_position_avails;
static long discrim_gets, discrim_frees, discrim_avails;
static long flat_gets, flat_frees, flat_avails;
static long discrim_pos_gets, discrim_pos_frees, discrim_pos_avails;
static long fpa_head_gets, fpa_head_frees, fpa_head_avails;
static long fpa_tree_gets, fpa_tree_frees, fpa_tree_avails;
static long fpa_pos_gets, fpa_pos_frees, fpa_pos_avails;
static long ac_match_pos_gets, ac_match_pos_frees, ac_match_pos_avails;
static long ac_match_free_vars_pos_gets, ac_match_free_vars_pos_frees, ac_match_free_vars_pos_avails;
static long literal_gets, literal_frees, literal_avails;
static long list_gets, list_frees, list_avails;
static long list_pos_gets, list_pos_frees, list_pos_avails;
static long para_pair_gets, para_pair_frees, para_pair_avails;
/* Insert new counters here. */

/*************
 *
 *     init_avail()
 *
 *************/

void init_avail()
{
    Alloc_block = NULL;
    Alloc_pos = NULL;

    term_avail = NULL;
    rel_avail = NULL;
    sym_ent_avail = NULL;
    gen_ptr_avail = NULL;
    context_avail = NULL;
    trail_avail = NULL;
    bt_node_avail = NULL;
    ac_position_avail = NULL;
    discrim_avail = NULL;
    flat_avail = NULL;
    discrim_pos_avail = NULL;
    fpa_head_avail = NULL;
    fpa_tree_avail = NULL;
    fpa_pos_avail = NULL;
    ac_match_pos_avail = NULL;
    ac_match_free_vars_pos_avail = NULL;
    literal_avail = NULL;
    list_avail = NULL;
    list_pos_avail = NULL;
    para_pair_avail = NULL;
    
    Malloc_calls = 0;
    
    term_gets = term_frees = term_avails = 0;
    rel_gets = rel_frees = rel_avails = 0;
    sym_ent_gets = sym_ent_frees = sym_ent_avails = 0;
    gen_ptr_gets = gen_ptr_frees = gen_ptr_avails = 0;
    context_gets = context_frees = context_avails = 0;
    trail_gets = trail_frees = trail_avails = 0;
    bt_node_gets = bt_node_frees = bt_node_avails = 0;
    ac_position_gets = ac_position_frees = ac_position_avails = 0;
    discrim_gets = discrim_frees = discrim_avails = 0;
    flat_gets = flat_frees = flat_avails = 0;
    discrim_pos_gets = discrim_pos_frees = discrim_pos_avails = 0;
    fpa_head_gets = fpa_head_frees = fpa_head_avails = 0;
    fpa_tree_gets = fpa_tree_frees = fpa_tree_avails = 0;
    fpa_pos_gets = fpa_pos_frees = fpa_pos_avails = 0;
    ac_match_pos_gets = ac_match_pos_frees = ac_match_pos_avails = 0;
    ac_match_free_vars_pos_gets = ac_match_free_vars_pos_frees = ac_match_free_vars_pos_avails = 0;
    literal_gets = literal_frees = literal_avails = 0;
    list_gets = list_frees = list_avails = 0;
    list_pos_gets = list_pos_frees = list_pos_avails = 0;
    para_pair_gets = para_pair_frees = para_pair_avails = 0;

}  /* init_avail */

/*************
 *
 *    char *tp_alloc(n)
 *
 *    Allocate n contiguous bytes, aligned on pointer boundry.
 *
 *************/

char *tp_alloc(n)
int n;
{
    char *return_block;
    int scale;
    
    /* if n is not a multiple of sizeof(int *), then round up so that it is */
    
    scale = sizeof(int *);
    if (n % scale != 0)
	n = n + (scale - (n % scale));
    
    if (Alloc_block == NULL || Alloc_block + TP_ALLOC_SIZE - Alloc_pos < n) {
        /* try to malloc a new block */
	if (n > TP_ALLOC_SIZE)
	    abend("in tp_alloc, request too big.");
	else if (Parms[MAX_MEM].val != 0 && ((Malloc_calls+1)*TP_ALLOC_SIZE)/1024 > Parms[MAX_MEM].val)
	    abend("in tp_alloc, max_mem parameter exceeded.");
	else {

	    Alloc_pos = Alloc_block = (char *) malloc((MALLOC_ARG_TYPE) TP_ALLOC_SIZE);

	    Malloc_calls++;
            Stats[K_MALLOCED] = (Malloc_calls * (TP_ALLOC_SIZE / 1024.));
	    if (Alloc_pos == NULL)
		abend("in tp_alloc, operating system cannot supply any more memory.");
	    }
        }
    return_block = Alloc_pos;
    Alloc_pos += n;
    return(return_block);
}  /* tp_alloc */

/*************
 *
 *   struct term *get_term()
 *
 *************/

struct term *get_term()
{
    struct term *p;
    
    term_gets++;
    if (term_avail == NULL)
	p = (struct term *) tp_alloc(sizeof(struct term));
    else {
	term_avails--;
	p = term_avail;
	term_avail = (struct term *) term_avail->farg;
	}
    p->sym_num = 0;
    p->farg = NULL;
    p->varnum = 0;
    p->bits = 0;
    p->fpa_id = 0;
    return(p);
}  /* get_term */

/*************
 *
 *    free_term()
 *
 *************/

void free_term(p)
struct term *p;
{
    term_frees++;
    term_avails++;
    p->farg = (struct rel *) term_avail;
    term_avail = p;
}  /* free_term */

/*************
 *
 *    struct rel *get_rel()
 *
 *************/

struct rel *get_rel()
{
    struct rel *p;
    
    rel_gets++;
    if (rel_avail == NULL)
	p = (struct rel *) tp_alloc(sizeof(struct rel));
    else {
	rel_avails--;
	p = rel_avail;
	rel_avail = rel_avail->narg;
	}
    p->narg = NULL;
    p->argval = NULL;
    return(p);
}  /* get_rel */

/*************
 *
 *    free_rel()
 *
 *************/

void free_rel(p)
struct rel *p;
{
    rel_frees++;
    rel_avails++;
    p->narg = rel_avail;
    rel_avail = p;
}  /* free_rel */

/*************
 *
 *    struct sym_ent *get_sym_ent()
 *
 *************/

struct sym_ent *get_sym_ent()
{
    struct sym_ent *p;
    
    sym_ent_gets++;
    if (sym_ent_avail == NULL)
	p = (struct sym_ent *) tp_alloc(sizeof(struct sym_ent));
    else {
	sym_ent_avails--;
	p = sym_ent_avail;
	sym_ent_avail = sym_ent_avail->next;
	}
    p->name[0] = '\0';
    p->sym_num = 0;
    p->arity = -1;
    p->lex_val = MAX_INT;
    p->lrpo_status = LRPO_LR_STATUS;
    p->special_op = 0;
    p->op_type = 0;
    p->op_prec = 0;
    p->assoc_comm = 0;

    p->next = NULL;
    return(p);
}  /* get_sym_ent */

/*************
 *
 *    free_sym_ent()
 *
 *************/

void free_sym_ent(p)
struct sym_ent *p;
{
    sym_ent_frees++;
    sym_ent_avails++;
    p->next = sym_ent_avail;
    sym_ent_avail = p;
}  /* free_sym_ent */

/*************
 *
 *    struct gen_ptr *get_gen_ptr()
 *
 *************/

struct gen_ptr *get_gen_ptr()
{
    struct gen_ptr *p;
    
    gen_ptr_gets++;
    if (gen_ptr_avail == NULL)
	p = (struct gen_ptr *) tp_alloc(sizeof(struct gen_ptr));
    else {
	gen_ptr_avails--;
	p = gen_ptr_avail;
	gen_ptr_avail = gen_ptr_avail->next;
	}
    p->u.t = NULL;
    p->next = NULL;
    return(p);
}  /* get_gen_ptr */

/*************
 *
 *    free_gen_ptr()
 *
 *************/

void free_gen_ptr(p)
struct gen_ptr *p;
{
    gen_ptr_frees++;
    gen_ptr_avails++;
    p->next = gen_ptr_avail;
    gen_ptr_avail = p;
}  /* free_gen_ptr */

/*************
 *
 *    struct context *get_context()
 *
 *************/

struct context *get_context()
{
    struct context *p;
    int i;
    static int count=0;
    
    context_gets++;
#if 0
    if (1) {
#else
    if (context_avail == NULL) {
#endif
	p = (struct context *) tp_alloc(sizeof(struct context));
	for (i=0; i<MAX_VARS; i++)
	    p->terms[i] = NULL;
        p->built_in_multiplier = count++;  /* never change */
	}
    else {
	context_avails--;
	p = context_avail;
	context_avail = context_avail->contexts[0];
	}
    p->contexts[0] = NULL;
    p->multiplier = p->built_in_multiplier;
    p->partial_term = NULL;
    return(p);
}  /* get_context */

/*************
 *
 *    free_context()
 *
 *************/

void free_context(p)
struct context *p;
{
#if 0  /* for checking that contexts are really clear when freed */
    int i;
    for (i=0; i<MAX_VARS; i++) {
	if (p->terms[i]) {
	    printf("ERROR, context %x, term &d not null.\n",p->contexts[i], i);
	    print_term_nl(stdout, p->terms[i]);
	    p->terms[i] = NULL;
	    }
	if (p->contexts[i]) {
	    printf("ERROR, context %x, context &d not null.\n",p->contexts[i], i);
	    print_term_nl(stdout, p->terms[i]);
	    p->contexts[i] = NULL;
	    }
	}
#endif
    context_frees++;
    context_avails++;
    p->contexts[0] = context_avail;
    context_avail = p;
}  /* free_context */

/*************
 *
 *    struct trail *get_trail()
 *
 *************/

struct trail *get_trail()
{
    struct trail *p;
    
    trail_gets++;
    if (trail_avail == NULL)
	p = (struct trail *) tp_alloc(sizeof(struct trail));
    else {
	trail_avails--;
	p = trail_avail;
	trail_avail = trail_avail->next;
	}
    p->next = NULL;
    return(p);
}  /* get_trail */

/*************
 *
 *    free_trail()
 *
 *************/

void free_trail(p)
struct trail *p;
{
    trail_frees++;
    trail_avails++;
    p->next = trail_avail;
    trail_avail = p;
}  /* free_trail */

/*************
 *
 *    struct bt_node *get_bt_node()
 *
 *************/

struct bt_node *get_bt_node()
{
    struct bt_node *p;
    
    bt_node_gets++;
    if (bt_node_avail == NULL)
	p = (struct bt_node *) tp_alloc(sizeof(struct bt_node));
    else {
	bt_node_avails--;
	p = bt_node_avail;
	bt_node_avail = bt_node_avail->next;
	}

    p->parent = NULL;
    p->first_child = NULL;
    p->last_child = NULL;
    p->next = NULL;
    p->prev = NULL;

    p->t1 = NULL;
    p->t2 = NULL;
    p->c1 = NULL;
    p->c2 = NULL;

    p->varnum = -1;
    p->cb = NULL;
    p->alternative = 0;
    p->partial = 0;

    return(p);
}  /* get_bt_node */

/*************
 *
 *    free_bt_node()
 *
 *************/

void free_bt_node(p)
struct bt_node *p;
{
    bt_node_frees++;
    bt_node_avails++;
    p->next = bt_node_avail;
    bt_node_avail = p;
}  /* free_bt_node */

/*************
 *
 *    struct ac_position *get_ac_position()
 *
 *************/

struct ac_position *get_ac_position()
{
    struct ac_position *p;
    
    ac_position_gets++;
    if (ac_position_avail == NULL)
	p = (struct ac_position *) tp_alloc(sizeof(struct ac_position));
    else {
	ac_position_avails--;
	p = ac_position_avail;
	ac_position_avail = ac_position_avail->next;
	}

    /* note, no initialization */

    return(p);
}  /* get_ac_position */

/*************
 *
 *    free_ac_position()
 *
 *************/

void free_ac_position(p)
struct ac_position *p;
{
    ac_position_frees++;
    ac_position_avails++;
    p->next = ac_position_avail;
    ac_position_avail = p;
}  /* free_ac_position */

/*************
 *
 *    struct discrim *get_discrim()
 *
 *************/

struct discrim *get_discrim()
{
    struct discrim *p;

    discrim_gets++;
    if (discrim_avail == NULL)
        p = (struct discrim *) tp_alloc(sizeof(struct discrim));
    else {
        discrim_avails--;
        p = discrim_avail;
        discrim_avail = discrim_avail->next;
        }

    p->next = NULL;
    p->type = 0;
    p->lab = 0;
    p->u.kids = NULL;

    return(p);
}  /* get_discrim */

/*************
 *
 *    free_discrim()
 *
 *************/

void free_discrim(p)
struct discrim *p;
{
    discrim_frees++;
    discrim_avails++;
    p->next = discrim_avail;
    discrim_avail = p;
}  /* free_discrim */

/*************
 *
 *    struct flat *get_flat()
 *
 *************/

struct flat *get_flat()
{
    struct flat *p;

    flat_gets++;
    if (flat_avail == NULL)
        p = (struct flat *) tp_alloc(sizeof(struct flat));
    else {
        flat_avails--;
        p = flat_avail;
        flat_avail = flat_avail->next;
        }

#if 1
    p->t = NULL;
    p->prev = NULL;
    p->next = NULL;
    p->last = NULL;
#endif

    p->alternatives = NULL;
    p->bound = 0;
    p->place_holder = 0;

    return(p);
}  /* get_flat */

/*************
 *
 *    free_flat()
 *
 *************/

void free_flat(p)
struct flat *p;
{
    flat_frees++;
    flat_avails++;
    p->next = flat_avail;
    flat_avail = p;
}  /* free_flat */

/*************
 *
 *    struct discrim_pos *get_discrim_pos()
 *
 *************/

struct discrim_pos *get_discrim_pos()
{
    struct discrim_pos *p;

    discrim_pos_gets++;
    if (discrim_pos_avail == NULL)
        p = (struct discrim_pos *) tp_alloc(sizeof(struct discrim_pos));
    else {
        discrim_pos_avails--;
        p = discrim_pos_avail;
        discrim_pos_avail = (struct discrim_pos *) discrim_pos_avail->data;
        }

    p->data = NULL;
    p->f = NULL;

    return(p);
}  /* get_discrim_pos */

/*************
 *
 *    free_discrim_pos()
 *
 *************/

void free_discrim_pos(p)
struct discrim_pos *p;
{
    discrim_pos_frees++;
    discrim_pos_avails++;
    p->data = (struct gen_ptr *) discrim_pos_avail;
    discrim_pos_avail = p;
}  /* free_discrim_pos */

/*************
 *
 *    struct fpa_head *get_fpa_head()
 *
 *************/

struct fpa_head *get_fpa_head()
{
    struct fpa_head *p;

    fpa_head_gets++;
    if (fpa_head_avail == NULL)
        p = (struct fpa_head *) tp_alloc(sizeof(struct fpa_head));
    else {
        fpa_head_avails--;
        p = fpa_head_avail;
        fpa_head_avail = fpa_head_avail->next;
        }

    p->path = NULL;
    p->terms = NULL;
    p->next = NULL;

    return(p);
}  /* get_fpa_head */

/*************
 *
 *    free_fpa_head()
 *
 *************/

void free_fpa_head(p)
struct fpa_head *p;
{
    fpa_head_frees++;
    fpa_head_avails++;
    p->next = fpa_head_avail;
    fpa_head_avail = p;
}  /* free_fpa_head */

/*************
 *
 *    struct fpa_tree *get_fpa_tree()
 *
 *************/

struct fpa_tree *get_fpa_tree()
{
    struct fpa_tree *p;

    fpa_tree_gets++;
    if (fpa_tree_avail == NULL)
        p = (struct fpa_tree *) tp_alloc(sizeof(struct fpa_tree));
    else {
        fpa_tree_avails--;
        p = fpa_tree_avail;
        fpa_tree_avail = fpa_tree_avail->left;
        }

    p->terms = NULL;
    p->left = NULL;
    p->right = NULL;
    p->left_term = NULL;
    p->right_term = NULL;
    p->path = NULL;

    return(p);
}  /* get_fpa_tree */

/*************
 *
 *    free_fpa_tree()
 *
 *************/

void free_fpa_tree(p)
struct fpa_tree *p;
{
    fpa_tree_frees++;
    fpa_tree_avails++;
    p->left = fpa_tree_avail;
    fpa_tree_avail = p;
}  /* free_fpa_tree */

/*************
 *
 *    struct fpa_pos *get_fpa_pos()
 *
 *************/

struct fpa_pos *get_fpa_pos()
{
    struct fpa_pos *p;

    fpa_pos_gets++;
    if (fpa_pos_avail == NULL)
        p = (struct fpa_pos *) tp_alloc(sizeof(struct fpa_pos));
    else {
        fpa_pos_avails--;
        p = fpa_pos_avail;
        fpa_pos_avail = fpa_pos_avail->next;
        }

    p->next = NULL;

    return(p);
}  /* get_fpa_pos */

/*************
 *
 *    free_fpa_pos()
 *
 *************/

void free_fpa_pos(p)
struct fpa_pos *p;
{
    fpa_pos_frees++;
    fpa_pos_avails++;
    p->next = fpa_pos_avail;
    fpa_pos_avail = p;
}  /* free_fpa_pos */

/*************
 *
 *    struct ac_match_pos *get_ac_match_pos()
 *
 *************/

struct ac_match_pos *get_ac_match_pos()
{
    struct ac_match_pos *p;

    ac_match_pos_gets++;
    if (ac_match_pos_avail == NULL)
        p = (struct ac_match_pos *) tp_alloc(sizeof(struct ac_match_pos));
    else {
        ac_match_pos_avails--;
        p = ac_match_pos_avail;
        ac_match_pos_avail = ac_match_pos_avail->next;
        }

    /* note, no initialization */

    return(p);
}  /* get_ac_match_pos */

/*************
 *
 *    free_ac_match_pos()
 *
 *************/

void free_ac_match_pos(p)
struct ac_match_pos *p;
{
    ac_match_pos_frees++;
    ac_match_pos_avails++;
    p->next = ac_match_pos_avail;
    ac_match_pos_avail = p;
}  /* free_ac_match_pos */

/*************
 *
 *    struct ac_match_free_vars_pos *get_ac_match_free_vars_pos()
 *
 *************/

struct ac_match_free_vars_pos *get_ac_match_free_vars_pos()
{
    struct ac_match_free_vars_pos *p;

    ac_match_free_vars_pos_gets++;
    if (ac_match_free_vars_pos_avail == NULL)
        p = (struct ac_match_free_vars_pos *) tp_alloc(sizeof(struct ac_match_free_vars_pos));
    else {
        ac_match_free_vars_pos_avails--;
        p = ac_match_free_vars_pos_avail;
        ac_match_free_vars_pos_avail = ac_match_free_vars_pos_avail->next;
        }

    /* note, no initialization */

    return(p);
}  /* get_ac_match_free_vars_pos */

/*************
 *
 *    free_ac_match_free_vars_pos()
 *
 *************/

void free_ac_match_free_vars_pos(p)
struct ac_match_free_vars_pos *p;
{
    ac_match_free_vars_pos_frees++;
    ac_match_free_vars_pos_avails++;
    p->next = ac_match_free_vars_pos_avail;
    ac_match_free_vars_pos_avail = p;
}  /* free_ac_match_free_vars_pos */

/*************
 *
 *    struct literal *get_literal()
 *
 *************/

struct literal *get_literal()
{
    struct literal *p;

    literal_gets++;
    if (literal_avail == NULL)
        p = (struct literal *) tp_alloc(sizeof(struct literal));
    else {
        literal_avails--;
        p = literal_avail;
        literal_avail = literal_avail->next;
        }

    p->weight = 0;
    p->sign = -1;
    p->from_parent = 0;
    p->into_parent = 0;
    p->bd_parent = 0;
    p->demod_parents = NULL;
    p->containers = NULL;
    p->from_parent_extended = 0;
    p->into_parent_extended = 0;

    p->id = 0;
    p->name = 0;
    p->bd_count = 0;
    return(p);
}  /* get_literal */

/*************
 *
 *    free_literal()
 *
 *************/

void free_literal(p)
struct literal *p;
{
    literal_frees++;
    literal_avails++;
    p->next = literal_avail;
    literal_avail = p;
}  /* free_literal */

/*************
 *
 *    struct list *get_list()
 *
 *************/

struct list *get_list()
{
    struct list *p;

    list_gets++;
    if (list_avail == NULL)
        p = (struct list *) tp_alloc(sizeof(struct list));
    else {
        list_avails--;
        p = list_avail;
        list_avail = list_avail->next;
        }

    p->first = NULL;
    p->last = NULL;
    p->next = NULL;

    return(p);
}  /* get_list */

/*************
 *
 *    free_list()
 *
 *************/

void free_list(p)
struct list *p;
{
    list_frees++;
    list_avails++;
    p->next = list_avail;
    list_avail = p;
}  /* free_list */

/*************
 *
 *    struct list_pos *get_list_pos()
 *
 *************/

struct list_pos *get_list_pos()
{
    struct list_pos *p;

    list_pos_gets++;
    if (list_pos_avail == NULL)
        p = (struct list_pos *) tp_alloc(sizeof(struct list_pos));
    else {
        list_pos_avails--;
        p = list_pos_avail;
        list_pos_avail = list_pos_avail->next;
        }

    p->prev = NULL;
    p->next = NULL;
    p->nocc = NULL;
    p->lit = NULL;
    p->container = NULL;

    return(p);
}  /* get_list_pos */

/*************
 *
 *    free_list_pos()
 *
 *************/

void free_list_pos(p)
struct list_pos *p;
{
    list_pos_frees++;
    list_pos_avails++;
    p->next = list_pos_avail;
    list_pos_avail = p;
}  /* free_list_pos */

/*************
 *
 *    struct para_pair *get_para_pair()
 *
 *************/

struct para_pair *get_para_pair()
{
    struct para_pair *p;

    para_pair_gets++;
    if (para_pair_avail == NULL)
        p = (struct para_pair *) tp_alloc(sizeof(struct para_pair));
    else {
        para_pair_avails--;
        p = para_pair_avail;
        para_pair_avail = para_pair_avail->next;
        }

    p->weight = 0;
    p->c1 = NULL;
    p->c2 = NULL;
    p->prev = NULL;
    p->next = NULL;

    return(p);
}  /* get_para_pair */

/*************
 *
 *    free_para_pair()
 *
 *************/

void free_para_pair(p)
struct para_pair *p;
{
    para_pair_frees++;
    para_pair_avails++;
    p->next = para_pair_avail;
    para_pair_avail = p;
}  /* free_para_pair */
/* Insert new routines here. */

/*************
 *
 *    print_mem()
 *
 *************/

void print_mem(fp)
FILE *fp;
{
    fprintf(fp, "\n------------- memory usage ------------\n");

    fprintf(fp, "%d mallocs of %d bytes each, %.1f K.\n",
	  Malloc_calls, TP_ALLOC_SIZE, (Malloc_calls * (TP_ALLOC_SIZE / 1024.)));

    fprintf(fp, "  type (bytes each)        gets      frees     in use      avail      bytes\n");
    fprintf(fp, "sym_ent (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct sym_ent), sym_ent_gets, sym_ent_frees, sym_ent_gets - sym_ent_frees, sym_ent_avails, (((sym_ent_gets - sym_ent_frees) + sym_ent_avails) * sizeof(struct sym_ent)) / 1024.);
    fprintf(fp, "term (%4d)         %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct term), term_gets, term_frees, term_gets - term_frees, term_avails, (((term_gets - term_frees) + term_avails) * sizeof(struct term)) / 1024.);
    fprintf(fp, "rel (%4d)          %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct rel), rel_gets, rel_frees, rel_gets - rel_frees, rel_avails, (((rel_gets - rel_frees) + rel_avails) * sizeof(struct rel)) / 1024.);
    fprintf(fp, "gen_ptr (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct gen_ptr), gen_ptr_gets, gen_ptr_frees, gen_ptr_gets - gen_ptr_frees, gen_ptr_avails, (((gen_ptr_gets - gen_ptr_frees) + gen_ptr_avails) * sizeof(struct gen_ptr)) / 1024.);
    fprintf(fp, "context (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct context), context_gets, context_frees, context_gets - context_frees, context_avails, (((context_gets - context_frees) + context_avails) * sizeof(struct context)) / 1024.);
    fprintf(fp, "trail (%4d)        %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct trail), trail_gets, trail_frees, trail_gets - trail_frees, trail_avails, (((trail_gets - trail_frees) + trail_avails) * sizeof(struct trail)) / 1024.);
    fprintf(fp, "bt_node (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct bt_node), bt_node_gets, bt_node_frees, bt_node_gets - bt_node_frees, bt_node_avails, (((bt_node_gets - bt_node_frees) + bt_node_avails) * sizeof(struct bt_node)) / 1024.);
    fprintf(fp, "ac_position (%4d)%11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct ac_position), ac_position_gets, ac_position_frees, ac_position_gets - ac_position_frees, ac_position_avails, (((ac_position_gets - ac_position_frees) + ac_position_avails) * sizeof(struct ac_position)) / 1024.);
    fprintf(fp, "discrim (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct discrim), discrim_gets, discrim_frees, discrim_gets - discrim_frees, discrim_avails, (((discrim_gets - discrim_frees) + discrim_avails) * sizeof(struct discrim)) / 1024.);
    fprintf(fp, "flat (%4d)         %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct flat), flat_gets, flat_frees, flat_gets - flat_frees, flat_avails, (((flat_gets - flat_frees) + flat_avails) * sizeof(struct flat)) / 1024.);
    fprintf(fp, "discrim_pos (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct discrim_pos), discrim_pos_gets, discrim_pos_frees, discrim_pos_gets - discrim_pos_frees, discrim_pos_avails, (((discrim_pos_gets - discrim_pos_frees) + discrim_pos_avails) * sizeof(struct discrim_pos)) / 1024.);
    fprintf(fp, "fpa_head (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fpa_head), fpa_head_gets, fpa_head_frees, fpa_head_gets - fpa_head_frees, fpa_head_avails, (((fpa_head_gets - fpa_head_frees) + fpa_head_avails) * sizeof(struct fpa_head)) / 1024.);
    fprintf(fp, "fpa_tree (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fpa_tree), fpa_tree_gets, fpa_tree_frees, fpa_tree_gets - fpa_tree_frees, fpa_tree_avails, (((fpa_tree_gets - fpa_tree_frees) + fpa_tree_avails) * sizeof(struct fpa_tree)) / 1024.);
    fprintf(fp, "fpa_pos (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fpa_pos), fpa_pos_gets, fpa_pos_frees, fpa_pos_gets - fpa_pos_frees, fpa_pos_avails, (((fpa_pos_gets - fpa_pos_frees) + fpa_pos_avails) * sizeof(struct fpa_pos)) / 1024.);
    fprintf(fp, "ac_match_pos (%4d)%11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct ac_match_pos), ac_match_pos_gets, ac_match_pos_frees, ac_match_pos_gets - ac_match_pos_frees, ac_match_pos_avails, (((ac_match_pos_gets - ac_match_pos_frees) + ac_match_pos_avails) * sizeof(struct ac_match_pos)) / 1024.);
    fprintf(fp, "ac_match_free_vars_pos (%4d)\n                    %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct ac_match_free_vars_pos), ac_match_free_vars_pos_gets, ac_match_free_vars_pos_frees, ac_match_free_vars_pos_gets - ac_match_free_vars_pos_frees, ac_match_free_vars_pos_avails, (((ac_match_free_vars_pos_gets - ac_match_free_vars_pos_frees) + ac_match_free_vars_pos_avails) * sizeof(struct ac_match_free_vars_pos)) / 1024.);
    fprintf(fp, "literal (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct literal), literal_gets, literal_frees, literal_gets - literal_frees, literal_avails, (((literal_gets - literal_frees) + literal_avails) * sizeof(struct literal)) / 1024.);
    fprintf(fp, "list (%4d)         %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct list), list_gets, list_frees, list_gets - list_frees, list_avails, (((list_gets - list_frees) + list_avails) * sizeof(struct list)) / 1024.);
    fprintf(fp, "list_pos (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct list_pos), list_pos_gets, list_pos_frees, list_pos_gets - list_pos_frees, list_pos_avails, (((list_pos_gets - list_pos_frees) + list_pos_avails) * sizeof(struct list_pos)) / 1024.);
    fprintf(fp, "para_pair (%4d)    %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct para_pair), para_pair_gets, para_pair_frees, para_pair_gets - para_pair_frees, para_pair_avails, (((para_pair_gets - para_pair_frees) + para_pair_avails) * sizeof(struct para_pair)) / 1024.);
/* Insert new print here. */
}  /* print_mem */

/*************
 *
 *   p_mem
 *
 *************/

void p_mem()
{
    print_mem(stdout);
}  /* p_mem */

/*************
 *
 *    int total_mem() -- How many K have been dynamically allocated?
 *
 *************/

int total_mem()
{
    return( (int) (Malloc_calls * (TP_ALLOC_SIZE / 1024.)));
}  /* total_mem */

