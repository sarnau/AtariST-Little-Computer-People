/*
 * Must sit between vdi_cls and initBRev.
 *
 * Included by stx_u1.c; never compiled on its own.
 */

#ifdef HOST

#include "hostgem.h"

#else

#include <gembind.h>            /* appl_init, graf_handle, graf_mouse, form_alert */

#endif

/* Boot-time AES set-up: appl_init, then graf_handle for the
   workstation handle (vdi_hnd) and the system character/box metrics,
   load the game palette main_pal, and remember TOS's own physical
   screen base in sv_phb so the compositor can tell it apart from its
   buffers.  It does NOT open the virtual workstation -- that is
   vdi_init's job. */
void
aes_init()
{

        appl_init();
        vdi_hnd = graf_handle(&gr_hwchar, &gr_hhchar,
                                 &gr_hwbox,  &gr_hhbox);
        Setpalette(main_pal);
        sv_phb = (void *) Physbase();
}
