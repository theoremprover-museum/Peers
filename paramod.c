#include "Header.h"
#define OWNER_OF_PARA_GLOBALS
#include "Paramod.h"
#include "Io.h"
#include "Unify.h"
#include "Index.h"
#include "Peers.h"

/*************
 *
 *     init_pp_index()
 *
 *************/

void init_pp_index()
{
    int i;
    for (i = 0; i < PP_INDEX_SIZE; i++) {
	Para_pairs_index[i] = NULL;
	Para_pairs_pending[i] = 0;
	Para_pairs_used[i] = 0;
	}
    Para_pairs_first = NULL;
    Para_pairs_last = NULL;
}  /* init_pp_index */

/*************
 *
 *     pp_index_wt(wt)
 *
 *************/

int pp_index_wt(wt)
int wt;     
{
    return(wt < 0 ? 0 : (wt >= PP_INDEX_SIZE ? PP_INDEX_SIZE - 1 : wt));
}  /* pp_index_wt */

/*************
 *
 *     store_in_pp(c1, c2)
 *
 *************/

void store_in_pp(c1, c2)
struct literal *c1, *c2;     
{
    int wt, i;
    struct para_pair *p, *p1;
    struct term *t1, *t2;

    if (Flags[ALPHA_PAIR_WEIGHT].val) {
	t1 = c1->atom->farg->argval;
	t2 = c2->atom->farg->argval;
	}
    else {
	t1 = c1->atom;
	t2 = c2->atom;
	}

    wt = term_weight(t1) + term_weight(t2);
    
    /* Make sure wt is in range [0..PP_INDEX_SIZE-1]. */
    wt = pp_index_wt(wt);

    if (wt <= Parms[MAX_PAIR_WEIGHT].val) {
	Para_pairs_pending[wt]++;  /* statistic only */
	p = get_para_pair();
	p->c1 = c1;
	p->c2 = c2;
	p->weight = wt;

	/* go to the first non-null position > wt */
	
	i = 0;
	while (i < PP_INDEX_SIZE && (Para_pairs_index[i] == NULL || i <= wt))
	    i++;
	
	if (i == PP_INDEX_SIZE) {  /* insert at end */
	    p->next = NULL;
	    p->prev = Para_pairs_last;
	    if (Para_pairs_last)
		Para_pairs_last->next = p;
	    else
		Para_pairs_first = p;
	    Para_pairs_last = p;
	    }
	else {  /* insert before current position */
	    p1 = Para_pairs_index[i];
	    
	    p->next = p1;
	    p->prev = p1->prev;
	    if (p->prev)
		p->prev->next = p;
	    else
		Para_pairs_first = p;
	    p1->prev = p;
	    }
	
	/* update index */
	
	if (Para_pairs_index[wt] == NULL)
	    Para_pairs_index[wt] = p;
	}

#if 0  /* following code checks pp list and index. */
    wt = -1;
    for (p = Para_pairs_first; p; p = p->next) {
	if (wt != p->weight) {
	    if (Para_pairs_index[p->weight] != p)
		printf("store: bad index entry.\n");
	    for (i = wt+1; i < p->weight; i++)
		if (Para_pairs_index[i])
		    printf("store: bad index not NULL.\n");
	    wt = p->weight;
	    }

	if (p->prev) {
	    if (p->prev->next != p)
		printf("store: bad prev.\n");
	    }
	else if (Para_pairs_first != p)
	    printf("store: bad start.\n");

	if (p->next) {
	    if (p->next->prev != p)
		printf("store: bad next.\n");
	    }
	else if (Para_pairs_last != p)
	    printf("store: bad end.\n");
	}
#endif

}  /* store_in_pp */

/*************
 *
 *     get_from_pp()
 *
 *************/

struct para_pair *get_from_pp()
{
    struct para_pair *p;
    int wt;

    p = Para_pairs_first;
    if (p) {
	Para_pairs_pending[p->weight]--;  /* statistic only */
	Para_pairs_used[p->weight]++;  /* statistic only */
	Para_pairs_first = p->next;
	if (Para_pairs_first)
	    Para_pairs_first->prev = NULL;
	else
	    Para_pairs_last = NULL;

	if (p->next == NULL || pp_index_wt(p->next->weight) != p->weight)
	    Para_pairs_index[p->weight] = NULL;
	else
	    Para_pairs_index[p->weight] = p->next;
	}
    return(p);
}  /* get_from_pp */

/*************
 *
 *     p_para_pairs_stats()
 *
 *************/

