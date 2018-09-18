#include "Header.h"
#define OWNER_OF_CLAUSE_GLOBALS
#include "Clause.h"
#include "Unify.h"

/*************
 *
 *     init_clause_table_id(tab)
 *
 *************/

void init_clause_table_id(tab)
struct gen_ptr *tab[];
{
    int i;
    for (i=0; i<CLAUSE_TAB_SIZE; i++)
	tab[i] = NULL;
}  /* init_clause_table_id */

/*************
 *
 *     store_clause_by_id(c, tab)
 *
 *************/

void store_clause_by_id(c, tab)
struct literal *c;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p1, *p2, *p3;

    i = c->id % CLAUSE_TAB_SIZE;
    for (p2 = NULL, p1 = tab[i]; p1 && p1->u.l->id < c->id; p2 = p1, p1 = p1->next);
    if (p1 && p1->u.l->id == c->id)
	{
	fprintf(Peer_fp, "Store_clause_by_id: clause already there.\n");
    	print_clause(Peer_fp, c);
	print_clause_table_id(Peer_fp, Id_table);
	abend("Store_clause_by_id: clause already there.");
	}
    p3 = get_gen_ptr();
    p3->u.l = c;
    p3->next = p1;
    if (p2)
	p2->next = p3;
    else
	tab[i] = p3;
}  /* store_clause_by_id */

/*************
 *
 *     delete_clause_by_id(c, tab)
 *
 *************/

void delete_clause_by_id(c, tab)
struct literal *c;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p1, *p2, *p3;

    i = c->id % CLAUSE_TAB_SIZE;
    for (p2 = NULL, p1 = tab[i]; p1 && p1->u.l->id < c->id; p2 = p1, p1 = p1->next);
    if (!p1 || p1->u.l->id != c->id)
	abend("delete_clause_by_id: cannot find clause.");
    if (p2)
	p2->next = p1->next;
    else
	tab[i] = p1->next;
}  /* delete_clause_by_id */

/*************
 *
 *     find_clause_by_id(id, tab)
 *
 *************/

struct literal *find_clause_by_id(id, tab)
int id;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p1;

    i = id % CLAUSE_TAB_SIZE;
    for (p1 = tab[i]; p1 && p1->u.l->id < id; p1 = p1->next);
    if (p1 && p1->u.l->id == id)
	return(p1->u.l);
    else {
#if 0
          if(p1)
    	     fprintf(Peer_fp,"Clause with id=%d not found before clause with id=%d.\n", id, p1->u.l->id);
#endif
          return((struct literal *) NULL);
          }
}  /* find_clause_by_id */

/*************
 *
 *     print_clause_table_id(fp, tab)
 *
 *************/

void print_clause_table_id(fp, tab)
FILE *fp;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p;

    fprintf(fp, "\nID clause table:\n");
    for (i=0; i<CLAUSE_TAB_SIZE; i++)
	for (p = tab[i]; p; p = p->next)
	    print_clause(fp, p->u.l);
	    
}  /* print_clause_table_id */

/*************
 *
 *     p_clause_table_id(tab)
 *
 *************/

void p_clause_table_id(tab)
struct gen_ptr *tab[];
{
    print_clause_table_id(stdout, tab);
}  /* p_clause_table_id */

/*************
 *
 *     init_clause_table_name(tab)
 *
 *************/

void init_clause_table_name(tab)
struct gen_ptr *tab[];
{
    int i;
    for (i=0; i<CLAUSE_TAB_SIZE; i++)
	tab[i] = NULL;
}  /* init_clause_table_name */

/*************
 *
 *     store_clause_by_name(c, tab)
 *
 *************/

void store_clause_by_name(c, tab)
struct literal *c;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p1, *p2, *p3;

    i = c->name % CLAUSE_TAB_SIZE;
    for (p2 = NULL, p1 = tab[i]; p1 && p1->u.l->name < c->name; p2 = p1, p1 = p1->next);
    if (p1 && p1->u.l->name == c->name)
    	{
    	fprintf(Peer_fp, "Store_clause_by_name: clause already there.\n");
    	print_clause(Peer_fp, c);
	print_clause_table_name(Peer_fp, Name_table);
	abend("Store_clause_by_name: clause already there.");
	}
    p3 = get_gen_ptr();
    p3->u.l = c;
    p3->next = p1;
    if (p2)
	p2->next = p3;
    else
	tab[i] = p3;
}  /* store_clause_by_name */

