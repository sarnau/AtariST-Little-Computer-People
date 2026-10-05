/*
 * dat_house.c -- the initialized globals that belong to the stx_u2
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The position of
 * this file's #include and the order of the declarations below set
 * the data layout; do not reorder them.
 *
 * Not compiled standalone -- included by stx_u2.c.
 */

BOOL16  organPlaying          = NO;

/* Three-letter abbreviations, so the calendar and the letter date
   line read "Sep 4, 1985". */
char *  monthNames[12] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
};

BOOL16          mouseHidden = 1;   /* YES while the mouse is hidden; hideMouse/showMouse only call graf_mouse on a change.  Starts YES, so the first hideMouse is a no-op until showMouse has shown it */

/* Lit-stove flame frames: cookMeal draws one of the first three at
   random.  The last three are never read and are not stove frames (a
   closet-door frame and two fireplace frames) -- leftovers in the
   original's data, kept because the table's size sets the layout. */
short   stoveFrames[6]    = { OBJ_STOVE_ON_1, OBJ_STOVE_ON_2, OBJ_STOVE_ON_3,
                              OBJ_DOOR_CLOSET_OPEN_2, OBJ_FIRE_OFF, OBJ_FIRE_1 };

/* TV pattern animation.
   Four vertical scanlines drawn inside the TV screen -- each is a
   constant-X, descending-Y run of 8 points.  Colours picked from
   tvBarColor (10, 5, 7, 13 in the main palette). */
short   tvBar0X[8] = { 293, 293, 293, 293, 293, 293, 293, 293 };

short   tvBar0Y[8] = {   /* bar 0 Y per step (one row up per step) */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   tvBar1X[8] = {   /* bar 1 X per step */ 297, 297, 297, 297, 297, 297, 297, 297 };

short   tvBar1Y[8] = {   /* bar 1 Y per step */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   tvBar2X[8] = {   /* bar 2 X per step */ 301, 301, 301, 301, 301, 301, 301, 301 };

short   tvBar2Y[8] = {   /* bar 2 Y per step */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   tvBar3X[8] = {   /* bar 3 X per step */ 305, 305, 305, 305, 305, 305, 305, 305 };

short   tvBar3Y[8] = {   /* bar 3 Y per step */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   tvBarColor[4] = {   /* colour index per bar (through colorPens) */ 10, 5, 7, 13 };

/* Days per month, January first; daysInMonth replaces February's 28 by
   29 in leap years. */
short daysPerMonth[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* The "last-drawn" clock hand positions (minute 5, hour 6).  t_min/t_hour
   start at 0 (BSS), so the first redrawHands call sees a mismatch
   and paints the initial 0:00 hands over the pre-drawn 5:06 default. */
short   clockMinute                         = 5;

short   clockHour                         = 6;   /* hour the clock's hour hand was last drawn for */

/* Circle-position table for the minute hand.  Indexed by the current
   minute/5 mod 12 giving one of 12 positions on a small circle around
   the clock centre; three padding entries at the end. */
short   minuteHandXY[15] = {
         0,   2,   3,   3,   3,   2,   0,  -2,
        -3,  -3,  -3,  -2,   0,   2,   3
};

/* Same shape for the hour hand, smaller radius (2 vs 3 pixels). */
short   hourHandXY[15] = {
         0,   1,   2,   2,   2,   1,   0,  -1,
        -2,  -2,  -2,  -1,   0,   1,   2
};

/* Starts at -1: the first frame of animRecPlayer (record-player needle
   sweep) skips the draw when needlePos is < 0, then decrements to -3,
   then wraps to 13. */
short   needlePos                         = -1;

short   vuLeds          = 0;   /* record player VU LEDs currently lit, one bit per LED (vuLedMasks); animRecPlayer toggles them */

/* Bit-mask toggles for the VU-meter LEDs, high bit first. */
unsigned short  vuLedMasks[8] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};
