/*
 * Must sit between vdi_cls and initBRev.
 *
 * Included by stx_u1.c; never compiled on its own.
 */
/* aes_init: appl_init + graf_handle + Setpalette + physbase snapshot.
   Does NOT call v_opnvwk (vdi_init's job). */

#ifdef HOST

#include "hostgem.h"

#else

#include <gembind.h>            /* appl_init, graf_handle, graf_mouse, form_alert */

#endif

void
aes_init()
{

        appl_init();
        vdi_hnd = graf_handle(&gr_hwchar, &gr_hhchar,
                                 &gr_hwbox,  &gr_hhbox);
        Setpalette(main_pal);
        sv_phb = (void *) Physbase();
}
