/*
 * parts/showResident.c -- included by stx_u2.c; never compiled on its own.
 */

/* showResident: restore the pointers hideResident() stashed. */

void
showResident()
{
        drawnImage[HW_SLOT_LCP_BODY] = savedBodyImg;
        drawnImage[HW_SLOT_LCP_HEAD] = savedHeadImg;
        lcpHidden     = NO;
}
