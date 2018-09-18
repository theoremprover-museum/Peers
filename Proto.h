/* Proto.h made 
Fri Mar  4 18:47:42 CST 1994
*/

/* main.c */

void print_banner();
void interrupt_interact();

/* clocks.c */

void init_clocks();
long clock_val();
void clock_reset();
char *get_time();
long system_time();
long run_time();
long wall_seconds();

/* avail.c */

void init_avail();
char *tp_alloc();
struct term *get_term();
void free_term();
struct rel *get_rel();
void free_rel();
struct sym_ent *get_sym_ent();
void free_sym_ent();
struct gen_ptr *get_gen_ptr();
void free_gen_ptr();
struct context *get_context();
void free_context();
struct trail *get_trail();
void free_trail();
struct bt_node *get_bt_node();
void free_bt_node();
struct ac_position *get_ac_position();
void free_ac_position();
struct discrim *get_discrim();
void free_discrim();
struct flat *get_flat();
void free_flat();
struct discrim_pos *get_discrim_pos();
void free_discrim_pos();
struct fpa_head *get_fpa_head();
void free_fpa_head();
struct fpa_tree *get_fpa_tree();
void free_fpa_tree();
struct fpa_pos *get_fpa_pos();
void free_fpa_pos();
struct ac_match_pos *get_ac_match_pos();
void free_ac_match_pos();
struct ac_match_free_vars_pos *get_ac_match_free_vars_pos();
void free_ac_match_free_vars_pos();
struct literal *get_literal();
void free_literal();
struct list *get_list();
void free_list();
struct list_pos *get_list_pos();
void free_list_pos();
struct para_pair *get_para_pair();
void free_para_pair();
void print_mem();
void p_mem();
int total_mem();

/* term.c */

struct term *copy_term();
struct term *copy_term_bits();
int ground_term();
int term_ident();
void zap_term();
int symbol_count();
int term_compare_ncv();
int term_compare_vf();
void merge_sort();

/* misc.c */

void init();
void abend();
void output_stats();
void print_stats();
void p_stats();
void print_times();
void p_times();
void read_preamble();
struct term *build_binary_term();
struct gen_ptr *reverse_gen_list();
int occurs_in();
struct gen_ptr *copy_gen_ptr_list();

/* io.c */

void init_symbol_table();
int str_int();
void int_str();
void cat_str();
void str_copy();
int str_ident();
void reverse();
int str_long();
void long_str();
int new_sym_num();
struct sym_ent *insert_sym();
int str_to_sn();
void print_syms();
void p_syms();
char *sn_to_str();
int sn_to_arity();
struct sym_ent *sn_to_node();
int in_sym_tab();
void free_sym_tab();
int is_symbol();
int initial_str();
int set_vars();
int set_vars_term();
int var_name();
struct gen_ptr *read_list();
void print_gen_list();
void write_term();
void display_term();
void print_term();
void p_term();
void d_term();
void print_term_nl();
void print_variable();
int declare_op();
void init_special_ops();
int process_op_command();
int proper_list();
void skip_white();
int name_sym();
struct term *str_to_term();
int read_buf();
struct term *term_fixup();
struct term *term_fixup_2();
struct term *read_term();
void set_assoc_comm();
int is_assoc_comm();
int process_ac_command();
void set_commutative();
int is_commutative();
int process_comm_command();
int process_multiset_command();
int process_lex_command();
void sym_tab_to_string();

/* options.c */

void init_options();
void print_options();
void p_options();
int change_flag();
int change_parm();
void check_options();

/* unify.c */

int occur_check();
int unify();
int match();
struct term *apply();
void clear_subst_2();
void clear_subst_1();
void print_subst();
void p_subst();
void print_trail();

/* ac.c */

int num_ac_args();
int num_ac_nv_args();
void flatten_deref();
int compare_ncv_context();
void sort_ac();
void elim_con_context();
void ac_mult_context();
void ac_prepare();
void set_up_basis_terms();
int unify_ac();
void unify_ac_cancel();

/* dioph.c */

int gcd();
int lcm();
void p_ac_basis();
int less_vec();
int var_check_1();
int var_check_2();
int add_solution();
int a_in_bounds();
int b_in_bounds();
int dio();
int superset_degree();
int next_combo_a1();
int next_combo_a();
int next_combo_b();
int next_combo_c();
int all_combos();
int all_combos_build();

/* btu.c */

struct bt_node *unify_bt_first();
struct bt_node *unify_bt_next();
void unify_bt_cancel();
struct bt_node *unify_bt_guts();
struct bt_node *unify_bt_backup();
int unify_commute();
void p_bt_tree();

/* btm.c */

