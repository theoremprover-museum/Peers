#include "Header.h"
#include "Index.h"
#include "Unify.h"

#define MAX_PATH 110  /* max number of integers in a path, incl. end mark */

#define INTERSECT  1
#define UNION      2
#define LEAF       3

/*************
 *
 *    fpa_init(depth) -- allocate and initilaize an FPA index.
 *
 *************/

struct fpa_index *fpa_init(depth)
int depth;
{
    int i;
    struct fpa_index *p;

    /* Get number of integers needed for largest path, incl. end marker. */
    i = (2*depth + 2*sizeof(int)) / sizeof(int);
    if (i > MAX_PATH)
	abend("fpa_init: depth too big; increase MAX_PATH.");

    p = (struct fpa_index *) tp_alloc(sizeof(struct fpa_index));
    p->depth = depth;
    for (i = 0; i < FPA_SIZE; i++)
	p->table[i] = 0;
    return(p);
}  /* fpa_init */

/*
 *
 * MESSY IMPLEMENTATION DETAIL:  Paths have one byte per member, plus
 * a word of 0s to mark the end.  When accessing members of a path,
 * we treat a path as an array of unsigned chars.  When comparing,
 * copying, and hashing paths, we treat them as arrays of ints (for
 * speed).  The "official" form (argument passing, etc) is as an array
 * of ints, because lint complains about possible alignment errors when
 * casting (unsigned char *) to (int *).
 *
 * The current position in the path (usually variable j) counts in bytes.
 *
 */

/*************
 *
 *    static void path_mark_end(path, j)
 *
 *    j (which counts bytes) is one past last entry.
 *
 *************/

static void path_mark_end(path, j)
int *path;
int j;
{
    int i, k, m;
    unsigned char *cpath;

    cpath = (unsigned char *) path;

    /* make sure the rest of the integer, starting with j, and the */
    /* whole next integer (unless j is at beginning) are all 0. */
    
    m = j % sizeof(int);  /* position of j in an int */

    if (m == 0)
	i = sizeof(int);  /* just fill int with 0s */
    else
	i = (2 * sizeof(int)) - m;  /* 0 rest of int and next int */

    for (k = 0; k < i; k++)
	cpath[j++] = 0;
    
}  /* path_mark_end */

/*************
 *
 *    static int hash_path(path)
 *
 *************/

static int hash_path(path)
int path[];
{
    int i, val;

    val = 0;

    for (i = 0; path[i] != 0; i++)
	val += path[i];

    return(abs(val) % FPA_SIZE);
}  /* hash_path */

/*************
 *
 *    static int path_comp(p1, p2)
 *
 *************/

static int path_comp(p1, p2)
int *p1;
int *p2;
{
    while (*p1 == *p2 && *p1 != 0 && *p2 != 0) {
	p1++;
	p2++;
	}

    if (*p1 < *p2)
	return(-1);
    else if (*p1 > *p2)
	return(1);
    else
	return(0);

}  /* path_comp */

/*************
 *
 *    static int path_size(path) -- in ints, including 0 word at end
 *
 *************/

static int path_size(path)
int *path;
{
    int i;
    int *p1;

    for (i = 1, p1 = path; *p1 != 0; p1++, i++);
    return(i);
}  /* path_size */

/*************
 *
 *    static int *path_copy(path)
 *
 *************/

static int *path_copy(path)
int *path;
{
    int i, j;
    int *p2;

    i = path_size(path);

    p2 = (int *) tp_alloc(i * sizeof(int));

    for (j = 0; j < i; j++)
	p2[j] = path[j];

    return(p2);
    
}  /* path_copy */

/*************
 *
 *    static insert_fpa_tab(term, path, table)
 *
 *        Insert a term into an FPA indexing list.  Create a new list
 *    if necessary.  The path is something like "1 f 2 g 4 h 3 a".
 *
 *************/

static void insert_fpa_tab(t, path, table)
struct term *t;
int *path;
struct fpa_head *table[];
{
    int hashval, c;
    struct gen_ptr *tp1, *tp2, *tp3;
    struct fpa_head *fp1, *fp2, *fp3;

    /* Treat path as integers here. */
    
    hashval = hash_path(path);
    fp1 = table[hashval];
    fp2 = NULL;

