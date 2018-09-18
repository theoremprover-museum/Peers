#include "Header.h"
#include "Io.h"

static struct symbol_table *Sym_tab;  /* pointer to symbol table */
static int Sym_ent_count;

/*************
 *
 *    init_symbol_table()
 *
 *************/

void init_symbol_table()
{
    int i;

    Sym_tab = (struct symbol_table *) tp_alloc(sizeof(struct symbol_table));
    for (i = 0; i < SYM_TAB_SIZE; i++)
	Sym_tab->table[i] = NULL;
    Sym_ent_count = 0;
}  /* init_symbol_table */

/*************
 *
 *    int str_int(string, int_ptr) -- Translate a string to an integer.
 *
 *        String has optional '+' or '-' as first character.
 *    Return(1) iff success.
 *
 *************/

int str_int(s, np)
char s[];
int *np;
{
    int i, sign, n;

    i = 0;
    sign = 1;
    if (s[0] == '+' || s[0] == '-') {
	if (s[0] == '-')
	    sign = -1;
	i = 1;
	}
    if (s[i] == '\0')
	return(0);
    else {
	n = 0;
	for( ; s[i] >= '0' && s[i] <= '9'; i++)
	    n = n * 10 + s[i] - '0';
	*np = n * sign;
	return(s[i] == '\0');
	}
}  /* str_int */

/*************
 *
 *    int_str(int, str) -- translate an integer to a string
 *
 *************/

void int_str(i, s)
int i;
char s[];
{
    int j, sign;

    if ((sign = i) < 0)
	i = -i;
    
    j = 0;
    if (i == 0)
        s[j++] = '0';
    else {
	while (i > 0) {
            s[j++] = i % 10 + '0';
            i = i / 10;
	    }
        }
    if (sign < 0)
	s[j++] = '-';
    s[j] = '\0';
    reverse(s);
}  /* int_str */

/*************
 *
 *    cat_str(s1, s2, s3)
 *
 *************/

void cat_str(s1,s2,s3)
char *s1;
char *s2;
char *s3;
{
    int i, j;

    for (i = 0; s1[i] != '\0'; i++)
        s3[i] = s1[i];
    for (j = 0; s2[j] != '\0'; j++, i++)
	s3[i] = s2[j];
    s3[i] = '\0';
}  /* cat_str */

/*************
 *
 *    str_copy(s, t)  --  Copy a string.
 *
 *************/

void str_copy(s, t)
char *s;
char *t;
{
    while (*t++ = *s++);
}  /* str_copy */

/*************
 *
 *     int str_ident(s, t) --  Identity of strings
 *
 *************/

int str_ident(s, t)
char *s;
char *t;
{
    for ( ; *s == *t; s++, t++)
	if (*s == '\0') return(1);
    return(0);
}  /* str_ident */

/*************
 *
 *    reverse(s) -- reverse a string
 *
 *************/

void reverse(s)
char s[];
{
    int i, j;
    char temp;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
	temp = s[i];
	s[i] = s[j];
	s[j] = temp;
	}
}  /* reverse */

/*************
 *
 *    int str_long(string, long_ptr) -- Translate a string to a long.
 *
 *        String has optional '+' or '-' as first character.
 *    Return(1) iff success.
 *
 *************/

int str_long(s, np)
char s[];
long *np;
{
    int i, sign;
    long n;

    i = 0;
    sign = 1;
    if (s[0] == '+' || s[0] == '-') {
	if (s[0] == '-')
	    sign = -1;
	i = 1;
	}
    if (s[i] == '\0')
	return(0);
    else {
	n = 0;
	for( ; s[i] >= '0' && s[i] <= '9'; i++)
	    n = n * 10 + s[i] - '0';
	*np = n * sign;
	return(s[i] == '\0');
	}
}  /* str_long */

/*************
 *
 *    long_str(int, str) -- translate a long to a string
 *
 *************/

void long_str(i, s)
long i;
char s[];
{
    int j;
    long sign;

    if ((sign = i) < 0)
	i = -i;
    
    j = 0;
    if (i == 0)
	s[j++] = '0';
    else {
	while (i > 0) {
	    s[j++] = i % 10 + '0';
	    i = i / 10;
	    }
	}
    if (sign < 0)
	s[j++] = '-';
    s[j] = '\0';
    reverse(s);
}  /* long_str */

/*************
 *
 *   int new_sym_num()
 *
 *   Return the next available symbol number.
 *
 *   The rightmost 8 bits will not be all zero.
 *   This is so that fpa will not map sym_nums to 0 (the
 *   code for variables).
 *
 *************/

int new_sym_num()
{
    Sym_ent_count++;

    if (Sym_ent_count % 256 == 0)
        Sym_ent_count++;

    return(Sym_ent_count);

}  /* new_sym_num */

/*************
 *
 *    struct sym_ent *insert_sym(string, arity)
 *
 *    Insert string/arity into the symbol table and return the symbol
 *    table node.  Do not check if string/arity is already there.
 *
 *************/

struct sym_ent *insert_sym(s, arity)
char *s;
int arity;
{
    struct sym_ent *p;
    int i;

    p = get_sym_ent();
    str_copy(s, p->name);
    p->arity = arity;
    p->sym_num = new_sym_num();
    i = p->sym_num % SYM_TAB_SIZE;
    p->next = Sym_tab->table[i];
    Sym_tab->table[i] = p;
    return(p);
}  /* insert_sym */

/*************
 *
 *    int str_to_sn(str, arity) -- Return a symbol number for string/arity.
 *
 *        If the given string/arity is already in the global symbol table,
 *    then return symbol number; else, create a new symbol table entry and
 *    return a new symbol number
 *
 *************/

int str_to_sn(str, arity)
char *str;
int arity;
{
    struct sym_ent *p, *save;
    int i;
    
    save = NULL;
    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p = Sym_tab->table[i];
	while (p != NULL) {
	    if (!str_ident(str, p->name))
	        p = p->next;
	    else if (p->arity != arity) {
		save = p;
		p = p->next;
		}
	    else
		return(p->sym_num);
	    }
	}

    if (save && !save->special_op &&
	Flags[CHECK_ARITY].val && !str_ident(str, "$Quantified")) {
	
	fprintf(stderr, "\007\n\nWARNING, multiple arity: %s/%d, %s/%d.\n\n",
		save->name, save->arity, str, arity);
	}

    /* String/arity not in table, so create an entry. */

    p = insert_sym(str, arity);

    return(p->sym_num);
	
}  /* str_to_sn */

/*************
 *
 *    print_syms(file_ptr) -- Display the symbol list.
 *
 *************/

void print_syms(fp)
 FILE *fp;
{
    struct sym_ent *p;
    int i;
    
    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p = Sym_tab->table[i];
	while (p != NULL) {
	    fprintf(fp, "%d  %s/%d, lex_val=%d, lrpo_status=%d\n",
		    p->sym_num, p->name, p->arity, p->lex_val, p->lrpo_status);
	    p = p->next;
	    }
	}
}  /* print_syms */

/*************
 *
 *    p_syms()
 *
 *************/

void p_syms()
{
    print_syms(stdout);
}  /* p_syms */

/*************
 *
 *    char *sn_to_str(sym_num)  --  given a symbol number, return the name
 *
 *************/

char *sn_to_str(sym_num)
int sym_num;
{
    struct sym_ent *p;
    
    p = Sym_tab->table[sym_num % SYM_TAB_SIZE];
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    if (p == NULL)
	return("");
    else
	return(p->name);
}  /* sn_to_str */

/*************
 *
 *    int sn_to_arity(sym_num)  --  given a symbol number, return the arity
 *
 *************/

int sn_to_arity(sym_num)
int sym_num;
{
    struct sym_ent *p;
    
    p = Sym_tab->table[sym_num % SYM_TAB_SIZE];
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    if (p == NULL)
	return(-1);
    else
	return(p->arity);
}  /* sn_to_arity */

/*************
 *
 *    int sn_to_node(sym_num)
 *
 *    Given a symbol number, return the symbol table node.
 *
 *************/

