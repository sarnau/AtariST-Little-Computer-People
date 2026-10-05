/*
 * stripeRow then blackRow, adjacent as in the original.
 */
/* stripeRow: paint row with 0x0033 (2 planes) -- light-cyan status stripe. */

void
stripeRow(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

#ifdef HOST
        /* Alcyon accepts a cast as an lvalue, and the original uses
           this compound form; clang cannot parse it at all.  Same
           arithmetic, spelled for the host.  See docs/history.md. */
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

/* blackRow: paint row with 0 -> palette index 0 (black) separator. */

void
blackRow(scraddr, row)
unsigned short *        scraddr;
short                   row;
{
        short   column;

#ifdef HOST
        /* Alcyon accepts a cast as an lvalue, and the original uses
           this compound form; clang cannot parse it at all.  Same
           arithmetic, spelled for the host.  See docs/history.md. */
        scraddr = (short *) ((char *) scraddr + row * 160);
#else
        (char *) scraddr += row * 160;
#endif
        for (column = 0; column < 80; column++)
                *scraddr++ = 0;
}