    while (fp1 && (c = path_comp(fp1->path, path)) == -1) {
	fp2 = fp1;
	fp1 = fp1->next;
	}
    
    if (!fp1 || c != 0) { /* need new fpa_head */
	fp3 = get_fpa_head();
	fp3->path = path_copy(path);
	tp1 = get_gen_ptr();
	fp3->terms = tp1;
	tp1->u.t = t;
    
	if (!fp2) {
	    /* insert at beginning */
	    fp3->next = table[hashval];
	    table[hashval] = fp3;
	    }
	else {  /* insert after fp2 */
	    fp3->next = fp1;
            fp2->next = fp3;
            }
	}

    else { /* we have a matching fpa_head, so insert t in its list */
    
	tp1 = fp1->terms;
	tp2 = NULL;
	/* keep list sorted, decreasing addresses */
	while (tp1 && tp1->u.t->fpa_id > t->fpa_id) {
	    tp2 = tp1;
	    tp1 = tp1->next;
	    }
	if (tp1 && tp1->u.t == t)
	    Stats[FPA_OVERLOADS]++;  /* term already in list */
	else {
	    tp3 = get_gen_ptr();
	    tp3->u.t = t;
	    if (!tp2) { /* insert at beginning */
		tp3->next = fp1->terms;
		fp1->terms = tp3;
		}
	    else { /* insert after tp2 */
		tp3->next = tp1;
		tp2->next = tp3;
		}
	    }
	}
}  /* insert_fpa_tab */

/*************
 *
 *    static delete_fpa_tab(term, path, table)
 *
 *        Delete a term from an FPA indexing list.  It is assumed that
 *    the corresponding `insert_fpa_tab' was previously made.
 *
 *************/

static void delete_fpa_tab(t, path, table)
struct term *t;
int *path;
struct fpa_head *table[];
{
    int hashval;
    struct gen_ptr *tp1, *tp2;
    struct fpa_head *fp1, *fp2;
    
    /* Treat path as integers here. */

    hashval = hash_path(path);
    fp1 = table[hashval];
    fp2 = NULL;

    while (fp1 && path_comp(fp1->path, path) != 0) {
	fp2 = fp1;
	fp1 = fp1->next;
	}
    
    if (!fp1)
	Stats[FPA_UNDERLOADS]++;  /* fpa list not found */
    else { /* we have a matching fpa_head, so look for t in its list */
    
	tp1 = fp1->terms;
	tp2 = NULL;
	/* list is sorted, decreasing addresses */
	while (tp1 && tp1->u.t->fpa_id > t->fpa_id) {
	    tp2 = tp1;
	    tp1 = tp1->next;
	    }
	if (!tp1 || tp1->u.t != t)
	    Stats[FPA_UNDERLOADS]++;  /* term not found in list */
	else {
	    if (!tp2) {  /* delete from beginning */
		fp1->terms = tp1->next;
		if (!fp1->terms) { /* delete fpa_head also */
		    if (!fp2)
		        table[hashval] = fp1->next;
		    else
		        fp2->next = fp1->next;
		    free_fpa_head(fp1);
		    /* don't worry about fp1->path; let it be lost forever */
		    }
		}
	    else  /* delete */
		tp2->next = tp1->next;
	    free_gen_ptr(tp1);
	    }
	}
}  /* delete_fpa_tab */

/*************
 *
 *   void term_fpa_rec
 *
 *   Recursive procedure called by fpa_insert and fpa_delete.
 *
 *************/

void term_fpa_rec(insert, t, super_term, table, path, j, bound)
int insert;
struct term *t;
struct term *super_term;
struct fpa_head *table[];
int *path;
int j;
int bound;
{
    int i;
    struct rel *r;
    unsigned char *cpath;

    cpath = (unsigned char *) path;
    
    /* `path' has the path from super_term to t */

    if (t->type == VARIABLE) /* variable contributes nothing */
	cpath[j++] = 0;
    else 
	cpath[j++] = t->sym_num;
    
    /* insert or delete path */

    path_mark_end(path, j);
    if (insert)
	insert_fpa_tab(super_term, path, table);
    else
	delete_fpa_tab(super_term, path, table);
    
    if (t->type == COMPLEX && bound > 0 && !is_assoc_comm(t->sym_num)) {
	i = 1;
	r = t->farg;
	while (r) {
	    cpath[j] = i++;
	    term_fpa_rec(insert, r->argval, super_term, table, path, j+1, bound-1);
	    r = r->narg;
	    }
	}
}  /* term_fpa_rec */

