
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#include <garglk/glk.h>

#include "config.h"

#include "define.h"
#include "game.h"
#include "main.h"
#include "interp.h"
#include "rnd.h"
#include "msg.h"
#include "debug.h"

int saved_seed;
int action;
winid_t mainwin;
winid_t roomwin;

frefid_t f_script_input;
strid_t	s_script_input;

frefid_t f_output;
strid_t	s_output;

frefid_t f_sourcecode;
strid_t	s_sourcecode;

void save_game(void);
void load_save_game(void);

void printstring(char *s);
void decomp_printstring(char *s);
int noun_item(int noun_word_num, int from);
int is_really_dark(void);
int is_dark(void);
void decomp_word(int word, uint16_t table, int num_words);
void print_table_string(uint16_t table, int table_offset);
void print_room_exits(int room);
void print_room_descr(int room);
void print_room_items(int room);
int word_lookup(char *word, uint16_t table, int num_words);
void do_fixwords(void);

uint8_t *print_string(uint8_t *string);
void str_print(char *msg);
void print_title_screen(void);
void run_game(void);

void print_syntax(void);
void do_switches(int argc, char *argv[]);

uint16_t fix_address(uint16_t ina);
uint16_t fix_word(uint16_t word);
uint8_t get_byte(uint8_t *mem);
uint16_t get_word(uint8_t *mem);
void set_bit(unsigned char *bits, int pos, int state);
int get_bit(unsigned char *bits, int pos);


void CountDownLight(void);

/* global data */
char *game_file;
long game_size;

char *script_file;
char *source_file;

struct HEADER		h;
struct DATAHEADER 	dh;

uint8_t		*game;
struct GAME *gm;
struct GAME *gm_undo;

char	*any_word;

int		move_room;
int 	game_flag;
int		inroom_flag;

int		dump_codes;

char	in_verb[32];
char	in_noun[32];

int		verb_num;
int		noun_num;
int		item_num;

int 	max_messages;
int		max_item_descr;

int 	scripting;
int		count_light;

char	strLogBuffer3[1024];

char* bunyon_strlwr(char *instr)
{
	char *x = instr;

	while(*instr!=0)
	{
		*instr = tolower(*instr);
		instr++;
	}

	return x;
}

char* bunyon_strupr(char *instr)
{
	char *x = instr;

	while(*instr!=0)
	{
		*instr = toupper(*instr);
		instr++;
	}

	return x;
}

uint16_t fix_address(uint16_t ina)
{
	return(ina-0x380);
}

uint16_t get_word(uint8_t *mem)
{
	uint16_t x;

#ifdef WE_ARE_BIG_ENDIAN
	x=*(uint16_t *)mem;
#else
	x=(*(mem+0)<<8);
	x+=(*(mem+1)&0xFF);

	x=*(uint16_t *)mem;
#endif
	return fix_word(x);
}

uint8_t get_byte(uint8_t *mem)
{
	return(*mem);
}

uint16_t fix_word(uint16_t word)
{
#ifdef WE_ARE_BIG_ENDIAN
	return word;
#else
	return (  ((word&0xFF)<<8) | ((word>>8)&0xFF) );
#endif
}


void set_bit(unsigned char *bits, int pos, int state)
{
	int mask = (0x80 >> (pos%8));

	if (state != 0)
		bits[pos / 8] = bits[pos / 8] | mask;
	else
		bits[pos / 8] = bits[pos / 8] & (~mask);

}

int get_bit(unsigned char *bits, int pos)
{
	int mask;
	mask = (0x80 >> (pos%8));
	return (bits[ pos / 8 ] & mask) == mask ? 1:0;
}

void print_the_string(int string_num)
{
	/*
	// get the offset of the string
	// and the string after it unless its the last string from start of
	//	 room exit table.
	*/

	uint8_t *msg, *msg_end;
	uint16_t msg1, msg2;

	msg = (uint8_t *)game+fix_address(fix_word(dh.p_message));
	msg += string_num*2;

	msg1 = fix_address(get_word((uint8_t *)msg));
	msg2 = fix_address(get_word((uint8_t *)msg+2));

	msg = game+msg1;
	msg_end = game+msg2;

	while(msg < msg_end)
	{
		msg=print_string(msg);

		if(msg < msg_end)
			str_print(" ");
	}
}


void num_print(int num)
{
	char buffer[32];

	sprintf(buffer, "%i", num);
	str_print(buffer);
}


uint8_t *print_string(uint8_t *string)
{
	uint8_t buffer[48];
	uint8_t *msg;
	int length;

	msg=string;
	length=msg[0];
	msg++;

	if(inroom_flag==1)
	{
		if(msg[0]!='*')
		{
			str_print("~");
			str_print(messages[msg_i_am_in_a]);
		}
		else
		{
			msg++;
			length--;
		}
	}

	strncpy((char*)buffer, (char*)msg, length);
	buffer[length]=0;

	str_print((char*)buffer);

	msg+=length;

	return(msg);
}

void str_print(char *msg)
{
	gprintf(msg);
}

