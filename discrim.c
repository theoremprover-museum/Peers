#include "Header.h"
#include "Index.h"
#include "Unify.h"

/*

This file contains code for two different types of discrimination tree
indexing.  Both types are for finding generalizations of a query term,
as when finding rewrite rules or subsuming literals.

The first type binds variables during the indexing and cannot handle
(in any way) AC function symbols.

The second type (whose routines usually have "wild" in the name) does
not bind variables (variables are wild cards) and handles AC function
symbols in two different ways, depending on the flag INDEX_AC_ARGS.
If the flag is clear, AC function symbols are simply treated as
constants.  If the flag is set, two levels of indexing occur on
arguments of AC symbols.  

The justification for the AC indexing is as follows.  Let t1 and t2 be
terms with the same AC symbol at their roots.  If t2 is an instance of t1, then
number_of_args(t1) <= number_of_args(t2), and
number_of_nonvariable_args(t1) <= number_of_non_variable_args(t2).
These two conditions correspond to the two levels of indexing for AC symbols.

*/

#define GO        1
#define BACKTRACK 2
#define SUCCESS   3
#define FAILURE   4
 
/*************
 *
 *     p_discrim_tree(d, n, depth)
 *
 *************/

void p_discrim_tree(d, n, depth)
struct discrim *d;
int n;
{
    int arity, i;

    for (i = 0; i < depth; i++)
	printf("  ");
    switch (d->type) {
      case NAME:
      case COMPLEX: printf("%s", sn_to_str(d->lab)); break;
      case VARIABLE: printf("v%d", d->lab); break;
      default: printf("\nRoot"); break;
	}
    if (n == 0) {
	struct gen_ptr *p;
	for (i = 0, p = d->u.data; p; i++, p = p->next);
	printf(": leaf node with %d objects.\n", i);
	}
    else {
	struct discrim *d1;
	printf(".\n");
	for (d1 = d->u.kids; d1; d1 = d1->next) {
	    switch (d1->type) {
	      case NAME:
	      case COMPLEX: arity = sn_to_arity(d1->lab); break;
	      case VARIABLE:  arity = 0; break;
		}
	    p_discrim_tree(d1, n+arity-1, depth+1);
	    }
	}
}  /* p_discrim_tree */

/*************
 *
 *    struct discrim *discrim_insert_rec(t, d)
 *
 *************/

static struct discrim *discrim_insert_rec(t, d)
struct term *t;
struct discrim *d;
{
    struct rel *r;
    struct discrim *d1, *prev, *d2;
    int varnum, sym;

    if (t->type == VARIABLE) {
	d1 = d->u.kids;
	prev = NULL;
	varnum = t->varnum;
	while (d1 && d1->type == VARIABLE && d1->lab < varnum) {
	    prev = d1;
	    d1 = d1->next;
	    }
	if (!d1 || d1->type != VARIABLE || d1->lab != varnum) {
	    d2 = get_discrim();
	    d2->type = VARIABLE;
	    d2->lab = t->varnum;
	    d2->next = d1;
	    if (!prev)
		d->u.kids = d2;
	    else
		prev->next = d2;
	    return(d2);
	    }
	else  /* found node */
	    return(d1);
	}

    else {  /* NAME || COMPLEX */
	d1 = d->u.kids;
	prev = NULL;
	/* arities fixed: handle both NAME and COMPLEX */
	sym = t->sym_num;
	while (d1 && d1->type == VARIABLE) {  /* skip variables */
	    prev = d1;
	    d1 = d1->next;
	    }
	while (d1 && d1->lab < sym) {
	    prev = d1;
	    d1 = d1->next;
	    }
	if (!d1 || d1->lab != sym) {
	    d2 = get_discrim();
	    d2->type = t->type;
	    d2->lab = sym;
	    d2->next = d1;
	    d1 = d2;
	    }
	else
	    d2 = NULL;  /* new node not required at this level */

	if (t->type == COMPLEX) {
	    for (r = t->farg; r; r = r->narg)
		d1 = discrim_insert_rec(r->argval, d1);
	    }

        if (d2)  /* link in new subtree (possibly a leaf) */
	    if (!prev)
		d->u.kids = d2;
	    else
		prev->next = d2;
	    
	return(d1);  /* d1 is leaf corresp. to end of input term */
	}
}  /* discrim_insert_rec */

