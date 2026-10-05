/* Nod in agreement.  Picks a head target from the direction the
   head currently faces (low three bits of headPose) and waits for it,
   then four times alternates between that direction level and tilted
   (bit 0x10) -- a nod -- before returning the head to where it started.
   chooseAction plays it when the resident accepts a request to play a game
   or the piano, and the move-in cutscene uses it.  It is also the body
   of action 35 (ACTION_NOD_OK), but no command, event or AI table
   ever queues that action. */
void
nodOk()
{
        short   entryCurrent;
        short   h;
        /* Only two locals: h doubles as the loop counter below.
           Do not add a separate one. */

        entryCurrent = headPose;
        h = headPose & HEAD_DIR_MASK;

        /* First turn toward a nearby front-ish pose, tilted lower. */
        if (h == HEAD_DIR_FRONT || h == HEAD_DIR_FRONT_RIGHT ||
            h == HEAD_DIR_FRONT_LEFT)
                headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        else if (h == HEAD_DIR_RIGHT)
                headTarget = HEAD_POSE(HEAD_DIR_FRONT_RIGHT, HEAD_TILT_LOWER);
        else if (h == HEAD_DIR_LEFT)
                headTarget = HEAD_POSE(HEAD_DIR_FRONT_LEFT, HEAD_TILT_LOWER);
        else if (h == HEAD_DIR_BACK_RIGHT || h == HEAD_DIR_BACK)
                headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
        else if (h == HEAD_DIR_BACK_LEFT)
                headTarget = HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER);

        headMode = HEAD_ANIM_DISABLED;
        waitHeadTurn();

        for (h = 0; h < 4; h++) {
                headTarget = headPose & HEAD_DIR_MASK;
                waitHeadTurn();
                headTarget = headPose | HEAD_TILT_LOWEST << HEAD_TILT_SHIFT;
                waitHeadTurn();
        }

        headTarget = entryCurrent;
        waitHeadTurn();
}
