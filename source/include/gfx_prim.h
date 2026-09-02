/* gfx_prim.h -- extern declarations for gfx_prim.c. */

#ifndef GFX_PRIM_H
#define GFX_PRIM_H

extern void drwLine();
extern void sc_sdtb();
extern void sc_sdtf();
extern void sc_firw();
extern void sc_firs();
extern void sc_firb();
extern void initVdi();
extern void exitVdi();
extern void drwPixel();
extern void blkcp32();
extern void cpyScr();
extern void aes_init();
extern void vdi_init();
#ifndef FAITHFUL
extern void vdi_cls();          /* STX 0x66fe, vdi_init's second half */
#endif
extern void stpScrB();
extern void vst_h20();
extern void rst_vsth();
extern void moff();

#endif /* GFX_PRIM_H */
