/*
 * parts/mq_rmev.c -- shared body; LCP_ORG links it in midi_seq.c,
 * LCP_STX in the 0xdece object (0xe64, right after mq_expN).  Files under parts/
 * are never compiled standalone.
 */
/* mq_rmev: remove 3-word entry at mi_evq[val]; shift later down.
   Returns 1 if more remain, 0 if empty.
   addr: midi_seq_remove_event() */

short
mq_rmev(val)
short   val;
{
#ifdef FAITHFUL
        short   res;
        short   i;

        if ((short)(val + 3) == mi_evi)
                res = 0;
        else {
                for (i = val; i < (short)(mi_evi - 3); i = i + 1)
                        mi_evq[i] = mi_evq[i + 3];
                res = 1;
        }
        mi_evi = mi_evi - 3;
        return res;
#else
        /* STX: one local; each arm shrinks the queue and returns. */
        short   i;

        if (val + 3 == mi_evi) {
                mi_evi -= 3;
                return 0;
        }
        for (i = val; i < mi_evi - 3; i++)
                mi_evq[i] = mi_evq[i + 3];
        mi_evi -= 3;
        return 1;
#endif
}