/*************
 *
 *     delete_clause_by_name(c, tab)
 *
 *************/

void delete_clause_by_name(c, tab)
struct literal *c;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p1, *p2, *p3;

    i = c->name % CLAUSE_TAB_SIZE;
    for (p2 = NULL, p1 = tab[i]; p1 && p1->u.l->name < c->name; p2 = p1, p1 = p1->next);
    if (!p1 || p1->u.l->name != c->name)
        	{
    	fprintf(Peer_fp, "Delete_clause_by_name: cannot find clause.\n");
    	print_clause(Peer_fp, c);
/*	print_clause_table_name(Peer_fp, Name_table); */
	abend("delete_clause_by_name: cannot find clause.");
		}
    if (p2)
	p2->next = p1->next;
    else
	tab[i] = p1->next;
}  /* delete_clause_by_name */

/*************
 *
 *     find_clause_by_name(name, tab)
 *
 *************/

struct literal *find_clause_by_name(name, tab)
int name;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p1;

    i = name % CLAUSE_TAB_SIZE;
    for (p1 = tab[i]; p1 && p1->u.l->name < name; p1 = p1->next);
    if (p1 && p1->u.l->name == name)
	return(p1->u.l);
    else
	return((struct literal *) NULL);
}  /* find_clause_by_name */

/*************
 *
 *     print_clause_table_name(fp, tab)
 *
 *************/

void print_clause_table_name(fp, tab)
FILE *fp;
struct gen_ptr *tab[];
{
    int i;
    struct gen_ptr *p;

    fprintf(fp, "\nName clause table:\n");
    for (i=0; i<CLAUSE_TAB_SIZE; i++)
	for (p = tab[i]; p; p = p->next)
	    print_clause(fp, p->u.l);
	    
}  /* print_clause_table_name */

/*************
 *
 *     p_clause_table_name(tab)
 *
 *************/

void p_clause_table_name(tab)
struct gen_ptr *tab[];
{
    print_clause_table_name(stdout, tab);
}  /* p_clause_table_name */

/*************
 *
 *    struct literal *copy_literal(lit) -- Return a copy of the literal.
 *
 *************/

struct literal *copy_literal(lit)
struct literal *lit;
{
struct literal *cp_lit;

	cp_lit = get_literal();
	cp_lit->sign = lit->sign;
	cp_lit->atom = copy_term(lit->atom);
	cp_lit->weight = lit->weight;
	cp_lit->from_parent = lit->from_parent;
	cp_lit->into_parent = lit->into_parent;
	cp_lit->bd_parent = lit->bd_parent;
	cp_lit->demod_parents = copy_gen_ptr_list(lit->demod_parents);
	cp_lit->from_parent_extended = lit->from_parent_extended;
	cp_lit->into_parent_extended = lit->into_parent_extended;
	cp_lit->id = lit->id;
	cp_lit->name = lit->name;
	cp_lit->bd_count = lit->bd_count;

return(cp_lit);

}  /* copy_literal */


/*************
 *
 *    zap_lit(lit)
 *
 *************/

void zap_lit(lit)
struct literal *lit;
{
    struct gen_ptr *p1, *p2;

    p1 = lit->demod_parents;
    while (p1) {
	p2 = p1;
	p1 = p1->next;
	free_gen_ptr(p2);
	}
    zap_term(lit->atom);
    free_literal(lit);
}  /* zap_lit */

/*************
 *
 *     insert_literal_by_weight(lit, l)
 *
 *************/

void insert_literal_by_weight(lit, l)
struct literal *lit;
struct list *l;
{
    struct list_pos *p;

    for (p = l->first; p && lit->weight >= p->lit->weight; p = p->next);
    if (p)
	list_insert_before(lit, p);
    else
	list_append(lit, l);

}  /* insert_literal_by_weight */

/*************
 *
 *    get_ancestors
 *
 *************/

void get_ancestors(lit, pp)
struct literal *lit;
struct gen_ptr **pp;
{
    struct gen_ptr *p1, *p2, *p3;
    struct literal *parent_lit;
    
