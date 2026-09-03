/*
 * parts/a_gotbn.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * ahouse functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_gotbn()
{
        g_actif = YES; a_takes();
        g_actif = YES; a_opcbc(1);
        g_actif = YES; a_kitcc();
        g_actif = YES; a_brust();
        g_actif = YES; a_gioob();
        g_actif = NO;
}