/*************
 *
 *    discrim_insert(t, root, object)
 *
 *    Insert into a discrimination tree index for finding more general
 *    terms.
 *      t:      key
 *      root:   the discrimination tree
 *      object: the object to be inserted
 *    If object is not a pointer to a term, cast it into one.
 *
 *************/

void discrim_insert(t, root, object)
struct term *t;
struct discrim *root;
void *object;
{
    struct discrim *d;
    struct gen_ptr *gp1,*gp2;

    d = discrim_insert_rec(t, root);
    gp1 = get_gen_ptr();
    gp1->u.v = object;

    /* Install at end of list. */
    if (d->u.data == NULL)
	d->u.data = gp1;
    else {
	for (gp2 = d->u.data; gp2->next; gp2 = gp2->next);
	gp2->next = gp1;
	}

}  /* discrim_insert */

/*************
 *
 *    struct discrim *discrim_end(t, is, path_p)
 *
 *    Given a discrimination tree (or a subtree) and a term, return the 
 *    node in the tree that corresponds to the last symbol in t (or NULL
 *    if the node doesn't exist).  *path_p is a list that is extended by
 *    this routine.  It is a list of pointers to the
 *    nodes in path from the parent of the returned node up to imd. 
 *    (It is needed for deletions, because nodes do not have pointers to
 *    parents.) 
 *
 *************/

static struct discrim *discrim_end(t, is, path_p)
struct term *t;
struct discrim *is;
struct gen_ptr **path_p;
{
    struct rel *r;
    struct discrim *d;
    struct gen_ptr *isp;
    int varnum, sym;

    /* add current node to the front of the path list. */

    isp = get_gen_ptr();
    isp->u.v = (void *) is;
    isp->next = *path_p;
    *path_p = isp;

    if (t->type == VARIABLE) {
	d = is->u.kids;
	varnum = t->varnum;
	while (d && d->type == VARIABLE && d->lab < varnum) 
	    d = d->next;

	if (!d || d->type != VARIABLE || d->lab != varnum)
	    return(NULL);
	else   /* found node */
	    return(d);
	}

    else {  /* NAME || COMPLEX */
	d = is->u.kids;
	sym = t->sym_num;  /* arities fixed: handle both NAME and COMPLEX */
	while (d && d->type == VARIABLE)  /* skip variables */
	    d = d->next;
	while (d && d->lab < sym)
	    d = d->next;

	if (!d || d->lab != sym)
	    return(NULL);
	else {
	    if (t->type == NAME)
		return(d);
	    else {
		r = t->farg;
		while (r && d) {
		    d = discrim_end(r->argval, d, path_p);
		    r = r->narg;
		    }
		return(d);
		}
	    }
	}
}  /* discrim_end */

/*************
 *
 *    discrim_delete(t, root, object)
 *
 *************/

void discrim_delete(t, root, object)
struct term *t;
struct discrim *root;
void *object;
{
    struct discrim *end, *i2, *i3, *parent;
    struct gen_ptr *tp1, *tp2;
    struct gen_ptr *isp1, *path;

    /* First find the correct leaf.  path is used to help with  */
    /* freeing nodes, because nodes don't have parent pointers. */

    path = NULL;
    end = discrim_end(t, root, &path);
    if (!end)
	abend("in discrim_delete, can't find end.");

    /* Free the pointer in the leaf-list */

    tp1 = end->u.data;
    tp2 = NULL;
    while(tp1 && tp1->u.v != object) {
	tp2 = tp1;
	tp1 = tp1->next;
	}
    if (!tp1)
	abend("in discrim_delete, can't find term.");

    if (!tp2)
	end->u.data = tp1->next;
    else
	tp2->next = tp1->next;
    free_gen_ptr(tp1);

    if (end->u.data == NULL) {
        /* free tree nodes from bottom up, using path to get parents */
	end->u.kids = NULL;  /* probably not necessary */
	isp1 = path;
	while (end->u.kids == NULL && end != root) {
	    parent = (struct discrim *) isp1->u.v;
	    isp1 = isp1->next;
	    i2 = parent->u.kids;
	    i3 = NULL;
	    while (i2 != end) {
		i3 = i2;
		i2 = i2->next;
		}
	    if (!i3)
		parent->u.kids = i2->next;
	    else
		i3->next = i2->next;
	    free_discrim(i2);
	    end = parent;
	    }
	}

    /* free path list */

    while (path) {
	isp1 = path;
	path = path->next;
	free_gen_ptr(isp1);
	}

}  /* discrim_delete */

