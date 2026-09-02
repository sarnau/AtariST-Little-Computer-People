/*
 * parts/setHTgt.c -- shared body; LCP_ORG links it in walk.c,
 * LCP_STX in the 0xdece object (0x400c object, with lcp_path).  Files under parts/
 * are never compiled standalone.
 */
/* Set head_anim_target if not already `target`. */
static void
setHTgt(target)
short   target;
{
        if (g_hastl != target) {
                g_hatas = target;
                g_hastl   = target;
        }
}