void run_game(void)
{
	/*  guts of game  */
	print_title_screen();

	roomwin = glk_window_open(mainwin, winmethod_Above | winmethod_Proportional, 30, wintype_TextBuffer, 0);
	assert(roomwin != NULL);

	glk_set_window(roomwin);
	glk_window_clear(roomwin);

	gprintf("Loading.....");

	glk_set_window(mainwin);
	glk_window_clear(mainwin);

	/* load game */
	load_save_game();

	/* create ANY word (1st word in dictionary) */
	decomp_word(0, dh.p_noun_table, dh.num_nouns);
	any_word = strdup(in_verb);
	bunyon_strupr(any_word);

	/* execute act auto NNN routines */
	run_implicit();

	/* issue a room look */
	move_room = 1;
	opcode_F0();
	move_room = 0;

	while(game_flag == 0)
	{
		count_light = 0;

		str_print(messages[msg_tell_me_what_to_do]);
		grab_input();
		str_print("~");

		/*
			run the commands!
			STATIC
		*/
		do_fixwords();

		verb_num=word_lookup(in_verb, dh.p_verb_table, dh.num_verbs);


		if(in_noun[0] == 0)
		{
			noun_num = 1;
			strcpy(in_noun, any_word);
		}

		noun_num=word_lookup(in_noun, dh.p_noun_table, dh.num_nouns);
		item_num=noun_item(noun_num, 0);


		if(verb_num == BAD_WORD && in_verb[0] == '#')
		{
			verb_num = debug_command();
			do_debug_command(verb_num);
		}
		else
		{

			/* run explicit verb/noun actions */
			run_explicit();
				/* str_print(messages[msg_ok]); */

			/* FIXME: dont countdown on doing save/invnetory etc.... */
			if(verb_num != BAD_WORD && count_light == 0)
				CountDownLight();

			/* do auto actions routines */
			if(game_flag == 0)
			{
				run_implicit();
			}

			if(game_flag == 0 && move_room != 0)
			{
				opcode_F0();
				move_room = 0;
			}
		}
	}

	gprintf("~~Press any key to exit.");
}

/*
	turns NORTH/N, into GO NORTH

	north, south, east, west, up, down
	convert N, S, E, W, U, D
	ScottFree carries "I" for brian howarth interpreters
	(probably no brian howarth games for the TI99/4A...)
*/
void do_fixwords(void)
{
	int i;

	for(i=0; fixwords[i]!=NULL; i+=2)
	{
		/* // if v/n = N/ANY, make GO/NORTH */
		if(strcmp(in_verb,fixwords[i+0])==0)
		{
			strcpy(in_noun, fixwords[i+1]);
			strcpy(in_verb, "GO");
		}

		/* // if v/n = NORTH/ANY, make GO/NORTH */
		if(strcmp(in_verb, fixwords[i+1])==0)
		{
			strcpy(in_noun, fixwords[i+1]);
			strcpy(in_verb, "GO");
		}

		/* // if v/n == GO/N, make it GO/NORTH */
		if(strcmp(in_noun, fixwords[i+0])==0)
			strcpy(in_noun, fixwords[i+1]);
	}

	/* fix for brian howarth games */
	for(i=0; fixwords2[i]!=NULL; i+=2)
	{
		if(strcmp(in_verb, fixwords2[i+0])==0)
		{
			strcpy(in_verb, fixwords2[i+1]);
		}
	}
}


int word_lookup(char *word, uint16_t table, int num_words)
{
	uint16_t	*wtable;
	int 	w;
	int 	current_word;
	int 	word_len;

	int		real_num;

	char	*w1;
	char	*w2;

	char	dword[32];
	char	kword[32];

	/* table is either verb or noun table */
	wtable = (uint16_t*)(game + fix_address(fix_word(table)));

	current_word = 0;

	w = BAD_WORD;

	real_num = current_word;

	strncpy(kword, word, 32);
	kword[dh.cmd_length] = 0;

	while(current_word <= num_words && w == BAD_WORD)
	{
		/* get first word offset
		   get second word offset
		   calc size length.
		*/

		do
		{
			w1 = (char*) game + fix_address(get_word((uint8_t *) wtable+(current_word*2)));
			w2 = (char*) game + fix_address(get_word((uint8_t *) wtable+((1+current_word)*2)));

			/* this tests for missing words. */
			if(w1 == w2)
			{
				real_num += 1;
				current_word += 1;
			}
		}while(w1 == w2);

		word_len = w2 - w1;

		/* is it a synonym? no, so our word id is current word number.*/
		/* synonym words are skipped numerically */
		if(w1[0] != '*')
		{
			real_num = current_word; /* += 1; */
		}
		else
		{
			w1 += 1;
			word_len -= 1;
		}

		strncpy(dword, w1, word_len);
		dword[word_len] = 0;

		if(strcmp(dword, kword) == 0)
				return real_num;


		current_word += 1;
	}

	return BAD_WORD;
}


void print_room_descr(int room)
{
	/* str_print("~~~---------------------------------------~"); */
	print_table_string(dh.p_room_descr, room);
	str_print("~");
}

void print_table_string(uint16_t table, int table_offset)
{
	uint8_t *msgx, *msgy;
	uint16_t msg1, msg2;

	int f;

	f = inroom_flag;

	msgx=game+fix_address(fix_word(table));

	//msg0=fix_address(get_word((uint8_t*)msgx));

	msgx+=table_offset*2;
	msg1=fix_address(get_word((uint8_t*)msgx));
	msg2=fix_address(get_word((uint8_t*)msgx+2));

	msgy=game+msg2;
	msgx=game+msg1;

	while(msgx<msgy)
	{
		msgx=print_string(msgx);
		if(msgx<msgy)
			str_print(" ");

		inroom_flag = 0;
	}

	inroom_flag = f;
}