/*************
 *
 *    void fpa_insert(term, index)
 *
 *        Insert a term into an FPA indexing index.  Level == 0
 *    gives indexing on functor only.  With the term f(a,x,g(b)),
 *    Level == 1 gives indexing on f, a, x, and g.
 *
 *************/

void fpa_insert(t, index)
struct term *t;
struct fpa_index *index;
{
    static int term_count;
    int path[MAX_PATH];

    /* t->fpa_id is used to order FPA lists.  Assign one if necessary. */
    if (t->fpa_id == 0)
	t->fpa_id = ++term_count;

    term_fpa_rec(1, t, t, index->table, path, 0, index->depth);
}  /* fpa_insert */

/*************
 *
 *    void fpa_delete(term, index)
 *
 *        Delete a term from an FPA indexing index.   The depth
 *    must be the same as when the term was given to fpa_insert.
 *
 *************/

void fpa_delete(t, index)
struct term *t;
struct fpa_index *index;
{
    int path[MAX_PATH];
  
    term_fpa_rec(0, t, t, index->table, path, 0, index->depth);
}  /* fpa_delete */

/*************
 *
 *   union_with_commutative_branch()
 *
 *************/

struct fpa_tree *union_with_commutative_branch(p, t, u_type, path, j, bound, table)
struct fpa_tree *p;
struct term *t;
int u_type;
int *path;
int j;
int bound;
struct fpa_head *table[];
{
    unsigned char *cpath;
    struct fpa_tree *p1, *p2, *p3, *p4;

#if 0
    printf("\nENTER union_with_commutative_branch with "); p_term(t);
    p_prop_tree(p);
#endif

    cpath = (unsigned char *) path;
    cpath[j] = 2;
    p1 = build_tree_local(t->farg->argval, u_type, path, j+1, bound-1, table);
    if (p1) {
	cpath[j] = 1;
	p2 = build_tree_local(t->farg->narg->argval, u_type, path, j+1, bound-1, table);
	if (p2) {
	    /* build UNION(p,INTERSECT(p1,p2)). */
	    p3 = get_fpa_tree();
	    p3->type = INTERSECT;
	    p3->left = p1;
	    p3->right = p2;

	    p4 = get_fpa_tree();
	    p4->type = UNION;
	    p4->left = p;
	    p4->right = p3;
	    }
	else {
	    zap_prop_tree(p1);
	    p4 = p;
	    }
	}
    else
	p4 = p;

#if 0
    printf("exit union_with_commutative_branch with\n");
    p_prop_tree(p4);
#endif    
    return(p4);
}  /* union_with_commutative_branch */

/*************
 *
 *    static struct fpa_tree *get_leaf_node(path, table)
 *
 *        Given a path, if an FPA list exists, then return it in a
 *    leaf node; else return(NULL).
 *
 *************/

static struct fpa_tree *get_leaf_node(path, table)
int *path;
struct fpa_head *table[];
{
    struct fpa_head *fp;
    struct fpa_tree *pp;
    int c;
    
    fp = table[hash_path(path)];
    while (fp && (c = path_comp(fp->path,path)) == -1)
	fp = fp->next;
    if (!fp | c != 0)
	return(NULL);
    else {
	pp = get_fpa_tree();
	pp->type = LEAF;
#if 0  /* Path field is for debugging only. */
	pp->path = path_copy(path);
#else	
	pp->path = NULL;
#endif	
	pp->terms = fp->terms;
	return(pp);
	}
}  /* get_leaf_node */

/*************
 *
 *    static int all_args_vars(t) -- are all subterms variables?
 *
 *************/

static int all_args_vars(t)
struct term *t;
{
    struct rel *r;

    if (t->type != COMPLEX)
	return(0);
    else {
        r = t->farg;
	while (r) {
	    if (r->argval->type != VARIABLE)
		return(0);
	    r = r->narg;
	    }
	return(1);
	}
}  /* all_args_vars */

