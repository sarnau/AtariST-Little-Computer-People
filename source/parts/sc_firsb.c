/*
 * parts/sc_firsb.c -- shared body; LCP_STX puts sc_firs (0x16e22) and
 * sc_firb (0x16e76) in the sprite object between sc_firw and strPr.
 * Files under parts/ are never compiled standalone.
 */
/* sc_firs: paint row with 0x0033 (2 planes) -- light-cyan status stripe.
   addr: sc_firs() */

void
sc_firs(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

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
                *scrptr++ = 0x0000;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
        }
}

/* sc_firb: paint row with 0 -> palette index 0 (black) separator.
   addr: sc_firb() */

void
sc_firb(scraddr, row)
unsigned short *        scraddr;
short                   row;
{
        short   column;

#ifdef HOST
        /* Alcyon takes a cast as an lvalue and the compound form is what
           emits `add.l d0,mem`; clang cannot parse it at all.  Same
           arithmetic, spelled for the host.  See CLAUDE.md. */
        scraddr = (short *) ((char *) scraddr + row * 160);
#else
        (char *) scraddr += row * 160;
#endif
        for (column = 0; column < 80; column++)
                *scraddr++ = 0;
}