void p_para_pairs_stats()
{
    int i;
    printf("\n<weight, pairs used, pairs pending>\n");
    for (i = 0; i < PP_INDEX_SIZE; i++) {
	if (Para_pairs_used[i] || Para_pairs_pending[i])
	    printf("<%d, %d, %d>\n", i, Para_pairs_used[i],
		                        Para_pairs_pending[i]);
	}
}  /* p_para_pairs_stats */

/*************
 *
 *    new_demodulator_input(demod)
 *
 *************/

void new_demodulator_input(demod)
struct literal *demod;     
{
    struct list_pos *p, *p2;
    struct list *destination, *to_be_back_demodulated;
    struct term *alpha;
    struct literal *lit, *new_lit;
    int i;

    if (Flags[PRINT_BACK_DEMOD].val) {
	printf("<%d,%d,%d> is a new demodulator.\n", owner(demod), lid(demod), birth_time(demod));
	fflush(stdout);
	}

    alpha = demod->atom->farg->argval;

    to_be_back_demodulated = get_list();

    /* For each clause in Usable or Sos that can be rewritten with demod, */
    /* move it to to_be_back_demodulated. */

    CLOCK_START(BD_FIND_TIME)
    for (i = 0; i < 2; i++) {
	p = (i == 0 ? Usable->first : Sos->first);
	while (p) {
	    lit = p->lit;
	    if (lit != demod && has_an_instance(lit->atom, alpha)) {
		Stats[CLAUSES_BACK_DEMODULATED]++;
		if (Flags[PRINT_BACK_DEMOD].val) {
		    printf("    -> <%d,%d,%d> back demodulating <%d,%d,%d>.\n",
			   owner(demod), lid(demod), birth_time(demod),
			   owner(lit), lid(lit), birth_time(lit));
		    fflush(stdout);
		    }
                if (Flags[INDEX_DEMOD].val && list_member(lit, Demodulators))
		    discrim_wild_delete(lit->atom->farg->argval, Demod_index, (void *) lit);
		list_append(lit, to_be_back_demodulated);
		}
	    p = p->next;
	    }
	}
    CLOCK_STOP(BD_FIND_TIME)

    list_append(demod, Demodulators);
    if (Flags[INDEX_DEMOD].val)
    	{
	discrim_wild_insert(demod->atom->farg->argval, Demod_index, (void *) demod);
	if (Flags[TWO_WAY_DEMOD].val)
	    discrim_wild_insert(demod->atom->farg->narg->argval, Demod_index, (void *) demod);
	}
    /* For each clause in to_be_back_demodulated, move it to */
    /* Deleted_clause, copy it, and process the copy. */
    
    while (to_be_back_demodulated->first) {
	lit = to_be_back_demodulated->first->lit;
	if (list_member(lit, Sos)) {
	    if (mine(lit))
		Stats[RESIDENTS_IN_SOS]--;
	    else
		Stats[VISITORS_IN_SOS]--;
	    }
	list_remove_all(lit);
	list_append(lit, Deleted_clauses);
	new_lit = get_literal();
	new_lit->sign = lit->sign;
	new_lit->atom = copy_term(lit->atom);
	new_lit->bd_parent = lit->id;
	process_input_clause(new_lit, Sos);
	}
    free_list(to_be_back_demodulated);
	
}  /* new_demodulator_input */

/*************
 *
 *    new_demodulator(demod)
 *
 *************/

void new_demodulator(demod)
struct literal *demod;     
{
    struct list_pos *p, *p2;
    struct list *to_be_back_demodulated;
    struct term *alpha;
    struct literal *lit, *new_lit;
    int i;

    if (Flags[PRINT_BACK_DEMOD].val) {
	fprintf(Peer_fp, "<%d,%d,%d> is a new demodulator.\n", owner(demod), lid(demod), birth_time(demod));
	fflush(Peer_fp);
	}

    alpha = demod->atom->farg->argval;

    to_be_back_demodulated = get_list();

    /* For each clause in Usable or Sos that can be rewritten with demod, */
    /* move it to to_be_back_demodulated. */

    CLOCK_START(BD_FIND_TIME)
    for (i = 0; i < 2; i++) {
	p = (i == 0 ? Usable->first : Sos->first);
	while (p) {
	    lit = p->lit;
	    if (lit != demod && has_an_instance(lit->atom, alpha)) {
		Stats[CLAUSES_BACK_DEMODULATED]++;
		if (Flags[PRINT_BACK_DEMOD].val) {
		    fprintf(Peer_fp, "    -> <%d,%d,%d> back demodulating <%d,%d,%d>.\n",
			   owner(demod), lid(demod), birth_time(demod),
			   owner(lit), lid(lit), birth_time(lit));
		    fflush(Peer_fp);
		    }
                if (list_member(lit, Demodulators)) {
		    if (Flags[INDEX_DEMOD].val)
			discrim_wild_delete(lit->atom->farg->argval, Demod_index, (void *) lit);
		    list_remove(lit, Demodulators); 
		    }
		list_append(lit, to_be_back_demodulated);
		}
	    p = p->next;
	    }
	}
    CLOCK_STOP(BD_FIND_TIME)

