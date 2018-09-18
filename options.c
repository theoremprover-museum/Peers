#include "Header.h"

/*************
 *
 *    init_options()
 *
 *************/

void init_options()
{
    int i;
    struct flag *f;
    struct parm *p;

    for (i = 0; i < MAX_FLAGS; i++)
	Flags[i].name = "";
    for (i = 0; i < MAX_PARMS; i++)
	Parms[i].name = "";

    /* Flags are Boolean-valued options */

    f = &(Flags[PROLOG_STYLE_VARIABLES]);
    f->name = "prolog_style_variables";
    f->val = 0;

    f = &(Flags[CHECK_ARITY]);
    f->name = "check_arity";
    f->val = 1;

    f = &(Flags[DISPLAY_TERMS]);
    f->name = "display_terms";
    f->val = 0;

    f = &(Flags[PRINT_GEN]);
    f->name = "print_gen";
    f->val = 0;

    f = &(Flags[DEMOD_HISTORY]);
    f->name = "demod_history";
    f->val = 1;

    f = &(Flags[INDEX_DEMOD]);
    f->name = "index_demod";
    f->val = 1;

    f = &(Flags[INDEX_AC_ARGS]);
    f->name = "index_ac_args";
    f->val = 1;

    f = &(Flags[PRINT_GIVEN]);
    f->name = "print_given";
    f->val = 1;

    f = &(Flags[ALPHA_PAIR_WEIGHT]);
    f->name = "alpha_pair_weight";
    f->val = 0;

    f = &(Flags[CHECK_INPUT_DEMODULATORS]);
    f->name = "check_input_demodulators";
    f->val = 1;

    f = &(Flags[INPUT_CONFLICT_CHECK]);
    f->name = "input_conflict_check";
    f->val = 1;

    f = &(Flags[LRPO]);
    f->name = "lrpo";
    f->val = 0;

    f = &(Flags[PARA_PAIRS]);
    f->name = "para_pairs";
    f->val = 0;

    f = &(Flags[PRINT_FORWARD_SUBSUMED]);
    f->name = "print_forward_subsumed";
    f->val = 0;

    f = &(Flags[PRINT_BACK_DEMOD]);
    f->name = "print_back_demod";
    f->val = 1;

    f = &(Flags[PRINT_LISTS_AT_END]);
    f->name = "print_lists_at_end";
    f->val = 0;

    f = &(Flags[OWN_IN_USABLE]);
    f->name = "own_in_usable";
    f->val = 0;

    f = &(Flags[OWN_IN_SOS]);
    f->name = "own_in_sos";
    f->val = 0;

    f = &(Flags[POST_PROCESS_NS]);
    f->name = "post_process_ns";
    f->val = 0;
    
    f = &(Flags[TWO_WAY_DEMOD]);
    f->name = "two_way_demod";
    f->val = 0;
    
    f = &(Flags[NAME_STRATEGIES]);
    f->name = "name_strategies";
    f->val = 1;
    
    /* Parms are integer-valued options */

    p = &(Parms[MAX_MEM]);
    p->name = "max_mem";
    p->val = 0;
    p->min = 0;
    p->max = MAX_INT;

    p = &(Parms[MAX_WEIGHT]);
    p->name = "max_weight";
    p->val = MAX_INT;
    p->min = -MAX_INT;
    p->max = MAX_INT;

    p = &(Parms[MAX_GIVEN]);
    p->name = "max_given";
    p->val = MAX_INT;
    p->min = 0;
    p->max = MAX_INT;

    p = &(Parms[WEIGHT_FUNCTION]);
    p->name = "weight_function";
    p->val = 0;
    p->min = 0;
    p->max = MAX_INT;

    p = &(Parms[MAX_PAIR_WEIGHT]);
    p->name = "max_pair_weight";
    p->val = MAX_INT;
    p->min = 0;
    p->max = MAX_INT;

    p = &(Parms[MAX_PROOFS]);
    p->name = "max_proofs";
    p->val = 1;
    p->min = 1;
    p->max = MAX_INT;

    p = &(Parms[REPORT_GIVEN]);
    p->name = "report_given";
    p->val = MAX_INT;
    p->min = 1;
    p->max = MAX_INT;

    p = &(Parms[AC_SUPERSET_LIMIT]);
    p->name = "ac_superset_limit";
    p->val = -1;
    p->min = -1;
    p->max = MAX_INT;

    p = &(Parms[MAX_SECONDS]);
    p->name = "max_seconds";
    p->val = MAX_INT;
    p->min = 0;
    p->max = MAX_INT;

    p = &(Parms[BACK_DEMOD_STRAT]);
    p->name = "back_demod_strat";
    p->val = 4;
    p->min = 1;
    p->max = 4;

    p = &(Parms[DECIDE_OWNER_STRAT]);
    p->name = "decide_owner_strat";
    p->val = 1;
    p->min = 1;
    p->max = 3;

    p = &(Parms[MAX_BD_INF_MSG]);
    p->name = "max_bd_inf_msg";
    p->val = MAX_INT;
    p->min = 0;
    p->max = MAX_INT;

}  /* init_options */

/*************
 *
 *    print_options(fp)
 *
 *************/

