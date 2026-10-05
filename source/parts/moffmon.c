/*
 * parts/moffmon.c -- included by stx_u2.c; never compiled on its own.
 * hideMouse and showMouse sit together, in this order, in the original.
 */

/* Idempotent AES mouse hide: moff_f guards against a repeated M_OFF. */


void
hideMouse()
{
        if (moff_f == NO) {
                graf_mouse(M_OFF, (void *) 0);
                moff_f = YES;
        }
}

/* The matching show: only undoes a hide hideMouse actually did. */

void
showMouse()
{
        if (moff_f != NO) {
                graf_mouse(M_ON, (void *) 0);
                moff_f = NO;
        }
}