    list_append(demod, Demodulators);
   if (Flags[INDEX_DEMOD].val)
    	{
	discrim_wild_insert(demod->atom->farg->argval, Demod_index, (void *) demod);
	if (Flags[TWO_WAY_DEMOD].val)
	    discrim_wild_insert(demod->atom->farg->narg->argval, Demod_index, (void *) demod);
	}

    /* For each clause in to_be_back_demodulated, move it to */
    /* Deleted_clauses, copy it, and process the copy. */
    
    while (to_be_back_demodulated->first) {
	lit = to_be_back_demodulated->first->lit;
	if (mine(lit)) {
	    Stats[RESIDENTS]--;
	    Stats[BD_RESIDENTS]++;
	    }
	else
	    Stats[BD_INF_MSGS]++;
	    
	if (list_member(lit, Sos)) {
	    list_remove(lit, Sos);
	    if (mine(lit))
		Stats[RESIDENTS_IN_SOS]--;
	    else
		Stats[VISITORS_IN_SOS]--;
	    }

	list_remove_all(lit);
        list_append(lit, Deleted_clauses);

	new_lit = get_literal();
	new_lit->sign = lit->sign;
	new_lit->atom = copy_term(lit->atom);

	switch (Parms[BACK_DEMOD_STRAT].val) {

	  case ALL_RAW:
	    new_lit->bd_parent = lit->id;
	    process_new_clause(new_lit);
	    break;

	  case R_PIDCBT_IM_RAW:
	    new_lit->bd_parent = lit->id;
	    if (mine(lit))
		process_new_settler(new_lit);
	    else
		process_new_clause(new_lit);
	    break;

	  case R_PIDLIDCBT_IM_RAW:
	    delete_clause_by_name(lit, Name_table);
	    new_lit->bd_parent = lit->id;
	    if (mine(lit)) {
		new_lit->name = lit->name;
		new_lit->id = lit->id;
		process_bd_name_strategies(new_lit);
		}
	    else
		process_new_clause(new_lit);
	    break;

	  case R_PIDLIDCBT_IM_NOC:
	    delete_clause_by_name(lit, Name_table);
	    new_lit->name = lit->name;
	    new_lit->id = lit->id;
	    if (mine(lit))
		new_lit->bd_parent = lit->id;
	    else {
		new_lit->bd_parent = lit->bd_parent;
		new_lit->from_parent = lit->from_parent;
		new_lit->into_parent = lit->into_parent;
		new_lit->from_parent_extended = lit->from_parent_extended;
		new_lit->into_parent_extended = lit->into_parent_extended;
		new_lit->bd_count = lit->bd_count + 1;
		new_lit->demod_parents = copy_gen_ptr_list(lit->demod_parents);
		}
	    process_bd_name_strategies(new_lit);
	    break;

	  default:
	    fprintf(Peer_fp, "new_demod, bad back_demod_strat=%d.\n", Parms[BACK_DEMOD_STRAT].val);
	    abend("new_demod, bad back_demod_strat");
	    break;
	    }
	}
    free_list(to_be_back_demodulated);
	
}  /* new_demodulator */

/*************
 *
 *     demodulate_literal()
 *
 *************/

void demodulate_literal(lit)
struct literal *lit;     
{
    struct gen_ptr *p, *q1, *q2;
    void *demods;

    CLOCK_START(DEMOD_TIME)
    if (Flags[INDEX_DEMOD].val)
	demods = (void *) Demod_index;
    else
	demods = (void *) Demodulators;
    p = NULL;
    lit->atom = demodulate_flat(lit->atom, demods, 0, &p);
    clear_demod_marks(lit->atom);
    p = reverse_gen_list(p, (struct gen_ptr *) NULL);
    for (q2=NULL, q1=lit->demod_parents; q1; q2=q1, q1=q1->next);
    if (q2)
	q2->next = p;
    else
	lit->demod_parents = p;
    CLOCK_STOP(DEMOD_TIME)
}  /* demodulate_literal */

/*************
 *
 *     paramod(from, from_atom, into, into_atom, into_term, where)
 *
 *     If an extension is being used, then from (into) is the
 *     unextended literal, and from_atom (into_atom) is the
 *     extension.  If an extension is not being used, then
 *     from->atom==from_atom (into->atom==into_atom).
 *
 *************/

