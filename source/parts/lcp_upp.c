/*
 * parts/lcp_upp.c -- shared body; LCP_ORG links it in parser.c,
 * LCP_STX in the 0xdece object (0x172e8, the last function of the sprite object).  Files under parts/
 * are never compiled standalone.
 */
/* addr: lcp_upp() */
short
lcp_upp(ch)
short   ch;
{
        /* STX returns the converted value directly rather than
           writing it back to the parameter, and spells the range with
           inclusive bounds -- the trailing else-skip branch after the
           returning then-arm is Alcyon's, not a second statement. */
        if (ch >= 'a' && ch <= 'z')
                return ch - 0x20;
        else
                return ch;
}
