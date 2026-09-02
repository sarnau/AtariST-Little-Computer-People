/*
 * stx_u3.c -- STX unity unit for the 0x14824-0x172e8 sprite object.
 *
 * Evidence: er_write 0x14824, sp_spud 0x148fe, sp_flih 0x14a76 and
 * sp_updb 0x16244 all fall in this range, and er_write's cluster
 * ends 24 bytes before sp_spud's begins -- one object.  See
 * stx_u1.c for the mechanism and CLAUDE.md for the cluster report.
 */

#ifndef FAITHFUL

#include "alerts.c"
#include "sprites.c"

#endif  /* !FAITHFUL */