    if (!lit) {
	p2 = get_gen_ptr();
	p2->u.l = NULL;
	p2->next = *pp;
	*pp = p2;
	}
    else {
	p1 = *pp;
	p3 = NULL;
	while (p1 && (!p1->u.l || p1->u.l->id < lit->id)) {
	    p3 = p1;
	    p1 = p1->next;
	    }
	if (!p1 || p1->u.l->id > lit->id) {
	    p2 = get_gen_ptr();
	    p2->u.l = lit;
	    if (!p3) {
		p2->next = *pp;
		*pp = p2;
		}
	    else {
		p2->next = p3->next;
		p3->next = p2;
		}
	    
	    if (lit->from_parent) {
	        parent_lit = find_clause_by_id(lit->from_parent, Id_table);
	        if (!parent_lit && Stats[PROOFS] > 0) {
                    fprintf(Peer_fp, "Clause with id=%d, from_parent of clause:\n", lit->from_parent);
                    print_clause(Peer_fp, lit);
                    fprintf(Peer_fp, "was not found.\n");
                    }
		get_ancestors(parent_lit, pp);
		}
	    if (lit->into_parent) {
	        parent_lit = find_clause_by_id(lit->into_parent, Id_table);
	        if (!parent_lit && Stats[PROOFS] > 0) {
                    fprintf(Peer_fp, "Clause with id=%d, into_parent of clause:\n", lit->into_parent);
                    print_clause(Peer_fp, lit);
                    fprintf(Peer_fp, "was not found.\n");
                    }
		get_ancestors(parent_lit, pp);
		}
	    if (lit->bd_parent) {
	        parent_lit = find_clause_by_id(lit->bd_parent, Id_table);
	        if (!parent_lit && Stats[PROOFS] > 0) {
                    fprintf(Peer_fp, "Clause with id=%d, bd_parent of clause:\n ", lit->bd_parent);
                    print_clause(Peer_fp, lit);
                    fprintf(Peer_fp, "was not found.\n");
                    }
		get_ancestors(parent_lit, pp);
		}
	    
	    for (p1 = lit->demod_parents; p1; p1 = p1->next) {
	        parent_lit = find_clause_by_id(p1->u.i, Id_table);
	        if (!parent_lit && Stats[PROOFS] > 0) {
                    fprintf(Peer_fp, "Clause with id=%d, demod_parent of clause:\n", p1->u.i);
                    print_clause(Peer_fp, lit);
                    fprintf(Peer_fp, "was not found.\n");
                    }
		get_ancestors(parent_lit, pp);
		}
	    }
	}
}  /* get_ancestors */

/*************
 *
 *    print_proof(fp, c1, c2)
 *
 *************/

void print_proof(fp, c1, c2)
FILE *fp;
struct literal *c1;
struct literal *c2;
{
    struct gen_ptr *p1, *p2;
    struct literal *c;

    fprintf(fp, "\n---------------- PROOF ----------------\n\n");
    p1 = NULL;
    get_ancestors(c1, &p1);
    if (c2) /* c2 is null if the empty clause was generated with x=x */
	get_ancestors(c2, &p1);
    while (p1) {
	c = p1->u.l;
	if (c)
	    print_clause(fp, c);
/* If !c, i.e. a clause is missing, an appropriate message has been already */
/* printed by get_ancestor, which has more information on who's missing than*/
/* print_proof.								    */
	p2 = p1;
	p1 = p1->next;
	free_gen_ptr(p2);
        }
    fprintf(fp, "\n------------ end of proof -------------\n\n");
}  /* print_proof */

/*************
 *
 *    print_clause(fp, lit)
 *
 *************/

void print_clause(fp,lit)
FILE *fp;
struct literal *lit;
{
    if (Flags[NAME_STRATEGIES].val && id1(lit->name) != id1(lit->id))
        fprintf(fp,"<??,%d,%d> (wt=%d) [", lid(lit),
	    birth_time(lit), lit->weight);

    else fprintf(fp,"<%d,%d,%d> (wt=%d) [", owner(lit), lid(lit),
	    birth_time(lit), lit->weight);

/* owner() and print_clause() are mutually recursive: owner() calls */
/* print_clause if the clause has bad Pid, id1(lit->name) != id1(lit->id) */
/* and if so print_clause() should not call owner again!		*/

    if (lit->from_parent && lit->into_parent)
	fprintf(fp,"para,%d:%d%s,%d:%d%s",
		id1(lit->from_parent), id2(lit->from_parent),
		lit->from_parent_extended ? "'" : "",
		id1(lit->into_parent), id2(lit->into_parent),
		lit->into_parent_extended ? "'" : "");

