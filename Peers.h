#include "Clause.h"  /* For CLAUSE_TAB_SIZE and hash tables of clauses. */

#define MAX_SYMBOL_ARRAY   200
#define MAX_DAC_STRING    10000
#define MAX_PROCESSES      100

/* types of work---messages and local work---and their priorities */
 
#define HALT_MSG		 1

#define OPTIONS_MSG		 2
#define SYM_TAB_MSG		 3
#define INPUT_PASSIVE_MSG	 4
#define INPUT_SOS_MSG		 5
#define INPUT_USABLE_MSG	 6
#define INPUT_DEMOD_MSG		 7
#define CLOCK_MSG		 8

#define INFERENCE_MSG		 9
#define NEW_SETTLER_MSG		10
/* Inference messages should be received and handled with higher priority */
/* than new settlers because new settlers are sent whereas inference      */
/* messages are broadcast and broadcasting is slower than sending,        */
/* to such an extent that a message sent after a message broadcast may    */
/* arrive earlier.                                                        */

#define TOKEN_MSG		11

#define EXPANSION_WORK		12


#define NUMBER_OF_WORK_TYPES	12

/* values for decide_owner_strategy, deciding who owns a clause */
/* NOTE: values are the same as the Parameter values. */

#define ROTATE        1				/* default */
#define SYNTAX        2
#define SELECT_MIN    3

/* values for back_demod_strategy, deciding how to treat a backward-demodulated clause */
/* NOTE: values are the same as the Parameter values. */

#define ALL_RAW			1
/* All clauses, both residents and received inference messages, are treated as raw	*/
/* clauses after backward demodulation.							*/
/* Flags[NAME_STRATEGIES].val == 0							*/

#define R_PIDCBT_IM_RAW		2
/* A backward demodulated resident keeps its pid, but gets new bt.			*/
/* A backward demodulated received inference messages is treated as a raw clause.	*/
/* Flags[NAME_STRATEGIES].val == 0							*/

#define R_PIDLIDCBT_IM_RAW	3
/* A backward demodulated resident keeps its pid and lid, but gets new bt.		*/
/* A backward demodulated received inference messages is treated as a raw clause.	*/
/* Flags[NAME_STRATEGIES].val == 1							*/

#define R_PIDLIDCBT_IM_NOC	4		/* default */
/* A backward demodulated resident keeps its pid and lid, but gets new bt.		*/
/* A backward demodulated received inference messages has no changes.			*/
/* Flags[NAME_STRATEGIES].val == 1							*/

/* values for logging */

#define LOG_START_EXPANSION      1
#define LOG_STOP_EXPANSION       2
#define LOG_START_NEW_SETTLER    3
#define LOG_STOP_NEW_SETTLER     4
#define LOG_START_INFERENCE_MSG  5
#define LOG_STOP_INFERENCE_MSG   6

#define LOG_SEND_NS      7
#define LOG_RECV_NS      8
#define LOG_BROADCAST_IM 9
#define LOG_RECV_IM     10

#define LOG_SOS_RES_SIZE  11
#define LOG_SOS_VIS_SIZE  12

#define LOG_SYNC           1000

/****************** Global variables for peer code ************/

#ifdef OWNER_OF_PEER_GLOBALS
#define PEER_GLOBAL         /* empty string if included by owner */
#else
#define PEER_GLOBAL extern  /* extern if included by anything else */
#endif

PEER_GLOBAL int Pid;              /* Peer ID */
PEER_GLOBAL int Number_of_peers;

/* fast symbol table */

PEER_GLOBAL struct sym_ent Symbol_array[MAX_SYMBOL_ARRAY];

/* clause lists */

PEER_GLOBAL struct list *Usable;
PEER_GLOBAL struct list *Sos;
PEER_GLOBAL struct list *Demodulators;
PEER_GLOBAL struct list *Passive;
PEER_GLOBAL struct list *Deleted_clauses;

/* indexes */

PEER_GLOBAL struct discrim *Demod_index;

PEER_GLOBAL int Peers_pseudo_clocks[MAX_PROCESSES];
/* Peers_pseudo_clock[i] contains the birth-time of the most recently 	*/
/* received inference message from i. It is used as the estimate of the */
/* work-load of peer i. Peers_pseudo_clock[Pid] is the birth-time of the*/
/* resident which was most recently selected as given clause or as part */
/* of a given pair. This information is used by decide_owner() with     */
/* the SELECT_MIN criterion.						*/

PEER_GLOBAL int Lid_counter;

PEER_GLOBAL int Pseudo_clock;

PEER_GLOBAL int Msgs_diff;
/* Used by the termination detection algorithm. It is incremented whenever */
/* a message is sent and decremented whenever a message is received. The   */
/* sum of the Msgs_diff at all the peers will be 0 when all sent messages  */
/* have been received.							   */

PEER_GLOBAL int Have_token;
/* Used by the termination detection algorithm: 1 if Pid has the token,    */
/* 0 otherwise.								   */

PEER_GLOBAL int How_many_times_token_received;
/* Used by the termination detection algorithm, so that Peer 0 can detect  */
/* to have received the token twice.					   */

/* The following array is used for building long messages. */

PEER_GLOBAL char Work_str[MAX_DAC_STRING];