/*************
 *
 *    discrim_retrieve_leaf(t_in, root, subst, ppos)
 *
 *************/

struct gen_ptr *discrim_retrieve_leaf(t_in, root, subst, ppos)
struct term *t_in;
struct discrim *root;
struct context *subst;
struct flat **ppos;
{
    struct flat *f, *f1, *f2, *f_save;
    struct term *t;
    struct rel *r;
    struct discrim *d;
    int lab, match, bound, status;

    f = *ppos;  /* Don't forget to reset before return. */
    t = t_in;

    if (t) {  /* if first call */
        d = root->u.kids;
        if (d) {
	    f = get_flat();
	    f->t = t;
	    f->last = f;
	    f->prev = NULL;
	    f->next = NULL;
	    f->place_holder = (t->type == COMPLEX);
            status = GO;
	    }
        else
            status = FAILURE;
        }
    else
        status = BACKTRACK;

    while (status == GO || status == BACKTRACK) {
	if (status == BACKTRACK) {
	    while (f && !f->alternatives) {  /* clean up HERE??? */
		if (f->bound) {
                    subst->terms[f->varnum] = NULL;
		    f->bound = 0;
		    }
		f_save = f;
		f = f->prev;
		}
	    if (f) {
		if (f->bound) {
                    subst->terms[f->varnum] = NULL;
		    f->bound = 0;
		    }
		d = f->alternatives;
		f->alternatives = NULL;
		status = GO;
		}
	    else
		status = FAILURE;
	    }

	if (status == GO) {
	    match = 0;
	    while (!match && d && d->type == VARIABLE) {
		lab = d->lab;
		if (subst->terms[lab]) { /* if already bound */
		    match = term_ident(subst->terms[lab], f->t);
		    bound = 0;
		    }
		else { /* bind variable in discrim tree */
		    match = 1;
		    subst->terms[lab] = f->t;
		    bound = 1;
		    }
		if (!match)
		    d = d->next;
		}
	    if (match) {
		/* push alternatives */
		f->alternatives = d->next;
		f->bound = bound;
		f->varnum = lab;
		f = f->last;
		}
	    else if (f->t->type == VARIABLE)
		status = BACKTRACK;
	    else {
		lab = f->t->sym_num;
		while (d && d->lab < lab)
		    d = d->next;
		if (!d || d->lab != lab)
		    status = BACKTRACK;
		else if (f->place_holder) {
		    /* insert skeleton in place_holder */
		    f1 = get_flat();
		    f1->t = f->t;
		    f1->prev = f->prev;
		    f1->last = f;
		    f_save = f1;
		    if (f1->prev)
			f1->prev->next = f1;
		    for (r = f->t->farg; r; r = r->narg) {
			if (r->narg)
			    f2 = get_flat();
			else
			    f2 = f;
			f2->place_holder = (r->argval->type == COMPLEX);
			f2->t = r->argval;
			f2->last = f2;
			f2->prev = f1;
			f1->next = f2;
			f1 = f2;
			}
		    f = f_save;
		    }
		}
	    if (status == GO) {
		if (f->next) {
		    f = f->next;
		    d = d->u.kids;
		    }
		else
		    status = SUCCESS;
		}
	    }
	}
    if (status == SUCCESS) {
	*ppos = f;
	return(d->u.data);
	}
    else {
	/* Free flats. */
	while (f_save) {
	    f1 = f_save;
	    f_save = f_save->next;
	    free_flat(f1);
	    }
	return(NULL);
	}
	   
}  /* discrim_retrieve_leaf */

