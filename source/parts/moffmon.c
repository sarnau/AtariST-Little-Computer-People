/*
 * parts/moffmon.c -- included by stx_u2.c; never compiled on its own.
 * moff and mon sit together, in this order, in the original.
 */

/* Idempotent AES mouse hide: moff_f guards against a repeated M_OFF. */


void
moff()
{
        if (moff_f == NO) {
                graf_mouse(M_OFF, (void *) 0);
                moff_f = YES;
        }
}

/* The matching show: only undoes a hide moff actually did. */

void
mon()
{
        if (moff_f != NO) {
                graf_mouse(M_ON, (void *) 0);
                moff_f = NO;
        }
}
