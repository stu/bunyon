#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>

#include <garglk/glk.h>

#include "config.h"
#include "define.h"
#include "game.h"
#include "main.h"
#include "rnd.h"
#include "interp.h"
#include "msg.h"

enum
{
	RC_NULL = 0,
	RC_OK,
	RC_RAN_ALL_BLOCKS,
	RC_RAN_ALL_BLOCKS_FAILED
};

struct Keyword
{
	char 	*name;
	int		opcode;
	int		count;
};

struct Keyword actions[] =
{
	{"has",			0xB7, 1 },
	{"here",		0xB8, 1 },
	{"avail",		0xB9, 1 },
	{"!here",		0xBA, 1 },
	{"!has",		0xBB, 1 },
	{"!avail",		0xBC, 1 },
	{"exists",		0xBD, 1 },
	{"!exists",		0xBE, 1 },
	{"in",			0xBF, 1 },
	{"!in",			0xC0, 1 },
	{"set",			0xC1, 1 },
	{"!set",		0xC2, 1 },
	{"something",	0xC3, 0 },
	{"nothing",		0xC4, 0 },
	{"le",			0xC5, 1 },
	{"gt",			0xC6, 1 },
	{"eq",			0xC7, 1 },
	{"!moved",		0xC8, 1 },
	{"moved",		0xC9, 1 },

	{"--0xCA--",	0xCA, 0 },
	{"--0xCB--",	0xCB, 0 },
	{"--0xCC--",	0xCC, 0 },
	{"--0xCD--",	0xCD, 0 },
	{"--0xCE--",	0xCE, 0 },
	{"--0xCF--",	0xCF, 0 },
	{"--0xD0--",	0xD0, 0 },
	{"--0xD1--",	0xD1, 0 },
	{"--0xD2--",	0xD2, 0 },
	{"--0xD3--",	0xD3, 0 },

	{"cls",			0xD4, 0 },
	{"pic",			0xD5, 0 },
	{"inv",			0xD6, 0 },
	{"!inv",		0xD7, 0 },
	{"ignore",		0xD8, 0 },
	{"success",		0xD9, 0 },
	{"try",			0xDA, 1 },
	{"get",			0xDB, 1 },
	{"drop",		0xDC, 1 },
	{"goto",		0xDD, 1 },
	{"zap",			0xDE, 1 },
	{"on",			0xDF, 0 },	/* on dark */
	{"off",	 		0xE0, 0 },	/* off dark */
	{"on",			0xE1, 1 },	/* set flag */
	{"off",			0xE2, 1 },	/* clear flag */
	{"on",			0xE3, 0 },
	{"off",			0xE4, 0 },
	{"die",			0xE5, 0 },
	{"move",		0xE6, 2 },
	{"quit",		0xE7, 0 },
	{".score",		0xE8, 0 },
	{".inv",		0xE9, 0 },
	{"refill",		0xEA, 0 },
	{"save",		0xEB, 0 },
	{"swap",		0xEC, 2 },	/* swap items */
	{"steal",		0xED, 1 },
	{"same",		0xEE, 2 },
	{"nop",			0xEF, 0 },

	{".room",		0xF0, 0 },

	{"--0xF1--",	0xF1, 0 },
	{"add",			0xF2, 0 },
	{"sub",			0xF3, 0 },


	{".timer",		0xF4, 0 },
	{"timer",		0xF5, 1 },

	{"add",			0xF6, 1 },
	{"sub",			0xF7, 1 },

	/* TODO : implement Select RV (0xF8) and Swap RV (0xF9) */
	{"select_rv",	0xF8, 0 },
	{"swap_rv",		0xF9, 1 },

	{"swap",		0xFA, 1 },	/* swap flag */

	{".noun",		0xFB, 0 },
	{".noun_nl", 	0xFC, 0 },
	{".nl",			0xFD, 0 },
	{"delay",		0xFE, 0 },

	{"",			0xFF, 0}
};

void decomp_code_chunk(uint8_t *code_chunk);

int opcode_B7(int item);
int opcode_B8(int item);
int opcode_B9(int item);
int opcode_BA(int item);
int opcode_BB(int item);
int opcode_BC(int item);
int opcode_BD(int item);
int opcode_BE(int item);
int opcode_BF(int room);
int opcode_C0(int room);
int opcode_C1(int flagnum);
int opcode_C2(int flagnum);
int opcode_C3(void);
int opcode_C4(void);
int opcode_C5(int num);
int opcode_C6(int num);
int opcode_C7(int num);
int opcode_C8(int item);
int opcode_C9(int item);

void opcode_D4(void);
void opcode_D6(void);
void opcode_D7(void);
void opcode_D8(void);
void opcode_D9(void);

int opcode_DB(int item);
void opcode_DC(int item);

void opcode_DD(int room);

void opcode_DF(void);
void opcode_E0(void);
void opcode_E1(int flagnum);
void opcode_E2(int flagnum);
void opcode_E3(void);
void opcode_E4(void);
void opcode_E5(void);
void opcode_E6(int room, int item);
void opcode_E7(void);
void opcode_E8(void);
void opcode_E9(void);
void opcode_EA(void);
void opcode_EB(void);

void opcode_EC(int item1, int item2);
void opcode_ED(int item);
void opcode_EE(int item1, int item2);
void opcode_EF(void);
void opcode_F0(void);