    else if (lit->bd_parent)
	fprintf(fp,"back_demod,%d:%d", id1(lit->bd_parent), id2(lit->bd_parent));
    if (lit->demod_parents) {
	struct gen_ptr *p;
	fprintf(fp,",demod");
	for (p = lit->demod_parents; p; p = p->next)
	    fprintf(fp, ",%d:%d", id1(p->u.i), id2(p->u.i));
	}
    fprintf(fp,"] ");
    if (lit->bd_count > 0)
	fprintf(fp, "(bd %d times) ", lit->bd_count);
    if (!lit->sign)
	fprintf(fp,"-(");
    print_term(fp, lit->atom);
    if (!lit->sign)
	fprintf(fp,")");
    fprintf(fp,".\n");
}  /* print_clause */

/*************
 *
 *    p_clause(lit)
 *
 *************/

void p_clause(lit)
struct literal *lit;     
{
    print_clause(stdout,lit);
}  /* p_clause */

/*************
 *
 *     alpha_is_ac(lit)
 *
 *************/

int alpha_is_ac(lit)
struct literal *lit;
{
    return(is_symbol(lit->atom, "=", 2) && is_assoc_comm(lit->atom->farg->argval->sym_num));
}  /* alpha_is_ac */

/*************
 *
 *     pos_eq(lit)
 *
 *************/

int pos_eq(lit)
struct literal *lit;
{
    return(lit->sign && is_symbol(lit->atom, "=", 2));
}  /* pos_eq */

/*************
 *
 *    renum_vars_recurse(term, varnums) -- called from renumber_vars.
 *
 *************/

void renum_vars_recurse(t, varnums)
struct term *t;
int varnums[];
{
    struct rel *r;
    int i;

    if (t->type == COMPLEX) {
	for (r = t->farg; r; r = r->narg)
            renum_vars_recurse(r->argval, varnums);
        }
    else if (t->type == VARIABLE) {
        i = 0;
        while (i < MAX_VARS && varnums[i] != -1 && varnums[i] != t->varnum)
	    i++;
        if (i == MAX_VARS)
            abend("in renum_vars_recurse, too many variables.");
        else {
            if (varnums[i] == -1)
                varnums[i] = t->varnum;
	    t->varnum = i;
            }
        }
}  /* renum_vars_recurse */

/*************
 *
 *    void renumber_vars(lit)
 *
 *    Renumber the variables of a literal, starting with 0.
 *    Abend if more than MAXVARS distinct variables.
 *
 *************/

void renumber_vars(lit)
struct literal *lit;
{
    int varnums[MAX_VARS];
    int i;

    for (i = 0; i < MAX_VARS; i++)
        varnums[i] = -1;
    renum_vars_recurse(lit->atom, varnums);
}  /* renumber_vars */

/*************
 *
 *    subsumes(lit1, lit2)
 *
 *************/

int subsumes(lit1, lit2)
struct literal *lit1, *lit2;
{
    struct context *c;
    struct bt_node *bt_position;
    int subsumed;

    c = get_context();
    if (lit1->sign != lit2->sign)
	subsumed = 0;
    else {
	bt_position = match_bt_first(lit1->atom, c, lit2->atom, 0);
	if (bt_position) {
	    subsumed = 1;
	    match_bt_cancel(bt_position);
	    }
	else
	    subsumed = 0;
	}
    free_context(c);
    return(subsumed);
}  /* subsume_list */

/*************
 *
 *    subsume_list(lit, l)
 *
 *************/

struct literal *subsume_list(lit, l)
struct literal *lit;
struct list *l;
{
    struct list_pos *p;
    struct literal *subsumer, *l2;
    struct context *c;
    struct bt_node *bt_position;

    c = get_context();
    for (p = l->first, subsumer = NULL; p && !subsumer; p = p->next) {
	l2 = p->lit;
	if (l2->sign == lit->sign) {
	    bt_position = match_bt_first(l2->atom, c, lit->atom, 0);
	    if (bt_position) {
		subsumer = l2;
		match_bt_cancel(bt_position);
		}
	    }
	}
    free_context(c);
    return(subsumer);
}  /* subsume_list */

/*************
 *
 *    conflict_list(lit, l)
 *
 *************/