struct sym_ent *sn_to_node(sym_num)
int sym_num;
{
    struct sym_ent *p;
    
    p = Sym_tab->table[sym_num % SYM_TAB_SIZE];
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    return(p);  /* possibly NULL */
}  /* sn_to_node */

/*************
 *
 *    int in_sym_tab(s)  --  is s in the symbol table?
 *
 *************/

int in_sym_tab(s)
char *s;
{
    struct sym_ent *p;
    int i;
    
    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p = Sym_tab->table[i];
	while (p != NULL) {
	    if (str_ident(p->name, s))
		return(1);
	    p = p->next;
	    }
	}
    return(0);
}  /* in_sym_tab */

/*************
 *
 *    free_sym_tab() -- free all symbols in the symbol table
 *
 *************/

void free_sym_tab()
{
    struct sym_ent *p1, *p2;
    int i;

    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p1 = Sym_tab->table[i];
	while (p1 != NULL) {
	    p2 = p1;
	    p1 = p1->next;
	    free_sym_ent(p2);
	    }
	Sym_tab->table[i] = NULL;
	}
}  /* free_sym_tab */

/*************
 *
 *    int is_symbol(t, str, arity)
 *    
 *    Does t have leading function symbol str with arity?
 *
 *************/

int is_symbol(t, str, arity)
struct term *t;
char *str;
int arity;
{
    return((t->type == COMPLEX || t->type == NAME) &&
	   sn_to_arity(t->sym_num) == arity &&
	   str_ident(sn_to_str(t->sym_num), str));
}  /* is_symbol */

/*************
 *
 *     int initial_str(s, t)  --  Is s an initial substring of t?
 *
 *************/

int initial_str(s, t)
char *s;
char *t;
{
    for ( ; *s == *t; s++, t++)
	if (*s == '\0') return(1);
    return(*s == '\0');
}  /* initial_str */
    
/*************
 *
 *    int set_vars(term)
 *
 *    Decide which of the names are really variables, and make
 *    into variables.  (This routine is used only on input terms.)
 *    Preserve the user's variable names by keeping the pointer into
 *    the symbol list.
 *
 *    If too many variables, return(0); else return(1).
 *
 *************/

int set_vars(t)
struct term *t;
{
    char *varnames[MAX_VARS];
    int i;
    
    for (i = 0; i < MAX_VARS; i++)
	varnames[i] = NULL;
    
    return(set_vars_term(t, varnames));
}  /* set_vars */

/*************
 *
 *     int set_vars_term(term, varnames)
 *
 *************/

int set_vars_term(t, varnames)
struct term *t;
char *varnames[];
{
    struct rel *r;
    int i, rc;
    
    if (t->type == COMPLEX) {
	r = t->farg;
	rc = 1;
	while (rc && r != NULL) {
	    rc = set_vars_term(r->argval, varnames);
	    r = r->narg;
	    }
	return(rc);
	}
    else if (var_name(sn_to_str(t->sym_num)) == 0)
	return(1);
    else {
	i = 0;
	t->type = VARIABLE;
	while (i < MAX_VARS && varnames[i] != NULL &&
	       varnames[i] != sn_to_str(t->sym_num))
	    i++;
	if (i == MAX_VARS)
	    return(0);
	else {
	    if (varnames[i] == NULL)
		varnames[i] = sn_to_str(t->sym_num);
	    t->varnum = i;
#if 1
	    t->sym_num = 0;  /* include this to destroy input variable names */
#endif
	    return(1);
	    }
	}
}  /* set_vars_term */

/*************
 *
 *    int var_name(string) -- Decide if a string represents a variable.
 *
 *        return("string is a variable")
 *
 *************/

int var_name(s)
char *s;
{
    if (Flags[PROLOG_STYLE_VARIABLES].val)
        return((*s >= 'A' && *s <= 'Z') || *s == '_');
    else
        return(*s >= 'u' && *s <= 'z');
}  /* var_name */

/*************
 *
 *    read_list(file_ptr, errors_ptr)
 *
 *    Read and return a list of terms.
 *
 *    The list must be terminated either with the term `end_of_list.'
 *    or with an actual EOF.
 *    Set errors_ptr to point to the number of errors found.
 *
 *************/

struct gen_ptr *read_list(fp, ep)
FILE *fp;
int *ep;
{
    struct gen_ptr *p1, *p2, *p3;
    struct term *t;
    int rc;

    *ep = 0;
    p3 = NULL;
    p2 = NULL;
    t = read_term(fp, &rc);
    while (rc == 0) {
	(*ep)++;
	t = read_term(fp, &rc);
	}

    /* keep going until t == NULL || t is end marker */

    while (t && (t->type != NAME ||
		 !str_ident(sn_to_str(t->sym_num), "end_of_list"))) {
	set_vars(t);
	p1 = get_gen_ptr();
	p1->u.t = t;
	if (p2 == NULL)
	    p3 = p1;
	else
	    p2->next = p1;
	p2 = p1;
	t = read_term(fp, &rc);
	while (rc == 0) {
	    (*ep)++;
	    t = read_term(fp, &rc);
	    }
	}
    if (t == NULL)
	return(p3);
    else {
	zap_term(t);
	return(p3);
	}
}  /* read_list */

/*************
 *
 *    print_gen_list(file_ptr, gen_ptr) -- Print a list of terms.
 *
 *        The list is printed with periods after each term, and
 *    the list is terminated with `end_of_list.' so that it can
 *    be read with read_list.
 *
 *************/

void print_gen_list(fp, p)
FILE *fp;
struct gen_ptr *p;
{
    while (p != NULL) {
	print_term(fp, p->u.t); fprintf(fp, ".\n");
	p = p->next;
	}
    fprintf(fp, "end_of_list.\n");
}  /* print_gen_list */


/****************************************

write_term outputs a term in readable format (w.r.t. infix, prefix,
and postfix operators) and without extra parentheses.  It it much
complicated by one feature: deciding where to omit space around
the special operators.  For example, just as we can input a+b+c
instead of a + b + c, we wish to output without spaces were possible.
(I'm sorry the code is so complicated---I couldn't see a simpler way
of doing it.)

There are 2 types of constant/operator/functor:

    NAME_SYM: string of alphanumerics, $, and _.  Also quoted string.
    SYM_SYM:  string of *+-/\^<>=`~:?@&!;# and sometimes | (if not in list)

For completeness, the other characters are
    ,()[]{} and sometimes | (if in list)   puctuation for building terms
    .                                      end of input term
    %                                      start of comment
    "'                                     for quoted strings

For this problem, tokens are of 4 types:
    NAME_SYM
    SYM_SYM
    OPEN_PAREN  '('
    OTHER_PUNC   other punctuation (including space)

Special ops that are NAME_SYMs are always surrounded by spaces.

Here are the space rules for SYM_SYM special ops:

    infix
        omit space before if preceding token is NAME_SYM or OTHER_PUNC
        omit space after if next token is is NAME_SYM or OTHER_PUNC
              (note that space is included if next is '(')

    prefix
        omit space before if preceding token is OTHER_PUNC
        omit space after if next token is is NAME_SYM or OTHER_PUNC

    postfix
        omit space before if preceding token is NAME_SYM or OTHER_PUNC
        always include space after (could omit if next token is OTHER_PUNC,
            but current mechanism won't handle that, and it's not
            that important)          

*****************************************/

/* Token types */

#define OPEN_PAREN  1
#define OTHER_PUNC  2
#define NAME_SYM    6
#define SYM_SYM     7

/*************
 *
 *    int next_token_type(t, n)
 *
 *    Find the next token type that would be output for t.
 *    n is precedence parameter as in write term.
 *
 *************/

