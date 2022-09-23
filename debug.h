#ifndef DEBUG_H
#define DEBUG_H
#ifdef __cplusplus
extern "C"{
#endif

enum DEBUG_WORDS
{
	DEBUG_HELP = 0x1000,
	DEBUG_CLS,
	DEBUG_LOOK,
	DEBUG_ITEM_LIST,
	DEBUG_FLAGS_DUMP,
	DEBUG_VARS_DUMP,
	/*DEBUG_GOTO,*/
	DEBUG_INV
};

extern int debug_command(void);
extern void do_debug_command(int cmd);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef DEBUG_H
