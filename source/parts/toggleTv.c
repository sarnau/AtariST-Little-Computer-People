/* ACTION_TOGGLE_TV: switch the TV off if tvRunning says it is on,
   otherwise on.  tvOff/tvOn do the walking and the animation. */
void
toggleTv()
{
        if (tvRunning != NO)
                tvOff();
        else
                tvOn();
}