/*************
 *
 *    struct fpa_tree *build_tree_local(term, unif_type, path, bound, table)
 *
 *    Return an FPA indexing tree--to be used with a sequence
 *    of get_next calls.
 *
 *        term:       An error if term is VARIABLE && unif_type != MORE_GEN
 *                    because everything satisfies that query.
 *        unif_type:  UNIFY, INSTANCE, MORE_GEN
 *        path:       must be 0 on initial call
 *        bound:      indexing bound (must be <= fpa_insert bound)
 *        table:   
 *
 *    Note:  If an appropriate fpa list does not exist, then part of
 *    the tree can sometimes be deleted.
 *
 *************/

struct fpa_tree *build_tree_local(t, u_type, path, j, bound, table)
struct term *t;
int u_type;
int *path;
int j;
int bound;
struct fpa_head *table[];
{
    int i, empty;
    struct rel *r;
    struct fpa_tree *p1, *p2, *p3;
    unsigned char *cpath;

    cpath = (unsigned char *) path;
    
    /* `path' has the path to `t' */

    if (t->type == VARIABLE) { /* variable */
	if (u_type != MORE_GEN) {  /* error if not "more general" */
	    output_stats(stdout);
	    abend("build_tree_local, var and not more general.");
	    return(NULL);  /* to quiet lint */
	    }
	else {
	    cpath[j++] = 0;
	    path_mark_end(path, j);
	    p1 = get_leaf_node(path, table);
	    return(p1);
	    }
	}
    else {  /* NAME or COMPLEX */
	cpath[j++] = t->sym_num;
	if (t->type == NAME || bound == 0 || is_assoc_comm(t->sym_num) ||
	    (u_type != MORE_GEN && all_args_vars(t))) {
	    path_mark_end(path, j);
	    p2 = get_leaf_node(path, table);
	    }
	else {

	    i = 1;
	    empty = 0;
	    p2 = NULL;
	    r = t->farg;
	    while (r && !empty) {
		cpath[j] = i++;
		/* skip this arg if var and "unify" or "instance" */
		if (r->argval->type != VARIABLE || u_type == MORE_GEN) {
		    p3 = build_tree_local(r->argval,u_type,path,j+1,bound-1,table);
		    if (!p3) {
			if (p2) {
			    zap_prop_tree(p2);
			    p2 = NULL;
			    }
			empty = 1;
		        }
		    else if (!p2)
			p2 = p3;
		    else {
		        p1 = get_fpa_tree();
		        p1->type = INTERSECT;
		        p1->left = p2;
		        p1->right = p3;
		        p2 = p1;
		        }
		    }
		r = r->narg;
		}
	    if (is_commutative(t->sym_num))
		p2 = union_with_commutative_branch(p2, t, u_type, path, j, bound, table);
	    }
    
	if (u_type != INSTANCE) {  /* if we don't want instances only, */
	    cpath[j-1] = 0;
	    path_mark_end(path, j);
	    p3 = get_leaf_node(path, table); /* variable */
	    }
	else
	    p3 = NULL;
    
	if (!p2)
	    return(p3);
	else if (!p3)
	    return(p2);
	else {  /* UNION them together */
	    p1 = get_fpa_tree();
	    p1->type = UNION;
	    p1->left = p2;
	    p1->right = p3;
	    return(p1);
	    }
	}
}  /* build_tree_local */

/*************
 *
 *    struct fpa_tree *build_tree(t, u_type, bound, table)
 *
 *************/

struct fpa_tree *build_tree(t, u_type, bound, table)
struct term *t;
int u_type;
int bound;
struct fpa_head *table[];
{
    int path[MAX_PATH];

    if (t->type == VARIABLE && u_type != MORE_GEN)
	return(build_for_all(table));
    else
	return(build_tree_local(t, u_type, path, 0, bound, table));
}  /* build_tree */


/*************
 *
 *    struct term *next_term(tree, maximum)
 *
 *        Get the first or next term that satisfies a unification condition.
 *    (Unification conditions are provided by `build_tree'.)
 *    `maximum' must be 0 on nonresursive calls.  A return of NULL indicates
 *    that there are none or no more terms that satisfy (and the tree has
 *    been deallocated).  If you want to stop getting terms before a NULL
 *    is returned, then please deallocate the tree with zap_prop_tree(tree).
 *
 *    Warning: a return of NULL means that the tree has been deallocated
 *
 *************/

