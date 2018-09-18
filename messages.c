#include "Header.h"
#include "Io.h"
#include "p4.h"
#include "Peers.h"

/*************
 *
 *     m4_broadcast(type, msg, size)
 *
 *************/

void m4_broadcast(type, msg, size)
int type;
char *msg;
int size;
{
    int i, my_id, num;

    my_id = p4_get_my_id();
    num = p4_num_total_ids();

    for (i = 0; i < num; i++)
	if (i != my_id)
	    p4_send(type, i, msg, size);
    
}  /* m4_broadcast */

/*************
 *
 *     m4_messages_available()
 *
 *************/

int m4_messages_available(type)
int type;
{
    int from;

    from = -1;
    return(p4_messages_available(&type, &from));
}  /* m4_messages_available */

/*************
 *
 *     options_to_string(s, size)
 *
 *************/

options_to_string(s, size)
char *s;
int size;
{
    int i, count;
    char s1[20];

    count = 0;
    s[0] = '\0';
    for (i=0; i<MAX_FLAGS; i++) {
	sprintf(s1, "%d ", Flags[i].val);
	count += strlen(s1);
	if (count >= size)
	    abend("in options_to_string, too many flags.");
	strcat(s, s1);
	}
    for (i=0; i<MAX_PARMS; i++) {
	sprintf(s1, "%d ", Parms[i].val);
	count += strlen(s1);
	if (count >= size)
	    abend("in options_to_string, too many parms.");
	strcat(s, s1);
	}
}  /* options_to_string */

/*************
 *
 *     string_to_options(s)
 *
 *************/

void string_to_options(s)
char *s;
{
    char *s1;
    int i;

    s1 = s;

    for (i=0; i<MAX_FLAGS; i++)
	Flags[i].val = next_int(&s1);

    for (i=0; i<MAX_PARMS; i++)
	Parms[i].val = next_int(&s1);
    
}  /* string_to_options */

/*************
 *
 *     more_in_string() - is there some non-space followed by a space?
 *
 *************/

int more_in_string(s)
char *s;     
{
    while (*s == ' ') s++;
    for (; *s != ' ' && *s != '\0'; s++);
    return(*s == ' ');
}  /* more_in_string */

/*************
 *
 *     next_str(sp, s2)
 *
 *     Assumes there is a string followed by a space.  Update sp.
 *
 *************/

void next_str(sp, s2)
char **sp;
char *s2;
{
    char *s;
    int i;

    s = *sp;
    while (*s == ' ') s++;
    for (i=0; *s != ' '; i++, s++)
	s2[i] = *s;
    s2[i] = '\0';
    *sp = s;
}  /* next_str */

/*************
 *
 *     next_int(sp)
 *
 *     Assumes there is an integer followed by a space.  Update sp.
 *
 *************/

int next_int(sp)
char **sp;
{
    char s1[MAX_NAME];

    next_str(sp, s1);
    return(atoi(s1));
}  /* next_int */

/*************
 *
 *     sym_array_to_normal_sym_tab()
 *
 *************/

void sym_array_to_normal_sym_tab(a, size)
struct sym_ent a[];
int size;
{
    int i, sn;
    struct sym_ent *p;

    for (i=0; i<size; i++) {
	if (a[i].sym_num > 0) {
	    sn = str_to_sn(a[i].name, a[i].arity);
	    if (sn != i)
		abend("sym_array_to_normal_sym_tab, sym_num mixup.");

	    p = sn_to_node(i);
	    if (a[i].assoc_comm)
		p->assoc_comm = 1;
	    else if (a[i].commutative)
		p->commutative = 1;
	    p->lrpo_status = a[i].lrpo_status;
	    p->lex_val = a[i].lex_val;
	    }
	}
}  /* sym_array_to_normal_sym_tab */

/*************
 *
 *    string_to_sym_array(s, a)
 *     
 *    string has a sequence of tuples:
 *    
 *         <symbol-number, arity, unif-type, name, lrpo-status, lex-val>
 *
 *    unif-type and name are strings, others are integers.
 *    There should be exactly one space after each object. 
 *
 *************/

void string_to_sym_array(s, a, a_size)
char *s;
struct sym_ent a[];
int a_size;
{
    char *s1, s2[MAX_NAME];
    int i, j, k;

    for (i = 0; i < a_size; i++)
	a[i].sym_num = -1;

    s1 = s;
    while (more_in_string(s1)) {
	k = next_int(&s1);
	if (k >= a_size)
	    abend("string_to_sym_array, array too small.");
	a[k].sym_num = k;

	a[k].arity = next_int(&s1);

	next_str(&s1, s2);
	a[k].assoc_comm = str_ident(s2, "ac");
	a[k].commutative = str_ident(s2, "comm");

	next_str(&s1, s2);
	strcpy(a[k].name, s2);

	a[k].lrpo_status = next_int(&s1);
	a[k].lex_val = next_int(&s1);
	}
#if 0
    for (j=0; j<a_size; j++)
	if (a[j].sym_num > 0)
	    printf("%d %d %s %d %d\n", a[j].sym_num, a[j].arity, a[j].name,
		   a[j].lrpo_status, a[j].lex_val);
#endif    
}  /* string_to_sym_array */

