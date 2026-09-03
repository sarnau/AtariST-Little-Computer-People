/*
 * parts/exitVdi.c -- shared body; LCP_STX 0x76d0, right after
 * initVdi.  Files under parts/ are never compiled standalone.
 */
void
exitVdi()
{
        Setscreen(sv_lgb, (void *)-1L, -1);     /* rez as word */
}
