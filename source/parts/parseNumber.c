/*
 * Included by stx_u3.c; never compiled on its own.
 */

/* Decimal string -> number, with an optional leading '-'.  NOTHING
   calls this, but Alcyon emits a static even when nothing references
   it, so the 1985 parser source still carried the helper and it must
   stay.  Name invented -- the binary keeps no symbol for a static. */
static short
parseNumber(p)
char *  p;
{
        short   val;
        short   sign;
        short   c;

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
