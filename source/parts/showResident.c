/* showResident: restore the pointers hideResident() stashed. */

void
showResident()
{
        drawnImage[HW_SLOT_LCP_BODY] = savedBodyImg;
        drawnImage[HW_SLOT_LCP_HEAD] = savedHeadImg;
        lcpHidden = NO;
}
