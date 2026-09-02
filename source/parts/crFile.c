/*
 * parts/crFile.c -- shared body; LCP_ORG links it in save.c,
 * LCP_STX in the 0xdece object (0x1488e, right after lcp_save).  Files under parts/
 * are never compiled standalone.
 */
/* addr: crFile() */
void
crFile(filename)
char *  filename;
{
        short   rval;
        short   iVar1;

        rval = access(filename, 4);
        if (rval == 0)
                return;

        for (;;) {
                iVar1 = Fcreate(filename, 0L);
                if (iVar1 >= 0)
                        break;
                er_write();
        }
        Fclose(iVar1);
}
