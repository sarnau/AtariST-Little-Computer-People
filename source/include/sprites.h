/* sprites.h -- extern declarations for sprites.c. */

#ifndef SPRITES_H
#define SPRITES_H

/* Source-packed byte size of one 16x21 LCP body/head sprite frame:
   21 rows * 4 bytes per row * 2 bit-planes.  Applies to every frame in
   BODY.LCP and PEn.LCP, and to the source stride used by sp_lcpf and
   sp_lbbd / sp_lbhd when walking the raw sprite table. */
#define LCP_BODY_FRAME_SIZE     (21 * 4 * 2)

/* Dilated body/head shape stride: 21 rows * 4 bytes per row.
   Half of LCP_BODY_FRAME_SIZE because the shape buffers (body_shp,
   hd_shp) collapse the 2-plane source into a single-plane
   silhouette used by sp_lbbd / sp_lbhd. */
#define LCP_BODY_SHAPE_SIZE     (21 * 4)

/* Expanded-sprite buffer size in SHORTS (image or mask, one plane
   set) that sp_lcpf writes into g_lsimg / g_lsmas / g_hsbuf /
   g_hsmas: height=21 rows, width=2 source columns, 4 dest shorts
   per column (horizontal 2x expansion into a 4-plane MFDB slot).
   Numerically equals LCP_BODY_FRAME_SIZE by coincidence -- the
   2x expansion in shorts cancels the 2 bytes/short factor -- but
   the unit is shorts, not bytes. */
#define LCP_BODY_DEST_WORDS     (21 * 4 * 2)

extern void sp_updb();
extern void sp_ssco();
extern void sp_sprs();
extern void lcp_hwt();
extern void hideLcp();
extern void showLcp();
extern void sp_ss02();
extern void sp_lcpf();
extern void sp_flih();
extern void sp_upds();
extern void sp_lchu();
extern void sp_imfs();
extern void sp_drin();
extern void sp_lbbd();
extern void sp_lbhd();
extern void sp_lbal();

#endif /* SPRITES_H */
