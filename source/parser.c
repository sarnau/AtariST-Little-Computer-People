/*
 * parser.c -- the NLP command parser (bitmask bag-of-words matcher).
 * addr: chk_encm(), cmd_upp(), chk_vwd(), lcp_upp()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "parser.h"
#include "random.h"
#include "vocab.h"

/* lcp_upp -> parts/lcp_upp.c (STX: 0x172e8, the last function of the sprite object). */

/* addr: cmd_upp() */
/* cmd_upp -> parts/cmd_upp.c (STX: 0x1711c, after chk_encm). */

/* chk_vwd -> parts/chk_vwd.c (STX: 0x171ae, after cmd_upp). */

/* chk_encm -> parts/chk_encm.c (STX: 0x16f9a, in the 0x148fe object between prCh and prsCmd). */
