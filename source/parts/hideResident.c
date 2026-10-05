/*
 * Stash the body/head image pointers, NULL them and raise lcpHidden.
 *
 * Included by stx_u2.c; never compiled on its own.
 */

void
hideResident()
{
        savedBodyImg  = drawnImage[HW_SLOT_LCP_BODY];
        savedHeadImg  = drawnImage[HW_SLOT_LCP_HEAD];
        drawnImage[HW_SLOT_LCP_BODY] = NULL;
        drawnImage[HW_SLOT_LCP_HEAD] = NULL;
        lcpHidden     = YES;
}