/*************
 *
 *    discrim_retrieve_first(t, root, subst, ppos)
 *
 *    Get the first object associated with a term more general than t.
 *
 *    Remember to call discrim_cancel(*ppos, context) if you don't want the
 *    whole sequence.
 *
 *************/

void *discrim_retrieve_first(t, root, subst, ppos)
struct term *t;
struct discrim *root;
struct context *subst;
struct discrim_pos **ppos;
{
    struct gen_ptr *tp;
    struct flat *f;
    struct discrim_pos *gp;

    tp = discrim_retrieve_leaf(t, root, subst, &f);
    if (!tp)
	return(NULL);
    else {
	gp = get_discrim_pos();
	gp->f = f;
	gp->data = tp;
	*ppos = gp;
	return(tp->u.v);
	}
}  /* discrim_retrieve_first */

/*************
 *
 *    discrim_retrieve_next(subst, ppos)
 *
 *    Get the next object associated with a term more general than t.
 *
 *    Remember to call discrim_cancel(*ppos, context) if you don't want the
 *    whole sequence.
 *
 *************/

void *discrim_retrieve_next(subst, ppos)
struct context *subst;
struct discrim_pos **ppos;
{
    struct gen_ptr *tp;
    struct discrim_pos *gp;
    
    gp = *ppos;
    tp = gp->data->next;
    if (tp) {  /* if any more terms in current leaf */
	gp->data = tp;
	return(tp->u.v);
	}
    else {  /* try for another leaf */
	tp = discrim_retrieve_leaf((struct term *) NULL,
				   (struct discrim*) NULL, subst, &(gp->f));
	if (tp) {
	    gp->data = tp;
	    return(tp->u.v);
	    }
	else {
	    free_discrim_pos(gp);
	    return(NULL);
	    }
	}
}  /* discrim_retrieve_next */

/*************
 *
 *    discrim_cancel(pos, subst)
 *
 *************/

void discrim_cancel(pos, subst)
struct discrim_pos *pos;
struct context *subst;
{
    struct flat *f1, *f2;

    f1 = pos->f;
    while (f1) {
	if (f1->bound)
	    subst->terms[f1->varnum] = NULL;
	f2 = f1;
	f1 = f1->prev;
	free_flat(f2);
	}
    free_discrim_pos(pos);
}  /* discrim_cancel */

/********************************* WILD ******************************/


/*************
 *
 *     p_discrim_wild_tree(d, n)
 *
 *************/

void p_discrim_wild_tree(d, n, depth)
struct discrim *d;
int n;
{
    int arity, i;

    for (i = 0; i < depth; i++)
	printf("  ");
    switch (d->type) {
      case NAME:
      case COMPLEX: printf("%s", sn_to_str(d->lab)); break;
      case VARIABLE: printf("*"); break;
      case AC_ARG_TYPE: printf("AC %d args", d->lab); break;
      case AC_NV_ARG_TYPE: printf("AC %d NV args", d->lab); break;
      default: printf("\nRoot"); break;
	}
    if (n == 0) {
	struct gen_ptr *p;
	for (i = 0, p = d->u.data; p; i++, p = p->next);
	printf(": leaf node with %d objects.\n", i);
	}
    else {
	struct discrim *d1;
	printf(".\n");
	for (d1 = d->u.kids; d1; d1 = d1->next) {
	    switch (d1->type) {
	      case NAME:
	      case COMPLEX:
		if (!Flags[INDEX_AC_ARGS].val && is_assoc_comm(d1->lab))
		    arity = 0;
		else
		    arity = sn_to_arity(d1->lab);
		break;
	      case VARIABLE:
	      case AC_ARG_TYPE:
	      case AC_NV_ARG_TYPE: arity = 0; break;
		}
	    p_discrim_wild_tree(d1, n+arity-1, depth+1);
	    }
	}
}  /* p_discrim_wild_tree */