static int next_token_type(t, n)
struct term *t;
int n;
{
    struct sym_ent *s;
    int na1;
    char *str;
    
    str = sn_to_str(t->sym_num);
    if (t->type == NAME) {
        if (str_ident(str, "$nil"))
	    return(OTHER_PUNC);
	else
	    return(name_sym(str) ? NAME_SYM : SYM_SYM);
	}
    else if (t->type == VARIABLE) {
        if (t->sym_num == 0)
            return(NAME_SYM);
        else
            return(name_sym(str) ? NAME_SYM : SYM_SYM);
	}
    else {  /* complex */
	if (t->sym_num == str_to_sn("$cons", 2))
	    return(OTHER_PUNC);
	else if (str_ident(sn_to_str(t->sym_num), "$Quantified")) {
	    /* parens if parent is special op */
	    if (n < 1000)
		return(OPEN_PAREN);
	    else
		return(next_token_type(t->farg->argval, 0));
	    }
	else {
	    s = sn_to_node(t->sym_num);
	    if (s->special_op && s->arity == 2) {
		na1 = s->op_prec;
		if (s->op_type == XFX || s->op_type == XFY)
		    na1--;
		if (s->op_prec > n)
		    return(OPEN_PAREN);
		else
		    return(next_token_type(t->farg->argval, na1));
		}
	    else if (s->special_op && s->arity == 1) {
		na1 = s->op_prec;
		if (s->op_type == FX || s->op_type == XF)
		    na1--;

		if (s->op_prec > n)
		    return(OPEN_PAREN);
		if (s->op_type == FX || s->op_type == FY)
		    return(name_sym(str) ? NAME_SYM : SYM_SYM);
		else
		    return(next_token_type(t->farg->argval, na1));
		}
	    else
		return(name_sym(str) ? NAME_SYM : SYM_SYM);
	    }
	}
}  /* next_token_type */

/*************
 *
 *    write_term(file_ptr, term, n, prev) -- print in readable form.
 *
 *************/

void write_term(fp, t, n, prev)
FILE *fp;
struct term *t;
int n;
int *prev;
{
    struct rel *r;
    struct term *t1;
    struct sym_ent *s;
    int na1, na2, next;
    char *str, *s1, *s2;
    
    if (t == NULL) {
	fprintf(fp, "<write_term gets NULL pointer>");
	return;
	}
    
    if (t->type == NAME) {
	str = sn_to_str(t->sym_num);
	if (str_ident(str, "$nil"))
	    { fprintf(fp, "[]"); *prev = OTHER_PUNC;}
	else {
	    fprintf(fp, "%s", str);
	    *prev = (name_sym(str) ? NAME_SYM : SYM_SYM);
	    }
	}

    else if (t->type == VARIABLE) {
	print_variable(fp, t);
	if (t->sym_num == 0)
	    *prev = NAME_SYM;
	else
	    *prev = (name_sym(sn_to_str(t->sym_num)) ? NAME_SYM : SYM_SYM);
	}

    else {  /* complex */
	str = sn_to_str(t->sym_num);
	
	if (str_ident(str, "$Quantified")) {  /* Quantified Formula */
	    /* parens if parent is special op */
	    if (n < 1000) {
		fprintf(fp, "("); *prev = OPEN_PAREN;
		}
	    for (r = t->farg; r; r = r->narg) {
		/* parens if special op in child */
		write_term(fp, r->argval, 0, prev);
		if (r->narg) {
		    fprintf(fp, " "); *prev = OTHER_PUNC;
		    }
		}
	    if (n < 1000) {
		fprintf(fp, ")"); *prev = OTHER_PUNC;
		}
	    }   /* end Formula */

	else if (t->sym_num == str_to_sn("$cons", 2)) {   /* list */
	    fprintf(fp, "["); *prev = OTHER_PUNC;
	    write_term(fp, t->farg->argval, 1000, prev);
	    t1 = t->farg->narg->argval;
	    while (t1->sym_num == str_to_sn("$cons", 2)) {
		fprintf(fp, ","); *prev = OTHER_PUNC;
		write_term(fp, t1->farg->argval, 1000, prev);
		t1 = t1->farg->narg->argval;
		}
	    if (t1->sym_num == str_to_sn("$nil", 0)) {
		fprintf(fp, "]"); *prev = OTHER_PUNC;
		}
	    else {
		fprintf(fp, "|"); *prev = OTHER_PUNC;
		write_term(fp, t1, 1000, prev);
		fprintf(fp, "]"); *prev = OTHER_PUNC;
		}
	    }   /* end list */

	else {
	    s = sn_to_node(t->sym_num);

	    if (s->special_op && s->arity == 2) {  /* infix */
		na1 = na2 = s->op_prec;
		if (s->op_type == XFX || s->op_type == XFY)
		    na1--;
		if (s->op_type == XFX || s->op_type == YFX)
		    na2--;
		if (s->op_prec > n) {
		    fprintf(fp, "("); *prev = OPEN_PAREN;
		    }
		write_term(fp, t->farg->argval, na1, prev);

		/* Decide on spaces around infix op. */

		if (name_sym(str))
		    s1 = s2 = " ";
		else {
		    if (*prev == OTHER_PUNC || *prev == NAME_SYM)
			s1 = "";
		    else
			s1 = " ";
		    next = next_token_type(t->farg->narg->argval, na2);
		    if (next == OTHER_PUNC || next == NAME_SYM)
			s2 = "";
		    else
			s2 = " ";
		    }
		    
		fprintf(fp, "%s%s%s", s1,str,s2);
		if (str_ident(s2, " "))
		    *prev = OTHER_PUNC;
		else
		    *prev = (name_sym(str) ? NAME_SYM : SYM_SYM);
		write_term(fp, t->farg->narg->argval, na2, prev);
		if (s->op_prec > n) {
		    fprintf(fp, ")"); *prev = OTHER_PUNC;
		    }
		}

	    else if (s->special_op && s->arity == 1) {  /* prefix,postfix */
		na1 = s->op_prec;
		if (s->op_type == FX || s->op_type == XF)
		    na1--;

		if (s->op_prec > n) {
		    fprintf(fp, "("); *prev = OPEN_PAREN;
		    }

		if (s->op_type == FX || s->op_type == FY) {
		
		    /* Decide on spaces around special prefix op. */

		    if (name_sym(str))
			s1 = s2 = " ";
		    else {
			if (*prev == OTHER_PUNC || *prev == OPEN_PAREN)
			    s1 = "";
			else
			    s1 = " ";
			next = next_token_type(t->farg->argval, na1);
			if (next == OTHER_PUNC || next == OPEN_PAREN || next == NAME_SYM)
			    s2 = "";
			else
			    s2 = " ";
			}
		    
		    fprintf(fp, "%s%s%s", s1,str,s2);
		    if (str_ident(s2, " "))
			*prev = OTHER_PUNC;
		    else
			*prev = (name_sym(str) ? NAME_SYM : SYM_SYM);
		    write_term(fp, t->farg->argval, na1, prev);
		    }
		else {
		    write_term(fp, t->farg->argval, na1, prev);

		    /* Decide on spaces around special postfix op. */

		    if (name_sym(str))
			s1 = s2 = " ";
		    else {
			if (*prev == OTHER_PUNC || *prev == NAME_SYM)
			    s1 = "";
			else
			    s1 = " ";
			/* Can't easily tell next token, so just output space. */
			s2 = " ";
			}
		    
		    fprintf(fp, "%s%s%s", s1,str,s2);
		    *prev = OTHER_PUNC;
		    }

		if (s->op_prec > n) {
		    fprintf(fp, ")"); *prev = OTHER_PUNC;
		    }
		}

	    else {  /* functor(args) */
		fprintf(fp, "%s", str);
		fprintf(fp, "("); *prev = OPEN_PAREN;
		r = t->farg;
		while(r != NULL) {
		    write_term(fp, r->argval, 1000, prev);
		    r = r->narg;
		    if(r != NULL) {
			fprintf(fp, ","); *prev = OTHER_PUNC;
			}
		    }
		fprintf(fp, ")"); *prev = OTHER_PUNC;
		}
	    }
	}
}  /* write_term */

/*************
 *
 *    display_term(file_ptr, term)  --  Display a term in internal form.
 *
 *************/

void display_term(fp,t)
FILE *fp;
struct term *t;
{
    struct rel *r;
    