struct term *next_term(n, max)
struct fpa_tree *n;
int max;
{
    struct gen_ptr *tp;
    struct term *t1, *t2;
    
    if (!n)
	return(NULL);
    else if (n->type == LEAF) {
	tp = n->terms;  /* fpa lists: terms have decreasing addresses */
	while (tp && max != 0 && tp->u.t->fpa_id > max)
	    tp = tp->next;
	if (!tp) {
	    zap_prop_tree(n);
	    return(NULL);
	    }
	else {
	    n->terms = tp->next;
	    return(tp->u.t);
	    }
	}
    
    else if (n->type == INTERSECT) {
	t1 = next_term(n->left, max);
	if (t1)
	    t2 = next_term(n->right, t1->fpa_id);
	else
	    t2 = (struct term *) 1;  /* anything but NULL */
	while (t1 != t2 && t1 && t2) {
	    if (t1->fpa_id > t2->fpa_id)
		t1 = next_term(n->left, t2->fpa_id);
	    else
		t2 = next_term(n->right, t1->fpa_id);
	    }
	if (!t1 || !t2) {
	    if (!t1)
	       n->left = NULL;
	    if (!t2)
	       n->right = NULL;
	    zap_prop_tree(n);
	    return(NULL); 
	    }
	else
	    return(t1);
	}
    
    else {  /* UNION node */
	/* first get the left term */
	t1 = n->left_term;
	if (!t1) {
	    /* it must be brought up */
	    if (n->left) {
		t1 = next_term(n->left, max);
		if (!t1)
		    n->left = NULL;
		}
	    }
	else  /* it was saved from a previous call */
	    n->left_term = NULL;
	/* at this point, n->left_term == NULL */
    
	/* now do the same for the right side */
	t2 = n->right_term;
	if (!t2) {
	    if (n->right) {
		t2 = next_term(n->right, max);
		if (!t2)
		    n->right = NULL;
		}
	    }
	else
	    n->right_term = NULL;
    
	/* now decide which of of t1 and t2 to return */
	if (!t1) {
	    if (!t2) {
		zap_prop_tree(n);
		return(NULL);
		}
	    else
		return(t2);
	    }
	else if (!t2)
	    return(t1);
	else if (t1 == t2)
	    return(t1);
	else if (t1->fpa_id > t2->fpa_id) {
	    n->right_term = t2;  /* save t2 for next time */
	    return(t1);
	    }
	else {
	    n->left_term = t1;  /* save t1 for next time */
	    return(t2);
	    }
	}
}  /* next_term */

/*************
 *
 *    struct fpa_tree *build_for_all(table)
 *
 *    This is called when someone needs terms that unify with or are
 *    instances of a variable.
 *    Every term in an FPA index should be in a list whose path consists
 *    of one symbol.  Build a tree that UNIONs together all such FPA lists.
 *
 *************/

struct fpa_tree *build_for_all(table)
struct fpa_head *table[];
{
    struct fpa_head *h;
    struct fpa_tree *p1, *p2, *p3;
    int i;
    unsigned char *cpath;

    p1 = NULL;
    for (i = 0; i < FPA_SIZE; i++) {
	for (h = table[i]; h; h = h->next) {
	    cpath = (unsigned char *) h->path;
	    if (cpath[1] == 0) {  /* if path is for first symbol only */
		/* Actually, if a term has more than 255 arguments, this will */
		/* introduce some duplicates, but that's ok. */
		p2 = get_fpa_tree();
		p2->type = LEAF;
		p2->path = h->path;
		p2->terms = h->terms;
		if (!p1)
		    p1 = p2;
		else {
		    p3 = get_fpa_tree();
		    p3->type = UNION;
		    p3->left = p1;
		    p3->right = p2;
		    p1 = p3;
		    }
		}
	    }
	}
    return(p1);
}  /* build_for_all */

/*************
 *
 *    zap_prop_tree(tree) -- Deallocate an FPA indexing tree.
 *
 *       `next_term' deallocates the tree as it proceeds, so it is not
 *    necessary to call zap_prop_tree if the most recent call to
 *    `next_term' returned NULL.
 *
 *************/

