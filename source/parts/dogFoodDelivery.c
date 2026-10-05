/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Dog-food delivery (ACTION_EVENT_DOG_FOOD, queued by Ctrl-D): the
   food delivery with isDogDelivery set, which makes foodDelivery take the
   feed-the-dog branch. */
void
dogFoodDelivery()
{
        isDogDelivery = YES;
        foodDelivery();
        isDogDelivery = NO;
}