    if (t == NULL)
	fprintf(fp, "<display_term gets NULL pointer>");
    else if (t->type == NAME) {
	fprintf(fp, "%s", sn_to_str(t->sym_num));
	}
    else if (t->type == VARIABLE)
	print_variable(fp, t);
    else {  /* complex */
	fprintf(fp, "%s", sn_to_str(t->sym_num));
	fprintf(fp, "(");
	r = t->farg;
	while(r != NULL) {
	    display_term(fp, r->argval);
	    r = r->narg;
	    if(r != NULL)
		fprintf(fp, ",");
	    }
	fprintf(fp, ")");
	}
#if 1
    if (t && t->bits)
	fprintf(fp,"'");
#endif    
}  /* display_term */

/*************
 *
 *    print_term(file_ptr, term)  --  Print a term to a file.
 *
 *    Flag determines write_term vs. display_term.
 *
 *************/

void print_term(fp,t)
FILE *fp;
struct term *t;
{
    int i;

    if (Flags[DISPLAY_TERMS].val)
	display_term(fp, t);
    else {
	i = OTHER_PUNC;  /* Assume previous token is punctuation. */
	write_term(fp, t, 1000, &i);
	}
}  /* print_term */

/*************
 *
 *    p_term(term)  --  print_term to the standard output.
 *
 *************/

void p_term(t)
struct term *t;
{
    print_term(stdout, t);
    printf("\n");
}  /* p_term */

/*************
 *
 *    d_term(term)  --  print_term to the standard output.
 *
 *************/

void d_term(t)
struct term *t;
{
    display_term(stdout, t);
}  /* p_term */

/*************
 *
 *    print_term_nl(fp, term)  --  print_term followed by newline
 *
 *************/

void print_term_nl(fp, t)
FILE *fp;
struct term *t;
{
    print_term(fp, t);
    fprintf(fp,"\n");
}  /* print_term_nl */

/*************
 *
 *   print_variable(fp, variable)
 *
 *************/

void print_variable(fp, t)
FILE *fp;
struct term *t;
{
    int i;

    if (t->sym_num != 0)
	fprintf(fp, "%s", sn_to_str(t->sym_num));
    else if (Flags[PROLOG_STYLE_VARIABLES].val) {
	fprintf(fp, "%c", (t->varnum % 26) + 'A');
	i = t->varnum / 26;
	if (i > 0)
	    fprintf(fp, "%d", i);
	}
    else {
	if (t->varnum <= 2)
	    fprintf(fp, "%c", 'x'+t->varnum);
	else if (t->varnum <= 5)
	    fprintf(fp, "%c", 'r'+t->varnum);
	else
	    fprintf(fp, "%c%d", 'v', t->varnum);
	}
}  /* print_variable */

/*************
 *
 *    int declare_op(prec, type, str)
 *
 *************/

int declare_op(prec, type, str)
int prec;
int type;
char *str;
{
    int arity, sn, save_flag;
    struct sym_ent *p;

    if (prec < 1 || prec > 999)
	return(0);

    switch (type) {
      case FX:
      case FY:
      case XF:
      case YF: arity = 1; break;
      case XFX:
      case XFY:
      case YFX: arity = 2; break;
      default: return(0);
	}

    save_flag = Flags[CHECK_ARITY].val;
    Flags[CHECK_ARITY].val = 0;
    sn = str_to_sn(str, arity);
    Flags[CHECK_ARITY].val = save_flag;

    p = sn_to_node(sn);

    /* Don't check if it's already special.  Allow it to change. */

    p->special_op = 1;
    p->op_type = type;
    p->op_prec = prec;
    return(1);
   
}  /* declare_op */

/*************
 *
 *    init_special_ops()
 *
 *    Declare the built-in special operators.
 *
 *************/

void init_special_ops()
{
    int rc;

    rc = declare_op(800,  XFX, "->");
    rc = declare_op(800,  XFX, "<->");
    rc = declare_op(790,  XFY, "|");
    rc = declare_op(780,  XFY, "&");

    rc = declare_op(700,  XFX, "=");
    rc = declare_op(700,  XFX, "!=");
    rc = declare_op(700,  XFX, "-->");

    rc = declare_op(700,  XFX, "<");
    rc = declare_op(700,  XFX, ">");
    rc = declare_op(700,  XFX, "<=");
    rc = declare_op(700,  XFX, ">=");
    rc = declare_op(700,  XFX, "==");

    rc = declare_op(700,  XFX, "@<");
    rc = declare_op(700,  XFX, "@>");
    rc = declare_op(700,  XFX, "@<=");
    rc = declare_op(700,  XFX, "@>=");

    rc = declare_op(500,  YFX, "+");
    rc = declare_op(500,  YFX, "-");

    rc = declare_op(500,   FX, "+");
    rc = declare_op(500,   FX, "-");

    rc = declare_op(400,  YFX, "*");
    rc = declare_op(400,  YFX, "/");

    rc = declare_op(300,  XFX, "mod");

}  /* init_special_ops */

/*************
 *
 *    int process_op_command(t)
 *
 *************/

int process_op_command(t)
struct term *t;
{
    int type, n, rc;
    struct term *t1, *t2;
    char *s;

    if (sn_to_arity(t->sym_num) != 3) {
	printf("op command must have arity 3.\n");
	return(0);
	}
    t1 = t->farg->argval;
    if (t1->type != NAME || !str_int(sn_to_str(t1->sym_num), &n) ||
	n < 1 || n > 999) {
	printf("\nERROR: first argument of op command must be 1..999.\n");
	return(0);
	}
    t1 = t->farg->narg->argval;
    s = sn_to_str(t1->sym_num);
    if (str_ident(s, "xfx"))
	type = XFX;
    else if (str_ident(s, "xfy"))
	type = XFY;
    else if (str_ident(s, "yfx"))
	type = YFX;
    else if (str_ident(s, "fx"))
	type = FX;
    else if (str_ident(s, "xf"))
	type = XF;
    else if (str_ident(s, "fy"))
	type = FY;
    else if (str_ident(s, "yf"))
	type = YF;
    else
	type = MAX_INT;
    
    if (type == MAX_INT || t1->type != NAME) {
	printf("\nERROR: second argument of op command must be xfx, xfy, yfx, xf, yf, fx, or fy.\n");
	return(0);
	}

    t1 = t->farg->narg->narg->argval;

    if (t1->type == NAME)
	rc = declare_op(n, type, sn_to_str(t1->sym_num));
    else if (proper_list(t1)) {
	for ( ; t1->type == COMPLEX; t1 = t1->farg->narg->argval) {
	    t2 = t1->farg->argval;
	    if (t2->type != NAME) {
		printf("\nERROR: list in op command must be all names.\n");
		return(0);
		}
	    rc = declare_op(n, type, sn_to_str(t2->sym_num));
	    }
	}
    else {
	printf("\nERROR: third argument of op command must be a name or a list.\n");
	return(0);
	}
    return(1);
}  /* process_op_command */

/*************
 *
 *    int proper_list(t)
 *
 *************/

int proper_list(t)
struct term *t;
{
    if (t->type == VARIABLE)
        return(0);
    else if (t->type == NAME)
        return(t->sym_num == str_to_sn("$nil", 0));
    else if (t->sym_num != str_to_sn("$cons", 2))
        return(0);
    else
        return(proper_list(t->farg->narg->argval));
}  /* proper_list */

/*************
 *
 *    void fill_in_op_data(p, t)
 *
 *************/

static void fill_in_op_data(p, t)
struct sequence_member *p;
struct term *t;
{
    struct sym_ent *nd;
    char *str;
    int i, flag;

    p->t = t;
    p->binary_type = p->unary_type = 0;
    p->binary_prec = p->unary_prec = 0;

