/*
 * parts/sc_firw.c -- shared body; LCP_ORG links it in gfx_prim.c,
 * LCP_STX in the 0xdece object (0x16dcc, immediately after sc_sctd (bsr.s)).  Files under parts/
 * are never compiled standalone.
 */
/* sc_firw: paint row (160 B) with 0x0FFF (palette entry 0xF, white).
   addr: sc_firw() */

void
sc_firw(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

#ifdef FAITHFUL
        scrptr = (unsigned short *)
                 ((char *) scrptr + (long) row * 160);
        for (i = 0; i < 20; i = i + 1) {
                scrptr[0] = 0x0000;
                scrptr[1] = 0xffff;
                scrptr[2] = 0xffff;
                scrptr[3] = 0xffff;
                scrptr = scrptr + 4;
        }
#else
        /* STX: a 16-bit row multiply and post-incremented stores. */
        (char *) scrptr += row * 160;
        for (i = 0; i < 20; i++) {
                *scrptr++ = 0x0000;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
        }
#endif
}