void zap_prop_tree(n)
struct fpa_tree *n;
{
    if (n) {
	zap_prop_tree(n->left);
	zap_prop_tree(n->right);
	free_fpa_tree(n);
	}
}  /* zap_prop_tree */

/*************
 *
 *    print_fpa_tab(file_ptr, table) --  Display all FPA lists in table.
 *
 *************/

void print_fpa_tab(fp, table)
FILE *fp;
struct fpa_head *table[];
{
    int i;
    struct fpa_head *f;
    struct gen_ptr *tp;
    
    fprintf(fp, "\nfpa table %d\n", (int) table);
    for (i=0; i<FPA_SIZE; i++)
	if (table[i]) {
	    fprintf(fp, "bucket %d\n", i);
	    f = table[i];
	    while (f) {
		print_path(fp, f->path);
		tp = f->terms;
		while (tp) {
		    fprintf(fp, " ");
		    print_term(fp, tp->u.t);
		    tp = tp->next;
		    }
		fprintf(fp, "\n");
		f = f->next;
		}
	    }
}  /* print_fpa_tab */

/*************
 *
 *    p_fpa_tab(table)
 *
 *************/

void p_fpa_tab(table)
struct fpa_head *table[];
{
    print_fpa_tab(stdout, table);
}  /* p_fpa_tab */

/*************
 *
 *    print_prop_tree(file_ptr, tree, depth)
 *
 *        Display an FPA lookup tree that has been returned from
 *    build_tree.  Level should be 0 on initial call.
 *
 *************/

void print_prop_tree(fp, n, depth)
FILE *fp;
struct fpa_tree *n;
int depth;
{
    int i;
    
    if (n) {
      
	for (i=0; i<depth; i++)
	    fprintf(fp, "  ");
	if (n->type == INTERSECT)
	    fprintf(fp, "AND\n");
	else if (n->type == UNION)
	    fprintf(fp, "OR\n");
	else
	    print_path(fp, n->path);
	print_prop_tree(fp, n->left, depth+1);
	print_prop_tree(fp, n->right, depth+1);
	}
}  /* print_prop_tree */

/*************
 *
 *    p_prop_tree(t)
 *
 *************/

void p_prop_tree(n)
struct fpa_tree *n;
{
    print_prop_tree(stdout, n, 0);
    printf("\n");
}  /* p_prop_tree */

/*************
 *
 *    print_path(fp, path) -- print an fpa path to a file
 *
 *************/

void print_path(fp, path)
FILE *fp;
int *path;
{
    int i;
    char *sym;
    unsigned char *cpath;

    cpath = (unsigned char *) path;

    /* example [f,2,g,1,f,1,h,1,a] */
    
    fprintf(fp, "[");
    for (i = 0; i%2 == 0 || cpath[i] != 0 ; i++) {
	if (i % 2 == 0) {
	    if (cpath[i] == 0)
		sym = "*";
	    else
		sym = sn_to_str( (short) cpath[i]);
	    fprintf(fp, "%s", sym);
	    }
	else
	    fprintf(fp, "%d", cpath[i]);

	if (i%2 == 1 || cpath[i+1] != 0)
	    fprintf(fp, ",");
	else
	    fprintf(fp, "]\n");
	}
}  /* print_path */

/*************
 *
 *    p_path(path) -- print an fpa path 
 *
 *************/

void p_path(path)
int *path;
{
    print_path(stdout, path);
}  /* p_path */

/*************
 *
 *    fpa_retrieve_first
 *
 *************/

struct term *fpa_retrieve_first(t,index,type,subst_query,subst_found,ppos)
struct term *t;
struct fpa_index *index;
int type;
struct context *subst_query;
struct context *subst_found;
struct fpa_pos **ppos;
{
    struct fpa_pos *p;

    p = get_fpa_pos();
    p->query_term = t;
    p->type = type;
    p->subst_query = subst_query;
    p->subst_found = subst_found;
    p->tr = NULL;
    p->tree = build_tree(t, type, index->depth, index->table);
#if 0
    printf("whole tree:\n"); p_prop_tree(p->tree);
#endif    

    *ppos = p;

    return(fpa_retrieve_next(p));
    
}  /* fpa_retrieve_first */

/*************
 *
 *    fpa_retrieve_next
 *
 *************/