void print_room_exits(int room)
{
	uint8_t *p;
	uint16_t x;
	char *exits[6]={"North",
					 "South",
					 "East",
					 "West",
					 "Up",
					 "Down"
					};
	int flag;


	p = (uint8_t*)game+fix_address(fix_word(dh.p_room_exit));
	p += room*6;


	{
		str_print(messages[msg_obv_exits]);
		for(x=0, flag=0; x<6; x++)
		{
			if(*(p+x)!=0)
			{
				if(flag!=0)
					str_print(", ");
				flag++;
				str_print(exits[x]);
			}
		}
		if(flag==0)
			str_print("none");
		str_print(".~~");
	}
}


int is_dark(void)
{
	/* Lightsource NOT Avail (location or inventory) */
	return opcode_BC(ITEM_LIGHTSOURCE);
}

int is_really_dark(void)
{
	int flag=X_FALSE;

	/* no lightsource and DARK FLAG is on. */
	if(is_dark()==X_TRUE && opcode_C1(FLAG_DARKFLAG)==X_TRUE)
		flag=X_TRUE;

	return flag;
}


void print_room_items(int room)
{
	uint8_t *p;
	int flag;

	uint16_t i;

	p=game+fix_address(fix_word(dh.p_obj_table));

	flag=0;

	str_print(messages[msg_visible_items]);

	for(i=0; i<=dh.num_objects; i++)
	{
		if(*(p+i)==room)
		{
			if(flag>0)
				gprintf(", ");

			print_table_string(dh.p_obj_descr, i);
			flag++;
		}
	}

	if(flag==0)
		str_print(messages[msg_nothing]);

	gprintf(".~");
}

/* turn a noun into an item number...  */
int noun_item(int noun_word_num, int from)
{
	uint8_t *p;
	int x;
	int last_ava=BAD_WORD;

	if(noun_word_num==BAD_WORD || noun_word_num==0)
		return BAD_WORD;

	/* use p_link_table?? */
	p=(uint8_t*)game+fix_address(fix_word(dh.p_obj_link));
	p += from;

	for(x=from; x <= dh.num_objects; x++, p++)
	{
		/* printf("noun=%i -- item=%i -- location=%i~", noun_word_num, x, *p); */

		if(*p == noun_word_num)
		{
			/* check if item is available. */
			if(opcode_B9(x)==X_TRUE)
			{
				return(x);
			}
			/* else
				last_ava=x; */
		}
	}

	return(last_ava);
}

int count_inventory(void)
{
	int obj_count;
	int i;
	uint8_t *p;

	obj_count = 0;

	p = game + fix_address(fix_word(dh.p_obj_table));

	for(i=0; i<=dh.num_objects; i++, p++)
	{
		if(*p == ROOM_INVENTORY)
			obj_count++;
	}

	return(obj_count);
}

void printstring(char *s)
{
	char *p;

	p = s;

	while(*p!=0x0)
	{
		switch(*p)
		{
			/* (C) symbol in ti99 font, intro screens */
			case 0x0A:
				glk_put_string("c");
				p++;
				/* TI99 ppl add a ) to close the (C symbol... */
				if(*p==')')
					*p=' ';
				break;

			/* block character in ti99font */
			case 0x7F:
				glk_put_string("#");
				p++;
				break;

			case '\t':
				glk_put_string("    ");
				p++;
				break;

			case '\\':
				p++;
				glk_put_string(" ");
				break;

			case '~':
				p++;
				glk_put_char('\n');
				break;

			default:
				glk_put_char(*p);
				p++;
				break;
		}
	}
}

void gprintf(char *strX, ...)
{
	va_list	args;

	va_start(args, strX);
	vsprintf(strLogBuffer3, strX, args);

	if(action==3)
		decomp_printstring(strLogBuffer3);
	else
		printstring(strLogBuffer3);
	va_end(args);
}


void load_game(void)
{
	FILE *fp;
	long fsize;

	int i;
	int calchsize;

	memset(&h, 0x0, sizeof(struct HEADER));

	fp = fopen(game_file, "rb");
	if(fp != NULL)
	{
		/* return file size */
		fseek(fp, 0x0L, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0x0L, SEEK_SET);

		fread(&h, 1, sizeof(struct HEADER), fp);

		/* measure file size against header file size */

		calchsize = (h.num_sectors*256)+ h.num_sectors_mod;

		if(fsize-128 != calchsize)
		{
			gprintf("Possible ERROR!! : Game data file size is correct.~");
			gprintf("\tSize is %li, Header reports %i~", fsize-128, calchsize);
		}
		else
		{
			/* gprintf("Header size is correct~"); */
		}

		/* round up */
		fsize =  (fsize + 31) & ~(16-1);
		game_size = fsize;

		game = calloc(1, fsize);
		if(game == NULL)
		{
			gprintf("Not enough memory to load game.~");
			fclose(fp);
			glk_exit();
		}

		fseek(fp, 0x0L, SEEK_SET);

		/* loads into GLOBAL variable */
		fread(game, 1, fsize, fp);
		fclose(fp);

		/* copy header into GLOBAL variable */
		memcpy(&dh, game+0x8A0, sizeof(struct DATAHEADER));

		gm->locn=dh.begin_locn;		/* set start location */
		gm->cur_timer=0;			/* set current timer to 0 */
		gm->inv_flag=0;				/* display inventory */
		gm->light = fix_word(dh.light_turns);	/* turns for light */
		gm->cur_timer = 0;			/* set current counter to 0 */
		gm->sgeo_game_flag = 0;

		game_flag=0;				/* run game flag */
		inroom_flag=0;				/* in a room flag (needed for printing) */

		verb_num = 0;
		noun_num = 0;
		item_num = 0;

		/* set undo to false */
		gm->undo_data_ok = 0;

		calc_max_messages();
		calc_max_items();

		/* reset random state + save */
		reset_rnd_seed();
		gm->rnd_seed = get_rnd_seed();

		/* clean timers */
		gm->cur_timer = 0;
		for(i=0; i<MAX_TIMERS; i++)
			gm->timers[i] = 0;

		/* clean RRV */
		for(i=0; i<MAX_RRV; i++)
			gm->room_rv[i] = 0;

		/* clean flags */
		for(i=0; i<MAX_FLAGS; i++)
			opcode_E2(i);

		/* test some pointers for valid game... */

		assert(game_size >= fix_address(fix_word(dh.p_obj_table)));
		assert(game_size >= fix_address(fix_word(dh.p_orig_items)));
		assert(game_size >= fix_address(fix_word(dh.p_obj_link)));
		assert(game_size >= fix_address(fix_word(dh.p_obj_descr)));
		assert(game_size >= fix_address(fix_word(dh.p_message)));
		assert(game_size >= fix_address(fix_word(dh.p_room_exit)));
		assert(game_size >= fix_address(fix_word(dh.p_room_descr)));
		assert(game_size >= fix_address(fix_word(dh.p_noun_table)));
		assert(game_size >= fix_address(fix_word(dh.p_verb_table)));
		assert(game_size >= fix_address(fix_word(dh.p_explicit)));
		assert(game_size >= fix_address(fix_word(dh.p_implicit)));
	}
	else
	{
		gprintf("Failed to open the file '%s'.~", game_file);
		glk_exit();
	}
}