    if (t->type == NAME) {
	str = sn_to_str(t->sym_num);
	for (i = flag = 0; i < SYM_TAB_SIZE && flag < 2; i++) {
	    for (nd = Sym_tab->table[i]; nd && flag < 2; nd = nd->next) {
		if (str_ident(str, nd->name) && nd->special_op) {
		    if (nd->arity == 1) {
			p->unary_type = nd->op_type;
			p->unary_prec = nd->op_prec;
			}
		    else {  /* must be binary */
			p->binary_type = nd->op_type;
			p->binary_prec = nd->op_prec;
			}
		    }
		}
	    }
	}
    
}  /* fill_in_op_data */

/*************
 *
 *    int is_white(c) -- including start-of-comment '%'.
 *
 *************/

static int is_white(c)
char c;
{
    return(c == ' ' ||
	   c == '\t' ||  /* tab */
	   c == '\n' ||  /* newline */
	   c == '\v' ||  /* vertical tab */
	   c == '\r' ||  /* carriage return */
	   c == '\f' ||  /* form feed */
	   c == '%');
}  /* is_white */

/*************
 *
 *    skip_white(buffer, position)
 *
 *    Advance the pointer to the next nonwhite, noncomment position.
 *
 *************/

void skip_white(buf, p)
char buf[];
int *p;
{
    char c;
    c = buf[*p];
    while (is_white(c)) {
	if (c == '%')  /* skip over comment */
	    while (buf[++(*p)] != '\n') ;
	c = buf[++(*p)];
	}
}  /* skip_white */

/*************
 *
 *    int is_symbol_char(c, in_list)
 *
 *************/

static int is_symbol_char(c, in_list)
char c;
int in_list;
{
    return(c == '+' ||
	   c == '-' ||
	   c == '*' ||
	   c == '/' ||
	   c == '\\' ||
	   c == '^' ||
	   c == '<' ||
	   c == '>' ||
	   c == '=' ||
	   c == '`' ||
	   c == '~' ||
	   c == ':' ||
	   c == '?' ||
	   c == '@' ||
	   c == '&' ||
	   
	   (c == '|' && !in_list) ||
	   
	   c == '!' ||
	   c == '#' ||
	   c == ';'    );
    
}  /* is_symbol_char */

/*************
 *
 *    int is_alpha_numeric(c) -- including _ and $
 *
 *************/

static int is_alpha_numeric(c)
char c;
{
    return(c >= '0' && c <= '9' ||
	   c >= 'a' && c <= 'z' ||
	   c >= 'A' && c <= 'Z' ||
	   c == '_' || c == '$');
}  /* is_alpha_numeric */

/*************
 *
 *    int name_sym(s)
 *
 *************/

int name_sym(s)
char *s;
{
    if (*s == '\'' || *s == '\"')
	return(1);  /* quoted string ok */
    else {
	for ( ; *s; s++)
	    if (!is_alpha_numeric(*s))
		return(0);
	return(1);
	}
}  /* name_sym */

/*************
 *
 *    get_name(buffer, position, name, in_list)
 *
 *************/

static void get_name(buf, p, name, in_list)
char buf[];
int *p;
char name[];
int in_list;
{
    int i, ok, okq;
    char c, q;
    
    i = 0; ok = 1; okq = 1;
    skip_white(buf, p);
    c = buf[*p];
    if (is_alpha_numeric(c)) {
	while ((ok = i < MAX_NAME-1) && is_alpha_numeric(c)) {
	    name[i++] = c;
	    c = buf[++(*p)];
	    }
	}
    else if (is_symbol_char(c, in_list)) {
	while ((ok = i < MAX_NAME-1) && is_symbol_char(c, in_list)) {
	    name[i++] = c;
	    c = buf[++(*p)];
	    }
	}
    else if (c == '\'' || c == '\"') {
	q = c;
	name[i++] = c;
	c = buf[++(*p)];
	while ((ok = i < MAX_NAME-1) && c != q && (okq = c != '\0')) {
	    name[i++] = c;
	    c = buf[++(*p)];
	    }
	if (okq) {
	    name[i++] = c;  /* quote char */
	    ++(*p);
	    }
	}
    
    if (!ok) {
	fprintf(stdout, "\nERROR, name too big, max is %d; ", MAX_NAME-1);
	name[0] = '\0';
	}
    else if (!okq) {
	fprintf(stdout, "\nERROR, quoted name has no end; ");
	name[0] = '\0';
	}
    else
	name[i] = '\0';
}  /* get_name */

/*************
 *
 *    print_error(fp, buf, pos)
 *
 *************/

static void print_error(fp, buf, pos)
FILE *fp;
char buf[];
int pos;
{
#if 0
    int i;

    fprintf(fp, "%s\n", buf);
    for (i = 0; i < pos; i++) {
	if (buf[i] == '\t')
	    fprintf(fp, "--------");  /* doesn't always work */
	else if (buf[i] == '\n')
	    fprintf(fp, "\n");
	else
	    fprintf(fp, "-");
	}
    fprintf(fp, "^\n");
#else
    int i;
    i = 0;
    if (buf[0] == '\n')
	i = 1;
    while (i < pos) {
	if (buf[i] == '%')  /* skip over comment */
            while (buf[++i] != '\n') ;
 	fprintf(fp, "%c", buf[i++]);
	}
    fprintf(fp, " ***HERE*** ");

    while (buf[i]) {
	if (buf[i] == '%')  /* skip over comment */
            while (buf[++i] != '\n') ;
 	fprintf(fp, "%c", buf[i++]);
	}
    fprintf(fp, "\n");

#endif
}  /* print_error */

/*************
 *
 *    struct term *seq_to_quant_term(seq, n)
 *
 *    Take a sequence of terms t1,...,tn and build term $Quantified(t1,...,tn).
 *    t1 is already known to be a quantifier.
 *    Check that t2,...,tn-1 are all names.
 *
 *************/

static struct term *seq_to_quant_term(seq, n)
struct sequence_member seq[];
int n;
{
    struct rel *r_prev, *r_new;
    struct term *t, *t1;
    int i;
    
    if (n == 1)
	return(NULL);
    
    for (i = 1; i < n-1; i++)
	if (seq[i].t->type != NAME)
	    return(NULL);
    
    t = get_term();
    t->type = COMPLEX;
    t->sym_num = str_to_sn("$Quantified", n);
    for (i = 0, r_prev = NULL; i < n; i++) {
	r_new = get_rel();
	if (!r_prev)
	    t->farg = r_new;
	else
	    r_prev->narg = r_new;
	t1 = copy_term(seq[i].t);
	r_new->argval = t1;
	r_prev = r_new;
	}
    return(t);
    
}  /* seq_to_quant_term */

/*************
 *
 *    struct term *seq_to_term(seq, start, end, m)
 *
 *    seq is an array of terms/operators, and start and end are
 *    indexes into seq.  This routine attempts to construct a term
 *    starting with start, ending with end, with precedence <= m.
 *
 *************/

