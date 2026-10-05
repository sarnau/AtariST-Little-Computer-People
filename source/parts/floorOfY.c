/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* Which floor a screen y belongs to: y above 140 (lower on the
   screen) is FLOOR_BOTTOM, above 77 FLOOR_MIDDLE, the rest FLOOR_TOP.
   The walking code uses it to decide when a target needs the stairs. */
short
floorOfY(y)
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
