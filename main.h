#ifndef MAIN_H
#define MAIN_H
#ifdef __cplusplus
extern "C"{
#endif


#ifdef WE_NEED_STRLWR
extern char* strlwr(char *instr);
#endif

extern void save_game(void);

extern void str_print(char *msg);
extern int count_inventory(void);
extern int is_really_dark(void);
extern int is_dark(void);

extern void print_room_items(int location);
extern void print_room_exits(int room);
extern void print_table_string(uint16_t table, int table_offset);
extern void print_room_descr(int room);
extern void print_the_string(int string_num);
extern void num_print(int num);
extern void run_game(void);
extern void load_game(void);

extern int noun_item(int noun_word_num, int from);
extern void gprintf(char *strX, ...);

extern uint16_t fix_address(uint16_t ina);
extern uint16_t fix_word(uint16_t word);
extern uint8_t get_byte(uint8_t *mem);
extern uint16_t get_word(uint8_t *mem);
extern void set_bit(unsigned char *bits, int pos, int state);
extern int get_bit(unsigned char *bits, int pos);

extern void calc_max_items(void);
extern void calc_max_messages(void);
extern void decompile_game(void);
extern void decomp_the_string(int msg);
extern void decomp_item_name(int item);
extern void decomp_word(int word, uint16_t table, int num_words);
extern void decomp_table_string(uint16_t table, int table_offset);

extern glui32 get_keypress(void);
extern void grab_input(void);

extern int max_messages;

extern int saved_seed;
extern int action;
extern winid_t mainwin;
extern winid_t roomwin;
extern frefid_t f_script_input;
extern strid_t	s_script_input;
extern frefid_t f_output;
extern strid_t	s_output;

/* global data */
extern 	char *game_file;
extern 	long game_size;
extern char *script_file;
extern char *source_file;

extern 	struct HEADER		h;
extern 	struct DATAHEADER 	dh;

extern 	uint8_t		*game;
extern 	struct GAME *gm;
extern 	struct GAME *gm_undo;

extern	int		move_room;
extern 	int 	game_flag;
extern 	int		inroom_flag;
extern	int		counter_idx;

extern 	char	in_noun[32];
extern	char	in_verb[32];
extern	int		verb_num;
extern	int		noun_num;
extern	int		item_num;
extern	int		scripting;
extern	int		count_light;
extern	int		dump_codes;

extern int		max_item_descr;

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef MAIN_H
