/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* nodOk: nod in agreement.  Picks a head target from the direction the
   head currently faces (low three bits of g_hacur) and waits for it,
   then four times alternates between that direction level and tilted
   (bit 0x10) -- a nod -- before returning the head to where it started.
   chooseAction plays it when the resident accepts a request to play a game
   or the piano, and the move-in cutscene uses it.  It is also the body
   of action 35 (ACTION_NOD_OK), but no command, event or AI table
   ever queues that action. */
void
nodOk()
{
        short   entry_current;
        short   h;
        /* Only two locals: h doubles as the loop counter below.
           Do not add a separate one. */

        entry_current = g_hacur;
        h = g_hacur & 7;

        if (h == 0 || h == 1 || h == 7)
                g_hatas = 8;
        else if (h == 2)                        /* HEAD_ANIM_SHOWER value */
                g_hatas = 9;
        else if (h == 6)
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE |
                                         7 /* HEAD_MODE_H_AMPLITUDE mask */;
        else if (h == 3 || h == 4)
                g_hatas = 10;
        else if (h == 5)
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE |
                                         HEAD_ANIM_SHOWER;

        g_hamod = HEAD_ANIM_DISABLED;
        waitHeadTurn();

        for (h = 0; h < 4; h++) {
                g_hatas = g_hacur & 7;
                waitHeadTurn();
                g_hatas = g_hacur | 0x10;
                waitHeadTurn();
        }

        g_hatas = entry_current;
        waitHeadTurn();
}