static struct term *seq_to_term(seq, start, end, m)
struct sequence_member seq[];
int start;
int end;
int m;
{
    int i, n, type;
    struct term *t1, *t2, *t3, *t;
    struct rel *r1, *r2;
    
    if (start == end) {
	t = copy_term(seq[start].t);
	return(t);
	}
    else {
        
        /* Check if first is prefix op that applies to rest. */

	if (seq[start].t->type == NAME) {
	    type = seq[start].unary_type;
	    n = seq[start].unary_prec;
	    t = seq[start].t;
	    
	    if (type == FX && n <= m) {
		t1 = seq_to_term(seq, start+1, end, n-1);
		if (t1) {
		    t3 = get_term();
		    t3->type = COMPLEX;
		    t3->sym_num = str_to_sn(sn_to_str(t->sym_num), 1);
		    r1 = get_rel();
		    t3->farg = r1;
		    r1->argval = t1;
		    return(t3);
		    }
		}
	    else if (type == FY && n <= m) {
		t1 = seq_to_term(seq, start+1, end, n);
		if (t1) {
		    t3 = get_term();
		    t3->type = COMPLEX;
		    t3->sym_num = str_to_sn(sn_to_str(t->sym_num), 1);
		    r1 = get_rel();
		    t3->farg = r1;
		    r1->argval = t1;
		    return(t3);
		    }
		}
	    }

        /* Check if last is postfix op that applies to all preceding. */

	if (seq[end].t->type == NAME) {
	    type = seq[end].unary_type;
	    n = seq[end].unary_prec;
	    t = seq[end].t;

	    if (type == XF && n <= m) {
		t1 = seq_to_term(seq, start, end-1, n-1);
		if (t1) {
		    t3 = get_term();
		    t3->type = COMPLEX;
		    t3->sym_num = str_to_sn(sn_to_str(t->sym_num), 1);
		    r1 = get_rel();
		    t3->farg = r1;
		    r1->argval = t1;
		    return(t3);
		    }
		}
	    else if (type == YF && n <= m) {
		t1 = seq_to_term(seq, start, end-1, n);
		if (t1) {
		    t3 = get_term();
		    t3->type = COMPLEX;
		    t3->sym_num = str_to_sn(sn_to_str(t->sym_num), 1);
		    r1 = get_rel();
		    t3->farg = r1;
		    r1->argval = t1;
		    return(t3);
		    }
		}
	    }
		
	/* Look for an infix operator. */

	for (i = start+1; i <= end-1; i++) {
	    if (seq[i].t->type == NAME) {
		type = seq[i].binary_type;
		n = seq[i].binary_prec;
		t = seq[i].t;

		if (type == XFY && n <= m) {
		    t1 = seq_to_term(seq, start, i-1, n-1);
		    if (t1) {
			t2 = seq_to_term(seq, i+1, end, n);
			if (!t2)
			    zap_term(t1);
			}
		    if (t1 && t2) {
			t3 = get_term();
			t3->type = COMPLEX;
			t3->sym_num = str_to_sn(sn_to_str(t->sym_num), 2);
			r1 = get_rel(); r2 = get_rel();
			t3->farg = r1; r1->narg = r2;
			r1->argval = t1; r2->argval = t2;
			return(t3);
			}
		    }
		else if (type == YFX && n <= m) {
		    t2 = seq_to_term(seq, i+1, end, n-1);
		    if (t2) {
			t1 = seq_to_term(seq, start, i-1, n);
			if (!t1)
			    zap_term(t2);
			}
		    if (t1 && t2) {
			t3 = get_term();
			t3->type = COMPLEX;
			t3->sym_num = str_to_sn(sn_to_str(t->sym_num), 2);
			r1 = get_rel(); r2 = get_rel();
			t3->farg = r1; r1->narg = r2;
			r1->argval = t1; r2->argval = t2;
			return(t3);
			}
		    }
		else if (type == XFX && n <= m) {
		    t1 = seq_to_term(seq, start, i-1, n-1);
		    if (t1) {
			t2 = seq_to_term(seq, i+1, end, n-1);
			if (!t2)
			    zap_term(t1);
			}
		    if (t1 && t2) {
			t3 = get_term();
			t3->type = COMPLEX;
			t3->sym_num = str_to_sn(sn_to_str(t->sym_num), 2);
			r1 = get_rel(); r2 = get_rel();
			t3->farg = r1; r1->narg = r2;
			r1->argval = t1; r2->argval = t2;
			return(t3);
			}
		    }
		}  /* name */
	    }  /* loop looking for infix op to apply */
	
	return(NULL);
	}
}  /* seq_to_term */

/*************
 *
 *    struct term *str_to_args(buffer, position, name)
 *
 *    name -- the functor.
 *
 *    start: functor(  a1 , a2 , a3 )
 *                   ^
 *    end:   functor(  a1 , a2 , a3 )
 *                                  ^
 *************/

static struct term *str_to_args(buf, p, name)
char buf[];
int *p;
char name[];
{
    struct term *t, *t_sub;
    struct rel *r1, *r2;
    int i;
    
    t = get_term();
    t->type = COMPLEX;
    r1 = NULL;
    i = 0;  /* count subterms to get arity */
    
    while (buf[*p] != ')') {
	i++;
	t_sub = str_to_term(buf, p, 0);
	if (t_sub == NULL)
	    return(NULL);
	else if (buf[*p] != ',' && buf[*p] != ')') {
	    fprintf(stdout, "\nERROR, comma or ) expected:\n");
	    print_error(stdout, buf, *p);
	    return(NULL);
	    }
	else {
	    r2 = get_rel();
	    r2->argval = t_sub;
	    if (r1 == NULL)
		t->farg = r2;
	    else
		r1->narg = r2;
	    r1 = r2;
	    if (buf[*p] == ',')
		(*p)++;          /* step past comma */
	    }
	}
    if (i == 0) {
	fprintf(stdout, "\nERROR, functor has no arguments:\n");
	print_error(stdout, buf, *p);
	return(NULL);
	}
    
    t->sym_num = str_to_sn(name, i);  /* functor */
    return(t);
    
}  /* str_to_args */

/*************
 *
 *    struct term *str_to_list(buffer, position)
 *
 *    start: [ a1 , a2 , a3 ]
 *           ^
 *    end:   [ a1 , a2 , a3 ]
 *                           ^
 *************/

static struct term *str_to_list(buf, p)
char buf[];
int *p;
{
    struct term *t_cons, *t_head, *t_tail, *t_return;
    struct rel *r_head, *r_tail;
    int go;
    
    (*p)++;  /* step past '[' */
    
    if (buf[*p] == ']') {                        /* [] */
	t_return = get_term();
	t_return->type = NAME;
	t_return->sym_num = str_to_sn("$nil", 0);
	(*p)++;  /* skip "]" */
	return(t_return);
	}
    else {                           /* [h|t], [t1,...,tn], or [t1,...,tn|t] */
	t_return = NULL; r_tail = NULL;
	go = 1;
	
	while (go) {
	    t_head = str_to_term(buf, p, 1);
	    if (t_head == NULL)
		return(NULL);  /* error */
	    t_cons = get_term();
	    if (r_tail == NULL)
		t_return = t_cons;
	    else
		r_tail->argval = t_cons;
	    t_cons->type = COMPLEX;
	    t_cons->sym_num = str_to_sn("$cons", 2);
	    r_head = get_rel();
	    t_cons->farg = r_head;
	    r_head->argval = t_head;
	    r_tail = get_rel();  
	    r_head->narg = r_tail;
	    go = (buf[*p] == ',');
	    if (go)
		(*p)++;  /* step past ',' */
	    }
	
	if (buf[*p] == ']') {
	    t_tail = get_term();
	    r_tail->argval = t_tail;
	    t_tail->type = NAME;
	    t_tail->sym_num = str_to_sn("$nil", 0);
	    (*p)++;  /* step past ']' */
	    return(t_return);
	    }
	else if (buf[*p] == '|') {
	    (*p)++;  /* step past '|' */
	    t_tail = str_to_term(buf, p, 1);
	    if (buf[*p] != ']') {
		fprintf(stdout, "\nERROR, ']' expected in list:\n");
		print_error(stdout, buf, *p);
		return(NULL);
		}
	    r_tail->argval = t_tail;
	    (*p)++;  /* step past ']' */
	    return(t_return);
	    }
	else {
	    fprintf(stdout, "\nERROR, ], |, or comma expected in list:\n");
	    print_error(stdout, buf, *p);
	    return(NULL);
	    }
	}
}  /* str_to_list */

/*************
 *
 *    int str_to_sequence(buffer, position, seq, in_list)
 *
 *    Read a sequence of operators/terms---It will be parsed into
 *    a term later in str_to_term.
 *    After successful call, position is the delimeter following the term.
 *
 *    Mutually recursive with str_to_term.
 *
 *    If a syntax error is found, print message and return(NULL).
 *
 *************/

