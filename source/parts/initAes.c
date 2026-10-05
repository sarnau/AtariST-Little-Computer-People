/*
 * Must sit between vdiClear and initMirror.
 *
 * Included by stx_u1.c; never compiled on its own.
 */

#ifdef HOST

#include "hostgem.h"

#else

#include <gembind.h>            /* appl_init, graf_handle, graf_mouse, form_alert */

#endif

/* Boot-time AES set-up: appl_init, then graf_handle for the
   workstation handle (physHandle) and the system character/box metrics,
   load the game palette mainPalette, and remember TOS's own physical
   screen base in tosPhysbase so the compositor can tell it apart from its
   buffers.  It does NOT open the virtual workstation -- that is
   vdiInit's job. */
void
initAes()
{

        appl_init();
        physHandle = graf_handle(&charWidth, &charHeight,
                                 &boxWidth,  &boxHeight);
        Setpalette(mainPalette);
        tosPhysbase = (void *) Physbase();
}