/*************
 *
 *     discrim_wild_insert_ac(t, d)
 *
 *************/

struct discrim *discrim_wild_insert_ac(t, d)
struct term *t;
struct discrim *d;
{
    int num_args, num_nv_args;
    struct discrim *d1, *d2, *dnew, *prev;

    num_args = num_ac_args(t, t->sym_num);

    for (d1 = d->u.kids, prev = NULL; d1 && d1->lab < num_args; prev = d1, d1 = d1->next);
    if (!d1 || d1->lab != num_args) {
	dnew = get_discrim();
	dnew->type = AC_ARG_TYPE;
	dnew->lab = num_args;
	dnew->next = d1;
	if (prev)
	    prev->next = dnew;
	else
	    d->u.kids = dnew;
	d1 = dnew;
	}

    num_nv_args = num_ac_nv_args(t, t->sym_num);

    for (d2 = d1->u.kids, prev = NULL; d2 && d2->lab < num_nv_args; prev = d2, d2 = d2->next);
    if (!d2 || d2->lab != num_nv_args) {
	dnew = get_discrim();
	dnew->type = AC_NV_ARG_TYPE;
	dnew->lab = num_nv_args;
	dnew->next = d2;
	if (prev)
	    prev->next = dnew;
	else
	    d1->u.kids = dnew;
	d2 = dnew;
	}
    return(d2);
    
}  /* discrim_wild_insert_ac */

/*************
 *
 *    struct discrim *discrim_wild_insert_rec(t, d)
 *
 *************/

static struct discrim *discrim_wild_insert_rec(t, d)
struct term *t;
struct discrim *d;
{
    struct rel *r;
    struct discrim *d1, *prev, *d2;
    int sym;

    if (t->type == VARIABLE) {
	d1 = d->u.kids;
	if (!d1 || d1->type != VARIABLE) {
	    d2 = get_discrim();
	    d2->type = VARIABLE;
	    d2->next = d1;
	    d->u.kids = d2;
	    return(d2);
	    }
	else  /* found node */
	    return(d1);
	}

    else {  /* NAME || COMPLEX */
	d1 = d->u.kids;
	prev = NULL;
	/* arities fixed: handle both NAME and COMPLEX */
	sym = t->sym_num;
	if (d1 && d1->type == VARIABLE) {  /* skip variable */
	    prev = d1;
	    d1 = d1->next;
	    }
	while (d1 && d1->lab < sym) {
	    prev = d1;
	    d1 = d1->next;
	    }
	if (!d1 || d1->lab != sym) {
	    d2 = get_discrim();
	    d2->type = t->type;
	    d2->lab = sym;
	    d2->next = d1;
	    d1 = d2;
	    }
	else
	    d2 = NULL;  /* new node not required at this level */

	if (is_assoc_comm(t->sym_num)) {
	    if (Flags[INDEX_AC_ARGS].val)
		d1 = discrim_wild_insert_ac(t, d1);
	    /* If flag is clear, AC symbol is treated like a NAME. */
	    }

	else if (t->type == COMPLEX) {
	    for (r = t->farg; r; r = r->narg)
		d1 = discrim_wild_insert_rec(r->argval, d1);
	    }

        if (d2)  /* link in new subtree (possibly a leaf) */
	    if (!prev)
		d->u.kids = d2;
	    else
		prev->next = d2;
	    
	return(d1);  /* d1 is leaf corresp. to end of input term */
	}
}  /* discrim_wild_insert_rec */

/*************
 *
 *    discrim_wild_insert(t, root, object)
 *
 *    Insert into a discrimination tree index for finding more general
 *    terms.
 *      t:      key
 *      root:   the discrimination tree
 *      object: the object to be inserted  (void *)
 *
 *************/