void opcode_F4(void);
void opcode_F5(int value);
void opcode_F6(int value);
void opcode_F7(int value);
void opcode_F8(void);
void opcode_F9(int room);
void opcode_FA(int timer);
void opcode_FB(void);
void opcode_FC(void);
void opcode_FD(void);
void opcode_FE(void);

void print_title_screen(void)
{
	char line[80];
	char line2[41];
	char *p;
	int lines;

	int screen_width = 80;

	p=(char*)game+0x80;		/* title screen offset starts at 0x80 */

	memset(line, 0x0, 80);
	memset(line2, 0x0, 41);
	memset(line2, '=', 40);

	sprintf(line, "[ Bunyon v" VERSION " -- By Stu George ]");
	lines = strlen(line);
	memmove(line2 + ((40-lines)/2), line, lines);

	/* ======================================== */

	gprintf("%s~", line2);

	for(lines=0; lines<24; lines++)
	{
		strncpy((char *)line, (char *)p, 40);
		p+=40;
		line[40]=0x0;
		if(screen_width>40)
			gprintf("%s~", line);
		else
			gprintf("%s", line);
	}

	gprintf("============[ PRESS ENTER ]=============~");

	while(get_keypress()!=keycode_Return);
}

/* parses the plain act auto NNN {} routines */
void run_implicit(void)
{
	int chance;
	int verses;
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
		/*
			p+0 = percentage of happening
			p+1 = size of code chunk
			p+2 = start of code
		*/

		verses = ptr[0];
		chance = 1 + xrnd(100);

		if(verses >= chance)
			run_code_chunk(ptr + 2);

		if(ptr[1] == 0)
			loop_flag = 1;

		/* skip code chunk */
		ptr += 1 + ptr[1];
	}
}

void decomp_tabout(int i)
{
	for( ; i>0; i--)
		glk_put_char(' ');
}

