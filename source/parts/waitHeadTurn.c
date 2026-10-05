/*
 * parts/waitHeadTurn.c -- included by stx_u3.c; never compiled on its own.
 * It must sit directly before gameTick so its call to gameTick stays a
 * short branch.
 */

/* Tick until the head animation reaches its target (headPose ==
   headTarget). */

void
waitHeadTurn()
{
        while (headPose != headTarget)
                gameTick(0);
}
