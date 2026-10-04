/*
 * parts/a_toggt.c -- switch the TV on or off.
 * Included by stx_u2.c; never compiled on its own.
 */

void
a_toggt()
{
        if (lcp_tv != NO)
                tt_off();
        else
                tt_on();
}