void decomp_printstring(char *s)
{
	char *p;

	p = s;

	while(*p!=0x0)
	{
		switch(*p)
		{
			/* (C) symbol in ti99 font, intro screens */
			case 0x0A:
				glk_put_string("c");
				p++;
				/* TI99 ppl add a ) to close the (C symbol... */
				if(*p==')')
					*p=' ';
				break;

			/* block character in ti99font */
			case 0x7F:
				glk_put_string("#");
				p++;
				break;

			case '\t':
				glk_put_string("    ");
				p++;
				break;
			case '~':
				glk_put_string("\n");
				p++;
				break;

			default:
				glk_put_char(*p);
				p++;
				break;
		}
	}
}


uint8_t *decomp_string(uint8_t *string)
{
	uint8_t buffer[48];
	uint8_t *msg;
	int length;

	msg=string;
	length=msg[0];
	msg++;

	if(inroom_flag==1)
	{
		if(msg[0]!='*')
		{
			str_print(messages[msg_i_am_in_a]);
		}
		else
		{
			msg++;
			length--;
		}
	}

	strncpy((char*)buffer, (char*)msg, length);
	buffer[length]=0;

	str_print((char*)buffer);

	msg+=length;

	return(msg);
}


void decomp_table_string(uint16_t table, int table_offset)
{
	uint8_t *msgx, *msgy;
	uint16_t msg1, msg2;

	int f;

	f = inroom_flag;

	msgx=game+fix_address(fix_word(table));

	//msg0=fix_address(get_word((uint8_t*)msgx));

	msgx+=table_offset*2;
	msg1=fix_address(get_word((uint8_t*)msgx));
	msg2=fix_address(get_word((uint8_t*)msgx+2));

	msgy=game+msg2;
	msgx=game+msg1;

	while(msgx<msgy)
	{
		msgx=decomp_string(msgx);
		if(msgx<msgy)
			str_print(" ");

		inroom_flag = 0;
	}

	inroom_flag = f;
}

void decomp_word(int word, uint16_t table, int num_words)
{
	uint16_t	*wtable;
	int 	i;
	int 	word_len;

	int		real_num;

	char	*w1;
	char	*w2;

	/* table is either verb or noun table */
	wtable = (uint16_t*)(game + fix_address(fix_word(table)));

	i = 0;
	real_num = i;

	while(i <= num_words)
	{
		/* get first word offset
		   get second word offset
		   calc size length.
		*/

		do
		{
			w1 = (char*) game + fix_address(get_word((uint8_t *) wtable+(i*2)));
			w2 = (char*) game + fix_address(get_word((uint8_t *) wtable+((1+i)*2)));

			/* this tests for missing words. */
			if(w1 == w2)
			{
				real_num += 1;
				i += 1;
			}
		}while(w1 == w2);

		word_len = w2 - w1;

		if(w1[0] != '*')
		{
			real_num = i; /* += 1; */
		}
		else
		{
			w1 += 1;
			word_len -= 1;
		}

		memset(in_verb, 0x0, 32);
		strncpy(in_verb, w1, word_len);
		bunyon_strlwr(in_verb);

		i += 1;

		if(real_num==word)
			return;
	}
}


void decomp_wordsyns(int word, uint16_t table, int num_words)
{
	uint16_t	*wtable;
	int 	i;
	int 	word_len;

	int		real_num;

	char	*w1;
	char	*w2;
	int		wo;

	/* table is either verb or noun table */
	wtable = (uint16_t*)(game + fix_address(fix_word(table)));

	i = 0;
	real_num = i;
	wo = 0;

	while(i <= num_words)
	{
		/* get first word offset
		   get second word offset
		   calc size length.
		*/

		do
		{
			w1 = (char*) game + fix_address(get_word((uint8_t *) wtable+(i*2)));
			w2 = (char*) game + fix_address(get_word((uint8_t *) wtable+((1+i)*2)));

			/* this tests for missing words. */
			if(w1 == w2)
			{
				real_num += 1;
				i += 1;
			}
		}while(w1 == w2);

		word_len = w2 - w1;

		if(w1[0] != '*')
		{
			real_num = i; /* += 1; */

			if(real_num>word)
				return;
		}
		else
		{
			w1 += 1;
			word_len -= 1;
		}

		memset(in_verb, 0x0, 32);
		strncpy(in_verb, w1, word_len);
		bunyon_strlwr(in_verb);

		i += 1;

		if(real_num == word)
		{
			if(wo>0)
				gprintf(", ");
			gprintf("%s", in_verb);
			wo++;
		}
	}
}