struct bt_node *match_bt_first();
struct bt_node *match_bt_next();
void match_bt_cancel();
struct bt_node *match_bt_guts();
struct bt_node *match_bt_backup();
int match_commute();
void compact_args();
void unbind_free_var();
int free_var_match();
struct term *build_partial_term();
void clear_partial_term();
int match_ac();
void p_acm();
void flatten();
void flatten_mult();
void elim_com();
void ac_mult();
void right_associate();
void ac_canonical();
int check_ac_canonical();

/* demod.c */

struct term *apply_demod();
struct term *contract_bt();
struct term *demodulate_flat();
void clear_demod_marks();

/* discrim.c */

void p_discrim_tree();
void discrim_insert();
void discrim_delete();
struct gen_ptr *discrim_retrieve_leaf();
void *discrim_retrieve_first();
void *discrim_retrieve_next();
void discrim_cancel();
void p_discrim_wild_tree();
struct discrim *discrim_wild_insert_ac();
void discrim_wild_insert();
void discrim_wild_delete();
struct gen_ptr *discrim_wild_retrieve_leaf();
void *discrim_wild_retrieve_first();
void *discrim_wild_retrieve_next();
void discrim_wild_cancel();

/* fpa.c */

struct fpa_index *fpa_init();
void term_fpa_rec();
void fpa_insert();
void fpa_delete();
struct fpa_tree *union_with_commutative_branch();
struct fpa_tree *build_tree_local();
struct fpa_tree *build_tree();
struct term *next_term();
struct fpa_tree *build_for_all();
void zap_prop_tree();
void print_fpa_tab();
void p_fpa_tab();
void print_prop_tree();
void p_prop_tree();
void print_path();
void p_path();
struct term *fpa_retrieve_first();
struct term *fpa_retrieve_next();
void fpa_cancel();
struct term *fpa_bt_retrieve_first();
struct term *fpa_bt_retrieve_next();
void fpa_bt_cancel();

/* list.c */

void list_append();
void list_prepend();
void list_insert_before();
void list_insert_after();
int list_remove();
int list_remove_all();
int list_member();
void print_list();
void p_list();
void list_zap();
void list_check();
int clause_in_list();

/* clause.c */

void init_clause_table_id();
void store_clause_by_id();
void delete_clause_by_id();
struct literal *find_clause_by_id();
void print_clause_table_id();
void p_clause_table_id();
void init_clause_table_name();
void store_clause_by_name();
void delete_clause_by_name();
struct literal *find_clause_by_name();
void print_clause_table_name();
void p_clause_table_name();
struct literal *copy_literal();
void zap_lit();
void insert_literal_by_weight();
void get_ancestors();
void print_proof();
void print_clause();
void p_clause();
int alpha_is_ac();
int pos_eq();
void renum_vars_recurse();
void renumber_vars();
int subsumes();
struct literal *subsume_list();
struct literal *conflict_list();
int term_weight();
void add_vars();
int orient_equality();
int has_an_instance();
int poly1();
struct term *extend_atom();
struct term *apply_substitute();
int oriented_eq_can_be_demod();
int clause_ident();
int clause_compare();

/* messages.c */

void m4_broadcast();
int m4_messages_available();
void string_to_options();
int more_in_string();
void next_str();
int next_int();
void sym_array_to_normal_sym_tab();
void string_to_sym_array();
void term_to_string_rec();
void term_to_string();
struct term *string_to_term_rec();
struct term *string_to_term();
void clause_to_string();
struct literal *string_to_clause();

/* paramod.c */

void init_pp_index();
int pp_index_wt();
void store_in_pp();
struct para_pair *get_from_pp();
void p_para_pairs_stats();
void new_demodulator_input();
void new_demodulator();
void demodulate_literal();
void paramod();
void paramodulate_pair();
void para_from_into();

/* peers.c */

void slave();
void peer_init_and_work();
void peer_work();
int work_available();
int find_high_priority_work();
void work_loop();
int forward_subsume();
void check_for_proof();
void term_list_to_literal_list();
void set_clauses();
int decide_owner();
void process_input_clause();
void process_new_clause();
void process_bd_name_strategies();
int update_db();
void process_inference_msg();
void process_inf_msg();
void process_new_settler();
int encode_ids();
int id1();
int id2();
void handle_new_settler_msg();
void handle_inference_msg();
struct literal *extract_given_clause();
int expansion_work_available();
void expansion_work_given();
void expansion_work_pairs();
void store_new_clause();
int birth_time();
int lid();
int owner();
int mine();
int clause_to_peer_function();

/* lrpo.c */

int lrpo();
int lrpo_greater();
int orient_literal_lrpo();
