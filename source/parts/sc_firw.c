/*
 * parts/sc_firw.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x16dcc, immediately after sc_sctd (bsr.s)). Files under
 * parts/ are never compiled standalone.
 */
/* sc_firw: paint row (160 B) with 0x0FFF (palette entry 0xF, white).
   addr: sc_firw() */

void
sc_firw(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

        /* STX: a 16-bit row multiply and post-incremented stores. */
#ifdef HOST
        /* Alcyon takes a cast as an lvalue and the compound form is what
           emits `add.l d0,mem`; clang cannot parse it at all.  Same
           arithmetic, spelled for the host.  See CLAUDE.md. */
        scrptr = (short *) ((char *) scrptr + row * 160);
#else
        (char *) scrptr += row * 160;
#endif
        for (i = 0; i < 20; i++) {
                *scrptr++ = 0x0000;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
        }
}