void discrim_wild_insert(t, root, object)
struct term *t;
struct discrim *root;
void *object;
{
    struct discrim *d;
    struct gen_ptr *gp1,*gp2;

    d = discrim_wild_insert_rec(t, root);
    gp1 = get_gen_ptr();
    gp1->u.v = object;

    /* Install at end of list. */
    if (d->u.data == NULL)
	d->u.data = gp1;
    else {
	for (gp2 = d->u.data; gp2->next; gp2 = gp2->next);
	gp2->next = gp1;
	}

}  /* discrim_wild_insert */

/*************
 *
 *    struct discrim *discrim_wild_end(t, is, path_p)
 *
 *    Given a discrimination tree (or a subtree) and a term, return the 
 *    node in the tree that corresponds to the last symbol in t (or NULL
 *    if the node doesn't exist).  *path_p is a list that is extended by
 *    this routine.  It is a list of pointers to the
 *    nodes in path from the parent of the returned node up to imd. 
 *    (It is needed for deletions, because nodes do not have pointers to
 *    parents.) 
 *
 *************/

static struct discrim *discrim_wild_end(t, d, path_p)
struct term *t;
struct discrim *d;
struct gen_ptr **path_p;
{
    struct rel *r;
    struct discrim *d1;
    struct gen_ptr *p;
    int sym;

    /* add current node to the front of the path list. */

    p = get_gen_ptr();
    p->u.v = (void *) d;
    p->next = *path_p;
    *path_p = p;

    if (t->type == VARIABLE) {
	d1 = d->u.kids;
	if (d1 && d1->type == VARIABLE)
	    return(d1);
	else
	    return(NULL);
	}

    else {  /* NAME || COMPLEX */
	d1 = d->u.kids;
	sym = t->sym_num;  /* arities fixed: handle both NAME and COMPLEX */
	if (d1 && d1->type == VARIABLE)  /* skip variables */
	    d1 = d1->next;
	while (d1 && d1->lab < sym)
	    d1 = d1->next;

	if (!d1 || d1->lab != sym)
	    return(NULL);
	else if (t->type == NAME)
		return(d1);
	else if (is_assoc_comm(t->sym_num)) {
	    if (!Flags[INDEX_AC_ARGS].val)
		return(d1);
	    else {
		int num_args, num_nv_args;
		struct discrim *d2, *d3;
		
		num_args = num_ac_args(t, t->sym_num);
		num_nv_args = num_ac_nv_args(t, t->sym_num);
		
		for (d2 = d1->u.kids; d2 && d2->lab != num_args; d2 = d2->next);
		if (!d2)
		    return(NULL);
		else {
		    for (d3 = d2->u.kids; d3 && d3->lab != num_nv_args; d3 = d3->next);
		    if (!d3)
			return(NULL);
		    else {
			
			p = get_gen_ptr();
			p->u.v = (void *) d1;
			p->next = *path_p;
			*path_p = p;
			
			p = get_gen_ptr();
			p->u.v = (void *) d2;
			p->next = *path_p;
			*path_p = p;
			
			return(d3);
			}
		    }
		}
	    }
	else {
	    r = t->farg;
	    while (r && d1) {
		d1 = discrim_wild_end(r->argval, d1, path_p);
		r = r->narg;
		}
	    return(d1);
	    }
	}
}  /* discrim_wild_end */

/*************
 *
 *    discrim_wild_delete(t, root, object)
 *
 *************/

