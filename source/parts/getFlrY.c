/*
 * Included by stx_u1.c; never compiled on its own.
 */
short
getFlrY(y)
short   y;
{
        /* One if/else-if/else chain on purpose: the original
           follows each arm's return with an else-skip branch. */
        if (y > 140)
                return FLOOR_BOTTOM;
        else if (y > 77)
                return FLOOR_MIDDLE;
        else
                return FLOOR_TOP;
}