struct literal *conflict_list(lit, l)
struct literal *lit;
struct list *l;
{
    struct list_pos *p;
    struct literal *l2;
    struct context *c1, *c2;
    struct bt_node *bt_position;
    int proof;

    c1 = get_context();
    c2 = get_context();
    for (p = l->first, proof = 0; p && !proof; p = p->next) {
	l2 = p->lit;
	if (l2->sign != lit->sign) {
	    bt_position = unify_bt_first(lit->atom, c1, l2->atom, c2);
	    if (bt_position) {
		proof = 1;
		unify_bt_cancel(bt_position);
		}
	    }
	}
    free_context(c1);
    free_context(c2);
    return(proof ? l2 : (struct literal *) NULL);
}  /* conflict_list */

/*************
 *
 *     term_weight(t)
 *
 *************/

int term_weight(t)
struct term *t;
{
    switch (Parms[WEIGHT_FUNCTION].val) {
      case 0: return(symbol_count(t));
      case 1: return(poly1(t));
      default: abend("in term_weight, weight_function out of range.");
	}
   
}  /* term_weight */

/*************
 *
 *     add_vars(t, v)
 *
 *************/

void add_vars(t, v)
struct term *t;
int v[];
{
    if (t->type == VARIABLE)
	v[t->varnum]++;
    else if (t->type == COMPLEX) {
	struct rel *r;
	for (r = t->farg; r; r = r->narg)
	    add_vars(r->argval, v);
	}
}  /* add_vars */

/*************
 *
 *     multi_superset(t1, t2n)
 *
 *************/

multi_superset(t1,t2)
struct term *t1, *t2;
{
    int i, ok, v1[MAX_VARS], v2[MAX_VARS];

    for (i=0; i<MAX_VARS; i++)
	v1[i] = v2[i] = 0;

    add_vars(t1, v1);
    add_vars(t2, v2);

    for (i=0, ok=1; i<MAX_VARS && ok; i++) {
	if (v2[i] > v1[i])
	    ok = 0;
	}
    return(ok);
    
}  /* multi_superset */

/*************
 *
 *    orient_equality(lit)
 *
 *************/

int orient_equality(lit)
struct literal *lit;
{
    struct term *t1, *t2;
    int w1, w2;

    if (Flags[LRPO].val)
	return(orient_literal_lrpo(lit));
    else {
	if (is_symbol(lit->atom, "=", 2)) {
	    t1 = lit->atom->farg->argval;
	    t2 = lit->atom->farg->narg->argval;
	    w1 = term_weight(t1);
	    w2 = term_weight(t2);
	    if (w2 > w1) {
		lit->atom->farg->argval = t2;
		lit->atom->farg->narg->argval = t1;
		}
	    return(w1 != w2 && lit->sign);
	    }
	else
	    return(0);
	}
}  /* orient_equality */

/*************
 *
 *    has_an_instance(t, alpha)
 *
 *************/

int has_an_instance(t, alpha)
struct term *t;
struct term *alpha;
{
    struct context *c;
    struct bt_node *bt_position;
    struct rel *r;
    int ok;

    c = get_context();
    if (bt_position = match_bt_first(alpha, c, t, 1)) {
	match_bt_cancel(bt_position);
	ok = 1;
	}
    else {
	for (r = t->farg, ok = 0; r && !ok; r = r->narg)
	    ok = has_an_instance(r->argval, alpha);
	}
    free_context(c);
    return(ok);
	
}  /* has_an_instance */

/*************
 *
 *     poly1(t) -- Stickel's function for x^3=x ring problem (CADE '84)
 *
 *************/

int poly1(t)
struct term *t;
{
    if (t->type == VARIABLE)
	return(2);
    else if (t->type == NAME)
	return(2);
    else {
	struct rel *r;
	int i;
        if (is_symbol(t, "=", 2))
	    return(poly1(t->farg->argval) + poly1(t->farg->narg->argval) + 1);
	else if (is_symbol(t, "+", 2))
	    return(poly1(t->farg->argval) + poly1(t->farg->narg->argval) + 1);
	else if (is_symbol(t, "*", 2))
	    return(poly1(t->farg->argval) * poly1(t->farg->narg->argval));
	else if (is_symbol(t, "m", 1))
	    return(poly1(t->farg->argval) * 7 + 1);
	else
	    return(-MAX_INT);
	}
}  /* poly1 */

/*************
 *
 *     extend_atom(lit)
 *
 *************/

