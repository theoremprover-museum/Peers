#include "Header.h"

/*************
 *
 *     list_append(clause, list)
 *
 *************/

void list_append(c, l)
struct literal *c;
struct list *l;
{
    struct list_pos *p;

    p = get_list_pos();
    p->container = l;
    p->lit = c;
    p->nocc = c->containers;
    c->containers = p;

    p->next = NULL;
    p->prev = l->last;
    l->last = p;
    if (p->prev)
	p->prev->next = p;
    else
	l->first = p;

}  /* list_append */

/*************
 *
 *     list_prepend(clause, list)
 *
 *************/

void list_prepend(c, l)
struct literal *c;
struct list *l;
{
    struct list_pos *p;

    p = get_list_pos();
    p->container = l;
    p->lit = c;
    p->nocc = c->containers;
    c->containers = p;

    p->prev = NULL;
    p->next = l->first;
    l->first = p;
    if (p->next)
	p->next->prev = p;
    else
	l->last = p;
    
}  /* list_prepend */

/*************
 *
 *     list_insert_before(clause, pos)
 *
 *************/

void list_insert_before(c, pos)
struct literal *c;
struct list_pos *pos;
{
    struct list_pos *p;

    p = get_list_pos();
    p->container = pos->container;
    p->lit = c;
    p->nocc = c->containers;
    c->containers = p;

    p->next = pos;
    p->prev = pos->prev;
    pos->prev = p;
    if (p->prev)
	p->prev->next = p;
    else
	pos->container->first = p;
    
}  /* list_insert_before */

/*************
 *
 *     list_insert_after(clause, pos)
 *
 *************/

void list_insert_after(c, pos)
struct literal *c;
struct list_pos *pos;
{
    struct list_pos *p;

    p = get_list_pos();
    p->container = pos->container;
    p->lit = c;
    p->nocc = c->containers;
    c->containers = p;

    p->prev = pos;
    p->next = pos->next;
    pos->next = p;
    if (p->next)
	p->next->prev = p;
    else
	pos->container->last = p;
    
}  /* list_insert_after */

/*************
 *
 *     list_remove(clause, list)
 *
 *     Return 1 if deleted.
 *     Return 0 if it couldn't be deleted because it's not there.
 *
 *************/

int list_remove(c, l)
struct literal *c;
struct list *l;
{
    struct list_pos *p, *prev;

    /* Find position from containment list of clause. */
    for (p = c->containers, prev = NULL;
	 p && p->container != l;
	 prev = p, p = p->nocc);
    
    if (p) {
	/* First update normal links. */
	if (p->prev)
	    p->prev->next = p->next;
	else
	    p->container->first = p->next;
	if (p->next)
	    p->next->prev = p->prev;
	else
	    p->container->last = p->prev;

	/* Now update containment links. */
	if (prev)
	    prev->nocc = p->nocc;
	else
	    c->containers = p->nocc;

	free_list_pos(p);
	return(1);
	}
    else
	return(0);
}  /* list_remove */

/*************
 *
 *     list_remove_all(clause)
 *
 *     Remove from all lists.
 *
 *************/

int list_remove_all(c)
struct literal *c;
{
    struct list_pos *p, *p2;

    p = c->containers;
    while (p) {
	if (p->prev)
	    p->prev->next = p->next;
	else
	    p->container->first = p->next;
	if (p->next)
	    p->next->prev = p->prev;
	else
	    p->container->last = p->prev;
	p2 = p;
	p = p->nocc;
	free_list_pos(p2);
	}
    c->containers = NULL;
}  /* list_remove_all */

/*************
 *
 *     list_member(c, l)
 *
 *************/

int list_member(c, l)
struct literal *c;
struct list *l;
{
    struct list_pos *p;
    int found;

    for (p = c->containers, found = 0; p && !found; p = p->nocc) {
	if (p->container == l)
	    found = 1;
	}
    return(found);
}  /* list_member */

/*************
 *
 *    print_list(fp, l)
 *
 *************/

void print_list(fp, l)
FILE *fp;
struct list *l;
{
    struct list_pos *p;

    for(p = l->first; p; p = p->next)
	print_clause(fp, p->lit);
    fprintf(fp, "end_of_list.\n");
}  /* print_list */

/*************
 *
 *    p_list(l)
 *
 *************/

void p_list(l)
struct list *l;
{
    print_list(stdout, l);
}  /* p_clause */

/*************
 *
 *     list_zap(l)
 *
 *     For each element, remove it, and if it occurs nowhere else, delete it.
 *
 *************/

void list_zap(l)
struct list *l;
{
    struct list_pos *p;
    struct literal *lit;

    p = l->first;
    while (p) {
	lit = p->lit;
	p = p->next;
	list_remove(lit, l);
	if (!lit->containers)
	    zap_lit(lit);
	}
}  /* list_zap */

/*************
 *
 *     list_check(l)
 *
 *************/

void list_check(l)
struct list *l;     
{
    struct list_pos *p;

    for (p = l->first; p; p = p->next) {
	if (p->container != l)
	    printf("error0\n");
	if (p->next) {
	    if (p->next->prev != p)
		printf("error1\n");
	    }
	else if (p != l->last)
	    printf("error2\n");
	if (p->prev) {
	    if (p->prev->next != p)
		printf("error3\n");
	    }
	else if (p != l->first)
	    printf("error4\n");
	}
}  /* list_check */

/*************
 *
 *   clause_in_list(c, l)
 *
 *************/

int clause_in_list(c, l)
struct literal *c;
struct list *l;
{
    struct list_pos *p;
    int found;

    for (p = l->first, found = 0; p && !found; p = p->next)
	found = clause_ident(c, p->lit);
    return(found);
}  /* clause_in_list */

