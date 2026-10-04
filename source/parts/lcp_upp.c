/*
 * parts/lcp_upp.c -- included by stx_u3.c; never compiled on its own.
 */
short
lcp_upp(ch)
short   ch;
{
        /* Returns the converted value directly, and spells the range
           with inclusive bounds.  The `else` after a returning
           then-arm is the original's: it compiles to an extra branch,
           so it is kept on purpose. */
        if (ch >= 'a' && ch <= 'z')
                return ch - 0x20;
        else
                return ch;
}