/*************
 *
 *     term_to_string_rec(t, s, size, ip)
 *
 *************/

void term_to_string_rec(t, s, size, ip)
struct term *t;
char *s;
int size;
int *ip;
{
    char s1[MAX_NAME];
    struct rel *r;

    if (t->type == VARIABLE) {
	sprintf(s1, "v%d ", t->varnum);
	*ip += strlen(s1);
	if (*ip >= size)
	    abend("term_to_string_rec, string too small.");
	strcat(s, s1);
	}
    else {
	sprintf(s1, "%d ", t->sym_num);
	*ip += strlen(s1);
	if (*ip >= size)
	    abend("term_to_string_rec, string too small.");
	strcat(s, s1);
	for (r = t->farg; r; r = r->narg)
	    term_to_string_rec(r->argval, s, size, ip);
	}
}  /* term_to_string_rec */

/*************
 *
 *     term_to_string(t, s, size)
 *
 *************/

void term_to_string(t, s, size)
struct term *t;
char *s;
int size;
{
    int i;
    
    s[0] = '\0';
    i = 0;
    term_to_string_rec(t, s, size, &i);
}  /* term_to_string */

/*************
 *
 *     string_to_term_rec()
 *
 *************/

struct term *string_to_term_rec(sp)
char **sp;
{
    struct term *t;
    struct rel *r1, *r2;
    char s1[MAX_NAME];
    int arity, j, n;

    next_str(sp, s1);
    if (s1[0] == 'v') {
	n = atoi(s1+1);
	t = get_term();
	t->type = VARIABLE;
	t->varnum = n;
	}
    else {
	n = atoi(s1);
	arity = Symbol_array[n].arity;
	t = get_term();
	t->sym_num = n;
	if (arity == 0)
	    t->type = NAME;
	else {
	    t->type = COMPLEX;
	    r2 = NULL;
	    for (j=0; j<arity; j++) {
		r1 = get_rel();
		r1->argval = string_to_term_rec(sp);
		if (r2)
		    r2->narg = r1;
		else
		    t->farg = r1;
		r2 = r1;
		}
	    }
	}
    return(t);
}  /* string_to_term_rec */

/*************
 *
 *     string_to_term()
 *
 *************/

struct term *string_to_term(s)
char *s;
{
    char *s1;

    s1 = s;
    return(string_to_term_rec(&s1));

}  /* string_to_term */

/*************
 *
 *    clause_to_string(c, s, size)
 *
 *    The string form of a clause (all integers separated by space, with
 *    a space at the end) is
 *
 *    "id name parent1 parent2 bd-parent number-of-demods <demods> weight sign <atom> "
 *
 *    This is for unit clauses only.
 *
 *************/

void clause_to_string(c, s, size)
struct literal *c;
char *s;
int size;
{
    char s1[MAX_DAC_STRING], s2[MAX_NAME];
    int i;
    struct gen_ptr *p;

    for (p=c->demod_parents, i=0; p; p=p->next, i++);    

    sprintf(s, "%d %d %d %d %d %d ",
	    c->id,
	    c->name,
	    c->from_parent,
	    c->into_parent,
	    c->bd_parent,
            i);
    
    for (p=c->demod_parents; p; p=p->next) {
	sprintf(s2, "%d ", p->u.i);
	strcat(s, s2);
	}

    sprintf(s2, "%d %d ", c->weight, c->sign);
    strcat(s, s2);

    term_to_string(c->atom, s1, MAX_DAC_STRING);
    
    if (strlen(s1) + strlen(s2) >= size)
	abend("clause_to_string, string too small");

    strcat(s, s1);
    
}  /* clause_to_string */

/*************
 *
 *     string_to_clause()
 *
 *    "id name parent1 parent2 bd-parent number-of-demods <demods> weight sign <atom> "
 *
 *************/

struct literal *string_to_clause(s)
char *s;
{
    struct literal *c;
    int n, i, j;
    struct gen_ptr *p1, *p2;

    c = get_literal();
    
    c->id = next_int(&s);
    c->name = next_int(&s);
    n = next_int(&s);
    c->from_parent = n;
    n = next_int(&s);
    c->into_parent = n;
    n = next_int(&s);
    c->bd_parent = n;
    j = next_int(&s);
    for (i=0, p2=NULL; i<j; i++) {
	n = next_int(&s);
	p1 = get_gen_ptr();
	if (p2)
	    p2->next = p1;
	else
	    c->demod_parents = p1;
	p1->u.i = n;
	p2 = p1;
	}

    c->weight = next_int(&s);
    c->sign = next_int(&s);
    c->atom = string_to_term(s);
    return(c);
}  /* string_to_clause */

