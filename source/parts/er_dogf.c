/*
 * parts/er_dogf.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * delivery functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
er_dogf()
{
        g_dvdog = YES;
        er_food();
        g_dvdog = NO;
}
