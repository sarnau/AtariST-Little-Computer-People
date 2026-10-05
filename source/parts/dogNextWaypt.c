/* Dog waypoint math.  Same shape as nextWaypoint but uses
   dogX/y and applies -3 X on middle-floor landing + -8 X on stair
   crest. */

void
dogNextWaypt()
{
        /* One local: every floor lookup is called inline (the first
           result goes on the stack for the compare).  The equal case
           is the ELSE arm, so its three assignments sit at the end.
           Alcyon evaluates the RIGHT operand first, so swapping the two
           sides of the first comparison changes the compiled code. */
        short   si;

        if (floorOfY(dogY) != floorOfY(dogYTarget)) {
                dogXWaypt = stairWaypts[si = (floorOfY(dogY) - 1) * 2];
                dogYWaypt = stairWaypts[si + 1];

                if (floorOfY(dogY) == FLOOR_MIDDLE) {
                        if (floorOfY(dogY) > floorOfY(dogYTarget)) {
                                dogXWaypt = xLanding - 3;
                                dogYWaypt = yLanding;
                        }
                }

                dogOnStairs = NO;
                if (dogX == dogXWaypt && dogY == dogYWaypt) {
                        if (floorOfY(dogY) == FLOOR_TOP)
                                dogX -= 8;
                        dogOnStairs = YES;
                        if (dogY > dogYTarget) {
                                dogXWaypt = stairWaypts[si + 2];
                                dogYWaypt = stairWaypts[si + 3];
                        } else {
                                dogYWaypt = stairWaypts[si - 1];
                                dogXWaypt = stairWaypts[si - 2];
                        }
                        if (floorOfY(dogY) == FLOOR_BOTTOM) {
                                dogXWaypt = xLanding;
                                dogYWaypt = yLanding;
                        }
                }
        } else {
                dogOnStairs = NO;
                dogXWaypt = dogXTarget;
                dogYWaypt = dogYTarget;
        }
}