void decomp_dict(int vorn, uint16_t table, int num_words)
{
	uint16_t	*wtable;
	int 	i;
	int 	word_len;
	char	*w1;
	char	*w2;

	int 	skip = 1;

	/* table is either verb or noun table */
	wtable = (uint16_t*)(game + fix_address(fix_word(table)));

	i = 1;

	while(i <= num_words)
	{
		do
		{
			w1 = (char*) game + fix_address(get_word((uint8_t *) wtable+(i*2)));
			w2 = (char*) game + fix_address(get_word((uint8_t *) wtable+((1+i)*2)));

			/* this tests for missing words. */
			if(w1 == w2)
			{
				i += 1;
			}
		}while(w1 == w2);

		word_len = w2 - w1;

		if(w1[0] != '*')
		{
			if(i > 0)
				if(!skip)
					gprintf(";~");

			skip = 0;

			if(vorn == 0)
			{
				switch(i)
				{
					case AUTO_VERB: skip = 1; break;
					case GO_VERB: skip = 1; break;
					case GET_VERB: skip = 1; break;
					case DROP_VERB: skip = 1; break;
				}

				if(!skip)
					gprintf("verb ");
			}
			else
			{
				switch(i)
				{
					case 1: skip = 1; break;
					case 2: skip = 1; break;
					case 3: skip = 1; break;
					case 4: skip = 1; break;
					case 5: skip = 1; break;
					case 6: skip = 1; break;
				}

				if(!skip)
					gprintf("noun ");
			}

		}
		else
		{
			if(!skip)
				gprintf(", ");

			w1 += 1;
			word_len -= 1;
		}

		memset(in_verb, 0x0, 32);
		strncpy(in_verb, w1, word_len);
		bunyon_strlwr(in_verb);

		i += 1;

		if(!skip)
			gprintf("%s", in_verb);
	}
	gprintf(";~~");
}


void decomp_implicit(void)
{
	uint8_t *ptr;
	int loop_flag;

	/* run all act auto codes */

	ptr = (uint8_t *)game + fix_address(fix_word(dh.p_implicit));
	loop_flag = 0;

	/* fall out, if no auto acts in the game. */
	if(*ptr == 0x0)
		loop_flag = 1;

	while(loop_flag == 0)
	{
		gprintf("~act auto %i~", ptr[0]);
		gprintf("{~");
		decomp_code_chunk(ptr + 2);
		gprintf("}~");

		if(ptr[1] == 0)
			loop_flag = 1;

		/* skip code chunk */
		ptr += 1 + ptr[1];
	}
}


void decomp_the_string(int string_num)
{
	uint8_t *msg, *msg_end;
	uint16_t msg1, msg2;

	msg = (uint8_t *)game+fix_address(fix_word(dh.p_message));
	msg += string_num*2;

	msg1 = fix_address(get_word((uint8_t *)msg));
	msg2 = fix_address(get_word((uint8_t *)msg+2));
	msg_end = NULL;

	msg = game+msg1;
	msg_end = game+msg2;

	while(msg < msg_end)
	{
		msg=print_string(msg);

		if(msg < msg_end)
			str_print(" ");
	}
}

void decomp_title(void)
{
	char line[80];
	char *p;
	int lines;
	char *q;

	p=(char*)game+0x80;		/* title screen offset starts at 0x80 */

	gprintf("~title_screen = ~");

	for(lines=0; lines<24; lines++)
	{
		strncpy((char *)line, (char *)p, 40);
		p+=40;
		line[40]=0x0;
		q = strchr(line, 0x0);

		if(q>line)
		{
			q--;
			while(*q==0x20)
				q--;

			q++;
			*q=0;
		}


		gprintf("\t\t\"%s\"", line);
		if(lines<23)
			gprintf(",");
		gprintf("~");
	}

	gprintf("\t\t;~");
}

void decomp_item_name(int item)
{
	gprintf("\t\t# ");
	decomp_table_string(dh.p_obj_descr, item);
}

void decomp_items(void)
{
	int i;
	uint8_t *ptr;
	uint8_t *oitem;
	uint8_t *descr;
	uint8_t *noun;

	ptr = game + fix_address(fix_word(dh.p_obj_table));		/* item table */
	oitem = game + fix_address(fix_word(dh.p_orig_items));	/* start room */
	descr = game + fix_address(fix_word(dh.p_obj_descr));	/* descr room */
	noun = game + fix_address(fix_word(dh.p_obj_link));	/* descr room */

	for(i=0; i<=dh.num_objects; i+=1)
	{
		gprintf("~item");

		if(i==ITEM_LIGHTSOURCE)
			gprintf(" (light)");

		gprintf(" itm_%03i", i);

		if(*noun != 0)
		{
			decomp_word(*noun, dh.p_noun_table, dh.num_nouns);
			gprintf(" (%s)", in_verb);
		}

		gprintf(" ~\t\"");
		if(i < max_item_descr)
			decomp_table_string(dh.p_obj_descr, i);
		gprintf("\"");

		if(*ptr == ROOM_LIMBO)
			gprintf(" void;~");
		else if(*ptr == ROOM_INVENTORY)
			gprintf(" player;~");
		else gprintf(" rm_%03i;~", *ptr);

		descr++;
		oitem++;
		ptr++;
		noun++;
	}
}