void discrim_wild_delete(t, root, object)
struct term *t;
struct discrim *root;
void *object;
{
    struct discrim *end, *i2, *i3, *parent;
    struct gen_ptr *tp1, *tp2;
    struct gen_ptr *isp1, *path;

    /* First find the correct leaf.  path is used to help with  */
    /* freeing nodes, because nodes don't have parent pointers. */

    path = NULL;
    end = discrim_wild_end(t, root, &path);
    if (!end)
	abend("in discrim_wild_delete, can't find end.");

    /* Free the pointer in the leaf-list */

    tp1 = end->u.data;
    tp2 = NULL;
    while(tp1 && tp1->u.v != object) {
	tp2 = tp1;
	tp1 = tp1->next;
	}
    if (!tp1)
	abend("in discrim_wild_delete, can't find term.");

    if (!tp2)
	end->u.data = tp1->next;
    else
	tp2->next = tp1->next;
    free_gen_ptr(tp1);

    if (end->u.data == NULL) {
        /* free tree nodes from bottom up, using path to get parents */
	end->u.kids = NULL;  /* probably not necessary */
	isp1 = path;
	while (end->u.kids == NULL && end != root) {
	    parent = (struct discrim *) isp1->u.v;
	    isp1 = isp1->next;
	    i2 = parent->u.kids;
	    i3 = NULL;
	    while (i2 != end) {
		i3 = i2;
		i2 = i2->next;
		}
	    if (!i3)
		parent->u.kids = i2->next;
	    else
		i3->next = i2->next;
	    free_discrim(i2);
	    end = parent;
	    }
	}

    /* free path list */

    while (path) {
	isp1 = path;
	path = path->next;
	free_gen_ptr(isp1);
	}

}  /* discrim_wild_delete */

/*************
 *
 *    discrim_wild_retrieve_leaf(t_in, root, ppos)
 *
 *************/

struct gen_ptr *discrim_wild_retrieve_leaf(t_in, root, ppos)
struct term *t_in;
struct discrim *root;
struct flat **ppos;
{
    struct flat *f, *f1, *f2, *f_save;
    struct term *t;
    struct rel *r;
    struct discrim *d;
    int lab, status;

    f = *ppos;  /* Don't forget to reset before return. */
    t = t_in;

    if (t) {  /* if first call */
        d = root->u.kids;
        if (d) {
	    f = get_flat();
	    f->t = t;
	    f->last = f;
	    f->prev = NULL;
	    f->next = NULL;

	    if (Flags[INDEX_AC_ARGS].val)
		f->place_holder = (t->type == COMPLEX);
	    else
		f->place_holder = (t->type == COMPLEX && !is_assoc_comm(t->sym_num));
				   
            status = GO;
	    }
        else
            status = FAILURE;
        }
    else
        status = BACKTRACK;

    while (status == GO || status == BACKTRACK) {

	/* Three things determine the state at this point.      */
        /* 1. d is the current node in the discrimination tree. */
	/* 2. f is the current node in the stack of flats.      */
	/* 3. status is either GO or BACKTRACK.                 */

	if (status == BACKTRACK) {
	    /* Back up to a place with an aternative branch. */
	    while (f && !f->alternatives) {
		f_save = f;
		f = f->prev;
		}
	    if (f) {
		d = f->alternatives;
		f->alternatives = NULL;
		status = GO;
		}
	    else {
		/* Free stack of flats and fail. */
		while (f_save) {
		    f1 = f_save;
		    f_save = f_save->next;
		    free_flat(f1);
		    }
		status = FAILURE;
		}
	    }

	if (status == GO) {
	    if (d && d->type == AC_ARG_TYPE) {
		if (d->lab <= f->num_ac_args)
		    f->alternatives = d->next;
		else
		    status = BACKTRACK;
		}
	    else if (d && d->type == AC_NV_ARG_TYPE) {
		if (d->lab <= f->num_ac_nv_args)
		    f->alternatives = d->next;
		else
		    status = BACKTRACK;
		}
	    else if (d && d->type == VARIABLE) {
		/* push alternatives */
		f->alternatives = d->next;
		f = f->last;
		}
	    else if (f->t->type == VARIABLE)
		status = BACKTRACK;
	    else {
		lab = f->t->sym_num;
		while (d && d->lab < lab)
		    d = d->next;
		if (!d || d->lab != lab)
		    status = BACKTRACK;
		else if (f->place_holder) {
		    /* Insert skeleton in place_holder.  This is tricky, because */
		    /* someone's "last" pointer may be pointing to f.  Therefore, */
		    /* f becomes the last argument of the skeleton. */

		    if (is_assoc_comm(f->t->sym_num)) {
			/* Can't get here if INDEX_AC_ARGS flag is clear. */
			f1 = get_flat();
			f1->t = f->t;
			f1->prev = f->prev;
			f1->last = f;
			if (f1->prev)
			    f1->prev->next = f1;

			f2 = get_flat();
			f2->prev = f1;
			f2->last = f2;
			f2->next = f;
			f->prev = f2;
			f1->next = f2;
			f->last = f;

			/* Now, f2 is the AC_ARGS node, and f is the AC_NV_ARGS node. */
			f2->num_ac_args = num_ac_args(f1->t, f1->t->sym_num);
			f->num_ac_nv_args = num_ac_nv_args(f1->t, f1->t->sym_num);

			f = f1;
			}

		    else {  /* non AC case */
			f1 = get_flat();
			f1->t = f->t;
			f1->prev = f->prev;
			f1->last = f;
			f_save = f1;
			if (f1->prev)
			    f1->prev->next = f1;
			for (r = f->t->farg; r; r = r->narg) {
			    if (r->narg)
				f2 = get_flat();
			    else
				f2 = f;
			    if (Flags[INDEX_AC_ARGS].val)
				f2->place_holder = (r->argval->type == COMPLEX);
			    else
				f2->place_holder = (r->argval->type == COMPLEX &&
						    !is_assoc_comm(r->argval->sym_num));
			    f2->t = r->argval;
			    f2->last = f2;
			    f2->prev = f1;
			    f1->next = f2;
			    f1 = f2;
			    }
			f = f_save;
			}
		    }
		}
	    if (status == GO) {
		if (f->next) {
		    f = f->next;
		    d = d->u.kids;
		    }
		else
		    status = SUCCESS;
		}
	    }
	}  /* while */
    if (status == SUCCESS) {
	*ppos = f;
	return(d->u.data);
	}
    else
	return(NULL);
	   
}  /* discrim_wild_retrieve_leaf */

