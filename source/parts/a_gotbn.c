/*
 * Included by stx_u2.c; never compiled on its own.
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