void calc_max_messages(void)
{
	uint8_t *msg;
	uint16_t msg1;

	msg = (uint8_t *)game+fix_address(fix_word(dh.p_message));
	msg1 = fix_address(get_word((uint8_t *)msg));
	max_messages = (msg1 - fix_address(fix_word(dh.p_message)))/2;
}

void calc_max_items(void)
{
	uint8_t *msg;
	uint16_t msg1;

	msg = (uint8_t *)game+fix_address(fix_word(dh.p_obj_descr));
	msg1 = fix_address(get_word((uint8_t *)msg));
	max_item_descr = (msg1 - fix_address(fix_word(dh.p_obj_descr)))/2;
}

void decomp_msgs(void)
{
	int string_num;
	int ms;

	uint8_t *msg;
	uint16_t msg1;

	msg = (uint8_t *)game+fix_address(fix_word(dh.p_message));
	msg1 = fix_address(get_word((uint8_t *)msg));
	msg1 -= 2;
	ms = msg1 - fix_address(fix_word(dh.p_message));
	ms /= 2;

	gprintf("# %i messages~~", ms);

	for(string_num=0; string_num<ms; string_num+=1)
	{
		gprintf("alias msg_%03i = \"", string_num);
		decomp_the_string(string_num);
		gprintf("\";~");
	}
}

void decomp_rooms(void)
{
	int i;
	char *exits[6]={"n_to",
					 "s_to",
					 "e_to",
					 "w_to",
					 "u_to",
					 "d_to"
					};
	uint8_t *p;
	uint16_t x;
	int flag;

	gprintf("~~# %i rooms~", dh.red_room);

	for(i=1; i<=dh.red_room; i+=1)
	{
		inroom_flag = 1;

		gprintf("~room rm_%03i ~", i);
		gprintf("\t\"");

		decomp_table_string(dh.p_room_descr, i);

		gprintf("\"");


		p = (uint8_t*)game+fix_address(fix_word(dh.p_room_exit));
		p += i*6;

		for(x=0, flag=0; x<6; x++)
		{
			if(*(p+x)!=0)
			{
				if(flag==0)
					gprintf(",~\t");
				else
					gprintf(", ");
				gprintf("%s rm_%03i", exits[x], *(p+x));
			}
		}

		gprintf(";~");
	}
}

void decomp_explicit(void)
{
	uint8_t *p;
	uint16_t addy;
	int flag;
	int i;

	gprintf("~~");

	for(i=0; i<=dh.num_verbs; i+=1)
	{
		p = game + fix_address(fix_word(dh.p_explicit));
		addy = get_word(p + ( (i ) *2) );

		if(addy != 0)
		{
			p=game+fix_address(addy);
			flag = 0;

			while(flag != 4)
			{
				gprintf("act ");
				decomp_word(i, dh.p_verb_table, dh.num_verbs);
				gprintf("%s ", in_verb);
				decomp_word(p[0], dh.p_noun_table, dh.num_nouns);
				gprintf("%s~{~", in_verb);

				if(p[1] == 0)
					flag = 4;	/* now run get/drop stuff */

				/* we have verb/noun match. run code! */
				decomp_code_chunk(p+2);

				/* go to next block. */
				p += 1 + p[1];

				gprintf("}~~");
			}
		}
	}
}


void decompile_game(void)
{
	int i;

	f_sourcecode =  glk_fileref_create_by_name(fileusage_TextMode, source_file, 0);
	s_sourcecode = glk_stream_open_file(f_sourcecode, filemode_Write, 0);
	glk_window_set_echo_stream(mainwin, s_sourcecode);

	decomp_title();

	gprintf("~");

	gprintf("word_length = %i;~", dh.cmd_length);
	gprintf("light_time = %i;~", fix_word(dh.light_turns));
	gprintf("max_load = %i;~", dh.max_items_carried);

	gprintf("~");

	for(i=0; i<MAX_FLAGS; i++)
	{
		gprintf("flag ");
		if(i==15)
			gprintf("(night) ");
		if(i==16)
			gprintf("(runout) ");

		gprintf("flag_%02i;~", i);
	}
	gprintf("~");

	for(i=0; i<MAX_TIMERS; i++)
	{
		gprintf("timreg tr_%02i;~", i);
	}
	gprintf("~");

	for(i=0; i<MAX_RRV; i++)
	{
		gprintf("roomreg rr_%02i;~", i);
	}
	gprintf("~");

	gprintf("noun(north) ");
	decomp_wordsyns(1, dh.p_noun_table, dh.num_nouns);
	gprintf(";~");

	gprintf("noun(south) ");
	decomp_wordsyns(2, dh.p_noun_table, dh.num_nouns);
	gprintf(";~");

	gprintf("noun(east) ");
	decomp_wordsyns(3, dh.p_noun_table, dh.num_nouns);
	gprintf(";~");

	gprintf("noun(west) ");
	decomp_wordsyns(4, dh.p_noun_table, dh.num_nouns);
	gprintf(";~");

	gprintf("noun(up) ");
	decomp_wordsyns(5, dh.p_noun_table, dh.num_nouns);
	gprintf(";~");

	gprintf("noun(down) ");
	decomp_wordsyns(6, dh.p_noun_table, dh.num_nouns);
	gprintf(";~");

	decomp_dict(1, dh.p_noun_table, dh.num_nouns);

	strcpy(in_verb, "drop");
	in_verb[dh.cmd_length] = 0;

	gprintf("verb(go) ");
	decomp_wordsyns(GO_VERB, dh.p_verb_table, dh.num_verbs);
	gprintf(";~");

	gprintf("verb(drop) ");
	decomp_wordsyns(DROP_VERB, dh.p_verb_table, dh.num_verbs);
	gprintf(";~");

	gprintf("verb(get) ");
	decomp_wordsyns(GET_VERB, dh.p_verb_table, dh.num_verbs);
	gprintf(";~");

	decomp_dict(0, dh.p_verb_table, dh.num_verbs);


	decomp_msgs();

	inroom_flag = 1;
	decomp_rooms();
	inroom_flag = 0;

	decomp_items();

	decomp_implicit();
	decomp_explicit();

	gprintf("~");

	gprintf("treasure=rm_%03i;~", dh.treasure_locn);
	gprintf("initial=rm_%03i;~", dh.begin_locn);

	gprintf("~");
	gprintf("~");
}



