/*  Things to do

1. initialize Process_status, deadlock???      DONE
2. why do all peers need to call init() ?
        because on symmetry, they need to reinitialize after the fork.
4. SELECT_MIN in decide_owner.			DONE
5. function to set d_strategy.
6. cancellation demodulates 0+x=x on input.    DONE (in a bad way)
7. clause <-> string, add birth_time, weight.  DONE
8. generate clauses with given clause alg.     DONE
10. extract_given_clause.                      DONE
11. /Back_demodulated/Deleted_clauses/         DONE
12. birth time.                                DONE
13. Fix if (Flags[POST_PROCESS_NS].val)
14. Still not quite sure about IDs		DONE
15. Wall clock times sometimes bad.
16. when sos empty in all peers, not all terminate.  see peer[01].g1.
17. ac matching bug in r2.in.			DONE
18. with 1 peer, when sos empties, it fails to terminate.  DONE
19. when process_new_clause does not check for HALT_MSG, things don't seem to terminate. DONE?
20. check that given clause (or pairs) have not been deleted by its child.
21. select_min doesn't keep residents balanced.

questions

*/

#include "Header.h"
#include "Io.h"
#include "Unify.h"
#include "Index.h"
#include "Paramod.h"
#define OWNER_OF_PEER_GLOBALS
#include "Peers.h"
#define ALOG_TRACE
#include "p4.h"

/*************
 *
 *   slave()
 *
 *************/

void slave()
{
    printf("Entering slave routine and calling peer_work.\n");
    peer_work();
}  /* slave */

/*************
 *
 *    peer_init_and_work
 *
 *************/

