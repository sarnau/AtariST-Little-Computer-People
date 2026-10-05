/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Walks the resident to the front door on the ground floor: the first
   step of collecting a delivery, of closing the front door while
   tidying up, and of the end of the move-in cutscene. */
void
walkToFrontDoor()
{
        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        walkToTarget();
}
