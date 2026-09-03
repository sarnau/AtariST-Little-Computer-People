/*
 * parts/moffmon.c -- shared body; LCP_STX puts moff (0xde36) and mon
 * (0xde5c) together at the head of the 0xdece object, right after
 * sf_so and immediately before lcp_lgt.  Files under parts/ are never
 * compiled standalone.
 */

#ifndef M_OFF
#define M_OFF           256
#endif

/* moff: idempotent AES mouse hide (moff_f guards repeat M_OFF).
   addr: mouse_off() */


void
moff()
{
        if (moff_f == NO) {
                graf_mouse(M_OFF, (void *) 0);
                moff_f = YES;
        }
}

/* mon (STX 0xde5c): the counterpart moff guards against, immediately
   after it in the same object.  LCP_ORG has no such function. */

#ifndef M_ON
#define M_ON            257
#endif

void
mon()
{
        if (moff_f != NO) {
                graf_mouse(M_ON, (void *) 0);
                moff_f = NO;
        }
}