/*************
 *
 *    discrim_wild_retrieve_first(t, root, ppos)
 *
 *    Get the first object associated with a term more general than t.
 *
 *    Remember to call discrim_wild_cancel(*ppos, context) if you don't want the
 *    whole sequence.
 *
 *************/

void *discrim_wild_retrieve_first(t, root, ppos)
struct term *t;
struct discrim *root;
struct discrim_pos **ppos;
{
    struct gen_ptr *tp;
    struct flat *f;
    struct discrim_pos *gp;

    tp = discrim_wild_retrieve_leaf(t, root, &f);
    if (!tp)
	return(NULL);
    else {
	gp = get_discrim_pos();
	gp->f = f;
	gp->data = tp;
	*ppos = gp;
	return(tp->u.v);
	}
}  /* discrim_wild_retrieve_first */

/*************
 *
 *    discrim_wild_retrieve_next(ppos)
 *
 *    Get the next object associated with a term more general than t.
 *
 *    Remember to call discrim_wild_cancel(*ppos, context) if you don't want the
 *    whole sequence.
 *
 *************/

void *discrim_wild_retrieve_next(ppos)
struct discrim_pos **ppos;
{
    struct gen_ptr *tp;
    struct discrim_pos *gp;
    
    gp = *ppos;
    tp = gp->data->next;
    if (tp) {  /* if any more terms in current leaf */
	gp->data = tp;
	return(tp->u.v);
	}
    else {  /* try for another leaf */
	tp = discrim_wild_retrieve_leaf((struct term *) NULL,
				   (struct discrim*) NULL, &(gp->f));
	if (tp) {
	    gp->data = tp;
	    return(tp->u.v);
	    }
	else {
	    free_discrim_pos(gp);
	    return(NULL);
	    }
	}
}  /* discrim_wild_retrieve_next */

/*************
 *
 *    discrim_wild_cancel(pos)
 *
 *************/

void discrim_wild_cancel(pos)
struct discrim_pos *pos;
{
    struct flat *f1, *f2;

    f1 = pos->f;
    while (f1) {
	f2 = f1;
	f1 = f1->prev;
	free_flat(f2);
	}
    free_discrim_pos(pos);
}  /* discrim_wild_cancel */

