/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Dog-food delivery (ACTION_EVENT_DOG_FOOD, queued by Ctrl-D): the
   food delivery with g_dvdog set, which makes er_food take the
   feed-the-dog branch. */
void
er_dogf()
{
        g_dvdog = YES;
        er_food();
        g_dvdog = NO;
}
