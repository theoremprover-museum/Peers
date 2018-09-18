#include "Header.h"
#include "Io.h"

/*************
 *
 *    int sym_precedence(sym_num_1, sym_num_2)
 *
 *    Return SAME_AS, GREATER_THAN, LESS_THAN, or NOT_COMPARABLE.
 *
 *************/

static int sym_precedence(sym_num_1, sym_num_2)
int sym_num_1;
int sym_num_2;
{
    int p1, p2;

    if (sym_num_1 == sym_num_2)
	return(SAME_AS);
    else {
	p1 = sn_to_node(sym_num_1)->lex_val;
	p2 = sn_to_node(sym_num_2)->lex_val;
	
	if (p1 == MAX_INT || p2 == MAX_INT)
	    return(NOT_COMPARABLE);
	else if (p1 > p2)
	    return(GREATER_THAN);
	else if (p1 < p2)
	    return(LESS_THAN);
	else
	    return(SAME_AS);
	}
}  /* sym_precedence */

/*************
 *
 *    int lrpo_status(sym_num)
 *
 *************/

static int lrpo_status(sym_num)
int sym_num;
{

    return(sn_to_node(sym_num)->lrpo_status);

}  /* lrpo_status */

/*************
 *
 *    int lrpo_lex(t1, t2) -- Is t1 > t2 ?
 *
 *    t1 and t2 have same functor and the functor has lr status.
 *
 *************/

static int lrpo_lex(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;
    int rc;

    /* First skip over any identical arguments. */
    /* (Same number of args, because same functor.) */

    for (r1 = t1->farg, r2 = t2->farg;
	 r1 != NULL && term_ident(r1->argval, r2->argval);
	 r1 = r1->narg, r2 = r2->narg) /* empty body */ ;

    if (r1 == NULL)
	rc = 0;  /* t1 and t2 identical */
    else if (lrpo(r1->argval, r2->argval)) {
	/* return (t1 > each remaining arg of t2) */
	for (r2 = r2->narg; r2 != NULL && lrpo(t1, r2->argval); r2 = r2->narg)
	    /* empty body */ ;
	rc = (r2 == NULL);
	}
    else {
	/* return (there is a remaining arg of t1 s.t. arg == t2 or arg > t2) */
	rc = 0;
	for (r1 = r1->narg; r1 != NULL; r1 = r1->narg) {
	    if (term_ident(r1->argval, t2) || lrpo(r1->argval, t2))
		rc = 1;
	    }
	}

    return(rc);

}  /* lrpo_lex */

/*************
 *
 *    int num_occurrences(t_arg, t) -- How many times does t_arg occur
 *    as an argument of t?
 *
 *************/

static int num_occurrences(t_arg, t)
struct term *t_arg;
struct term *t;
{
    struct rel *r;
    int i;

    for (i = 0, r = t->farg; r != NULL; r = r->narg)
       if (term_ident(r->argval, t_arg))
	   i++;

    return(i);

}  /* num_occurrences */

/*************
 *
 *   struct term *set_multiset_diff(t1, t2) 
 *
 *   Construct the multiset difference, then return the set of that.
 *   Result must be deallocated by caller with zap_term.
 *
 *   In other words, viewing a term as a multiset of its arguments,
 *   find the set of t1's arguments that have more occurrences in
 *   t1 than in t2.
 *
 *************/

static struct term *set_multiset_diff(t1, t2)
struct term *t1;
struct term *t2;
{
    struct term *t_result;
    struct rel *prev, *curr, *r1, *r;
    int i;

    t_result = get_term();
    prev = NULL;
    i = 0;
    for (r1 = t1->farg; r1 != NULL; r1 = r1->narg) {
	/* First check if a preceeding occurrence of this arg has */
	/* already been done. */

        for (r = t1->farg; r != r1 && term_ident(r->argval, r1->argval) == 0; r = r->narg)
	    /* empty body */ ;

        if (r == r1 && num_occurrences(r1->argval, t1) > num_occurrences(r1->argval, t2)) {
	    i++;
	    curr = get_rel();
	    curr->argval = copy_term(r1->argval);
	    if (prev == NULL)
	        t_result->farg = curr;
	    else
                prev->narg = curr;
	    prev = curr;
	    }
	}
	    
    t_result->type = (i == 0 ? NAME : COMPLEX) ;
    /* note that no sym_num is assigned; this should be ok */
    return(t_result);

}  /* set_multiset_diff */