struct term *fpa_retrieve_next(pos)
struct fpa_pos *pos;
{
    struct term *tq, *tf;
    struct context *cq, *cf;
    struct trail *tr;
    int ok;

    tq = pos->query_term;
    cq = pos->subst_query; cf = pos->subst_found;

    clear_subst_1(pos->tr);
    tf = next_term(pos->tree, 0);
    ok = 0;
    while (tf && !ok) {
#if 0
	printf("potential mate: "); p_term(tf);
#endif	
	tr = NULL;
	switch (pos->type) {
	  case UNIFY:
	    ok = unify(tq, cq, tf, cf, &tr); break;
	  case MORE_GEN:
	    ok = match(tf, cf, tq, &tr); break;
	  case INSTANCE:
	    ok = match(tq, cq, tf, &tr); break;
	  default:
	    ok = 1; break;
	    }
	if (!ok)
	    tf = next_term(pos->tree, 0);
	}

    if (ok) {
	pos->tr = tr;
	return(tf);
	}
    else {
	free_fpa_pos(pos);
	return(NULL);
	}
}  /* fpa_retrieve_next */

/*************
 *
 *    fpa_cancel
 *
 *************/

void fpa_cancel(pos)
struct fpa_pos *pos;     
{
    zap_prop_tree(pos->tree);
    free_fpa_pos(pos);
}  /* fpa_cancel */

/*************
 *
 *    fpa_bt_retrieve_first
 *
 *************/

struct term *fpa_bt_retrieve_first(t,index,type,subst_query,subst_found,ppos)
struct term *t;
struct fpa_index *index;
int type;
struct context *subst_query;
struct context *subst_found;
struct fpa_pos **ppos;
{
    struct fpa_pos *p;

    p = get_fpa_pos();
    p->query_term = t;
    p->type = type;
    p->subst_query = subst_query;
    p->subst_found = subst_found;
    p->bt_position = NULL;
    p->tree = build_tree(t, type, index->depth, index->table);
#if 0
    printf("whole tree:\n"); p_prop_tree(p->tree);
#endif    

    *ppos = p;

    return(fpa_bt_retrieve_next(p));
    
}  /* fpa_bt_retrieve_first */

/*************
 *
 *    fpa_bt_retrieve_next
 *
 *************/

struct term *fpa_bt_retrieve_next(pos)
struct fpa_pos *pos;
{
    struct term *tq, *tf;
    struct context *cq, *cf;
    struct trail *tr;
    int ok;

    tq = pos->query_term;
    cq = pos->subst_query; cf = pos->subst_found;

    if (pos->bt_position) {
	switch (pos->type) {
	  case UNIFY:
	    pos->bt_position = unify_bt_next(pos->bt_position);
	    break;
	  case MORE_GEN:
	  case INSTANCE:
	    pos->bt_position = match_bt_next(pos->bt_position);
	    break;
	    }
	}
    
    if (!pos->bt_position) {
	tf = next_term(pos->tree, 0);
	while (tf && !pos->bt_position) {
#if 1
	    printf("potential mate: "); p_term(tf);
#endif	
	    switch (pos->type) {
	      case UNIFY:
		pos->bt_position = unify_bt_first(tq, cq, tf, cf);
		break;
	      case MORE_GEN:
		pos->bt_position = match_bt_first(tf, cf, tq, 0);
		break;
	      case INSTANCE:
		pos->bt_position = match_bt_first(tq, cq, tf, 0);
		break;
		}
	    if (!pos->bt_position)
		tf = next_term(pos->tree, 0);
	    else
		pos->found_term = tf;
	    }
	}

    if (pos->bt_position)
	return(pos->found_term);
    else {
	free_fpa_pos(pos);
	return(NULL);
	}
}  /* fpa_bt_retrieve_next */

/*************
 *
 *    fpa_bt_cancel
 *
 *************/

void fpa_bt_cancel(pos)
struct fpa_pos *pos;     
{
    zap_prop_tree(pos->tree);
    switch (pos->type) {
      case UNIFY: unify_bt_cancel(pos->bt_position); break;
      case MORE_GEN:
      case INSTANCE: match_bt_cancel(pos->bt_position); break;
	}
    free_fpa_pos(pos);
}  /* fpa_bt_cancel */