static int str_to_sequence(buf, p, seq, in_list)
char buf[];
int *p;
struct sequence_member seq[];
int in_list;
{
    char name[MAX_NAME], c;
    struct term *t;
    int done, n, white;;
    
    done = 0; n = 0;
    while (!done) {
	
	get_name(buf, p, name, in_list);
	white = is_white(buf[*p]);  /* f(a) vs. f (a) */
	skip_white(buf, p);
	
	if (name[0] == '\0' && buf[*p] != '[' && buf[*p] != '(' && buf[*p] != '{') {
	    fprintf(stdout, "\nERROR, name expected:\n");
	    print_error(stdout, buf, *p);
	    return(NULL);
	    }
	
	else if (name[0] == '\0' && buf[*p] == '(') {         /* (term) */
	    (*p)++;  /* step past '(' */
	    t = str_to_term(buf, p, 0);
	    if (t == NULL)
		return(NULL);
	    if (buf[*p] != ')') {
		fprintf(stdout, "\nERROR, ')' expected:\n");
		print_error(stdout, buf, *p);
		return(NULL);
		}
	    (*p)++;  /* step past ')' */
	    }
	
	else if (name[0] == '\0' && buf[*p] == '{') {         /* {term} */
	    (*p)++;  /* step past '{' */
	    t = str_to_term(buf, p, 0);
	    if (t == NULL)
		return(NULL);
	    if (buf[*p] != '}') {
		fprintf(stdout, "\nERROR, '}' expected:\n");
		print_error(stdout, buf, *p);
		return(NULL);
		}
	    (*p)++;  /* step past '}' */
	    }
	
	else if (name[0] != '\0' && !white && buf[*p] == '(') {  /* f(args) */
	    (*p)++;  /* step past '(' */
	    t = str_to_args(buf, p, name);
	    if (t == NULL)
		return(NULL);
	    (*p)++;  /* step past ')' */
	    }
	
	else if (name[0] == '\0' && buf[*p] == '[') {           /* list */
	    t = str_to_list(buf, p);
	    if (t == NULL)
		return(NULL);
	    }
	
	else if (name[0] != '\0') {                           /* name */
	    t = get_term();
	    t->type = NAME;
	    /* If it's an operator, change arity later. */
	    t->sym_num = str_to_sn(name, 0);
	    }
	
	else {
	    fprintf(stdout, "\nERROR, unrecognized error in term:\n");
	    print_error(stdout, buf, *p);
	    return(NULL);
	    }
	
	/* We have a term t. */
	
	if (n == MAX_COMPLEX) {
	    fprintf(stdout, "\nERROR, term too big:\n");
	    print_error(stdout, buf, *p);
	    return(NULL);
	    }
	else {
	    fill_in_op_data(&seq[n], t);
	    n++;
	    }
	
	skip_white(buf, p);
	c = buf[*p];
	done = (c == ',' || c == ')' || c == '}' || c == ']' ||
		c == '.' || c == '\0' || (in_list && c == '|'));
	}
    return(n);
}  /* str_to_sequence */

/*************
 *
 *    struct term *str_to_term(buffer, position, in_list)
 *
 *    Parse a string and build a term.
 *    Mutually recursive with str_to_sequence.
 *    After successful call, position is the delimeter following the term.
 *
 *    If a syntax error is found, print message and return(NULL).
 *
 *************/

struct term *str_to_term(buf, p, in_list)
char buf[];
int *p;
int in_list;
{
    struct sequence_member seq[MAX_COMPLEX];
    struct term *t;
    int n, i, save_pos;

    save_pos = *p;

    n = str_to_sequence(buf, p, seq, in_list);
    if (n == 0)
	return(NULL);

    else if (seq[0].t->type == NAME && n > 1 &&
	     (str_ident(sn_to_str(seq[0].t->sym_num), "all") ||
	      str_ident(sn_to_str(seq[0].t->sym_num), "exists"))) {
	t = seq_to_quant_term(seq, n);
	if (!t) {
	    fprintf(stdout, "\nERROR in quantifier prefix starting here:\n");
	    print_error(stdout, buf, save_pos);
	    }
	}

    else {
	t = seq_to_term(seq, 0, n-1, 1000);
	
	if (!t) {
	    fprintf(stdout, "\nERROR, the %d terms/operators in the following sequence are OK, but they\ncould not be combined into a single term with special operators.\n", n);
	    for (i = 0; i < n; i++)
		{ p_term(seq[i].t); printf("  ");}
	    printf("\n");
	    fprintf(stdout, "The context of the bad sequence is:\n");
	    print_error(stdout, buf, save_pos);
	    }
	}
    
    for (i = 0; i < n; i++)
	if (seq[i].t != NULL)
	    zap_term(seq[i].t);
    return(t);
}  /* str_to_term */

/*************
 *
 *     int read_buf(file_ptr, buffer)
 *
 *    Read characters into buffer until one of the following:
 *        1.  '.' is reached ('.' goes into the buffer)
 *        2.  EOF is reached:  buf[0] = '\0' (an error occurs if
 *                 any nonwhite space precedes EOF)
 *        3.  MAX_BUF characters have been read (an error occurs)
 *
 *    If error occurs, return(0), else return(1).
 *
 *************/

int read_buf(fp, buf)
FILE *fp;
char buf[];
{
    int c, qc, i, j, ok, eof, eof_q, max, max_q;

    ok = eof = eof_q = max = max_q = 0;  /* stop conditions */
    i = 0;
    
    while (!ok && !eof && !eof_q && !max && !max_q) {

	c = getc(fp);
	if (c == '%') {  /* comment--discard rest of line */
	    while (c != '\n' && c != EOF)
		c = getc(fp);
	    }
	if (c =='.')
	    ok = 1;
	else if (c == EOF)
	    eof = 1;
	else if (i == MAX_BUF-1)
	    max = 1;
	else {
	    buf[i++] = c;
	    if (c == '\'' || c == '\"') {
		qc = c;
		c = getc(fp);
		while (c != qc && c != EOF && i != MAX_BUF-1) {
		    buf[i++] = c;
		    c = getc(fp);
		    }
		if (i == MAX_BUF-1)
		    max_q = 1;
		else if (c == EOF)
		    eof_q = 1;
		else
		    buf[i++] = c;
		}
	    }
	}
    
    if (ok) {
	buf[i++] = '.';
	buf[i] = '\0';
	return(1);
	}
    else if (eof) {
	/* white space at end of file is OK */
	j = 0;
	buf[i] = '\0';
	skip_white(buf, &j);
	if (i != j) {
	    fprintf(stdout, "\nERROR, characters after last period: %s\n", buf);
	    buf[0] = '\0';
	    return(0);
	    }
	else {
	    buf[0] = '\0';
	    return(1);
	    }
	}
    else if (eof_q) {
	buf[i] = '\0';
	abend("in read_buf, quoted string has no end.");
	}
    else if (max) {
	buf[i] = '\0';
	abend("in read_buf, input string had too many characters (MAX_BUF exceeded).");
	}
    else {  /* max_q */
	buf[i] = '\0';
	abend("input string (which contains quote) has too many characters (MAX_BUF exceeded).");
	}
    return(0);  /* to quiet lint */
}  /* read_buf */

/*************
 *
 *    struct term *term_fixup(t)
 *
 *    change !=(a,b) to -(=(a,b))
 *    change -(3)    to -3              not recursive, -(-(3)) -> -(-3))
 *    change +(3)    to +3              not recursive, +(+(3)) +> +(+3))
 *
 *************/

struct term *term_fixup(t)
struct term *t;
{
    struct rel *r, *r1;
    struct term *t1;
    int neg;
    char *s, str[MAX_NAME];
    long l;

    if (t->type == COMPLEX) {
	if (is_symbol(t, "!=", 2)) {
	    t1 = get_term(); t1->type = COMPLEX;
	    r1 = get_rel();
	    t1->farg = r1;
	    r1->argval = t;
	    t1->sym_num = str_to_sn("-", 1);
	    t->sym_num = str_to_sn("=", 2);
	    t = t1;
	    }

	else if ((neg = is_symbol(t, "-", 1)) || is_symbol(t, "+", 1)) {
	    t1 = t->farg->argval;
	    s = sn_to_str(t1->sym_num);
	    if (t1->type == NAME && str_long(s, &l)) {
		cat_str((neg ? "-" : "+"), s, str);
		t1->sym_num = str_to_sn(str, 0);
		free_rel(t->farg);
		free_term(t);
		t = t1;
		}
	    }

	for (r = t->farg; r; r = r->narg)
	    r->argval = term_fixup(r->argval);

	}
    return(t);
}  /* term_fixup */

