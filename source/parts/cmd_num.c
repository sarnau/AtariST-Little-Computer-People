/*
 * parts/cmd_num.c -- shared body; LCP_STX 0x17278, between prsCmd
 * and lcp_upp at the tail of the sprite object.  Files under parts/
 * are never compiled standalone.
 */

/* Decimal string -> number, with an optional leading '-'.  NOTHING in
   LCP_STX calls this (no jsr or bsr anywhere in the image targets
   0x17278); Alcyon emits a static even when nothing references it, so
   the 1985 parser source still carried the helper.  Name invented --
   the binary keeps no symbol for a static. */
static short
cmd_num(p)
char *  p;
{
        short   val;            /* -2  */
        short   sign;           /* -4  */
        short   c;              /* -6  */

        val = 0;
        if ((c = *p) == '-') {
                sign = -1;
                p++;
        } else
                sign = 1;
        while ((c = *p++) >= '0' && c <= '9')
                val = val * 10 + c - '0';
        return val * sign;
}
