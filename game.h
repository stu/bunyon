#ifndef GAME_H
#define GAME_H
#ifdef __cplusplus
extern "C"{
#endif


#pragma pack(1)
struct HEADER
{
	char gname[10]			;
	char nul1[2]			;
	char something			;
	char nul2[2]			;
	char num_sectors		;
	char num_sectors_mod	;
	char nul3[0x6F]			;
};

struct DATAHEADER
{
	uint8_t	num_objects			;		/* number of objects */
	uint8_t	num_verbs			;		/* number of verbs */
	uint8_t	num_nouns			;		/* number of nouns */
	uint8_t	red_room			;		/* the red room (dead room) */
	uint8_t	max_items_carried	;		/* max number of items can be carried */
	uint8_t	begin_locn			;		/* room to start in */
	uint8_t	num_treasures		;		/* number of treasures */
	uint8_t	cmd_length			;		/* number of letters in commands */
	uint16_t	light_turns			;		/* max number of turns light lasts */
	uint8_t	treasure_locn		;		/* location of where to store treasures */
	uint8_t	strange				;		/* !?! not known. */

	uint16_t	p_obj_table			;		/* pointer to object table */
	uint16_t	p_orig_items		;		/* pointer to original items */
	uint16_t	p_obj_link			;		/* pointer to link table from noun to object */
	uint16_t	p_obj_descr			;		/* pointer to object descriptions */
	uint16_t	p_message			;		/* pointer to message pointers */
	uint16_t	p_room_exit			;		/* pointer to room exits table */
	uint16_t	p_room_descr		;		/* pointer to room descr table */

	uint16_t	p_noun_table		;		/* pointer to noun table */
	uint16_t	p_verb_table		;		/* pointer to verb table */

	uint16_t	p_explicit			;		/* pointer to explicit action table */
	uint16_t	p_implicit			;		/* pointer to implicit actions */
	uint16_t	saved_room			;		/* saved room */
	uint16_t	nul1[2]				;		/* nul1 */
	uint16_t	saved_timer			;		/* saved timer */
	uint16_t	nul2[14]			;		/* nul2 */
	uint16_t	save_area			;		/* save area */
	uint16_t	nul3[4]				;		/* nul3 */
	uint16_t	dynamic_area		;		/* dynamic part of file */
};

#pragma pack()

#define MAX_TIMERS		16
#define MAX_ITEMS		254
#define MAX_RRV			16
#define MAX_FLAGS		32

#define X_TRUE			1
#define X_FALSE			0

struct GAME
{
	int		locn;				/* current location */
	unsigned char bitflags[MAX_FLAGS/8];	/* bitflags */
	int		inv_flag;			/* inventory flag */
	long		rnd_seed;			/* random seed */
	int		light;				/* light turns */
	int		cur_timer;			/* current timer */
	int		timers[MAX_TIMERS];	/* 16 timers */
	int		room_rv[MAX_RRV];	/* room rv (?) for select_rv */
	int		items[MAX_ITEMS];	/* you have these items. */

	char	undo_data_ok;		/* can we do an undo? */
	int		sgeo_game_flag;		/* cooked game */
};


#ifdef __cplusplus
};
#endif
#endif        //  #ifndef GAME_H