void print_options(fp)
FILE *fp;
{
    int i, j;

    fprintf(fp, "\n--------------- options ---------------\n");

    j = 0;
    for (i = 0; i < MAX_FLAGS; i++)  /* print set flags */
	if (Flags[i].name[0] != '\0') {
            fprintf(fp, "%s", Flags[i].val ? "set(" : "clear(");
	    fflush(stdout);
	    fprintf(fp, "%s). ", Flags[i].name);
	    fflush(stdout);
	    j++;
	    if (j % 3 == 0)
	        fprintf(fp, "\n");
	    fflush(stdout);
            }

    fprintf(fp, "\n\n");

    j = 0;
    for (i = 0; i < MAX_PARMS; i++)  /* print parms */
	if (Parms[i].name[0] != '\0') {
	    fprintf(fp, "assign(");
	    fprintf(fp, "%s, %d). ", Parms[i].name, Parms[i].val);
	    j++;
	    if (j % 3 == 0)
		fprintf(fp, "\n");
	    }
    fprintf(fp, "\n");

}  /* print_options */

/*************
 *
 *    p_options()
 *
 *************/

void p_options()
{
    print_options(stdout);
}  /* p_options */

/*************
 *
 *    int change_flag(fp, term, set)
 *
 *    Assume term is COMPLEX, with either `set' or `clear' as functor.
 *
 *    Warning and error messages go to file fp.
 *
 *************/

int change_flag(fp, t, set)
FILE *fp;
struct term *t;
int set;
{
    char *flag_name;
    int i, found;

    if (t->farg == NULL || t->farg->narg != NULL ||
		           t->farg->argval->type == COMPLEX) {
	fprintf(fp, "ERROR: ");
	print_term(fp, t);
	fprintf(fp, " must have one simple argument.\n");
	Stats[INPUT_ERRORS]++;
	return(0);
	}
    else {
	flag_name = sn_to_str(t->farg->argval->sym_num);
	found = 0;
	i = 0;
	while (i < MAX_FLAGS && found == 0)
	    if (str_ident(flag_name, Flags[i].name))
		found = 1;
	    else
		i++;
	if (found == 0) {
	    fprintf(fp, "ERROR: ");
	    print_term(fp, t);
	    fprintf(fp, " flag name not found.\n");
	    Stats[INPUT_ERRORS]++;
	    return(0);
	    }
	else if (Flags[i].val == set) {
	    fprintf(fp, "WARNING: ");
	    print_term(fp, t);
	    if (set)
		fprintf(fp, " flag already set.\n");
	    else
		fprintf(fp, " flag already clear.\n");
	    return(1);
	    }
	else {
	    Flags[i].val = set;
	    return(1);
	    }
	}
}  /* change_flag */

/*************
 *
 *    int change_parm(fp, term, set)
 *
 *    Assume term is COMPLEX, with either `assign' as functor.
 *
 *    Warning and error messages go to file fp.
 *
 *************/

int change_parm(fp, t)
FILE *fp;
struct term *t;
{
    char *parm_name, *int_name;
    int i, found, new_val, rc;

    if (t->farg == NULL || t->farg->narg == NULL ||
		           t->farg->narg->narg != NULL ||
		           t->farg->argval->type == COMPLEX ||
		           t->farg->narg->argval->type == COMPLEX) {
	fprintf(fp, "ERROR: ");
	print_term(fp, t);
	fprintf(fp, " must have two simple arguments.\n");
	Stats[INPUT_ERRORS]++;
	return(0);
	}
    else {
	parm_name = sn_to_str(t->farg->argval->sym_num);
	found = 0;
	i = 0;
	while (i < MAX_PARMS && found == 0)
	    if (str_ident(parm_name, Parms[i].name))
		found = 1;
	    else
		i++;
	if (found == 0) {
	    fprintf(fp, "ERROR: ");
	    print_term(fp, t);
	    fprintf(fp, " parm name not found.\n");
	    Stats[INPUT_ERRORS]++;
	    return(0);
	    }
	else {
	    int_name = sn_to_str(t->farg->narg->argval->sym_num);
	    rc = str_int(int_name, &new_val);
	    if (rc == 0) {
		fprintf(fp, "ERROR: ");
		print_term(fp, t);
		fprintf(fp, " second argument must be integer.\n");
		Stats[INPUT_ERRORS]++;
		return(0);
		}
	    else if (new_val < Parms[i].min || new_val > Parms[i].max) {
		fprintf(fp, "ERROR: ");
		print_term(fp, t);
		fprintf(fp, " integer must be in range [%d,%d].\n",
				Parms[i].min, Parms[i].max);
		Stats[INPUT_ERRORS]++;
		return(0);
		}
	    else if (new_val == Parms[i].val) {
		fprintf(fp, "WARNING: ");
		print_term(fp, t);
		fprintf(fp, " already has that value.\n");
		return(1);
		}
	    else {
		Parms[i].val = new_val;
		return(1);
		}
	    }
	}
}  /* change_parm */

/*************
 *
 *    check_options()  --  check for inconsistent or strange settings
 *
 *    If a bad combination of settings is found, either a warning
 *    message is printed, or an ABEND occurs.
 *
 *************/

void check_options()
{
}  /* check_options */