/*************
 *
 *   int lrpo_multiset(t1, t2) -- Is t1 > t2 in the lrpo multiset ordering?
 *
 *   t1 and t2 have functors with the same precedence.
 *
 *   let n(a,t) be the number of occurrences of a (as an argument) in t.
 *
 *   t1 >(multiset) t2 iff for each arg a2 of t2 with n(a2,t2) > n(a2,t1),
 *   there is an arg a1 of t1 such that  n(a1,t1) > n(a1,t2), and a1>a2.
 *
 *************/

static int lrpo_multiset(t1, t2)
struct term *t1;
struct term *t2;
{
    struct term *s1, *s2;
    struct rel *r1, *r2;
    int ok;

    s1 = set_multiset_diff(t1, t2);  /* builds and returns a set */
    s2 = set_multiset_diff(t2, t1);  /* builds and returns a set */

    /*
    return (s2 not empty and foreach arg a2 of s2
	    there is an arg a1 of s1 such that lrpo(a1, a2)).
    */

    if (s2->farg == NULL)
       ok = 0;
    else {
	for (r2 = s2->farg, ok = 1; r2 && ok; r2 = r2->narg) {
	    for (r1 = s1->farg, ok = 0; r1 && !ok ; r1 = r1->narg)
		ok = lrpo(r1->argval, r2->argval);
	    }
	}

    zap_term(s1);
    zap_term(s2);

    return(ok);

}  /* lrpo_multiset */

/*************
 *
 *   int lrpo(t1, t2) - Is t1 > t2 in the lexicographic recursive
 *                      path ordering?
 *
 *************/

int lrpo(t1, t2)
struct term *t1;
struct term *t2;
{
    int p;
    struct rel *r;

    if (t1->type == VARIABLE)
	/* varaiable never greater than anything */
	return(0);  
    else if (t2->type == VARIABLE)
	/* t1 > variable iff t1 properly contains that variable */
	return(occurs_in(t2, t1));
    else if (t1->sym_num == t2->sym_num &&
             lrpo_status(t1->sym_num) == LRPO_LR_STATUS)
	return(lrpo_lex(t1, t2));
    else {
	p = sym_precedence(t1->sym_num, t2->sym_num);
	if (p == SAME_AS)
	    return(lrpo_multiset(t1,t2));
	else if (p == GREATER_THAN) {
	    /* return (t1 > each arg of t2) */
	    for (r = t2->farg; r != NULL && lrpo(t1, r->argval); r = r->narg)
		/* empty body */ ;
	    return(r == NULL);
	    }
	else {  /* LESS_THEN or NOT_COMPARABLE */
	    /* return (there is an arg of t1 s.t. arg == t2 or arg > t2) */
	    for (r = t1->farg; r != NULL; r = r->narg) {
		if (term_ident(r->argval, t2) || lrpo(r->argval, t2))
		    return(1);
		}
	    return(0);
	    }
	}
}  /* lrpo */

/*************
 *
 *   int lrpo_greater(t1, t2) - Is t1 > t2 in the lexicographic
 *                              recursive path ordering?
 *
 *    Time this routine.
 *
 *************/

int lrpo_greater(t1, t2)
struct term *t1;
struct term *t2;
{
    int rc;

    CLOCK_START(LRPO_TIME)
    rc = lrpo(t1,t2);
    CLOCK_STOP(LRPO_TIME)
    return(rc);

}  /* lrpo_greater */

/*************
 *
 *    int orient_literal_lrpo(c)
 *   
 *    Flip args if the right side is heavier.
 *
 *    return
 *       1: the left side is greater (after possible flip)
 *       0: neither side is greater (it was not flipped)
 *
 *************/

int orient_literal_lrpo(c)
     struct literal *c;
{
    struct rel *r1, *r2;
    struct term *alpha, *beta;
    int first_bigger, second_bigger;
    
    if (is_symbol(c->atom, "=", 2)) {
	r1 = c->atom->farg;
	r2 = r1->narg;
	alpha = r1->argval;
	beta  = r2->argval;
	
	first_bigger = lrpo_greater(alpha, beta);
	if (!first_bigger) {
	    second_bigger = lrpo_greater(beta, alpha);
	    if (second_bigger) {
		r1->argval = beta;
		r2->argval = alpha;
		}
	    }
	return(first_bigger || second_bigger);
	}
    else
	return(0);
}  /* orient_literal_lrpo */

