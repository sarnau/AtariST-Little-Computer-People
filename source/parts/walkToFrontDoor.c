/* Walks the resident to the front door on the ground floor: the first
   step of collecting a delivery, of closing the front door while
   tidying up, and of the end of the move-in cutscene. */
void
walkToFrontDoor()
{
        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
}
