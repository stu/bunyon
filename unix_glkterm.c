
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>

#include <garglk/glk.h>

#include "config.h"

#include "define.h"
#include "game.h"
#include "interp.h"
#include "rnd.h"
#include "msg.h"
#include "main.h"

#include "glkstart.h"
#include "unix_glkterm.h"

void print_syntax(void);
void do_switches(int argc, char *argv[]);
void load_game(void);

glkunix_argumentlist_t 	glkunix_arguments[] =
{
	{ "",   glkunix_arg_ValueFollows,  "filename: The game file to load."},
	{ "-s", glkunix_arg_ValueFollows,  "-s {file} : Input script File"},
	{ "-t", glkunix_arg_ValueFollows,  "-t {file} : Output transcript file"},
	{ "-v", glkunix_arg_NoValue,       "-v Version" },
	{ "-d",	glkunix_arg_NoValue,       "-d Debug codes" },
	{ "-x",	glkunix_arg_ValueFollows,  "-x {file} : Decompile game" },
	{ "-r", glkunix_arg_ValueFollows,  "-r NUM : Set random seed (only works with input script.)" },
	{ NULL, glkunix_arg_End, NULL }
};


void do_startup(void)
{
	/* nothing to do here... */
	action = 0;
}

int glkunix_startup_code(glkunix_startup_t *data)
{
	int i;

	do_startup();

	action = 1;

	if(data->argc < 2)
	{
		action = 1;
	}
	else
	for(i=1; i<data->argc; i++)
	{
		if(strcmp(data->argv[i], "-?")==0)
		{
			action = 1;
			/* return TRUE; */
			break;
		}
		else if(strcmp(data->argv[i], "-v")==0)
		{
			action = 1;
			/* return TRUE; */
			break;
		}
		else if(strcmp(data->argv[i], "-d")==0)
		{
			dump_codes = 1;
		}
		else if(strcmp(data->argv[i], "-x")==0)
		{
			i += 1;
			if(source_file != NULL)
			{
				action = 1;
				/* return TRUE; */
				break;
			}
			source_file = strdup(data->argv[i]);
			action = 3;
		}
		else if(strcmp(data->argv[i], "-t")==0)
		{
			i += 1;
			if(source_file != NULL)
			{
				action = 1;
				/* return TRUE; */
				break;
			}
			source_file = strdup(data->argv[i]);
		}
		else if(strcmp(data->argv[i], "-r")==0)
		{
			i+= 1;
			saved_seed = atol(data->argv[i]);
		}
		else if(strcmp(data->argv[i], "-s")==0)
		{
			i += 1;
			if(script_file != NULL)
			{
				action = 1;
				/* return TRUE; */
				break;
			}
			script_file = strdup(data->argv[i]);
		}
		else
		{
			if(game_file != NULL)
			{
				action = 1;
				/* return TRUE; */
				break;
			}
			else
			{
				action = 2;
				game_file = strdup(data->argv[i]);
			}
		}
	}

	if(game_file == NULL)
	{
		action = 1;
	}

	if(action == 1)
	{
		mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
		assert(mainwin != NULL);

		glk_set_window(mainwin);
		glk_window_clear(mainwin);

		print_syntax();
		return FALSE;
	}

	return TRUE;
}


void print_syntax(void)
{
	glui32 res;

	glk_set_window(mainwin);
	glk_window_clear(mainwin);

	gprintf("~~Bunyon v" VERSION "; An interpreter for TI99/4A Scott Adams Games~"
			"Written by Stuart George - contact via http://public.xdi.org/=stu~");

	res = glk_gestalt(gestalt_Version, 0);
	gprintf("Running on glk v%i.%i.%i~", (res>>16), (res>>8)&0xFF, res&0xFF);

	gprintf("~Syntax; bunyon {-switch -switch ... -switch} gamefile~"
			"-s {file} : Input script file~"
			"-t {file} : Output script file~"
			"-x {file} : Decompile game to file~"
			"-d : Enable debug codes~"
			"-r {num} : Set random seed (only works with input script.)~"
			"-v : Version~"
			"-? : Help~");
}

