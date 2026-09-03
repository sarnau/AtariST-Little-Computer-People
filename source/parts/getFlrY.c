/*
 * parts/getFlrY.c -- shared body; LCP_STX puts it at 0x5224 in the
 * 0x400c object, right after lcp_flwp and before ldObj.
 * Files under parts/ are never compiled standalone.
 */
/* addr: getFlrY() */
short
getFlrY(y)
short   y;
{
        /* STX: one if/else-if/else chain -- each arm's return is
           followed by the else-skip branch. */
        if (y > 140)
                return 1;
        else if (y > 77)
                return 2;
        else
                return 3;
}