/*************
 *
 *    struct term *term_fixup_2(t)
 *
 *    change  -(=(a,b)) to !=(a,b)
 *
 *************/

struct term *term_fixup_2(t)
struct term *t;
{
    struct term *t1;
    struct rel *r;

    if (is_symbol(t, "-", 1) && is_symbol(t->farg->argval, "=", 2)) {
	t1 = t->farg->argval;
	t1->sym_num = str_to_sn("!=", 2);
	free_rel(t->farg);
	free_term(t);
	t = t1;
	}

    if (t->type == COMPLEX)
	for (r = t->farg; r; r = r->narg)
	    r->argval = term_fixup_2(r->argval);
    
    return(t);
}  /* term_fixup_2 */

/*************
 *
 *    struct term *read_term(file_ptr, retcd_ptr) --
 *
 *    Read and return then next term.
 *    It is assumed that the next term in the file is terminated
 *    with a period.   NULL is returned if EOF is reached first.
 *
 *    If an error is found, return(0); else return(1).
 *
 *************/

struct term *read_term(fp, rcp)
FILE *fp;
int *rcp;
{
    char buf[MAX_BUF+1];  /* one extra for \0 at end */
    int p, rc;
    struct term *t;
    
    rc = read_buf(fp, buf);
    if (rc == 0) {  /* error */
	*rcp = 0;
	return(NULL);
	}
    else if (buf[0] == '\0') {  /* ok. EOF */
	*rcp = 1;
	return(NULL);
	}
    else {
	p = 0;
	t = str_to_term(buf, &p, 0);
	if (t == NULL) {
	    *rcp = 0;
	    return(NULL);
	    }
	else {
	    skip_white(buf, &p);
	    if (buf[p] != '.') {
	        fprintf(stdout, "\nERROR, text after term:\n");
	        print_error(stdout, buf, p);
	        *rcp = 0;
	        return(NULL);
	        }
	    else {
		t = term_fixup(t);
		*rcp = 1;
		return(t);
		}
	    }
	}
}  /* read_term */

/*************
 *
 *    set_assoc_comm
 *
 *************/

void set_assoc_comm(str)
char *str;
{
    int sn;
    struct sym_ent *p;

    sn = str_to_sn(str, 2);
    p = sn_to_node(sn);
    p->assoc_comm = 1;

}  /* set_assoc_comm */

/*************
 *
 *    is_assoc_comm
 *
 *************/

int is_assoc_comm(sn)
int sn;
{
    struct sym_ent *p;

    p = sn_to_node(sn);
    if (!p)
	return(0);
    else
        return(p->assoc_comm);

}  /* set_assoc_comm */

/*************
 *
 *    int process_ac_command(t)
 *
 *************/

int process_ac_command(t)
struct term *t;
{
    struct term *t1, *t2;

    if (sn_to_arity(t->sym_num) != 1) {
	printf("ac command must have arity 1.\n");
	return(0);
	}
    else {
	t1 = t->farg->argval;
	
	if (t1->type == NAME)
	    set_assoc_comm(sn_to_str(t1->sym_num));
	else if (proper_list(t1)) {
	    for ( ; t1->type == COMPLEX; t1 = t1->farg->narg->argval) {
		t2 = t1->farg->argval;
		if (t2->type != NAME) {
		    printf("ERROR: list in assoc_comm command must be all names.\n");
		    return(0);
		    }
		set_assoc_comm(sn_to_str(t2->sym_num));
		}
	    }
	else {
	    printf("ERROR: argument of assoc_comm command must be a name or a list.\n");
	    return(0);
	    }
	return(1);
	}
}  /* process_ac_command */

/*************
 *
 *    set_commutative
 *
 *************/

void set_commutative(str)
char *str;
{
    int sn;
    struct sym_ent *p;

    sn = str_to_sn(str, 2);
    p = sn_to_node(sn);
    p->commutative = 1;

}  /* set_commutative */

/*************
 *
 *    is_commutative
 *
 *************/

int is_commutative(sn)
int sn;
{
    struct sym_ent *p;

    p = sn_to_node(sn);
    if (!p)
	return(0);
    else
        return(p->commutative);

}  /* set_commutative */

/*************
 *
 *    int process_comm_command(t)
 *
 *************/

int process_comm_command(t)
struct term *t;
{
    struct term *t1, *t2;

    if (sn_to_arity(t->sym_num) != 1) {
	printf("commutative command must have arity 1.\n");
	return(0);
	}
    else {
	t1 = t->farg->argval;
	
	if (t1->type == NAME)
	    set_commutative(sn_to_str(t1->sym_num));
	else if (proper_list(t1)) {
	    for ( ; t1->type == COMPLEX; t1 = t1->farg->narg->argval) {
		t2 = t1->farg->argval;
		if (t2->type != NAME) {
		    printf("ERROR: list in commutative command must be all names.\n");
		    return(0);
		    }
		set_commutative(sn_to_str(t2->sym_num));
		}
	    }
	else {
	    printf("ERROR: argument of commutative command must be a name or a list.\n");
	    return(0);
	    }
	return(1);
	}
}  /* process_comm_command */

/*************
 *
 *    int process_multiset_command(t)
 *
 *************/

int process_multiset_command(t)
struct term *t;
{
    struct term *t1, *t2;

    if (sn_to_arity(t->sym_num) != 1) {
	printf("lrpo_multiset_status command must have arity 1.\n");
	return(0);
	}
    else {
	t1 = t->farg->argval;
	
	 if (proper_list(t1)) {
	    for ( ; t1->type == COMPLEX; t1 = t1->farg->narg->argval) {
		t2 = t1->farg->argval;
		if (t2->type != COMPLEX) {
		    printf("ERROR: list in multiset command must be all complex terms.\n");
		    return(0);
		    }
		sn_to_node(t2->sym_num)->lrpo_status = LRPO_MULTISET_STATUS;
		}
	    }
	else {
	    printf("ERROR: argument of multiset command must be a list of complex terms.\n");
	    return(0);
	    }
	return(1);
	}
}  /* process_multiset_command */

/*************
 *
 *    int process_lex_command(t)
 *
 *************/

int process_lex_command(t)
struct term *t;
{
    struct term *t1, *t2;
    int count;

    if (sn_to_arity(t->sym_num) != 1) {
	printf("lex command must have arity 1.\n");
	return(0);
	}
    else {
	count = 0;
	t1 = t->farg->argval;
	
	if (proper_list(t1)) {
	    for ( ; t1->type == COMPLEX; t1 = t1->farg->narg->argval) {
		t2 = t1->farg->argval;
		if (t2->type == VARIABLE) {
		    printf("ERROR: variable in lex command.\n");
		    return(0);
		    }
		sn_to_node(t2->sym_num)->lex_val = ++count;
		}
	    return(1);
	    }
	else {
	    printf("ERROR: argument of lex command must be a list.\n");
	    return(0);
	    }
	}
}  /* process_lex_command */

/*************
 *
 *     sym_tab_to_string(s, size)
 *
 *************/

void sym_tab_to_string(s, size)
char s[];
int size;
{
    int i, n, count;
    struct sym_ent *p;
    char s1[MAX_NAME+10];

    count = 0;
    s[0] = '\0';
    for (i=0; i<SYM_TAB_SIZE; i++) {
	for (p=Sym_tab->table[i]; p; p=p->next) {
	    sprintf(s1, "%d %d %s %s %d %d ",
		    p->sym_num,
		    p->arity,
		    p->assoc_comm ? "ac" : (p->commutative ? "comm" : "--"),
		    p->name,
		    p->lrpo_status,
		    p->lex_val);
	    count += strlen(s1);
	    if (count >= size)
		abend("in sym_tab_to_string, too many symbols.");
	    strcat(s, s1);
	    }
	}
#if 0
    p_syms();
    printf("%s\n", s);
#endif    
}  /* sym_tab_to_string */

