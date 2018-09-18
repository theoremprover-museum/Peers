#include "Header.h"

/*************
 *
 *    struct term *copy_term(term) -- Return a copy of the term.
 *
 *    The bits field is not copied.
 *
 *************/

struct term *copy_term(t)
struct term *t;
{
    struct rel *r, *r2, *r3;
    struct term *t2;

    t2 = get_term();
    t2->type = t->type;
    t2->sym_num = t->sym_num;
    t2->varnum = t->varnum;
    if (t->type != COMPLEX)
	return(t2);
    else {
	r3 = NULL;
	r = t->farg;
	while (r != NULL) {
	    r2 = get_rel();
	    if (r3 == NULL)
		t2->farg = r2;
	    else 
		r3->narg = r2;
	    r2->argval = copy_term(r->argval);
	    r3 = r2;
	    r = r->narg;
	    }
	return(t2);
	}
}  /* copy_term */

/*************
 *
 *    struct term *copy_term_bits(term) -- Return a copy of the term.
 *
 *    The bits field is copied.
 *
 *************/

struct term *copy_term_bits(t)
struct term *t;
{
    struct rel *r, *r2, *r3;
    struct term *t2;

    t2 = get_term();
    t2->type = t->type;
    t2->sym_num = t->sym_num;
    t2->varnum = t->varnum;
    t2->bits = t->bits;
    if (t->type != COMPLEX)
	return(t2);
    else {
	r3 = NULL;
	r = t->farg;
	while (r != NULL) {
	    r2 = get_rel();
	    if (r3 == NULL)
		t2->farg = r2;
	    else 
		r3->narg = r2;
	    r2->argval = copy_term_bits(r->argval);
	    r3 = r2;
	    r = r->narg;
	    }
	return(t2);
	}
}  /* copy_term_bits */

/*************
 *
 *    int ground_term(t) -- is a term ground?
 *
 *************/

int ground_term(t)
struct term *t;
{
    struct rel *r;
    int ok;

    if (t->type == NAME)
	return(1);
    else if (t->type == VARIABLE)
	return(0);
    else { /* COMPLEX */
	ok = 1;
	for (r = t->farg; r != NULL && ok; r = r->narg)
	    ok = ground_term(r->argval);
	return(ok);
	}
}  /* ground_term */

/*************
 *
 *    int term_ident(term1, term2) -- Compare two terms.
 *
 *        If identical return(1); else return(0).  The bits
 *    field is not checked.
 *
 *************/

int term_ident(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;

    if (t1->type != t2->type)
	return(0);
    else if (t1->type == COMPLEX) {
	if (t1->sym_num != t2->sym_num)
	    return(0);
	else {
	    r1 = t1->farg;
	    r2 = t2->farg;
	    while (r1 && term_ident(r1->argval,r2->argval)) {
		r1 = r1->narg;
		r2 = r2->narg;
		}
	    return(r1 == NULL);
	    }
	}
    else if (t1->type == VARIABLE)
	return(t1->varnum == t2->varnum);
    else  /* NAME */
	return(t1->sym_num == t2->sym_num);
}  /* term_ident */  

/*************
 *
 *    zap_term(term)
 *
 *        Deallocate a nonshared term.  A warning is printed
 *    the term or any of its subterms contains a list of superterms.
 *
 *************/

void zap_term(t)
struct term *t;
{
    struct rel *r1, *r2;
    
    if (t->type == COMPLEX) { /* complex term */
	r1 = t->farg;
	while (r1 != NULL) {
	    zap_term(r1->argval);
	    r2 = r1;
	    r1 = r1->narg;
	    free_rel(r2);
	    }
	}
    free_term(t);
}  /* zap_term */

/*************
 *
 *    symbol_count
 *
 *************/

int symbol_count(t)
struct term *t;     
{
    struct rel *r;
    int n;

    if (t->type != COMPLEX)
	return(1);
    else {
	for (r = t->farg, n = 1; r; r = r->narg)
	    n += symbol_count(r->argval);
	return(n);
	}
}  /* symbol_count  */

