/* Remove 3-word entry at noteQueue[val]; shift later down.
   Returns 1 if more remain, 0 if empty. */

short
removeQueued(val)
short   val;
{
        /* Each arm shrinks the queue itself and returns. */
        short   i;

        if (val + 3 == queueLen) {
                queueLen -= 3;
                return 0;
        }
        for (i = val; i < queueLen - 3; i++)
                noteQueue[i] = noteQueue[i + 3];
        queueLen -= 3;
        return 1;
}
