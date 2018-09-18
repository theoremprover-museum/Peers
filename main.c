#include "p4.h"

#define IN_MAIN  /* so that global vars in header.h will not be external */
#include "Header.h"
#include "Unify.h"
#include "Index.h"
#include "Io.h"
#include "Peers.h"

#include <signal.h>  /* for call to signal */

/*************
 *
 *    void print_banner(argc, argv)
 *
 *************/

void print_banner(argc, argv)
int argc;
char *argv[];
{
    int i;
    char host[64];

    if (gethostname(host, 64) != 0)
	str_copy("???", host);

    printf("----- OPS 0.2, Sept. 1992 -----\nThe job began on %s, %s", host, get_time());

    printf("The command was \"");
    for(i = 0; i < argc; i++)
        printf("%s%s", argv[i], (i < argc-1 ? " " : ""));
    printf("\".\n\n");
    
}  /* print_banner */

/*************
 *
 *    void interrupt_interact()
 *
 *    This routine provides some primitive interaction with the user.
 *
 *************/

void interrupt_interact()
{
    FILE *fp;
    struct term *t;
    int rc, go_back;

    fp = fopen("/dev/tty", "r+");

    if (fp == NULL)
	printf("interaction failure: cannot find tty.\n");
    else {
	printf("\n --- BEGIN INTERACTION ---\n\n");
	fprintf(fp, "\n --- BEGIN INTERACTION ---\n");
	fprintf(fp, "Commands are kill, continue, set(...), assign(...,...), and stats.\n");
	fprintf(fp, "All commands must end with a period.\n> ");
	fflush(fp);
        fseek(fp, (long) 0, 2);  /* position at end of file */
	t = read_term(fp, &rc);
	fflush(fp);
	go_back = 0;
	while ((t != NULL || rc == 0) && !go_back) {
	    if (t == NULL)
		fprintf(fp, " malformed term; try again.\n> ");
	    else if (t->type == NAME || t->type == VARIABLE) {
		if (str_ident("stats", sn_to_str(t->sym_num))) {
		    output_stats(stdout);
		    fflush(stdout);
		    fprintf(fp, " ok.\n> ");
		    }
		else if (str_ident("kill", sn_to_str(t->sym_num))) {
		    printf("\nkilled during interaction.\n");
		    fprintf(stderr, "killed during interaction.\007\n");
		    fprintf(fp, " ok.");
		    fclose(fp);
		    output_stats(stdout);
		    exit(INTERRUPT_EXIT);
		    }
		else if (str_ident("continue", sn_to_str(t->sym_num))) {
		    fprintf(fp, " ok.");
		    go_back = 1;
		    }
		else
		    fprintf(fp, " command not understood.\n> ");
		}
	    else if (str_ident("set", sn_to_str(t->sym_num))) {
		if (change_flag(fp, t, 1)) {
		    print_term(stdout, t); printf(".\n");
		    fprintf(fp, " ok.\n> ");
		    }
		}
	    else if (str_ident("clear", sn_to_str(t->sym_num))) {
		if (change_flag(fp, t, 0)) {
		    print_term(stdout, t); printf(".\n");
		    fprintf(fp, " ok.\n> ");
		    }
		}
	    else if (str_ident("assign", sn_to_str(t->sym_num))) {
		if (change_parm(fp, t)) {
		    print_term(stdout, t); printf(".\n");
		    fprintf(fp, " ok.\n> ");
		    }
		}
	    else
		fprintf(fp, " command not understood.\n> ");
	    
	    if (t != NULL)
		zap_term(t);
	    if (go_back == 0) {
		fflush(fp);
		fseek(fp, (long) 0, 2);  /* position at end of file */
		t = read_term(fp, &rc);
		fflush(fp);
		}
	    }
	
	printf("\n --- end interaction ---\n\n");
	fprintf(fp,"\n --- end interaction ---\n");
	
	fclose(fp);
	}

}  /* interrupt_interact */

/*************
 *
 *    main
 *
 *************/

main(argc,argv)
int argc;
char *argv[];
{
    p4_initenv(&argc,argv);
#if 0    
    p4_set_dbg_level(0);
    signal(SIGINT,  interrupt_interact);
#endif    
    if (p4_get_my_id() == 0) {
	print_banner(argc, argv);
        peer_init_and_work();
	}
    else
        peer_work();
    p4_wait_for_end();
}  /* main */

