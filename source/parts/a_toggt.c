/*
 * parts/a_toggt.c -- switch the TV on or off.
 * Included by stx_u2.c; never compiled on its own.
 */

/* ACTION_TOGGLE_TV: switch the TV off if lcp_tv says it is on,
   otherwise on.  tt_off/tt_on do the walking and the animation. */
void
a_toggt()
{
        if (lcp_tv != NO)
                tt_off();
        else
                tt_on();
}