/* countdown to dark */
void CountDownLight(void)
{
	/* brian howarth uses -1, but i dont think his games made the ti.. */

	/* does light EXIST? */
	if(opcode_BD(ITEM_LIGHTSOURCE)==X_TRUE && gm->light >= 0)
	{
		gm->light -= 1;

		if(gm->light < 0)
		{
			opcode_E1(FLAG_RUNOUT);

			/* if lamp is in current room or inventory */
			if(opcode_B9(ITEM_LIGHTSOURCE)==X_TRUE)
			{
				gprintf("%s", messages[msg_light_out]);

				/* issue look command! */
				move_room = 1;
			}
		}
		else if (gm->light < 25)
		{
			/* if lamp is available */
			if(opcode_B9(ITEM_LIGHTSOURCE)==X_TRUE)
			{
				if(gm->light % 5 ==0 )
				{
					gprintf("%s", messages[msg_dim_light]);
				}
			}
		}
	}
}

glui32 get_keypress(void)
{
	event_t ev;
	glui32 key;

	/* cancel all input events pending. */
	glk_cancel_char_event(mainwin);
	glk_cancel_line_event(mainwin, &ev);

	glk_request_char_event(mainwin);
	do
	{
		glk_select(&ev);

		if(ev.type == evtype_CharInput)
			key = ev.val1;

	}while(ev.type != evtype_CharInput);

	return key;
}


void glk_main(void)
{
	/* initialise the colours for the windows... */
	glk_stylehint_set(wintype_TextBuffer, style_Normal, stylehint_BackColor, 0x0073C873);
	glk_stylehint_set(wintype_TextBuffer, style_Normal, stylehint_TextColor, 0x00000000);
	glk_stylehint_set(wintype_TextBuffer, style_Normal, stylehint_Weight, 1);
	glk_stylehint_set(wintype_TextBuffer, style_Normal, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_TextBuffer, style_Normal, stylehint_ReverseColor, 0);

	glk_stylehint_set(wintype_TextBuffer, style_Input, stylehint_BackColor, 0x0073C873);
	glk_stylehint_set(wintype_TextBuffer, style_Input, stylehint_TextColor, 0x00000000);
	glk_stylehint_set(wintype_TextBuffer, style_Input, stylehint_Weight, 1);
	glk_stylehint_set(wintype_TextBuffer, style_Input, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_TextBuffer, style_Input, stylehint_ReverseColor, 0);

	mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	assert(mainwin != NULL);


	/* create new game + undo data.. */
	gm = (struct GAME*)calloc(1, sizeof(struct GAME));
	gm_undo = (struct GAME*)calloc(1, sizeof(struct GAME));
	load_game();

	glk_set_window(mainwin);
	glk_window_clear(mainwin);

	if(action == 3)
	{
		decompile_game();
		glk_exit();
	}

	assert(gm != NULL);
	assert(gm_undo != NULL);

	/* open script input */
	if(script_file != NULL)
	{
		f_script_input = NULL;
		s_script_input = NULL;

		f_script_input =  glk_fileref_create_by_name(fileusage_InputRecord, script_file, 0);
		if (!f_script_input)
		{
			gprintf("Unable to place script file.~");
			glk_exit();
		}

		s_script_input = glk_stream_open_file(f_script_input, filemode_Read, 0);
		if(!s_script_input)
		{
			gprintf("~~Could not open script file for input.~");
			glk_exit();
		}

		scripting = 1;
		gprintf("input action script is on with : %s~", script_file);


		set_rnd_seed(saved_seed);
	}

	if(source_file != NULL)
	{
		f_output = NULL;
		s_output = NULL;

		f_output =  glk_fileref_create_by_name(fileusage_Transcript, source_file, 0);

		if(!f_output)
		{
			gprintf("Unable to create source output file.~");
			glk_exit();
		}

		s_output = glk_stream_open_file(f_output, filemode_Write, 0);
		glk_window_set_echo_stream(mainwin, s_output);

		gprintf("output transcript is on with : %s~", source_file);
	}

	run_game();
}

