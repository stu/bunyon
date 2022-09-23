#ifndef DEFINE_H
#define DEFINE_H
#ifdef __cplusplus
extern "C"{
#endif

/* verbs from 1 */
#define AUTO_VERB		0
#define GO_VERB			1
#define GET_VERB		10
#define DROP_VERB		18
#define BAD_WORD		255

/* flags from 0 */
#define FLAG_DARKFLAG		15
#define FLAG_RUNOUT			16

/* items from 0 */
#define ITEM_LIGHTSOURCE	9

/* rooms from 0 */
#define ROOM_LIMBO			0
#define ROOM_INVENTORY		255

#define VERSION		"0.5"


#ifdef __cplusplus
};
#endif
#endif        //  #ifndef DEFINE_H
