/*
 * parts/sp_lcha.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x16368, between sp_updb and sp_lchu). Files under parts/
 * are never compiled standalone.
 */

/* Head-animation mode bits (sprhead.c's own defines, repeated here so
   the unity unit can compile this body). */
#undef  HEAD_MODE_H_AMPLITUDE
#undef  HEAD_MODE_H_RANGE
#define HEAD_MODE_H_AMPLITUDE           0x03
#define HEAD_MODE_H_RANGE               0x0c
#ifndef HEAD_MODE_V_RANGE
#define HEAD_MODE_V_RANGE               0x60
#define HEAD_MODE_V_OVERRIDE            0x80
#endif

/* addr: sp_lcha() */
void
sp_lcha()
{
        /* Eleven shorts, one of them (-14) never touched. */
        short   anim_mode;
        short   curTilt;
        short   tgtTilt;
        short   current_pos;
        short   curDir;
        short   tgtDir;
        short   unused;
        short   target_frame;
        short   random_seed;
        short   movement_mask;
        short   faceDir;

        if (g_hacur != g_hatas ||
            g_hamod < 0)
                goto apply_current;

        g_hadec--;
        if (g_hadec > 0)
                goto apply_current;

        /* Pick a fresh target.  Coin-flip between a horizontal
           adjustment and a vertical one -- STX tests for the bit being
           CLEAR and puts the horizontal picker first. */
        g_hadec = rndRng(2, 9);

        if ((rnd() & 0x10) == 0) {
                /* Horizontal picker. */
                if ((g_hamod & HEAD_MODE_H_AMPLITUDE) == 0)
                        random_seed = ((rnd() & HEAD_MODE_H_AMPLITUDE) | 1) - 1;
                else
                        random_seed = (g_hamod & HEAD_MODE_H_AMPLITUDE) - 1;

                if (((g_hamod & HEAD_MODE_H_RANGE) == 0 &&
                     (rnd() & 8) != 0) ||
                    (g_hamod & HEAD_MODE_H_RANGE) >= 8)
                        random_seed = -random_seed;

                random_seed = (hd_dang[lcp_st] + random_seed) & 7;
                if (lcp_face == FACING_LEFT)
                        random_seed = (8 - random_seed) & 7;
                g_hatas = (g_hatas & 0x18) | random_seed;
        } else {
                /* Vertical picker. */
                /* Embedded: the store's own flags drive the test. */
                if ((movement_mask = g_hamod & HEAD_MODE_V_RANGE) == 0) {
                        movement_mask = rnd() & HEAD_MODE_V_RANGE;
                        if (movement_mask == 0)
                                movement_mask = 0x40;
                }
                movement_mask = (movement_mask >> 5) - 1;
                if ((g_hamod & (HEAD_MODE_V_OVERRIDE |
                                HEAD_MODE_V_RANGE)) <= 0x80)
                        movement_mask = ((rnd() & 4) >> 2) +
                                        (movement_mask & 1);
                else
                        movement_mask = 7 - (g_hamod >> 5);
                g_hatas = (g_hatas & 7) | (movement_mask << 3);
        }

apply_current:
        if (g_hatas >= 0) {
                curTilt = g_hacur & 0x18;
                tgtTilt = g_hatas & 0x18;
                if ((current_pos = tgtTilt - curTilt) > 0)
                        g_hacur += 8;
                else if (current_pos < 0)
                        g_hacur -= 8;

                curDir = g_hacur & 7;
                tgtDir = g_hatas & 7;
                target_frame = hd_mvd[(tgtDir - curDir) + 7];
                if (target_frame == 99) {
                        faceDir = (hd_dang[lcp_st] + (lcp_face << 2)) & 7;
                        target_frame = hd_mvd[(faceDir - curDir) + 7];
                }
                if (target_frame == 99)
                        target_frame = -1;

                g_hacur = (g_hacur & 0x18) +
                          ((g_hacur + target_frame) & 7);
        }

        if (g_hacur >= 0 && g_hacur < 0x80) {
                g_hsfra = hd_tilt[(g_hacur & 0x18) >> 3];
                if ((anim_mode = g_hacur & 7) <= 4) {
                        g_hsfra += anim_mode;
                        g_hsmif = NO;
                } else {
                        g_hsfra += 8 - anim_mode;
                        g_hsmif = YES;
                }
        }
}