struct term *extend_atom(lit)
struct literal *lit;
{
    struct term *t, *a, *b, *ax, *bx;
    struct rel *r1, *r2;

    t = get_term(); t->type = COMPLEX; t->sym_num = lit->atom->sym_num;
    a = get_term(); a->type = COMPLEX; a->sym_num = lit->atom->farg->argval->sym_num;
    b = get_term(); b->type = COMPLEX; b->sym_num = lit->atom->farg->argval->sym_num;
    ax = get_term(); ax->type = VARIABLE; ax->varnum = MAX_VARS-1;
    bx = get_term(); bx->type = VARIABLE; bx->varnum = MAX_VARS-1;

    r1 = get_rel(); r2 = get_rel(); r1->narg = r2;
    r1->argval = lit->atom->farg->narg->argval;
    r2->argval = bx;
    b->farg = r1;

    r1 = get_rel(); r2 = get_rel(); r1->narg = r2;
    r1->argval = lit->atom->farg->argval;
    r2->argval = ax;
    a->farg = r1;

    r1 = get_rel(); r2 = get_rel(); r1->narg = r2;
    r1->argval = a;
    r2->argval = b;
    t->farg = r1;
    
    return(t);
}  /* extend_atom */

/*************
 *
 *     zap_extended_atom(atom)
 *
 *************/

zap_extended_atom(atom)
struct term *atom;
{
    free_term(atom->farg->narg->argval->farg->narg->argval);
    free_rel(atom->farg->narg->argval->farg->narg);
    free_rel(atom->farg->narg->argval->farg);
    free_term(atom->farg->narg->argval);
    free_rel(atom->farg->narg);

    free_term(atom->farg->argval->farg->narg->argval);
    free_rel(atom->farg->argval->farg->narg);
    free_rel(atom->farg->argval->farg);
    free_term(atom->farg->argval);
    free_rel(atom->farg);

    free_term(atom);
}  /* zap_extended_atom */

/*************
 *
 *    apply_substitute(beta, c_from, t, into_term, c_into)
 *
 *    This routine is similar to apply, except that when it reaches the into
 *    term, the appropriate instance of beta is returned.
 *
 *************/

struct term *apply_substitute(beta, c_from, t, into_term, c_into)
struct term *beta;
struct context *c_from;
struct term *t;
struct term *into_term;
struct context *c_into;
{
    struct rel *r1, *r2, *r3;
    struct term *t2;

    if (t == into_term)
	return(apply(beta, c_from));
    else if (t->type != COMPLEX)
	return(apply(t, c_into));
    else {
        t2 = get_term();
        t2->type = COMPLEX;
        t2->sym_num = t->sym_num;
        r3 = NULL;
	for (r1 = t->farg; r1; r1 = r1->narg) {
            r2 = get_rel();
            if (r3)
                r3->narg = r2;
            else
                t2->farg = r2;
	    r3 = r2;
	    r2->argval = apply_substitute(beta, c_from, r1->argval, into_term, c_into);
	    }
	return(t2);
	}
}  /* apply_substitute */

/*************
 *
 *     oriented_eq_can_be_demod(lit)
 *
 *************/

int oriented_eq_can_be_demod(lit)
struct literal *lit;
{
    if (!pos_eq(lit))
	return(0);
    else if (Flags[LRPO].val)
	return(1);
    else {
	struct term *alpha, *beta;
	
	alpha = lit->atom->farg->argval;
	beta = lit->atom->farg->narg->argval;
	return(term_weight(alpha) > term_weight(beta) &&
	       (Parms[WEIGHT_FUNCTION].val > 0 ||
		multi_superset(alpha, beta)));
	}
}  /* oriented_eq_can_be_demod */

/*************
 *
 *   clause_ident(c1, c2)
 *
 *************/

int clause_ident(c1, c2)
struct literal *c1, *c2;
{
    return(c1->sign == c2->sign && term_ident(c1->atom, c2->atom));
}  /* clause_ident */

/*************
 *
 *   clause_compare()
 *
 *************/

int clause_compare(c1, c2)
struct literal *c1, *c2;
{
    if (c1->weight > c2->weight)
	return(GREATER_THAN);
    else if (c1->weight < c2->weight)
	return(LESS_THAN);
    else if (clause_ident(c1, c2))
	return(SAME_AS);
    else
	return(NOT_COMPARABLE);
}  /* clause_compare */

