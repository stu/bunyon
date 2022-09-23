#ifndef MSG_H
#define MSG_H
#ifdef __cplusplus
extern "C"{
#endif

enum
{
	msg_i_am_in_a = 0,
	msg_visible_items,
	msg_nothing,
	msg_tell_me_what_to_do,
	msg_dangerous_dark,
	msg_ok,
	msg_fell_broke,
	msg_cant_go,
	msg_no_understand,
	msg_beyond_power,
	msg_huh,
	msg_too_dark,
	msg_obv_exits,
	msg_too_much,
	msg_dead,
	msg_carry,
	msg_cant_do_yet,
	msg_light_out,
	msg_dim_light,
	msg_game_over,

	MSG_MAX
};

extern char *fixwords[14];
extern char *fixwords2[4];
extern char *messages[MSG_MAX];

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef MSG_H
