/*
 * hideMouse and showMouse sit together, in this order, in the
 * original.
 */

/* Idempotent AES mouse hide: mouseHidden guards against a repeated M_OFF. */


void
hideMouse()
{
        if (mouseHidden == NO) {
                graf_mouse(M_OFF, (void *) 0);
                mouseHidden = YES;
        }
}

/* The matching show: only undoes a hide hideMouse actually did. */

void
showMouse()
{
        if (mouseHidden != NO) {
                graf_mouse(M_ON, (void *) 0);
                mouseHidden = NO;
        }
}
