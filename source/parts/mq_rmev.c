/*
 * parts/mq_rmev.c -- included by midi_seq.c; never compiled on its own.
 */
/* mq_rmev: remove 3-word entry at mi_evq[val]; shift later down.
   Returns 1 if more remain, 0 if empty. */

short
mq_rmev(val)
short   val;
{
        /* Each arm shrinks the queue itself and returns. */
        short   i;

        if (val + 3 == mi_evi) {
                mi_evi -= 3;
                return 0;
        }
        for (i = val; i < mi_evi - 3; i++)
                mi_evq[i] = mi_evq[i + 3];
        mi_evi -= 3;
        return 1;
}