/*************
 *
 *    int term_compare_ncv(t1, t2) -- Compare two terms.
 *
 *    NAME < COMPLEX < VARIABLE.
 *    Return SAME_AS, GREATER_THEN, or LESS_THAN.
 *
 *************/

int term_compare_ncv(t1,t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;
    int  rc;

    if (t1->type != t2->type) {
	if (t1->type == NAME)
	    rc = LESS_THAN;
        else if (t2->type == NAME)
	    rc = GREATER_THAN;
        else if (t1->type == COMPLEX)
	    rc = LESS_THAN;
	else
	    rc = GREATER_THAN;
	}

    else if (t1->type == NAME) {
	if (t1->sym_num == t2->sym_num)
	    rc = SAME_AS;
	else if (t1->sym_num < t2->sym_num)
	    rc = LESS_THAN;
	else
	    rc = GREATER_THAN;
	}
    else if (t1->type == COMPLEX) {
	if (t1->sym_num == t2->sym_num) {
	    /* Assume they have same number of args. */
	    r1 = t1->farg;
	    r2 = t2->farg;
	    while (r1 && (rc = term_compare_ncv(r1->argval, r2->argval)) == SAME_AS) {
		r1 = r1->narg;
		r2 = r2->narg;
		}
	    }
	else if (t1->sym_num < t2->sym_num)
	    rc = LESS_THAN;
	else
	    rc = GREATER_THAN;
	}
    else {
	/* both variables */

	if (t1->varnum < t2->varnum)
	    rc = LESS_THAN;
	else if (t1->varnum > t2->varnum)
	    rc = GREATER_THAN;
	else
	    rc = SAME_AS;
	}
    return(rc);
}  /* term_compare_ncv */

/*************
 *
 *    int term_compare_vf(t1, t2) -- Compare two terms.
 *
 *    VARIABLE smallest, rest lexicographically by sym_num.
 *    Return SAME_AS, GREATER_THEN, or LESS_THAN.
 *
 *************/

int term_compare_vf(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;
    int i;

    if (t1->type == VARIABLE)
        if (t2->type == VARIABLE)
            if (t1->varnum == t2->varnum)
                return(SAME_AS);
            else
                return(t1->varnum > t2->varnum ? GREATER_THAN : LESS_THAN);
        else
            return(LESS_THAN);

    else if (t2->type == VARIABLE)
        return(GREATER_THAN);

    else if (t1->sym_num == t2->sym_num) {
	if (t1->type == NAME)
	    return(SAME_AS);
	else {  /* both COMPLEX with same functor */
	    r1 = t1->farg;
	    r2 = t2->farg;
	    i = SAME_AS;
	    while (r1 && (i = term_compare_vf(r1->argval,r2->argval)) == SAME_AS) {
		r1 = r1->narg;
		r2 = r2->narg;
		}
	    return(i);
	    }
	}
    else
	return(t1->sym_num > t2->sym_num ? GREATER_THAN : LESS_THAN);

}  /* term_compare_vf */  

/*************
 *
 *    merge_sort
 *
 *************/

void merge_sort(a, w, start, end, comp_proc)
struct term *a[];  /* array to sort */
struct term *w[];  /* work array */
int start;         /* index of first element */
int end;           /* index of last element */
int (*comp_proc)();
{
    int mid, i, i1, i2, e1, e2;

    if (start < end) {
	mid = (start+end)/2;
	merge_sort(a, w, start, mid, comp_proc);
	merge_sort(a, w, mid+1, end, comp_proc);
	i1 = start; e1 = mid;
	i2 = mid+1; e2 = end;
	i = start;
	while (i1 <= e1 && i2 <= e2) {
	    if ((*comp_proc)(a[i1], a[i2]) == LESS_THAN)
		w[i++] = a[i1++];
	    else
		w[i++] = a[i2++];
	    }

	if (i2 > e2)
	    while (i1 <= e1)
		w[i++] = a[i1++];
	else
	    while (i2 <= e2)
		w[i++] = a[i2++];

	for (i = start; i <= end; i++)
	    a[i] = w[i];
	}
}  /* merge_sort */