void grab_input(void)
{
	/* // take input, split into verb/noun. */
	char	ibuff[80];
	char	*p;
	int 	i;

	event_t ev;

	memset(ibuff, 0x0, 80);

	if(s_script_input != NULL)
	{
		gprintf("(script) ");

		memset(ibuff, 0x0, 80);
		glk_get_line_stream(s_script_input, ibuff, 30);
		p=strchr(ibuff, 0x0A); if(p!=NULL) *p=0x0;
		p=strchr(ibuff, 0x0D); if(p!=NULL) *p=0x0;

		/* close stream with blank line, or eof */
		if(ibuff[0] == 0)
		{
			glk_stream_close(s_script_input, NULL);
			s_script_input = NULL;
			gprintf(" (script closed) ");
		}
		else
		{
			glk_set_style(style_Input);
			gprintf("%s", ibuff);
			glk_set_style(style_Normal);
		}
	}

	/* if we hit end of stream, this catches our input... */
	if(s_script_input == NULL)
	{
		glk_request_line_event(mainwin, ibuff, 30, 0);

		i = 0;
		while(i==0)
		{
			glk_select(&ev);

			switch(ev.type)
			{
				case evtype_LineInput:
					if(ev.win==mainwin)
					{
						i = 1;
					}
					break;

				default:
					break;
			}
		}
	}

	/* strip cr/lf from input */
	p = ibuff;
	while(p!=NULL)
	{
		p = strchr(ibuff, 0x0A);
		if(p != NULL)
			*p = 0x0;
	}

	p = ibuff;
	while(p!=NULL)
	{
		p = strchr(ibuff, 0x0D);
		if(p != NULL)
			*p = 0x0;
	}

	/* strip spaces from front */
	p = ibuff;
	while(*p == ' ')
		p++;

	/* move to front. */
	memmove(ibuff, p, strlen(p)+1);

	/* strip spaces from back */
	p = strchr(ibuff, 0x0);
	p--;

	while(*p == ' ')
		p--;

	p++;
	*p = 0x0;

	/* clear verb/noun */
	memset(in_verb, 0x0, 32);
	memset(in_noun, 0x0, 32);

	/* make verb/noun */
	i = 0;
	p = ibuff;
	while(*p != 0x0 && i < 32 && *p!=' ')
	{
		in_verb[i++] = *p++;
	}

	/* skip to noun */
	while(*p==' ')
		p++;

	i=0;
	while(*p != 0x0 && i<32)
	{
		in_noun[i++] = *p++;
	}

	/* if(in_noun[0] == 0)
		strcpy(in_noun, any_word); */

	/* make upper case */
	p = in_verb;
	while(*p != 0x0)
	{
		*p = toupper(*p);
		p++;
	}

	p = in_noun;
	while(*p != 0x0)
	{
		*p = toupper(*p);
		p++;
	}
}

void load_save_game(void)
{
	glui32 i;
	FILE *fp;
	char *p;
	char *xbuff;

	gprintf("In this adventure, you may abbreviate any word by typing its first %i letters, and directions by typing 1 letter.~", dh.cmd_length);

	gprintf("~Use TAB to switch between windows and use PAGE UP/DOWN to scroll text~");

	gprintf("~Want to restore a previously saved game, Y/N? ");
	i = get_keypress();

	if(i == 'Y' || i == 'y')
	{
		xbuff = malloc(strlen(game_file) + 8);
		strcpy(xbuff, game_file);

		gprintf("Y ~Which save game to load, 0-9? ");
		i = get_keypress();

		if(i>='0' && i <= '9')
		{
			p = strchr(xbuff, 0x0);
			sprintf(p, ".sav_%c", (int)i);

			fp = fopen(xbuff, "rb");

			if(fp == NULL)
			{
				gprintf("~Could not open %s for reading.", xbuff);
			}
			else
			{
				gprintf("~Reading %s", xbuff);
				fread(gm, 1, sizeof(struct GAME), fp);
				fclose(fp);

				/* copy in saved items to buffer */
				p =(char*) game + fix_address(fix_word(dh.p_obj_table));
				memmove(p, (char*)gm->items, dh.num_objects+1);
			}
		}

		free(xbuff);
	}
	else
		gprintf("N~");

	gprintf("~~");
}


void save_game(void)
{
	char *xbuff;
	char *p;
	glui32 key;

	FILE *fp;

	/* copy in saved items to buffer */
	p = (char*) game + fix_address(fix_word(dh.p_obj_table));
	memmove(gm->items, p, 1+dh.num_objects);

	/* copy filename, and add .sav_N => n = 0-9 */
	xbuff = malloc(strlen(game_file) + 8);
	strcpy(xbuff, game_file);
	strcat(xbuff, ".sav_");

	key = 0;
	while(key == 0)
	{
		gprintf("~What save slot (0 to 9) or, ESC to not save? ");
		key = (char)get_keypress();

		if((key >= '0' && key <='9') || key==keycode_Escape)
		{
			if(key != keycode_Escape)
			{
				glk_put_char((char)(key&0xFF));
				str_print("~");
			}
			break;
		}
		else
		{
			key = 0;
		}
	}

	if(key!=keycode_Escape)
	{
		p = strchr(xbuff, 0x0);
		sprintf(p, "%c", (char)key&0xFF);

		gprintf("~Saving in slot %i.", key-0x30);

		fp = fopen(xbuff, "wb");
		if(fp == NULL)
		{
			gprintf("~Error trying to open %s for writing.~~GAME NOT SAVED.~", xbuff);
		}
		else
		{
			fwrite(gm, 1, sizeof(struct GAME), fp);
			fclose(fp);

			gprintf("~Saved as %s.~~", xbuff);
		}
	}
	else
		str_print("~Save Aborted.~");

	free(xbuff);
}

