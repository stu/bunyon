#ifndef INTERP_H
#define INTERP_H
#ifdef __cplusplus
extern "C"{
#endif

extern void print_title_screen(void);
extern void run_implicit(void);
extern int run_explicit(void);
extern int run_code_chunk(uint8_t *code_chunk);
extern void decomp_code_chunk(uint8_t *code_chunk);

extern int opcode_B9(int item);		/* item is available */
extern int opcode_BC(int item);		/* item NOT available */
extern int opcode_BD(int item);		/* item is NOT available */
extern int opcode_C1(int flagnum);		/* is bit N set */
extern void opcode_E1(int flagnum);	/* set bit */
extern void opcode_E2(int flagnum);	/* clear bit */
extern void opcode_F0(void);			/* room look */

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef INTERP_H
