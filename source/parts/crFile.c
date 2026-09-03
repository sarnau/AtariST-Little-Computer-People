/*
 * parts/crFile.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x1488e, right after lcp_save). Files under parts/ are never
 * compiled standalone.
 */
/* addr: crFile() */
void
crFile(filename)
char *  filename;
{
        /* STX: link #-10 -- the create attribute goes through a third
           local, and the retry is a goto loop. */
        short   rval;
        short   iVar1;
        short   attr;

        rval = access(filename, 4);
        if (rval == 0)
                return;

again:
        attr  = 0;
        iVar1 = Fcreate(filename, attr);
        if (iVar1 < 0) {
                er_write();
                goto again;
        }
        Fclose(iVar1);
}
