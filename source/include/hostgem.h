/*
 * hostgem.h -- host-build stand-ins for the Alcyon GEM headers.
 *
 * The port includes <vdibind.h>, <ostruct.h>, <gembind.h> and
 * <obdefs.h> from the Alcyon TOOLS/INCLUDE tree.  Those exist only in
 * the cross-toolchain, so `make` (the host cc syntax/semantic check)
 * cannot see them.  This header supplies the handful of declarations
 * the port actually uses from them, and is included INSTEAD of the
 * four whenever HOST is defined.
 *
 * Layout matters even though nothing here runs: structs.h loads the
 * 128-byte HYBER save straight into the LCP struct, and the DTA is the
 * 44-byte GEMDOS disk transfer area.  Alcyon's int is 16 bits, so
 * every field the ST headers declare `int` is `short` here -- with the
 * host's 32-bit int these structures would be the wrong size and the
 * host build would stop being a check on the real layout.
 *
 * Never seen by the Atari build: HOST is not defined there, and
 * alcyon_build.sh passes only -D__ALCYON__ plus ALCYON_CPPFLAGS.
 */

#ifndef HOSTGEM_H
#define HOSTGEM_H

#ifndef HOST
#error "hostgem.h is for the host build only -- include the real GEM headers"
#endif

/* <vdibind.h>: the VDI memory form definition block.  Nine fields, the
   eight extents 16-bit, as on the ST. */
#ifndef _MFDB_DEFINED
#define _MFDB_DEFINED
typedef struct {
        char *          fd_addr;
        short           fd_w;
        short           fd_h;
        short           fd_wdwidth;
        short           fd_stand;
        short           fd_nplanes;
        short           fd_r1;
        short           fd_r2;
        short           fd_r3;
} MFDB;
#endif

/* <ostruct.h>: the 44-byte GEMDOS disk transfer area. */
#ifndef _OSTRUCT_H
#define _OSTRUCT_H
typedef struct {
        char            d_reserved[21];
        char            d_attrib;
        unsigned short  d_time;
        unsigned short  d_date;
        long            d_length;
        char            d_fname[14];
} _DTA;
#endif

/* <obdefs.h>: the raster-op and writing-mode constants the port uses,
   with the DK's values. */
#define ALL_WHITE       0
#define MD_REPLACE      1
#define MD_TRANS        2
#define S_ONLY          3
#define NOTS_AND_D      4
#define S_XOR_D         6

/* er_nomem writes to stderr on the host. */
#include <stdio.h>

/* <vdibind.h> / <gembind.h>: K&R declarations for the bindings the port
   calls but does not define in the unit doing the calling.  Alcyon
   would take these implicitly; modern clang will not.
   vdiown.h already declares the nine bindings the game supplies
   itself; these are the ones only the DK headers had. */
extern void     vqt_attributes();
extern void     vst_height();
extern void     v_opnvwk();
extern void     v_clsvwk();
extern void     vro_cpyfm();
extern short    appl_init();
extern void     appl_exit();
extern short    graf_handle();
extern short    graf_mouse();

#endif /* HOSTGEM_H */
