/* paperRow: fill one 160-byte scan line with colour index 14 (bit
   planes 1-3 set, plane 0 clear) -- the letter paper. */

void
paperRow(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

        /* A 16-bit row multiply (no (long) cast) and post-incremented
           stores, as in the original. */
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
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
        }
}
