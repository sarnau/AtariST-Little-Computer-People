/*
 * parts/redrawHands.c -- included by stx_u2.c; never compiled on its own.
 * Redraws the clock hands when the minute changes: erase in white, draw
 * in grey.
 */
void
redrawHands()
{
        if (clockMinute == t_min)
                return;
        drawHands(clockMinute, clockHour, COLOR_white);
        clockMinute = t_min;
        clockHour   = t_hour;
        /* The cached copies, not t_min/t_hour: the original reads the
           two globals back for this call. */
        drawHands(clockMinute, clockHour, COLOR_grey);
}
