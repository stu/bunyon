#include <stdio.h>
#include "msg.h"

char *fixwords[]=
{
	(char *)"N", (char *)"NORTH",
	(char *)"S", (char *)"SOUTH",
	(char *)"E", (char *)"EAST",
	(char *)"W", (char *)"WEST",
	(char *)"U", (char *)"UP",
	(char *)"D", (char *)"DOWN",
	NULL, NULL
};

char *fixwords2[]=
{
	(char *)"I", (char *)"INVENTORY",
	NULL, NULL
};

char *messages[]=
{
	(char*)"I am in a ",						/* msg_i_am_in_a */
	(char*)"Visible items are : ",				/* msg_visible_items */
	(char*)"Nothing",							/* msg_nothing */
	(char*)"~What shall I do? ", 				/* msg_tell_me_what_to_do */
	(char*)"Dangerous to move in the dark!~", 	/* msg_dangerous_dark */
	(char*)"OK.",								/* msg_ok */
	(char*)"I fell down and broke my neck.",	/* msg_fell_broke */
	(char*)"I can't go in that direction.",		/* msg_cant_go */
	(char*)"I don't understand the command.",	/* msg_no_understand */
	(char*)"It's beyond my power to do that.",	/* msg_beyond_power */
	(char*)"Huh?",								/* msg_huh */
	(char*)"I cant see. It is too dark!~",		/* msg_too_dark */
	(char*)"~Obvious exits : ",					/* msg_obv_exits */
	(char*)"I am carrying too much.~",			/* msg_too_much */
	(char*)"~I'm dead...",						/* msg_dead */
	(char*)"I am carrying: ",					/* msg_carry */
	(char*)"I can't do that yet.",				/* msg_cant_do_yet */
	(char*)"Light went out!",					/* msg_light_out */
	(char*)"Light is going dim",				/* msg_dim_light */
	(char*)"~This adventure is over.",			/* msg_game_over */
};

