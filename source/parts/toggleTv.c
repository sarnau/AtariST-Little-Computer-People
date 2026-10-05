/*
 * parts/toggleTv.c -- switch the TV on or off.
 * Included by stx_u2.c; never compiled on its own.
 */

/* ACTION_TOGGLE_TV: switch the TV off if tvRunning says it is on,
   otherwise on.  tvOff/tvOn do the walking and the animation. */
void
toggleTv()
{
        if (tvRunning != NO)
                tvOff();
        else
                tvOn();
}
