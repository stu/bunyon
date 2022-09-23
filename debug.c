
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

char *debug_commands[]=
{
	"#HELP",
	"#CLS",
	"#LOOK",
	"#DUMP_ITEMS",
	"#DUMP_FLAGS",
	"#DUMP_TIMERS",
	/*"#GOTO",*/
	"#INV",

	NULL
};


int debug_command(void)
{
	int i;

	for(i=0; debug_commands[i] != NULL; i+=1)
	{
		if(strcmp(in_verb, debug_commands[i])==0)
			return DEBUG_HELP + i;
	}

	return 0;
}

void debug_help(void)
{
	gprintf("~"
			"#help - This help file.~"
			"#look - Does a room look. (not this bypasses any inbuilt logic.~"
			"#dump_items - Lists items and locations.~"
			"#dump_flags - Lists flag values.~"
			"#dump_timers - Lists timer values.~"
			"#inv - toggle show/hide inventory.~"
			/* "#goto {room} - Jump to room number.~~" */);
}

void debug_inv(void)
{
	if(gm->inv_flag == 0)
		gm->inv_flag = 1;
	else
		gm->inv_flag = 0;

	opcode_F0();
}

void debug_look(void)
{
	int flag;

	gprintf("## Room : %02i~", gm->locn);
	flag = get_bit(gm->bitflags, FLAG_DARKFLAG);
	set_bit(gm->bitflags, FLAG_DARKFLAG, 0);
	move_room = 1;
	opcode_F0();
	move_room = 0;
	set_bit(gm->bitflags, FLAG_DARKFLAG, flag);
}

void debug_flags(void)
{
	int i;

	for(i=0; i<MAX_FLAGS; i+=1)
	{
		gprintf("flag%02i=%c", i, get_bit(gm->bitflags, i) == 0 ? '0':'1');
		if(i%2==1)
			gprintf("~");
		else
			gprintf("    ");
	}
}

void debug_vars(void)
{
	int i;

	gprintf("current timer = %i~", gm->cur_timer);
	for(i=0; i<MAX_TIMERS; i+=1)
	{
		gprintf("timer %02i=%-5i", i, gm->timers[i]);
		if(i%2==1)
			gprintf("~");
		else
			gprintf("    ");
	}
}

void debug_items(void)
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
		gprintf("%03i", i);
		if(i==ITEM_LIGHTSOURCE)
			gprintf(" (light)");

		if(*noun != 0)
		{
			decomp_word(*noun, dh.p_noun_table, dh.num_nouns);
			gprintf(" (%s)", in_verb);
		}

		gprintf(" \"");
		if(i < max_item_descr)
			decomp_table_string(dh.p_obj_descr, i);
		gprintf("\"");

		if(*ptr == ROOM_LIMBO)
			gprintf(" in Limbo~");
		else if(*ptr == ROOM_INVENTORY)
			gprintf(" in Inventory~");
		else
		{
			/* gprintf(" in room %02i~", *ptr); */
			gprintf(" in room %03i \"", *ptr);
			decomp_table_string(dh.p_room_descr, *ptr);
			gprintf("\"~");
		}

		descr++;
		oitem++;
		ptr++;
		noun++;
	}
}

void do_debug_command(int cmd)
{
	switch(cmd)
	{
		case DEBUG_ITEM_LIST:
			debug_items();
			break;

		case DEBUG_FLAGS_DUMP:
			debug_flags();
			break;

		case DEBUG_VARS_DUMP:
			debug_vars();
			break;

		case DEBUG_HELP:
			debug_help();
			break;

		case DEBUG_LOOK:
			debug_look();
			break;

		case DEBUG_INV:
			debug_inv();
			break;

		default:
			gprintf("~Unknown/Unimplemented debug command %i.~", cmd);
			break;
	}
}
