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

#ifdef FAITHFUL
        scrptr = (unsigned short *)
                 ((char *) scrptr + (long) row * 160);
        for (i = 0; i < 20; i = i + 1) {
                scrptr[0] = 0x0000;
                scrptr[1] = 0x0000;
                scrptr[2] = 0xffff;
                scrptr[3] = 0xffff;
                scrptr = scrptr + 4;
        }
#else
        (char *) scrptr += row * 160;
        for (i = 0; i < 20; i++) {
                *scrptr++ = 0x0000;
                *scrptr++ = 0x0000;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
        }
#endif
}

/* sc_firb: paint row with 0 -> palette index 0 (black) separator.
   addr: sc_firb() */

void
sc_firb(scraddr, row)
unsigned short *        scraddr;
short                   row;
{
        short   column;

#ifdef FAITHFUL
        scraddr = (unsigned short *)
                  ((char *) scraddr + (long) row * 160);
        for (column = 0; column < 80; column = column + 1) {
                *scraddr = 0;
                scraddr = scraddr + 1;
        }
#else
        (char *) scraddr += row * 160;
        for (column = 0; column < 80; column++)
                *scraddr++ = 0;
#endif
}