void peer_init_and_work()
{
    int i;
    struct list_pos *p;
    struct literal *c;
    int i0, i1;
    struct list *usable, *sos, *passive, *demodulators;
	/* temp lists for the input phase */

    ALOG_ENABLE;
    ALOG_MASTER(0, ALOG_TRUNCATE);

    ALOG_DEFINE(LOG_START_EXPANSION, "Start Expansion", "");
    ALOG_DEFINE(LOG_STOP_EXPANSION, "Stop Expansion", "");
    ALOG_DEFINE(LOG_START_NEW_SETTLER, "Start New Settler", "");
    ALOG_DEFINE(LOG_STOP_NEW_SETTLER, "Stop New Settler", "");
    ALOG_DEFINE(LOG_START_INFERENCE_MSG, "Start Inference Message", "");
    ALOG_DEFINE(LOG_STOP_INFERENCE_MSG, "Stop Inference Message", "");

    ALOG_DEFINE(LOG_SEND_NS, "Send new setter msg", "");
    ALOG_DEFINE(LOG_RECV_NS, "Receive new setter msg", "");
    ALOG_DEFINE(LOG_BROADCAST_IM, "Broadcast inference msg", "");
    ALOG_DEFINE(LOG_RECV_IM, "Receive inference msg", "");

    ALOG_DEFINE(LOG_SOS_RES_SIZE, "Sos residents", "");
    ALOG_DEFINE(LOG_SOS_VIS_SIZE, "Sos visitors", "");

    ALOG_DEFINE(LOG_SYNC, "Synchronize", "");

    init();  /* OPS init */
    
    Lid_counter = 0;
    Pseudo_clock = 0;
    Msgs_diff = 0;
    Have_token = 1;	/* Peer 0 has the token at the beginning. */
    How_many_times_token_received = 0;

    Deleted_clauses = get_list();
    Demod_index = get_discrim();
    init_clause_table_name(Name_table);
    init_clause_table_id(Id_table);
    init_clause_table_id(Already_broadcast_table);

    Pid = 0;		/* this code is executed only by peer0 */

    read_preamble();  /* read options */
    
    if (Parms[BACK_DEMOD_STRAT].val == ALL_RAW || Parms[BACK_DEMOD_STRAT].val == R_PIDCBT_IM_RAW)
        Flags[NAME_STRATEGIES].val = 0;

/* The default is Flags[NAME_STRATEGIES].val == 1, but it is turned off */
/* if it is not a name_strategy.					*/

    /* Read input clauses into temporary lists. */

    usable = get_list();
    sos = get_list();
    demodulators = get_list();
    passive = get_list();

    read_lists(stdin, usable, sos, passive, demodulators);

    if (Stats[INPUT_ERRORS] != 0) {
	fprintf(stderr, "\nInput errors were found.\007\n\n");
	printf("Input errors were found.\n");
	return;
	}

    /* Create all other peers. */

    i0 = p4_clock();
    p4_create_procgroup();
    i1 = p4_clock();
    Number_of_peers = p4_num_total_slaves() + 1;

    printf("\nPeer0 created %d additional peers in %.2f seconds.\n",
	   Number_of_peers-1, (i1-i0)/1000.);
    printf("run_time=%.2f, system_time=%.2f.\n\n",
	   run_time()/1000., system_time()/1000.);
    fprintf(stderr, "Procgroup created.\007\n");

    /* Process input clauses and decide owners. */

    set_clauses(usable, sos, passive, demodulators);

    /* Broadcast a new set of options to the peers. */

    options_to_string(Work_str, MAX_DAC_STRING);
    m4_broadcast(OPTIONS_MSG, Work_str, strlen(Work_str)+1);
    Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */

    /* Broadcast a new symbol table to the peers. */

    sym_tab_to_string(Work_str, MAX_DAC_STRING);
    string_to_sym_array(Work_str, Symbol_array, MAX_SYMBOL_ARRAY);
    m4_broadcast(SYM_TAB_MSG, Work_str, strlen(Work_str)+1);
    Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */

	/* Broadcast Pseudo_clock to the peers to init their		*/
	/* Lid_counter and Pseudo_clock.				*/
	/* It is important to broadcast Pseudo_clock before the 	*/
	/* input clauses, to avoid that another peer starts using 	*/
	/* the input clauses to generate raw clauses before having 	*/
	/* received the appropriate values to initialize its counters 	*/
	/* in such a way to avoid conflicts.				*/
	
    sprintf(Work_str, "%d ", Pseudo_clock);
    m4_broadcast(CLOCK_MSG, Work_str, strlen(Work_str)+1);
    Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */

        /* Broadcast the input clauses to the peers. */

    for (p = Sos->first; p; p = p->next) {
	c = p->lit;
	clause_to_string(c, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_SOS_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
	store_clause_by_id(c, Already_broadcast_table);
	}
    for (p = Usable->first; p; p = p->next) {
	c = p->lit;
	clause_to_string(c, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_USABLE_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
	store_clause_by_id(c, Already_broadcast_table);
	}

    /* Must broadcast demods after usable and after sos.	*/
    /* This is to handle the case where an equation c in	*/
    /* Sos or Usable has been turned into a demod.		*/
    /* By broadcasting Sos and Usable first, we make sure	*/
    /* that c will be received first and thus stored by the	*/
    /* other peers with the name and id set for its occurrence	*/
    /* in Sos or Usable.					*/ 

    for (p = Demodulators->first; p; p = p->next) {
	clause_to_string(p->lit, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_DEMOD_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
	}
    for (p = Passive->first; p; p = p->next) {
	clause_to_string(p->lit, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_PASSIVE_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
	}
    
    peer_work();

    printf("peer_init_and_work exiting normally\n");

}  /* peer_init_and_work */

/*************
 *
 *    peer_work()
 *
 *************/

void peer_work()
{
    int i;
    char s1[MAX_NAME], hostname[64];

    /* Peer initialization. */

    Pid = p4_get_my_id();
    Number_of_peers = p4_num_total_slaves() + 1;

    ALOG_SETUP(Pid, ALOG_TRUNCATE);

    if (gethostname(hostname, 64) != 0)
	str_copy("???", hostname);

    sprintf(s1,"peer%d", Pid);
    Peer_fp = fopen(s1, "w");
    fprintf(Peer_fp, "Peer %d (of %d), host %s, %s\n\n",
	    Pid, Number_of_peers, hostname, get_time());
    fflush(Peer_fp);

    if (Pid != 0) {
	init();  /* OPS init */
	
	Msgs_diff = 0;
	Have_token = 0;
	How_many_times_token_received = 0;

	Usable = get_list();
	Sos = get_list();
	Passive = get_list();
	Demodulators = get_list();
	Deleted_clauses = get_list();
	Demod_index = get_discrim();
	init_clause_table_name(Name_table);
	init_clause_table_id(Id_table);
	init_clause_table_id(Already_broadcast_table);
	}
    
    work_loop();

#if 0
    print_clause_table_id(Peer_fp, Id_table);
    if (Flags[NAME_STRATEGIES].val)
        print_clause_table_name(Peer_fp, Name_table);
#endif
    if (Flags[PRINT_LISTS_AT_END].val) {
	fprintf(Peer_fp, "\nUsable:\n");
	print_list(Peer_fp, Usable);
	fprintf(Peer_fp, "\nSos:\n");
	print_list(Peer_fp, Sos);
	fprintf(Peer_fp, "\nDemodulators:\n");
	print_list(Peer_fp, Demodulators);
	}
	
    print_stats(Peer_fp);
    print_times(Peer_fp);
    fprintf(Peer_fp, "\nPeer %d finishes work_loop.\n", Pid);
    fprintf(Peer_fp, "THE END\n");
    fflush(Peer_fp);
    ALOG_OUTPUT;

}  /* peer_work */

/*************
 *
 *   work_available(type)
 *
 *   This routine determines if there is work of a given type available.
 *   `Work' is either an incoming message to process or something the
 *   process decides to do for itself.
 *
 *************/

int work_available(type)
int type;
{
    switch (type) {
      case HALT_MSG:
      case OPTIONS_MSG: 
      case SYM_TAB_MSG:    
      case INPUT_SOS_MSG:
      case INPUT_PASSIVE_MSG:
      case INPUT_DEMOD_MSG:
      case INPUT_USABLE_MSG:
      case CLOCK_MSG:
      case INFERENCE_MSG:
      case NEW_SETTLER_MSG:
      case TOKEN_MSG:
      	return(m4_messages_available(type));
      case EXPANSION_WORK:
	return(expansion_work_available());
      default:
	fprintf(Peer_fp, "work_available, bad type=%d.\n", type);
	abend("work_available, bad work type");
	return(0);  /* to quiet lint */
	}
}  /* work_available */

/*************
 *
 *   find_high_priority_work()
 *
 *   This process finds the type of the highest-priority work that is
 *   available.  If no work is available, it returns -1 (which usually
 *   means that the process should wait for a message).
 *
 *   The priorities are specified with #define statements, with the n types
 *   assigned 1,2,...,n  (1 is highest priority).  n is NUMBER_OF_WORK_TYPES.
 *
 *************/

int find_high_priority_work()
{
    int i;
    for (i=1; i<=NUMBER_OF_WORK_TYPES; i++)
	if (work_available(i))
	    break;
    return(i>NUMBER_OF_WORK_TYPES ? -1 : i);
}  /* find_high_priority_work */

/*************
 *
 *   work_loop()
 *
 *************/

void work_loop()
{
    int go, work_type, from, size, to;
    char *incoming, *s;
    struct literal *lit;

    ALOG_LOG(Pid, LOG_SYNC, 0, "");

    go = 1;
    while (go) {
	work_type = find_high_priority_work();
	switch (work_type) {

	    /* cases for `message' work */

	  case -1:  /* No messages available.  Wait and receive any message. */

	    if (Number_of_peers == 1) {
		fprintf(Peer_fp, "Peer 0 stops, because it's idle, and there are no other peers.\n");
		fflush(Peer_fp);
		go = 0;
		break;
		}
	    else if (Pid == 0 && Have_token && Msgs_diff == 0 && How_many_times_token_received >= 2) {
		    fprintf(Peer_fp, "\nPeer %d stops, because no one is busy.\n", Pid);
		    fflush(Peer_fp);
		    m4_broadcast(HALT_MSG, "", 0);
		    go = 0;
		    break;
		    }
		  else if (Have_token) {
	    		to = (Pid + 1) % Number_of_peers;
    			sprintf(Work_str, "%d ", Msgs_diff);
	    		p4_send(TOKEN_MSG, to, Work_str, strlen(Work_str)+1);
			Msgs_diff = 0;
/* It resets to 0 the local counter, having sent its value in the token. */
			Have_token = 0;
		        fprintf(Peer_fp, "\nPeer %d sends token message.\n", Pid);
		        fflush(Peer_fp);
			  }
	    
	  case HALT_MSG:        
	  case OPTIONS_MSG: 
	  case SYM_TAB_MSG:    
	  case INPUT_SOS_MSG:
	  case INPUT_PASSIVE_MSG:
	  case INPUT_DEMOD_MSG:
	  case INPUT_USABLE_MSG:
	  case CLOCK_MSG:	  
	  case INFERENCE_MSG:
	  case NEW_SETTLER_MSG:
	  case TOKEN_MSG:

	    /* This part handles all message types, including wait-for-mess */

	    from = -1;
	    incoming = NULL;
	    p4_recv(&work_type, &from, &incoming, &size);

	    switch (work_type) {

	      case HALT_MSG:
		go = 0;
		fprintf(Peer_fp, "\nPeer %d receives halt message in work_loop from Peer %d.\n", Pid, from);
		fflush(Peer_fp);
		break;

	      case TOKEN_MSG:
		Have_token = 1;
		How_many_times_token_received++;
		s = incoming;
		Msgs_diff = Msgs_diff + next_int(&s);
		fprintf(Peer_fp, "Peer %d gets token message from %d.\n", Pid, from);
		fflush(Peer_fp);
		break;

	      case SYM_TAB_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		string_to_sym_array(incoming, Symbol_array, MAX_SYMBOL_ARRAY);
		init_symbol_table();
		sym_array_to_normal_sym_tab(Symbol_array, MAX_SYMBOL_ARRAY);
		fprintf(Peer_fp, "Peer %d gets new symbols.\n", Pid);
		fflush(Peer_fp);
		break;

	      case OPTIONS_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */

    if (Parms[BACK_DEMOD_STRAT].val == ALL_RAW || Parms[BACK_DEMOD_STRAT].val == R_PIDCBT_IM_RAW)
        Flags[NAME_STRATEGIES].val = 0;

/* The default is Flags[NAME_STRATEGIES].val == 1, but it is turned off */
/* if it is not a name_strategy.					*/

		string_to_options(incoming);
		fprintf(Peer_fp, "Peer %d gets new options.\n", Pid);
		fflush(Peer_fp);
		break;

	      case CLOCK_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		s = incoming;
		Lid_counter = next_int(&s);
		Pseudo_clock = Lid_counter;
		fprintf(Peer_fp, "Peer %d gets Pseudo_clock=%d.\n",
			Pid, Pseudo_clock);
		fflush(Peer_fp);
		break;

	      case INPUT_PASSIVE_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input passive: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		list_append(lit, Passive);
		store_clause_by_id(lit, Id_table);
		if (Flags[NAME_STRATEGIES].val)
		   store_clause_by_name(lit, Name_table);
		break;

	      case INPUT_DEMOD_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input demod: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		list_append(lit, Demodulators);
		discrim_wild_insert(lit->atom->farg->argval, Demod_index, (void *) lit);
		if (Flags[TWO_WAY_DEMOD].val)
		    discrim_wild_insert(lit->atom->farg->narg->argval, Demod_index, (void *) lit);
		if (!find_clause_by_id(lit->id, Id_table)) {
		    store_clause_by_id(lit, Id_table);
		    if (Flags[NAME_STRATEGIES].val)
		        store_clause_by_name(lit, Name_table);
		    }
		break;

	      case INPUT_USABLE_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input usable: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		if (Flags[OWN_IN_USABLE].val) {
		    lit->id = encode_ids(Pid, birth_time(lit));
		    if (Flags[NAME_STRATEGIES].val)
		        lit->name = encode_ids(Pid, lid(lit));
		    }
		list_append(lit, Usable);
		store_clause_by_id(lit, Id_table);
		if (Flags[NAME_STRATEGIES].val)
		    store_clause_by_name(lit, Name_table);
		store_clause_by_id(lit, Already_broadcast_table);
		if (Flags[PARA_PAIRS].val) {
		    struct list_pos *p;
		    for (p = Usable->first; p; p = p->next) {
			/* make sure that at least one of the two clauses (equations) in the pair is a resident */
			if (mine(lit) || mine(p->lit))
			    store_in_pp(lit, p->lit);
			}
		    }
		if (mine(lit))
		    Stats[RESIDENTS]++;
		break;

	      case INPUT_SOS_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input sos: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		if (Flags[OWN_IN_SOS].val) {
		    lit->id = encode_ids(Pid, birth_time(lit));
		    if (Flags[NAME_STRATEGIES].val)
		        lit->name = encode_ids(Pid, lid(lit));
		    }
		store_new_clause(lit);
		store_clause_by_id(lit, Id_table);
		if (Flags[NAME_STRATEGIES].val)
		    store_clause_by_name(lit, Name_table);
		store_clause_by_id(lit, Already_broadcast_table);
		if (mine(lit))
		    Stats[RESIDENTS]++;
		break;

	      case INFERENCE_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
                ALOG_LOG(Pid, LOG_RECV_IM, 0, "");
		Stats[IM_RECEIVED]++;
		
		CLOCK_START(INF_MSG_TIME);
                ALOG_LOG(Pid, LOG_START_INFERENCE_MSG, 0, "");
		handle_inference_msg(incoming,from);
                ALOG_LOG(Pid, LOG_STOP_INFERENCE_MSG, 0, "");
		CLOCK_STOP(INF_MSG_TIME);
		break;

	      case NEW_SETTLER_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		ALOG_LOG(Pid, LOG_RECV_NS, 0, "");
		Stats[NS_RECEIVED]++;

		CLOCK_START(NEW_SETTLER_TIME);
                ALOG_LOG(Pid, LOG_START_NEW_SETTLER, 0, "");
		handle_new_settler_msg(incoming);
                ALOG_LOG(Pid, LOG_STOP_NEW_SETTLER, 0, "");
		CLOCK_STOP(NEW_SETTLER_TIME);
		break;
	
		}
	    p4_msg_free(incoming);
	    break;

            /* end of message processing */
	       
	    /* cases for local work */

	  case EXPANSION_WORK:
		ALOG_LOG(Pid, LOG_SOS_RES_SIZE, Stats[RESIDENTS_IN_SOS], "");
		ALOG_LOG(Pid, LOG_SOS_VIS_SIZE, Stats[VISITORS_IN_SOS], "");
		CLOCK_START(EXPANSION_TIME);
		ALOG_LOG(Pid, LOG_START_EXPANSION, 0, "");
		if (Flags[PARA_PAIRS].val)
		    expansion_work_pairs();
		else
		    expansion_work_given();
		ALOG_LOG(Pid, LOG_STOP_EXPANSION, 0, "");
		CLOCK_STOP(EXPANSION_TIME);
	    break;

            /* end of local work */

	    }
	}
    ALOG_LOG(Pid, LOG_SYNC, 0, "");
}  /* work_loop */

/*************
 *
 *    forward_subsume(lit)
 *
 *************/

int forward_subsume(lit)
struct literal *lit;
{
    struct term *t1, *t2;
    struct literal *subsumer;

    if (lit->sign && is_symbol(lit->atom, "=", 2)) {
	t1 = lit->atom->farg->argval;
	t2 = lit->atom->farg->narg->argval;
	if (term_ident(t1,t2))
	    return(1);
	}

    subsumer = subsume_list(lit, Passive);
    if (!subsumer)
	subsumer = subsume_list(lit, Usable);
    if (!subsumer)
	subsumer = subsume_list(lit, Sos);

    return(subsumer != NULL);

}  /* forward_subsume */

/*************
 *
 *    check_for_proof(lit)
 *
 *************/

void check_for_proof(lit)
struct literal *lit;
{
    struct term *t1, *t2;
    struct context *c1, *c2;
    struct bt_node *bt_position;
    struct literal *conflictor;
    int xx_proof;

    xx_proof = 0;

    if (!lit->sign && is_symbol(lit->atom, "=", 2)) {
        c1 = get_context();
        c2 = get_context();
	t1 = lit->atom->farg->argval;
	t2 = lit->atom->farg->narg->argval;
	bt_position = unify_bt_first(t1, c1, t2, c1);
	if (bt_position) {
	    xx_proof = 1;
	    unify_bt_cancel(bt_position);
	    }
	free_context(c1);
	free_context(c2);
	}

    if (xx_proof)
	conflictor = NULL;
    else {
	conflictor = conflict_list(lit, Passive);
	if (!conflictor)
	    conflictor = conflict_list(lit, Usable);
	if (!conflictor)
	    conflictor = conflict_list(lit, Sos);
	}

    if (xx_proof || conflictor) {

	if (birth_time(lit) == 0) {
	    /* lit would have been sent as a new settler, but we */
	    /* assign id fields so that the proof is printed properly. */
	    lit->id = encode_ids(Pid, ++Pseudo_clock);
	    if (Flags[NAME_STRATEGIES].val)
		lit->name = encode_ids(Pid, ++Lid_counter);
	    }
	
	fprintf(stderr, "---------------- PROOF ----------------");
	fprintf(stderr, "\007\n\n");
	if (conflictor)
	    fprintf(Peer_fp, "\nUNIT CONFLICT from %d:%d and %d:%d",
		   owner(lit), birth_time(lit),
		   owner(conflictor), birth_time(conflictor));
	else
	    fprintf(Peer_fp, "\nUNIT CONFLICT from %d:%d and x=x",
		   owner(lit), birth_time(lit));

	fprintf(Peer_fp, " at %6.2f seconds.\n", run_time() / 1000.);

	Stats[PROOFS]++;

        print_proof(Peer_fp, lit, conflictor);
	fflush(Peer_fp);

	if (Stats[PROOFS] >= Parms[MAX_PROOFS].val) {
#if 0
	    print_clause_table_id(Peer_fp, Id_table);
	    if (Flags[NAME_STRATEGIES].val)
		print_clause_table_name(Peer_fp, Name_table);
#endif	    
	    m4_broadcast(HALT_MSG, "", 0);
	    if (Flags[PRINT_LISTS_AT_END].val) {
		fprintf(Peer_fp, "\nUsable:\n");
		print_list(Peer_fp, Usable);
		fprintf(Peer_fp, "\nSos:\n");
		print_list(Peer_fp, Sos);
		fprintf(Peer_fp, "\nDemodulators:\n");
		print_list(Peer_fp, Demodulators);
		}
	    print_stats(Peer_fp);
	    print_times(Peer_fp);
	    fprintf(Peer_fp, "\nPeer %d found a proof.\n", Pid);
	    fprintf(Peer_fp, "THE END\n");
	    fflush(Peer_fp);
	    ALOG_OUTPUT;
	    p4_wait_for_end();
	    exit(PROOF_EXIT);
	    }
	}
}  /* check_for_proof */

/*************
 *
 *    term_list_to_literal_list(p, l)
 *
 *************/

void term_list_to_literal_list(p, l)
struct gen_ptr *p;
struct list *l;
{
    struct gen_ptr *p1;
    struct term *t;
    struct literal *lit;

    while (p) {
	t = p->u.t;
	lit = get_literal();
	lit->sign = !is_symbol(t, "-", 1);
	if (!lit->sign) {
	    lit->atom = t->farg->argval;
	    free_rel(t->farg);
	    free_term(t);
	    }
	else
	    lit->atom = t;
	lit->weight = -1;
	list_append(lit, l);
	p1 = p;
	p = p->next;
	free_gen_ptr(p1);
	}
}  /* term_list_to_literal_list */

/*************
 *
 *     read_lists(fp, usable, sos, passive, demodulators)
 *
 *************/

read_lists(fp, usable, sos, passive, demodulators)
FILE *fp;
struct list *usable, *sos, *passive, *demodulators;
{
    struct term *t;
    int rc;
    struct gen_ptr *p;

    t = read_term(stdin, &rc);

    while (t || rc == 0) {
        if (rc == 0)
	    Stats[INPUT_ERRORS]++;
        else if (!is_symbol(t, "list", 1)) {
	    printf("bad list command: ");
	    p_term(t);
	    Stats[INPUT_ERRORS]++;
	    }
	else {
	    p = read_list(fp, &rc);
	    Stats[INPUT_ERRORS] += rc;
	    if (str_ident(sn_to_str(t->farg->argval->sym_num), "usable"))
		term_list_to_literal_list(p, usable);
	    else if (str_ident(sn_to_str(t->farg->argval->sym_num), "sos"))
		term_list_to_literal_list(p, sos);
	    else if (str_ident(sn_to_str(t->farg->argval->sym_num), "passive"))
		term_list_to_literal_list(p, passive);
	    else if (str_ident(sn_to_str(t->farg->argval->sym_num), "demodulators"))
		term_list_to_literal_list(p, demodulators);
	    else {
		printf("bad list command: ");
		p_term(t);
		Stats[INPUT_ERRORS]++;
		}
	    }
	t = read_term(stdin, &rc);
	}
	
    if (Stats[INPUT_ERRORS] == 0) {
	printf("\nUsable:\n");
	p_list(usable);
	printf("\nSos:\n");
	p_list(sos);
	printf("\nDemodulators:\n");
	p_list(demodulators);
	printf("\nPassive:\n");
	p_list(passive);
	}
}  /* read_lists */

/*************
 *
 *     set_clauses()
 *
 *************/

void set_clauses(usable, sos, passive, demodulators)
struct list *usable, *sos, *passive, *demodulators;
{
    struct list_pos *p, *q;
    struct literal *lit;
    
    Usable = get_list();
    Sos = get_list();
    Demodulators = get_list();
    Passive = passive; /* Passive list doesn't change, so just move the pointer. */

    for (p = Passive->first; p; p = p->next) {
	lit = p->lit;
	ac_canonical(lit->atom);
	lit->id = encode_ids(0, ++Pseudo_clock);
        store_clause_by_id(lit, Id_table);
	if (Flags[NAME_STRATEGIES].val) {
	    lit->name = encode_ids(0, ++Lid_counter);
            store_clause_by_name(lit, Name_table);
	    }
	}

	/* Input clauses in Passive belong to Peer0, by default.        */
	/* Since they are used only for forward subsumption and         */
	/* unit_conflict, ownership does not affect their usage.        */
	/* Same remark applies to input Demodulators below.             */
 
    p = usable->first;
    while(p) {
	q = p;
	p = p->next;
	lit = q->lit;
	list_remove(lit, usable);
	process_input_clause(lit, Usable);
	}
    free_list(usable);

    p = sos->first;
    while(p) {
	q = p;
	p = p->next;
	lit = q->lit;
	list_remove(lit, sos);
	process_input_clause(lit, Sos);
	}
    free_list(sos);

    p = demodulators->first;
    while(p) {
	q = p;
	p = p->next;
	lit = q->lit;
	ac_canonical(lit->atom);
	lit->id = encode_ids(0, ++Pseudo_clock);
        store_clause_by_id(lit, Id_table);
	if (Flags[NAME_STRATEGIES].val) {
	    lit->name = encode_ids(0, ++Lid_counter);
            store_clause_by_name(lit, Name_table);
	    }
        discrim_wild_insert(lit->atom->farg->argval, Demod_index, (void *) lit);
        if (Flags[TWO_WAY_DEMOD].val)
            discrim_wild_insert(lit->atom->farg->narg->argval, Demod_index, (void *) lit);
	list_remove(lit, demodulators);
	list_append(lit, Demodulators);
	}
    free_list(demodulators);

    if (Flags[PARA_PAIRS].val) {
        for (p = Sos->first; p; p = p->next) {
            list_append(p->lit, Usable);
            for (q = Usable->first; q; q = q->next) {
		/* make sure that at least one of the two equations in the pair is a resident */
		if (mine(p->lit) || mine(q->lit))
		    store_in_pp(p->lit, q->lit);
		}
            }
        list_zap(Sos);
	}

    printf("\nAfter processing input:\n");
    printf("\nUsable:\n");
    p_list(Usable);
    printf("\nSos:\n");
    p_list(Sos);
    printf("\nDemodulators:\n");
    p_list(Demodulators);
    printf("\nPassive:\n");
    p_list(Passive);
    
}  /* set_clauses */

/*************
 *
 *   decide_owner(lit, lst)
 *
 *
 *   lst   Sos    -- input Sos clause
 *         Usable -- input Usable clause
 *         NULL   -- not an input clause
 *   
 *
 *************/

int decide_owner(lit, lst)
struct literal *lit;
struct list *lst;
{
    static int next_choice = -1;

    if (next_choice == -1)
	next_choice = Pid;	/* The first choice is oneself. */

    if (Number_of_peers == 1)
        return(Pid);
    else if (Flags[OWN_IN_SOS].val && lst == Sos)
	return(Pid);
    else if (Flags[OWN_IN_USABLE].val && lst == Usable)
	return(Pid);
    else {

	switch (Parms[DECIDE_OWNER_STRAT].val) {
	  case ROTATE:
	    {
	    int cand;
	    cand = next_choice++ % Number_of_peers;
	    return(cand);
	    }
	  case SYNTAX:
	    return(clause_to_peer_function(lit));
	  case SELECT_MIN:
	    {
	    int cand, j, min;
	    
	    min = MAX_INT;
	    for (j = 0; j < Number_of_peers; j++)
		if (Peers_pseudo_clocks[j] < min) {
		    min = Peers_pseudo_clocks[j];
		    cand = j;
		    }
	    return(cand);
	    }
	  default:
	    abend("decide_owner, bad decide_owner_strategy");
	    return(0);  /* to quiet lint */
	    }
	}
}  /* decide_owner */

/*************
 *
 *    process_input_clause(lit, lst)
 *
 *************/

void process_input_clause(lit, lst)
struct literal *lit;
struct list *lst;
{
    int oriented, owner;

    ac_canonical(lit->atom);
    demodulate_literal(lit);
    oriented = orient_equality(lit);

    lit->weight = term_weight(lit->atom);

    if (forward_subsume(lit)) {
        printf("subsumed on input: "); p_clause(lit);
        Stats[CLAUSES_FORWARD_SUBSUMED]++;
        zap_lit(lit);
        }
    else {
        renumber_vars(lit);  /* This can make it non-ac-canonical. */
        ac_canonical(lit->atom);

        owner = decide_owner(lit, lst);

        lit->id = encode_ids(owner, ++Pseudo_clock);
        store_clause_by_id(lit, Id_table);

	if (Flags[NAME_STRATEGIES].val) {
            lit->name = encode_ids(owner, ++Lid_counter);
            store_clause_by_name(lit, Name_table);
	    }

	/* If PARA_PAIRS, handle para_pair storing after processing input clauses. */

	if (mine(lit))
	    Stats[RESIDENTS]++;

	if (lst == Sos) {
	    insert_literal_by_weight(lit, Sos);
	    if (mine(lit))
		Stats[RESIDENTS_IN_SOS]++;
	    else
		Stats[VISITORS_IN_SOS]++;
	    }
	else
	    list_append(lit, Usable);

        check_for_proof(lit);

        if ((pos_eq(lit) && !Flags[CHECK_INPUT_DEMODULATORS].val) ||
	    (oriented && oriented_eq_can_be_demod(lit)))
	    new_demodulator_input(lit);
        }
}  /* process_input_clause */

/*************
 *
 *    process_new_clause(lit)
 *
 *************/

void process_new_clause(lit)
struct literal *lit;     
{
    int oriented, owner, from, type;

    Stats[CLAUSES_GENERATED]++;
    if (Flags[PRINT_GEN].val) {
        printf("Processing #%d: ", Stats[CLAUSES_GENERATED]);
        p_clause(lit);
        }
    
    /* This should be done often, and this seems like a good place. */
    from = -1;
    type = HALT_MSG;
    if (p4_messages_available(&type,&from)) {
#if 0	
	print_clause_table_id(Peer_fp, Id_table);
	if (Flags[NAME_STRATEGIES].val)
	    print_clause_table_name(Peer_fp, Name_table);
#endif	
	fprintf(Peer_fp, "\nPeer %d receives halt message in process_new_clause from Peer %d.\n", Pid, from);
	if (Flags[PRINT_LISTS_AT_END].val) {
	    fprintf(Peer_fp, "\nUsable:\n");
	    print_list(Peer_fp, Usable);
	    fprintf(Peer_fp, "\nSos:\n");
	    print_list(Peer_fp, Sos);
	    fprintf(Peer_fp, "\nDemodulators:\n");
	    print_list(Peer_fp, Demodulators);
	    }
	print_stats(Peer_fp);
	print_times(Peer_fp);
	fprintf(Peer_fp, "THE END\n");
	fflush(Peer_fp);
	ALOG_OUTPUT;
	p4_wait_for_end();
	exit(0);
	}
    
    ac_canonical(lit->atom);
    demodulate_literal(lit);
    oriented = orient_equality(lit);

    lit->weight = term_weight(lit->atom);

    if (lit->weight > Parms[MAX_WEIGHT].val) {
        zap_lit(lit);
        Stats[CLAUSES_WT_DELETE]++;
        }
    else if (forward_subsume(lit)) {
	if (Flags[PRINT_FORWARD_SUBSUMED].val) {
	    fprintf(Peer_fp, "new clause forward subsumed: ");
	    print_clause(Peer_fp, lit); fflush(Peer_fp);
	    }
        Stats[CLAUSES_FORWARD_SUBSUMED]++;
	zap_lit(lit);
	}
    else {
	renumber_vars(lit);  /* This can make it non-ac-canonical. */
	ac_canonical(lit->atom);

	owner = decide_owner(lit, (struct list *) NULL);

	if (owner == Pid) {
	    /* Keep the clause. */
	    Stats[RESIDENTS]++;
	    Stats[NEW_CLAUSE_KEPT]++;

	    lit->id = encode_ids(owner, ++Pseudo_clock);
	    store_clause_by_id(lit, Id_table);
	    if (Flags[NAME_STRATEGIES].val) {
	        lit->name = encode_ids(owner, ++Lid_counter);
	        store_clause_by_name(lit, Name_table);
		}
	    Stats[CLAUSES_KEPT]++;
	    fprintf(Peer_fp, "\n** KEPT: ");
	    print_clause(Peer_fp, lit);
	    fflush(Peer_fp);

	    store_new_clause(lit);
	    
	    if (oriented && oriented_eq_can_be_demod(lit))
		new_demodulator(lit);
	    check_for_proof(lit);
	    }
	else {
	    /* Send away the clause as a new settler. */
	    lit->id = encode_ids(owner, 0);
	    if (Flags[NAME_STRATEGIES].val)
	        lit->name = encode_ids(owner, 0);

	    fprintf(Peer_fp, "\nNew settler sent to peer %d: ", owner);
	    print_clause(Peer_fp, lit);
	    fflush(Peer_fp);

#if 0
	    if (Flags[POST_PROCESS_NS].val) {
		if (oriented && oriented_eq_can_be_demod(lit))
		    new_demodulator(lit);
		}
#endif	    

	    check_for_proof(lit);

	    clause_to_string(lit, Work_str, MAX_DAC_STRING);

	    ALOG_LOG(Pid, LOG_SEND_NS, 0, "");
	    Stats[NS_SENT]++;
	    p4_send(NEW_SETTLER_MSG, owner, Work_str, strlen(Work_str)+1);

	    Msgs_diff++;
/* Sent a message: increment the difference between sent and received. */

	    }
	}
}  /* process_new_clause */

/*************
 *
 *    process_bd_name_strategies(lit)
 *
 *    lit is either resident
 *    (back_demod_strat == R_PIDLIDCBT_IM_RAW or R_PIDLIDCBT_IM_NOC)
 *    or inference-msg (back_demod_strat == R_PIDLIDCBT_IM_NOC), and it is from
 *    either Usable or Sos.  It has already been removed from 
 *    all lists.
 *
 *************/

void process_bd_name_strategies(lit)
struct literal *lit;     
{
    int oriented;

    demodulate_literal(lit);
    oriented = orient_equality(lit);

    lit->weight = term_weight(lit->atom);

    if (lit->weight > Parms[MAX_WEIGHT].val) {
        zap_lit(lit);
        Stats[CLAUSES_WT_DELETE]++;
        }
    else if (forward_subsume(lit)) {
	if (Flags[PRINT_FORWARD_SUBSUMED].val) {
	    fprintf(Peer_fp, "back demod is forward subsumed: ");
	    print_clause(Peer_fp, lit); fflush(Peer_fp);
	    }
	zap_lit(lit);
	}
    else {
	renumber_vars(lit);  /* This can make it non-ac-canonical. */
	ac_canonical(lit->atom);

	/* If lit is a resident, increase the birth_time. */

	if (mine(lit)) {
	    lit->id = encode_ids(Pid, ++Pseudo_clock); /* strategy 3 or 4, resident */
	    Stats[RESIDENTS]++;
	    }
	else 
	    delete_clause_by_id(lit, Id_table);  /* strategy 4, inference message */

	store_clause_by_id(lit, Id_table);

	store_clause_by_name(lit, Name_table);

	if (mine(lit))
	    Stats[BD_KEPT_NAME_RES]++;
	else
	    Stats[BD_KEPT_NAME_IM]++;
	fprintf(Peer_fp, "\n** KEPT (back_demod): ");
	print_clause(Peer_fp, lit);
	fflush(Peer_fp);

	store_new_clause(lit);
	    
	if (oriented && oriented_eq_can_be_demod(lit))
	    new_demodulator(lit);
	check_for_proof(lit);
	}
}  /* process_bd_name_strategies */

/*************
 *
 *   update_db(lit)
 *
 *   return 
 *         1: lit should be kept
 *         0: lit should be deleted
 *
 *************/

int update_db(lit)
struct literal *lit;     
{
    struct literal *c;
    int lit_comp_c, delete_c;

    c = find_clause_by_name(lit->name, Name_table);

    if (!c)
	return(1);

    else {

	Stats[CLAUSES_UDB_DELETED]++;
	lit_comp_c = clause_compare(lit, c);

	if (lit_comp_c == GREATER_THAN)
	    delete_c = 0;
	else if (lit_comp_c == LESS_THAN)
	    delete_c = 1;
	else if (birth_time(lit) <= birth_time(c))
	    delete_c = 0;
	else
	    delete_c = 1;

	if (delete_c) {

	    if (mine(c)) {
		fprintf(Peer_fp, "\n\nWARNING!! resident (Pid=%d) deleted in update_DB\n", Pid);
		fprintf(Peer_fp, "Deleter: ");
		print_clause(Peer_fp, lit);
		fprintf(Peer_fp, "Deleted: ");
		print_clause(Peer_fp, c);
		fprintf(Peer_fp, "\n\n");
		fflush(Peer_fp);
		}
	    
	    if (list_member(c, Sos)) {
		if (mine(c))
		    Stats[RESIDENTS_IN_SOS]--;
		else
		    Stats[VISITORS_IN_SOS]--;
		}
	    
	    if (Flags[INDEX_DEMOD].val && list_member(c, Demodulators))
		discrim_wild_delete(c->atom->farg->argval, Demod_index, (void *) c);
	    
	    list_remove_all(c);
	    delete_clause_by_name(c, Name_table);
	    list_append(c, Deleted_clauses);

	    fprintf(Peer_fp, "Clause ");
	    print_clause(Peer_fp, c);
            fprintf(Peer_fp, " deleted in update_db by: ");
	    print_clause(Peer_fp, lit);
	    fflush(Peer_fp);

	    return(1);
	    }
	else
	    return(0);
	}
}  /* update_db */

/*************
 *
 *    process_inference_msg(lit)
 *
 *************/

void process_inference_msg(lit)
struct literal *lit;     
{
    int oriented;

    if ((Flags[NAME_STRATEGIES].val) && !update_db(lit)) {
	fprintf(Peer_fp, "Inference Message deleted by update_db: ");
	fprintf(Peer_fp, "<%d,%d,%d>\n",  owner(lit), lid(lit), birth_time(lit));
	fflush(Peer_fp);
	zap_lit(lit);
	}
    else {
	/* lit not deleted by update_db */
	
	demodulate_literal(lit);
	    
	oriented = orient_equality(lit);
	
	lit->weight = term_weight(lit->atom);
	
	if (lit->weight > Parms[MAX_WEIGHT].val) {
	    zap_lit(lit);
	    Stats[CLAUSES_WT_DELETE]++;
	    }
	else if (forward_subsume(lit)) {
	    if (Flags[PRINT_FORWARD_SUBSUMED].val) {
		fprintf(Peer_fp, "inference message forward subsumed: ");
		print_clause(Peer_fp, lit); fflush(Peer_fp);
		}
		print_clause(Peer_fp, lit);
		zap_lit(lit);
	    }
	else {
	    renumber_vars(lit);  /* This can make it non-ac-canonical. */
	    ac_canonical(lit->atom);

	    Stats[IM_KEPT]++;
	    fprintf(Peer_fp ,"\n** Inference message KEPT: ");
	    print_clause(Peer_fp, lit);
	    fflush(Peer_fp);
	    
	    store_clause_by_id(lit, Id_table);
	    if (Flags[NAME_STRATEGIES].val)
	        store_clause_by_name(lit, Name_table);

	    store_new_clause(lit); /* append in either Usable or Sos */
	    
	    if (oriented && oriented_eq_can_be_demod(lit)) {
		if (lit->weight <= Parms[MAX_BD_INF_MSG].val)
		    /* make into demodulator and back demodulate */
		    new_demodulator(lit);
		else {
		    /* make into demodulator but do not back demodulate now */
		    list_append(lit, Demodulators);
		    if (Flags[INDEX_DEMOD].val) {
			discrim_wild_insert(lit->atom->farg->argval,
					    Demod_index, (void *) lit);
			if (Flags[TWO_WAY_DEMOD].val)
			    discrim_wild_insert(lit->atom->farg->narg->argval,
					    Demod_index, (void *) lit);
			}
		    }
		}
	    check_for_proof(lit);
	    }
	}
}  /* process_inference_msg */

/*************
 *
 *    process_inf_msg(lit)
 *
 *************/

void process_inf_msg(lit)
struct literal *lit;     
{
    int oriented;

    if (Flags[NAME_STRATEGIES].val && !update_db(lit)) {
	fprintf(Peer_fp, "Inference Message deleted by update_db: ");
	fprintf(Peer_fp, "<%d,%d,%d>\n",  owner(lit), lid(lit), birth_time(lit));
	fflush(Peer_fp);
	if (!find_clause_by_id(lit, Id_table)) {
	    store_clause_by_id(lit, Id_table);
	    list_append(lit, Deleted_clauses);
/* If an incoming inference message is deleted by update_db, we save it     */
/* in Deleted_clauses and we store it by id for the purpose of proof        */
/* reconstruction. Storing by id does not cause conflicts, because the      */
/* clause whose presence determined the deletion of the incoming inference  */
/* message has its same name, but different id. The id must be different    */
/* because the birth-time must be different. And the birth-time must be     */
/* different because they come from the same process. Indeed they must have */
/* the same pid if they have the same name.				    */
            }
	}
    else {
	/* lit not deleted by update_db */
	
	oriented = orient_equality(lit);
	
	lit->weight = term_weight(lit->atom);
	
	renumber_vars(lit);  /* This can make it non-ac-canonical. */
	ac_canonical(lit->atom);

	Stats[IM_KEPT]++;
	fprintf(Peer_fp ,"\n** Inference message KEPT: ");
	print_clause(Peer_fp, lit);
	fflush(Peer_fp);
	    
	store_clause_by_id(lit, Id_table);
	if (Flags[NAME_STRATEGIES].val)
	    store_clause_by_name(lit, Name_table);

	store_new_clause(lit); /* append in either Usable or Sos */
	    
	if (oriented && oriented_eq_can_be_demod(lit)) {
	    if (lit->weight <= Parms[MAX_BD_INF_MSG].val)
	    /* make into demodulator and back demodulate */
		new_demodulator(lit);
	    else {
		 /* make into demodulator but do not back demodulate now */
		 list_append(lit, Demodulators);
		 if (Flags[INDEX_DEMOD].val) {
		      discrim_wild_insert(lit->atom->farg->argval,
					    Demod_index, (void *) lit);
		      if (Flags[TWO_WAY_DEMOD].val)
			  discrim_wild_insert(lit->atom->farg->narg->argval,
					    Demod_index, (void *) lit);
			}
		    }
		}
	    check_for_proof(lit);
	}
}  /* process_inf_msg */

/*************
 *
 *   process_new_settler(lit)
 *
 *************/

void process_new_settler(lit)
struct literal *lit;
{
    int oriented;

    demodulate_literal(lit);
    oriented = orient_equality(lit);

    lit->weight = term_weight(lit->atom);

    if (lit->weight > Parms[MAX_WEIGHT].val) {
        zap_lit(lit);
        Stats[CLAUSES_WT_DELETE]++;
        }
    else if (forward_subsume(lit)) {
	if (Flags[PRINT_FORWARD_SUBSUMED].val) {
	    fprintf(Peer_fp, "new settler forward subsumed: ");
	    print_clause(Peer_fp, lit); fflush(Peer_fp);
	    }
	zap_lit(lit);
	}
    else {
	renumber_vars(lit);  /* This can make it non-ac-canonical. */
	ac_canonical(lit->atom);

	lit->id = encode_ids(Pid, ++Pseudo_clock);
	store_clause_by_id(lit, Id_table);

	if (Flags[NAME_STRATEGIES].val) {
	    lit->name = encode_ids(Pid, ++Lid_counter);
	    store_clause_by_name(lit, Name_table);
	}

	Stats[RESIDENTS]++;
	Stats[NS_KEPT]++;
	fprintf(Peer_fp, "\n** New Settler KEPT: ");
	print_clause(Peer_fp, lit);
	fflush(Peer_fp);
	
	store_new_clause(lit);
	
	if (oriented && oriented_eq_can_be_demod(lit))
	    new_demodulator(lit);
	check_for_proof(lit);
	}
}  /* process_new_settler */

/*************
 *
 *   encode_ids()
 *
 *************/

int encode_ids(id1, id2)
int id1, id2;
{
    return(id1*1000000 + id2);
}  /* encode_ids */

/*************
 *
 *   id1()
 *
 *************/

int id1(id)
int id;     
{
    return(id / 1000000);
}  /* id1 */

/*************
 *
 *   id2()
 *
 *************/

int id2(id)
int id;     
{
    return(id % 1000000);
}  /* id2 */


/*************
 *
 *   handle_new_settler_msg(msg)
 *
 *************/

void handle_new_settler_msg(msg)
char *msg;
{
    struct literal *lit;

    lit = string_to_clause(msg);
    fprintf(Peer_fp, "New settler received: "); print_clause(Peer_fp, lit);

    process_new_settler(lit);
}  /* handle_new_settler_msg */


/*************
 *
 *   handle_inference_msg(msg,from)
 *
 *************/

void handle_inference_msg(msg,from)
char *msg;
int from;
{
    struct literal *lit;

    lit = string_to_clause(msg);
    fprintf(Peer_fp, "Inference_message received from Peer %d: ", from);
    print_clause(Peer_fp, lit);
    if (Peers_pseudo_clocks[from] < birth_time(lit))
        Peers_pseudo_clocks[from] = birth_time(lit);
/* If the incoming inference message allows to improve the estimate of the */
/* work-load at the sender, then update Peers_pseudo_clocks. */
    process_inf_msg(lit);
}  /* handle_inference_msg */

/*************
 *
 *    extract_given_clause()
 *
 *	Since Sos is kept sorted by weight, extracting the first one amounts
 *	to extracting the lightest first.
 *
 *************/

struct literal *extract_given_clause()
{
    struct literal *lit;

    if (!Sos->first)
        return(NULL);
    else {
        lit = Sos->first->lit;
        list_remove(lit, Sos);
	if (mine(lit))
	    Stats[RESIDENTS_IN_SOS]--;
	else
	    Stats[VISITORS_IN_SOS]--;
        return(lit);
        }
}  /* extract_given_clause */

/*************
 *
 *   expansion_work_available()
 *
 *************/

int expansion_work_available()
{
    if (Flags[PARA_PAIRS].val)
	return(Para_pairs_first != NULL);
    else
	return(Sos->first != NULL);
}  /* expansion_work_available */

/*************
 *
 *   expansion_work_given()
 *
 *************/

void expansion_work_given()
{
    struct literal *given_clause, *u;
    struct list_pos *p;

    given_clause = extract_given_clause();
    list_append(given_clause, Usable);

    Stats[CLAUSES_GIVEN]++;

    if (Flags[PRINT_GIVEN].val) {
	fprintf(Peer_fp, "given clause #%d: ", Stats[CLAUSES_GIVEN]);
	print_clause(Peer_fp, given_clause);
	fflush(Peer_fp);
	}

    if (mine(given_clause) && !find_clause_by_id(given_clause->id, Already_broadcast_table)) {
	clause_to_string(given_clause, Work_str, MAX_DAC_STRING);
	ALOG_LOG(Pid, LOG_BROADCAST_IM, 0, "");
	Stats[IM_BROADCAST]++;
	m4_broadcast(INFERENCE_MSG, Work_str, strlen(Work_str)+1);
	Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
	store_clause_by_id(given_clause, Already_broadcast_table);
	Peers_pseudo_clocks[Pid] = birth_time(given_clause);
	}

    /* para into residents only, para from anything */

    if (mine(given_clause)) {
	for (p = Usable->first; p; p = p->next) {
	    u = p->lit;
	    para_from_into(u, given_clause, ALL);
	    if (mine(u))
		para_from_into(given_clause, u, ALL_BUT_TOP);
		/* It uses ALL_BUT_TOP, because the topmost position has been already */
		/* covered by para_from_into(u, given_clause, ALL). */
	    }
	}
    else {
	for (p = Usable->first; p; p = p->next) {
	    u = p->lit;
	    if (mine(u))
		para_from_into(given_clause, u, ALL);
	    }
	}
}  /* expansion_work_given */

/*************
 *
 *    expansion_work_pairs()
 *
 *************/

void expansion_work_pairs()
{
    struct literal *c1, *c2;
    struct para_pair *pp;

    pp = get_from_pp();
    c1 = pp->c1;
    c2 = pp->c2;
    free_para_pair(pp);

    if (list_member(c1, Usable) && list_member(c2, Usable)) {

	Stats[CLAUSES_GIVEN]++;

	if (Flags[PRINT_GIVEN].val) {
	    fprintf(Peer_fp, "paramodulate pair #%d:\n", Stats[CLAUSES_GIVEN]);
	    fprintf(Peer_fp, "    "); print_clause(Peer_fp, c1);
	    fprintf(Peer_fp, "    "); print_clause(Peer_fp, c2);
	    fflush(Peer_fp);
	    }
	
	if (mine(c1) && !find_clause_by_id(c1->id, Already_broadcast_table)) {
	    clause_to_string(c1, Work_str, MAX_DAC_STRING);
	    ALOG_LOG(Pid, LOG_BROADCAST_IM, 0, "");
	    Stats[IM_BROADCAST]++;
	    m4_broadcast(INFERENCE_MSG, Work_str, strlen(Work_str)+1);
	    Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
	    store_clause_by_id(c1, Already_broadcast_table);
	    Peers_pseudo_clocks[Pid] = birth_time(c1);
	    }
	
	if (mine(c2) && !find_clause_by_id(c2->id, Already_broadcast_table)) {
	    clause_to_string(c2, Work_str, MAX_DAC_STRING);
	    ALOG_LOG(Pid, LOG_BROADCAST_IM, 0, "");
	    Stats[IM_BROADCAST]++;
	    m4_broadcast(INFERENCE_MSG, Work_str, strlen(Work_str)+1);
	    Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
	    store_clause_by_id(c2, Already_broadcast_table);
	    Peers_pseudo_clocks[Pid] = birth_time(c2);
	    }
	
	if (mine(c1))
	    para_from_into(c2, c1, ALL);
	
	if (mine(c2)) {
	    if (mine(c1))
		para_from_into(c1,c2, ALL_BUT_TOP);
	    else
		para_from_into(c1,c2, ALL);
	    }
	}
}  /* expansion_work_pairs */

/*************
 *
 *   store_new_clause(lit)
 *
 *************/

void store_new_clause(lit)
struct literal *lit;
{
    if (Flags[PARA_PAIRS].val) {
	struct list_pos *p;

        list_append(lit, Usable);
        for (p = Usable->first; p; p = p->next) {
	    /* make sure that at least one of the two equations in the pair is a resident */
	    if (mine(lit) || mine(p->lit))
            store_in_pp(lit, p->lit);
	    }
	}
    else {
	insert_literal_by_weight(lit, Sos);
	if (mine(lit))
	    Stats[RESIDENTS_IN_SOS]++;
	else
	    Stats[VISITORS_IN_SOS]++;
	}
    
}  /* store_new_clause */

/*************
 *
 *   birth_time(lit)
 *
 *************/

int birth_time(lit)
struct literal *lit;
{
    return(id2(lit->id));
}  /* birth_time */

/*************
 *
 *   lid(lit)
 *
 *************/

int lid(lit)
struct literal *lit;
{
    return(id2(lit->name));
}  /* lid */

/*************
 *
 *   owner(lit)
 *
 *************/

int owner(lit)
struct literal *lit;
{
    if (Flags[NAME_STRATEGIES].val && id1(lit->name) != id1(lit->id)) {
	    fprintf(Peer_fp, "WARNING, the following clause has bad Pids.\n");
	    print_clause(Peer_fp, lit); fflush(Peer_fp);
	    }
    return(id1(lit->id));
}  /* owner */

/*************
 *
 *   mine(lit)
 *
 *************/

int mine(lit)
struct literal *lit;     
{
    return(owner(lit) == Pid);
}  /* mine */

/*************
 *
 *   term_to_int()
 *
 *************/

#if 0

static int term_to_int(t)
struct term *t;
{
    int i;

    if (t->type == VARIABLE)
        i = t->varnum;
    else if (t->type == NAME)
        i = t->sym_num;
    else {
        struct rel *r;
        int pos;

        i = t->sym_num;
        pos = 1;
        for (r = t->farg; r; r = r->narg)
            if (sn_to_node(t->sym_num)->lrpo_status == LRPO_MULTISET_STATUS)
                i += term_to_int(r->argval);
            else if (sn_to_node(t->sym_num)->lrpo_status == LRPO_LR_STATUS)
		i += (pos++) * term_to_int(r->argval);
            else        /* no LRPO_RL_STATUS ? */
		i += term_to_int(r->argval) / (pos++);
        }
    return (i);
}  /* term_to_int */

#else

static int term_to_int(t)
struct term *t;
{
    int i;

    if (t->type == VARIABLE)
        /* i = t->varnum; */
	   i = 0;
	   /* So that not only copies but also variants go to the same peer. */
    else if (t->type == NAME)
        i = t->sym_num;
    else {
        struct rel *r;

        i = t->sym_num;
        for (r = t->farg; r; r = r->narg)
            i += term_to_int(r->argval);
        }
    return (i);
}  /* term_to_int */

#endif

/*************
 *
 *   clause_to_peer_function()
 *
 *************/

int clause_to_peer_function(lit)
struct literal *lit;
{
    return(term_to_int(lit->atom) % Number_of_peers);
}  /* clause_to_peer_function */
