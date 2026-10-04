/*
 * Included by stx_u2.c; never compiled on its own.
 */

void
a_getd()
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
        lcp_hwt();

        for (h = 0; h < 4; h++) {
                g_hatas = g_hacur & 7;
                lcp_hwt();
                g_hatas = g_hacur | 0x10;
                lcp_hwt();
        }

        g_hatas = entry_current;
        lcp_hwt();
}