void paramod(from, from_atom, into, into_atom, into_term, where)
struct literal *from;
struct term *from_atom;
struct literal *into;
struct term *into_atom;
struct term *into_term;
int where;
{
    struct rel *r;
    struct context *c_from, *c_into;
    struct bt_node *unify_position;
    struct term *alpha, *beta;
    struct literal *paramodulant;

#if 0
    printf("%d%s -> %d%s: %s.\n",
	   from->id, from->atom==from_atom ? " " : "'",
	   into->id, into->atom==into_atom ? " " : "'",
	   where == ALL ? "all" : (where == TOP_ONLY ? "top_only" : "all_but_top"));
#endif

    if (into_term->type == COMPLEX && (where == ALL || where == ALL_BUT_TOP)) {
	for (r = into_term->farg; r; r = r->narg)
	    paramod(from, from_atom, into, into_atom, r->argval, ALL);
	}

    if (into_term->type != VARIABLE && (where == ALL || where == TOP_ONLY)) {
	c_from = get_context();
	c_into = get_context();
	alpha = from_atom->farg->argval;
	beta = from_atom->farg->narg->argval;
	unify_position = unify_bt_first(alpha, c_from, into_term, c_into);
	while (unify_position) {
	    paramodulant = get_literal();
	    paramodulant->sign = into->sign;
	    paramodulant->from_parent = from->id;
	    paramodulant->into_parent = into->id;
	    paramodulant->from_parent_extended = from->atom != from_atom;
	    paramodulant->into_parent_extended = into->atom != into_atom;
	    paramodulant->atom = apply_substitute(beta, c_from, into_atom,
						  into_term, c_into);


	    Stats[EXP_GENERATED]++;
	    process_new_clause(paramodulant);

	    unify_position = unify_bt_next(unify_position);
	    }
	free_context(c_from);
	free_context(c_into);
	}
	
}  /* paramod */

/*************
 *
 *    paramodulate_pair(c, d) -- including extensions
 *
 *************/

void paramodulate_pair(c, d)
struct literal *c;
struct literal *d;
{
    struct term *c_ext_atom, *d_ext_atom;
    int from_c, from_d;

    from_c = pos_eq(c); from_d = pos_eq(d);

    if (from_c)
	paramod(c, c->atom, d, d->atom, d->atom->farg->argval, ALL);
    if (from_d && c != d) {
	if (from_c)
	    paramod(d, d->atom, c, c->atom, c->atom->farg->argval, ALL_BUT_TOP);
	else
	    paramod(d, d->atom, c, c->atom, c->atom->farg->argval, ALL);
	}

    /* Note that negative equalities are not extended. */

    c_ext_atom = from_c && alpha_is_ac(c) ? extend_atom(c) : NULL;
    d_ext_atom = from_d && alpha_is_ac(d) ? extend_atom(d) : NULL;

    if (c_ext_atom)
	paramod(c, c_ext_atom, d, d->atom, d->atom->farg->argval, ALL);
    if (d_ext_atom && c != d)
	paramod(d, d_ext_atom, c, c->atom, c->atom->farg->argval, ALL);

    /* Note that when going into extensions, go into top level only. */

    if (c_ext_atom && d_ext_atom)
	paramod(c, c_ext_atom, d, d_ext_atom, d_ext_atom->farg->argval, TOP_ONLY);

    if (c_ext_atom)
	zap_extended_atom(c_ext_atom);
    if (d_ext_atom)
	zap_extended_atom(d_ext_atom);

}  /* paramodulate_pair */

/*************
 *
 *   para_from_into(c, d, where)
 *
 *   assume where is ALL or ALL_BUT_TOP
 *
 *************/

void para_from_into(c, d, where)
struct literal *c, *d;
int where;
{
    struct term *c_ext_atom, *d_ext_atom;
    int from_c;

    from_c = pos_eq(c);

    if (from_c)
	paramod(c, c->atom, d, d->atom, d->atom->farg->argval, where);

    /* Note that negative equalities are not extended. */

    c_ext_atom = from_c && alpha_is_ac(c) ? extend_atom(c) : NULL;
    d_ext_atom = pos_eq(d) && alpha_is_ac(d) ? extend_atom(d) : NULL;

    if (c_ext_atom)
	paramod(c, c_ext_atom, d, d->atom, d->atom->farg->argval, where);

    /* Note that when going into extensions, go into top level only. */

    if (c_ext_atom && d_ext_atom && where == ALL)
	paramod(c, c_ext_atom, d, d_ext_atom, d_ext_atom->farg->argval, TOP_ONLY);

    if (c_ext_atom)
	zap_extended_atom(c_ext_atom);
    if (d_ext_atom)
	zap_extended_atom(d_ext_atom);

}  /* para_from_into */