void decomp_code_chunk(uint8_t *code_chunk)
{
	int run_code = 0;
	int index = 0;
	int tabout = 4;
	int dotry[32];
	int tryindex=0;

	int	total_size;

	total_size = *(code_chunk-1);
	if(total_size >0)		/* 0 = end chunk */
		total_size -= 2;

	for(tryindex=0; tryindex<32; tryindex++)
		dotry[tryindex] = 0;

	tryindex = 0;

	/* gprintf("Size : %03X~", total_size); */

	while(run_code == 0)
	{
		if(code_chunk[index] >= 0xB7)
		{
			/* int xx;
			int xc= 1 + actions[(code_chunk[index]) - 0xB7].count; */

			/* gprintf("%03X ", index);

			for(xx=0; xx < xc; xx+=1)
				gprintf("%02X ", code_chunk[index+xx]);

			for( ; xx < 3; xx += 1)
				gprintf("   ", 0);*/

			decomp_tabout(tabout);
			gprintf("%s", actions[(code_chunk[index]) - 0xB7].name );
		}
		else
		{
			/* int xx;
			int xc = 1 + actions[(code_chunk[index]) - 0xB7].count; */
			/* int xc = 1;
			int xx; */
			/* gprintf("%03X ", index); */

			/* assert(xc>=0 && xc <= 5); */
			/* for(xx=0; xx < xc; xx+=1)
				gprintf("%02X ", code_chunk[index+xx]);

			for( ; xx < 3; xx += 1)
				gprintf("   ", 0);*/

			decomp_tabout(tabout);
			/* gprintf("print msg_%03i #", code_chunk[index]); */
			gprintf("msg_%03i;", code_chunk[index]);
		}

		switch(code_chunk[index])
		{
			case 0xB7:		/* ITEM is in inventory */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xB8:		/* ITEM is in room */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xB9:		/* ITEM is available */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xBA:		/* ITEM is NOT in room */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xBB:		/* ITEM is NOT in inventory */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xBC:		/* object NOT available */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xBD:		/* object exists */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xBE:		/* object NOT exists */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xBF:		/* Player is in room X */
				gprintf(" (rm_%03i);", code_chunk[index + 1]);
				break;

			case 0xC0:		/* Player not in room X */
				gprintf(" (rm_%03i);", code_chunk[index + 1]);
				break;

			case 0xC1:
				gprintf(" (flag_%02i);", code_chunk[index + 1]);
				break;

			case 0xC2:		/* !set = if bitN is clear */
				gprintf(" (flag_%02i);", code_chunk[index + 1]);
				break;

			case 0xC3:		/* something */
				gprintf(";");
				break;

			case 0xC4:		/* nothing */
				gprintf(";");
				break;

			case 0xC5:		/* less than */
				gprintf(" (%i);", code_chunk[index + 1]);
				break;

			case 0xC6:		/* greater than */
				gprintf(" (%i);", code_chunk[index + 1]);
				break;

			case 0xC7:		/* == */
				gprintf(" (%i);", code_chunk[index + 1]);
				break;

			case 0xC8:		/* not moved item? */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xC9:		/* moved item? */
				gprintf(" (itm_%03i);", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xD4:		/* clear screen */
				gprintf(";");
				break;

			case 0xD6:		/* inv */
				gprintf(";");
				break;

			case 0xD7:		/* !inv */
				gprintf(";");
				break;

			case 0xD8:		/* ignore */
				gprintf(";");
				break;

			case 0xD9:		/* success */
				gprintf(";");
				break;

			case 0xDA:		/* try */
				gprintf("~");
				decomp_tabout(tabout);
				/* gprintf("             "); */
				gprintf("{");
				tabout += 4;
				dotry[tryindex++] = index + code_chunk[index+1] + 1;
				break;

			case 0xDB:		/* get item */
				gprintf(" itm_%03i;", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xDC:		/* drop item */
				gprintf(" itm_%03i;", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xDD:		/* goto room */
				gprintf(" rm_%03i;", code_chunk[index + 1]);
				break;

			case 0xDE:		/* move item B to room A */
				gprintf(" itm_%03i;", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xDF:		/* on darkness */
				gprintf(" flag_%02i;", FLAG_DARKFLAG);
				break;

			case 0xE0:		/* off darkness */
				gprintf(" flag_%02i;", FLAG_DARKFLAG);
				break;

			case 0xE1:		/* set flag X on */
				gprintf(" flag_%02i;", code_chunk[index + 1]);
				break;

			case 0xE2:		/* set flag X off */
				gprintf(" flag_%02i;", code_chunk[index + 1]);
				break;

			case 0xE3:		/* set flag 0 ON */
				gprintf(" flag_%02i;", 0);
				break;

			case 0xE4:		/* set flag 0 OFF */
				gprintf(" flag_%02i;",0);
				break;

			case 0xE5:		/* die */
				gprintf(";");
				break;

			case 0xE6:		/* move item B to room A */
				gprintf(" rm_%03i, itm_%03i;", code_chunk[index + 1], code_chunk[index + 2]);
				decomp_item_name(code_chunk[index + 2]);
				break;

			case 0xE7:		/* quit */
				gprintf(";");
				break;

			case 0xE8:		/* print score */
				gprintf(";");
				break;

			case 0xE9:		/* list contents of inventory */
				gprintf(";");
				break;

			case 0xEA:		/* refill */
				gprintf(";");
				break;

			case 0xEB:		/* save */
				gprintf(";");
				break;

			case 0xEC:		/* swap items 1 and 2 around */
				gprintf(" itm_%03i, itm_%03i;", code_chunk[index + 1], code_chunk[index + 2]);
				decomp_item_name(code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 2]);
				break;

			case 0xED:		/* move an item to the inventory */
				gprintf(" itm_%03i;", code_chunk[index + 1]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xEE:		/* make item1 same room as item2 */
				gprintf(" itm_%03i, rm_%03i;", code_chunk[index + 1], code_chunk[index + 2]);
				decomp_item_name(code_chunk[index + 1]);
				break;

			case 0xEF:		/* nop */
				gprintf(";");
				break;

			case 0xF0:		/* look at room */
				gprintf(";");
				break;

			case 0xF1:		/* TODO :: UNKOWN!! */
				gprintf(";");
				break;

			case 0xF2:		/* add 1 to timer */
				gprintf(" 1;");
				break;

			case 0xF3:		/* sub 1 from timer */
				gprintf(" 1;");
				break;

			case 0xF4:		/* print current timer */
				gprintf(";");
				break;

			case 0xF5:		/* set timer */
				gprintf(" %i;", code_chunk[index + 1]);
				break;

			case 0xF6:		/*  add to timer */
				gprintf(" %i;", code_chunk[index + 1]);
				break;

			case 0xF7:		/* sub from timer */
				gprintf(" %i;", code_chunk[index + 1]);
				break;

			case 0xF8:		/*  TODO: SELECT_RV */
				/* gprintf(" %i", code_chunk[index + 1]); */
				gprintf(";");
				break;

			case 0xF9:		/* TODO: SWAP_RV */
				gprintf(" %i;", code_chunk[index+1]);
				break;

			case 0xFA:		/* swap timer */
				gprintf(" tr_%02i;", code_chunk[index + 1]);
				break;

			case 0xFB:		/* print noun */
				gprintf(";");
				break;

			case 0xFC:		/* print nount + newline */
				gprintf(";");
				break;

			case 0xFD:		/* print newline */
				gprintf(";");
				break;

			case 0xFE:		/* delay */
				gprintf(";");
				break;

			case 0xFF:		/* end of code block. */
				run_code = 1;
				break;

			default:
				if(code_chunk[index] <= 0xB6)
				{
					/* print_the_string(code_chunk[index]); */
					decomp_tabout(tabout);
					gprintf("# \"");
					inroom_flag = 0;
					if(code_chunk[index] < max_messages)
						decomp_the_string(code_chunk[index]);
					else
					{
						gprintf("*** msg_%03i", code_chunk[index]);
					}
					gprintf("\";");
				}
				else
				{
					run_code=1;

					gprintf("~ERR!: %04i) %02X %02X %02X~", index, code_chunk[0 + index], code_chunk[1 + index], code_chunk[2 + index]);
					glk_exit();
				}
				break;
		}


		if(run_code == 1 && code_chunk[index] == 0xFF && tryindex>0)
		{
			/* we hit an END command inside a TRY block. */
			gprintf("end;");
		}

		if(code_chunk[index] >= 0xB7)
			index += 1 + actions[(code_chunk[0 + index])- 0xB7].count;
		else
			index += 1;

		gprintf("~");

		/* we are now past the 0xFF end code.. */

		if(tryindex > 0 && index >= dotry[tryindex-1])
		{
			tryindex -= 1;
			tabout-=4;
			run_code = 0;

			/* gprintf("             "); */
			decomp_tabout(tabout);
			gprintf("}~");
		}
	}
}



/*
	returns 0 for successful
			1 for fail
			2 for try?
*/
int run_code_chunk(uint8_t *code_chunk)
{
	int run_code = 0;
	int index = 0;
	int rc = 0;

	int try_index;
	int try[32];

	/* set RC to fail (0=ok, 1=fail  */
	rc = 0;

	try_index = 0;

	if(dump_codes==1)
		gprintf("");

	while(run_code == 0)
	{
		if(dump_codes == 1)
		{
			if(code_chunk[0 + index] >= 0xb7)
			{
				int j;

				gprintf("%04i) [%02X] %s", index, code_chunk[0 + index], actions[(code_chunk[0 + index])- 0xB7].name );

				for(j = 0; j< actions[(code_chunk[0 + index])- 0xB7].count; j+=1)
				{
					gprintf(" %i [%02X]", code_chunk[1 + index + j], code_chunk[1 + index + j]);
				}
				gprintf("~");
			}
			else
			{
				gprintf("%04i) message %i [%02X]~", index, code_chunk[0+index], code_chunk[0+index]);
			}
		}


		switch(code_chunk[index])
		{
			case 0xB7:		/* ITEM is in inventory */
				if(opcode_B7(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xB8:		/* ITEM is in room */
				if(opcode_B8(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xB9:		/* ITEM is available */
				if(opcode_B9(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xBA:		/* ITEM is NOT in room */
				if(opcode_BA(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xBB:		/* ITEM is NOT in inventory */
				if(opcode_BB(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xBC:		/* object NOT available */
				if(opcode_BC(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xBD:		/* object exists */
				if(opcode_BD(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xBE:		/* object NOT exists */
				if(opcode_BE(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xBF:		/* Player is in room X */
				if(opcode_BF(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC0:		/* Player not in room X */
				if(opcode_C0(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC1:
				if( opcode_C1(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC2:		/* !set = if bitN is clear */
				if( opcode_C2(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC3:		/* something */
				if(opcode_C3() == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC4:		/* nothing */
				if(opcode_C4() == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC5:		/* less than */
				if(opcode_C5(code_chunk[index+1])==X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC6:		/* greater than */
				if(opcode_C6(code_chunk[index+1])==X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC7:		/* == */
				if(opcode_C7(code_chunk[index+1])==X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC8:		/* not moved item? */
				if(opcode_C8(code_chunk[index+1])== X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xC9:		/* moved item? */
				if(opcode_C9(code_chunk[index+1])== X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xD4:		/* clear screen */
				opcode_D4();
				break;

			case 0xD6:		/* inv */
				opcode_D6();
				break;

			case 0xD7:		/* !inv */
				opcode_D7();
				break;

			case 0xD8:		/* ignore */
				opcode_D8();
				break;

			case 0xD9:		/* success */
				opcode_D9();
				break;

			case 0xDA:		/* TODO: fix this opcode! try? */
				if(try_index>=32)
				{
					gprintf("~~ERROR :: Hit upper limit on try method.~");
					glk_exit();
				}

				try[try_index++] = index + code_chunk[index+1] + 1;
				break;

			case 0xDB:		/* get item */
				if(opcode_DB(code_chunk[index + 1]) == X_TRUE)
				{
				}
				else
				{
					run_code = 1;
					rc = 1;
				}
				break;

			case 0xDC:		/* drop item */
				opcode_DC(code_chunk[index+1]);
				break;

			case 0xDD:		/* goto room */
				opcode_DD(code_chunk[index+1]);
				break;

			case 0xDE:		/* move item B to room A */
				opcode_E6(ROOM_LIMBO, code_chunk[index + 1]);
				break;

			case 0xDF:		/* on darkness */
				opcode_DF();
				break;

			case 0xE0:		/* off darkness */
				opcode_E0();
				break;

			case 0xE1:		/* set flag X on */
				opcode_E1(code_chunk[index + 1]);
				break;

			case 0xE2:		/* set flag X off */
				opcode_E2(code_chunk[index + 1]);
				break;

			case 0xE3:		/* set flag 0 ON */
				opcode_E3();
				break;

			case 0xE4:		/* set flag 0 OFF */
				opcode_E4();
				break;

			case 0xE5:		/* die */
				opcode_E5();
				count_light = 1;
				break;

			case 0xE6:		/* move item B to room A */
				opcode_E6(code_chunk[index + 1], code_chunk[index + 2]);
				break;

			case 0xE7:		/* quit */
				opcode_E7();
				count_light = 1;
				break;

			case 0xE8:		/* print score */
				opcode_E8();
				break;

			case 0xE9:		/* list contents of inventory */
				opcode_E9();
				count_light = 1;
				break;

			case 0xEA:		/* refill */
				opcode_EA();
				break;

			case 0xEB:		/* save */
				opcode_EB();
				count_light = 1;
				break;

			case 0xEC:		/* swap items 1 and 2 around */
				opcode_EC(code_chunk[index + 1], code_chunk[index + 2]);
				break;

			case 0xED:		/* move an item to the inventory */
				opcode_ED(code_chunk[index + 1]);
				break;

			case 0xEE:		/* make item1 same room as item2 */
				opcode_EE(code_chunk[index + 1], code_chunk[index + 2]);
				break;

			case 0xEF:		/* nop */
				opcode_EF();
				break;

			case 0xF0:		/* look at room */
				opcode_F0();
				break;

			case 0xF2:		/* add 1 to timer */
				opcode_F6(1);
				break;

			case 0xF3:		/* sub 1 from timer */
				opcode_F7(1);
				break;

			case 0xF4:		/* print current timer */
				opcode_F4();
				break;

			case 0xF5:		/* set timer */
				opcode_F5(code_chunk[index + 1]);
				break;

			case 0xF6:		/*  add to timer */
				opcode_F6(code_chunk[index + 1]);
				break;

			case 0xF7:		/* sub from timer */
				opcode_F7(code_chunk[index + 1]);
				break;

			case 0xF8:		/* select room counter */
				opcode_F8();
				break;

			case 0xF9:		/* swap room counter */
				opcode_F9(code_chunk[index+1]);
				break;

			case 0xFA:		/* swap timer */
				opcode_FA(code_chunk[index+1]);
				break;

			case 0xFB:		/* print noun */
				opcode_FB();
				break;

			case 0xFC:		/* print nount + newline */
				opcode_FC();
				break;

			case 0xFD:		/* print newline */
				opcode_FD();
				break;

			case 0xFE:		/* delay */
				opcode_FE();
				break;

			case 0xFF:		/* end of code block. */
				rc = 0;
				run_code=1;
				try_index = 0;	/* drop out of all try blocks! */
				break;

			default:
				if(code_chunk[index] <= 0xB6)
				{
					print_the_string(code_chunk[index]);
					gprintf(" ");
				}
				else
				{
					run_code=1;

					gprintf("~ERR!: %04i) %02X %02X %02X~", index, code_chunk[0 + index], code_chunk[1 + index], code_chunk[2 + index]);
					glk_exit();
				}
				break;
		}

		if(game_flag == 1)
			return 0;


		/* we are on the 0xFF opcode, or have fallen through */
		if(run_code == 1 && try_index > 0)
		{
			if(code_chunk[index] == 0xFF)
			{
				run_code = 1;
			}
			else
			{
				/* not matched at end of TRY block */
				/* or AT end of try block */
				index = try[try_index-1];
				try_index -= 1;
				try[try_index] = 0;
				run_code = 0;
			}
		}
		else
		{
			/* continue */
			if(code_chunk[index] >= 0xB7)
				index += 1 + actions[(code_chunk[0 + index])- 0xB7].count;
			else
				index += 1;
		}
	}

	return rc;
}

/*
	ITEM is in inventory
*/
int opcode_B7(int item)
{
	uint8_t *p;

	p = game + item + fix_address(fix_word(dh.p_obj_table));

	if(*p == ROOM_INVENTORY)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	ITEM is in room
*/
int opcode_B8(int item)
{
	uint8_t *p;

	p = game + item + fix_address(fix_word(dh.p_obj_table));
	if(*p == gm->locn)
		return X_TRUE;
	else
		return X_FALSE;
}


/*
	Is ITEM available (in inventory or in room)
*/
int opcode_B9(int item)
{
	if(opcode_B8(item) == X_TRUE || opcode_B7(item) == X_TRUE)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	ITEM is in NOT room
*/
int opcode_BA(int item)
{
	uint8_t *p;

	p = game + item + fix_address(fix_word(dh.p_obj_table));
	if(*p != gm->locn)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	ITEM is NOT in inventory
*/
int opcode_BB(int item)
{
	uint8_t *p;

	p = game + item + fix_address(fix_word(dh.p_obj_table));

	if(*p != ROOM_INVENTORY)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Is ITEM NOT available (in inventory or in room)
*/
int opcode_BC(int item)
{
	if(opcode_BA(item) == X_TRUE && opcode_BB(item) == X_TRUE)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Object exists
*/
int opcode_BD(int item)
{
	uint8_t *p;

	p = game + item + fix_address(fix_word(dh.p_obj_table));

	/* item is not in room 0 or in inventory */
	if(*p != 0)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Object does NOT exists
*/
int opcode_BE(int item)
{
	uint8_t *p;

	p = game + item + fix_address(fix_word(dh.p_obj_table));

	if(*p==0)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Is player in room?
*/
int opcode_BF(int room)
{
	if(gm->locn == room)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Is player is NOT in room?
*/
int opcode_C0(int room)
{
	if(gm->locn != room)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Check if flag N is SET, return TRUE..
*/
int opcode_C1(int flagnum)
{
	assert(flagnum < 32);

	if(get_bit(gm->bitflags, flagnum)!=0)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Check if flag N is NOT SET, return TRUE..
*/
int opcode_C2(int flagnum)
{
	assert(flagnum < 32);

	if(get_bit(gm->bitflags, flagnum)==0)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Something
*/
int opcode_C3(void)
{
	if(count_inventory() > 0)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Nothing
*/
int opcode_C4(void)
{
	if(count_inventory() == 0)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Less than equal to
*/
int opcode_C5(int num)
{
	if(gm->cur_timer <= num)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Greater than
*/
int opcode_C6(int num)
{
	if(gm->cur_timer > num)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	equal
*/
int opcode_C7(int num)
{
	if(gm->cur_timer == num)
		return X_TRUE;
	else
		return X_FALSE;
}


/*
	not Moved?
*/
int opcode_C8(int item)
{
	uint8_t *p, *q;

	p = game + item + fix_address(fix_word(dh.p_obj_table));
	q = game + item + fix_address(fix_word(dh.p_orig_items));

	if(*p == *q)
		return X_TRUE;
	else
		return X_FALSE;
}

/*
	Moved?
*/
int opcode_C9(int item)
{
	uint8_t *p, *q;

	p = game + item + fix_address(fix_word(dh.p_obj_table));
	q = game + item + fix_address(fix_word(dh.p_orig_items));

	if(*p != *q)
		return X_TRUE;
	else
		return X_FALSE;
}


/*
	clear screen
*/
void opcode_D4(void)
{
	/* do nothing... */
}

/*
	Turn automatic inventory on
*/
void opcode_D6(void)
{
	gm->inv_flag = 0;
}

/*
	Turn automatic inventory off
*/
void opcode_D7(void)
{
	gm->inv_flag = 1;
}

/*
	TODO: ingore
*/
void opcode_D8(void)
{
}

/*
	TODO : success
*/
void opcode_D9(void)
{
}


/*
	Pickup item.
*/
int opcode_DB(int item)
{
	/*
		1 = need room in inventory
		2 = item in room
	*/

	if(count_inventory()<dh.max_items_carried)
	{
		opcode_E6(ROOM_INVENTORY, item);

		/* cause a room-look to happen on object change. */
		move_room = 1;

		return X_TRUE;
	}
	else
	{
		gprintf(messages[msg_too_much]);

		return X_FALSE;
	}
}

/*
	Drop item
*/
void opcode_DC(int item)
{
	uint8_t *p;

	p = game + item + fix_address(fix_word(dh.p_obj_table));

	*p=gm->locn;

	/* cause a room-look to happen on object change. */
	move_room = 1;
}

/*
	Move player to room
*/
void opcode_DD(int room)
{
	gm->locn = room;
	move_room = 1;
}


/*
	Set darkness on
*/
void opcode_DF(void)
{
	opcode_E1(FLAG_DARKFLAG);

	/* cause a room-look to happen on object change. */
	move_room = 1;
}

/*
	Set darkness off
*/
void opcode_E0(void)
{
	opcode_E2(FLAG_DARKFLAG);
	/* cause a room-look to happen on object change. */
	move_room = 1;
}

/*
	Set flag state.
*/
void opcode_E1(int flagnum)
{
	set_bit(gm->bitflags, flagnum, 1);
}

/*
	Clear flag
*/
void opcode_E2(int flagnum)
{
	set_bit(gm->bitflags, flagnum, 0);
}

/*
	Turn flag ON
*/
void opcode_E3(void)
{
	opcode_E1(0);
}

/*
	Turn flag OFF
*/
void opcode_E4(void)
{
	opcode_E2(0);
}

/*
	Die
*/
void opcode_E5(void)
{
	/* print I'm DEAD! */

	gprintf(messages[msg_dead]);
	gprintf(messages[msg_game_over]);

	/* remove dark bit... */
	opcode_E0();

	/* does gamestate need to be reset? */
	/* NO! game does not seem to reset itself from limbo.... */

	/* move to red room? */
	opcode_DD(dh.red_room);
}

/*
	Move item B to room A
*/
void opcode_E6(int room, int item)
{
	uint8_t *ptr;

	ptr = game + item + fix_address(fix_word(dh.p_obj_table));

	/* cause a room-look to happen on object change. */
	if(*ptr == gm->locn || *ptr == ROOM_INVENTORY || room == gm->locn)
		move_room = 1;

	*ptr = room;
}

/*
	Quit
*/
void opcode_E7(void)
{
	game_flag = 1;
}


/*
	print score
*/
void opcode_E8(void)
{
	uint16_t	x;
	int score;
	uint8_t	*p, *desc, *k;
	int num_treasures;

	p = game + fix_address(fix_word(dh.p_obj_table));
	desc = game + fix_address(fix_word(dh.p_obj_descr));

	score = 0;

	for(x=0; x <= dh.num_objects; x++, p++)
	{
		if(*p==dh.treasure_locn)
		{
			k=game+fix_address(get_word(desc+(x*2)));
			if(*(k+1)=='*')
				score += 1;
		}
	}

	/* SGEO : 20061206 : fix no treasures bug */
	num_treasures = dh.num_treasures;
	if(num_treasures == 0)
		num_treasures = 1;

	gprintf("~I have stored %i treasures. On a Scale of 0 to 100, that rates %i.~", score, (score*100)/num_treasures);

	/* SGEO : 20061206 */
	/* only mark win situation if we have more than zero treasures when typing 'score' */
	if(score==dh.num_treasures && dh.num_treasures > 0)
	{
		gprintf("~Well done.~This Adventure is over.");
		game_flag=1;
	}
}

/*
	List contents of inventory.
*/
void opcode_E9(void)
{
	int flag, tmp;
	uint8_t *p;

	p=game+fix_address(fix_word(dh.p_obj_table));

	gprintf(messages[msg_carry]);
	for(tmp=0, flag=0; tmp<=dh.num_objects; tmp++, p++)
	{
		if(*p==ROOM_INVENTORY)
		{
			if(flag!=0)
				gprintf(", ");
			flag++;
			print_table_string(dh.p_obj_descr, tmp);
		}
	}
	if(flag==0)
		str_print(messages[msg_nothing]);

	gprintf(".~");
}

/*
	Refill light source
*/
void opcode_EA(void)
{
	gm->light = dh.light_turns;

	if(opcode_B9(ITEM_LIGHTSOURCE)==X_TRUE)
	{
		move_room = 1;
	}

	/* set darkness off */
	opcode_E0();
}

/*
	Save
*/
void opcode_EB(void)
{
	save_game();
}

/*
	Swap items 1 and 2
*/
void opcode_EC(int item1, int item2)
{
	uint8_t *p1, *p2;
	uint8_t tmpitem;

	p1 = game + item1 + fix_address(fix_word(dh.p_obj_table));
	p2 = game + item2 + fix_address(fix_word(dh.p_obj_table));

	/* cause a room-look to happen on object change. */
	if(*p1 == gm->locn || *p2 == gm->locn || *p1==ROOM_INVENTORY || *p2==ROOM_INVENTORY)
		move_room = 1;

	tmpitem = *p1;
	*p1 = *p2;
	*p2 = tmpitem;
}


/*
	Steal an item. move an item into inventory.
*/
void opcode_ED(int item)
{
	opcode_E6(ROOM_INVENTORY, item);

	/* cause a room-look to happen on object change. */
	if(gm->inv_flag == 0)
		move_room = 1;
}

/*
	Same
*/
void opcode_EE(int item1, int item2)
{
	uint8_t *p1, *p2;

	p1 = game + item1 + fix_address(fix_word(dh.p_obj_table));
	p2 = game + item2 + fix_address(fix_word(dh.p_obj_table));

	/* cause a room-look to happen on object change. */
	if(*p1 == gm->locn || *p2 == gm->locn || *p1 == ROOM_INVENTORY || *p2 == ROOM_INVENTORY)
		move_room = 1;

	*p1 = *p2;
}

/*
	NOP
*/
void opcode_EF(void)
{
}


/*
	Look at room
*/
void opcode_F0(void)
{
	glk_set_window(roomwin);
	glk_window_clear(roomwin);

	if( is_really_dark() == X_FALSE)
	{
		inroom_flag = 1;
		print_room_descr(gm->locn);
		inroom_flag = 0;

		print_room_exits(gm->locn);
		print_room_items(gm->locn);

		if(gm->inv_flag==0)
		{
			gprintf("~");
			opcode_E9();
		}
		else
			gprintf("~");
	}
	else
	{
		str_print(messages[msg_too_dark]);
	}

	glk_set_window(mainwin);
}

/*
	Print timer
*/
void opcode_F4(void)
{
	/* TI99/4A driver outputs spaces around the number... */
	/* gprintf(" "); */
	num_print(gm->cur_timer);
	gprintf(" ");
}


/*
	Set timer value...
*/
void opcode_F5(int value)
{
	gm->cur_timer = value;

	/* users verb num.. has no affect on the game... */
	/* s,g,e,o,r,g,e */

	switch(value)
	{
		case 's':
			if(verb_num == 0) verb_num++; else verb_num = 0;
			break;
		case 'g':
			if(verb_num == 1) verb_num++;
			else { if(verb_num == 5) verb_num++; else verb_num = 0; }
			break;
		case 'e':
			if(verb_num == 2)
			{
				verb_num++;
			}
			else
			{
				if(verb_num == 6)
				{
					gm->sgeo_game_flag = 1;
					gm->cur_timer = 0;
					verb_num = 0;
				}
				else
					verb_num = 0;
			}
			break;
		case 'o':
			if(verb_num == 3) verb_num++; else verb_num = 0;
			break;
		case 'r':
			if(verb_num == 4) verb_num++; else verb_num = 0;
			break;
	}
}

/*
	Add to timer
*/
void opcode_F6(int value)
{
	gm->cur_timer += value;
}

/*
	sub from timer
*/
void opcode_F7(int value)
{
	gm->cur_timer -= value;

	if(gm->cur_timer < -1)
		gm->cur_timer = -1;
}

/*
	store room in swap
*/
void opcode_F8(void)
{
	opcode_F9(0);
	/* gm->room_rv[0] = gm->locn; */
}

/*
	Swap
*/
void opcode_F9(int room)
{
	int temp;

	assert(room < MAX_RRV);

	temp = gm->locn;
	gm->locn = gm->room_rv[room];
	gm->room_rv[room] = temp;

	move_room = 1;
}


/*
	Swap Timer register
*/
void opcode_FA(int timer)
{
	int temp;

	assert(timer < MAX_TIMERS);

	temp = gm->timers[timer];
	gm->timers[timer] = gm->cur_timer;
	gm->cur_timer = temp;
}

/*
	Print noun
*/
void opcode_FB(void)
{
	gprintf(in_noun);
}

/*
	Print noun and newline
*/
void opcode_FC(void)
{
	opcode_FB();
	opcode_FD();
}

/*
	print newline
*/
void opcode_FD(void)
{
	gprintf("~");
}

/*
	delay
*/
void opcode_FE(void)
{
	time_t t;

	if(scripting==0)
	{
		t=time(NULL);
		while(time(NULL)-t<2);
	}
}



int go_in_direction(void)
{
	uint8_t *p;

	if(is_really_dark() == X_TRUE)
		gprintf(messages[msg_dangerous_dark]);

	p=(uint8_t*)game + fix_address(fix_word(dh.p_room_exit));

	p += (gm->locn * 6) + (noun_num - 1);

	if(*p != 0)
	{
		gprintf(messages[msg_ok]);

		/* move to room */
		opcode_DD(*p);

		return X_TRUE;
	}
	else
	{
		if(is_really_dark() == X_TRUE)
		{
			gprintf(messages[msg_fell_broke]);
			game_flag=1;
		}
		else
			gprintf(messages[msg_cant_go]);

		return X_FALSE;
	}
}

/* parses verb noun actions */
int run_explicit(void)
{
	uint8_t *p;
	uint16_t addy;
	int flag;
	int rc;

	if(verb_num != BAD_WORD)
	{
		if(noun_num == BAD_WORD)
		{
			if(in_noun[0] == 0)
				gprintf("%s", messages[msg_huh]);
			else
				gprintf("I don't know what a %s is.~", in_noun);
			return 0;
		}

		/* GO in a direction! */
		if(gm->sgeo_game_flag == 0 && verb_num==1 && (noun_num >= 1 && noun_num <= 6))
		{
			go_in_direction();
		}
		else
		{
			rc = 0;

			/* continue_code: */
			/* run code.... */
			p = game + fix_address(fix_word(dh.p_explicit));
			addy = get_word(p + ( (verb_num ) *2) );

			flag = RC_NULL;
			if(addy != 0)
			{
				p=game+fix_address(addy);

				/* process all code blocks for this verb
				   until we come to the end of code blocks
				   or until we successfully end a block.
				*/

				flag = RC_NULL;
				rc = 0;
				while(flag == RC_NULL)
				{
					/* we match CLIMB NOUN or CLIMB ANY */
					if(p[0] == noun_num || p[0] == AUTO_VERB)
					{
						/* we have verb/noun match. run code! */
						rc = run_code_chunk(p+2);

						if(rc == 0)		/* success */
						{
							flag = RC_OK;
							/* gprintf("~%s~", messages[msg_ok]); */
							return 0;
							break;
						}
						else			/* failure */
						{
							if(p[1] == 0)
								flag = RC_RAN_ALL_BLOCKS_FAILED;
							else
								p += 1 + p[1];
						}
					}
					else
					{
						if(p[1] == 0)
							flag = RC_RAN_ALL_BLOCKS;
						else
							p += 1 + p[1];
					}
				}

				/* if its an SGEO game, run code to do directions before we auto handle it. */
				if(gm->sgeo_game_flag == 1 && (verb_num==GO_VERB && (noun_num >= 1 && noun_num <= 6)))
				{
					go_in_direction();
					/* dont do anything below here. mark flag as 2 */
					flag = RC_RAN_ALL_BLOCKS;
					return 0;
				}
			}

			if(flag == RC_OK && (verb_num == GET_VERB || verb_num == DROP_VERB))
			{
				if(item_num!=BAD_WORD)
					gprintf(messages[msg_cant_do_yet]);
				else
					gprintf(messages[msg_no_understand]);

				return 0;
			}

			/* do generic get/drop */
			if((flag == RC_NULL || flag == RC_RAN_ALL_BLOCKS) && (verb_num == GET_VERB || verb_num == DROP_VERB))
			{
				int iflag = 0;

				switch(verb_num)
				{
					case GET_VERB:
						/* item exists  */
						while(iflag == 0)
						{
							if(opcode_B8(item_num) == X_TRUE)
							{
								if(opcode_DB(item_num) == X_TRUE)
								{
									iflag = 1;
									gprintf(messages[msg_ok]);
								}
								else
								{
									/* get failed.... */
									iflag = 1;
								}
							}
							else
							{
								/* look for another item of same name... */
								if(item_num != BAD_WORD)
								{
									item_num = noun_item(noun_num, item_num+1);
								}

								if(item_num == BAD_WORD)
								{
									gprintf(messages[msg_beyond_power]);
									iflag = 1;
								}
							}
						}
						break;

					case DROP_VERB:
						while(iflag == 0)
						{
							p=game + item_num + fix_address(fix_word(dh.p_obj_table));

							if(item_num != BAD_WORD && *p==ROOM_INVENTORY)
							{
								opcode_DC(item_num);
								gprintf(messages[msg_ok]);
								iflag = 1;
							}
							else
							{
								if(item_num != BAD_WORD)
								{
									item_num = noun_item(noun_num, item_num+1);
								}
								else
								{
									if(item_num!=BAD_WORD)
										gprintf(messages[msg_beyond_power]);

									if(item_num==BAD_WORD)
										gprintf(messages[msg_huh]);

									iflag = 1;
								}
							}
						}
						break;
				}
			}
			else
			{
				if(item_num!=BAD_WORD)
					gprintf(messages[msg_cant_do_yet]);
				else
					gprintf(messages[msg_no_understand]);

				return 0;
			}
		}
	}
	else
	{
		gprintf("I don't know how to \"%s\" something.~", in_verb);
	}

	return 0;
}
