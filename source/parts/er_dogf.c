/*
 * parts/er_dogf.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from delivery.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in delivery.c.
 * Files under parts/ are never compiled standalone.
 */

void
er_dogf()
{
        g_dvdog = YES;
        er_food();
        g_dvdog = NO;
}
