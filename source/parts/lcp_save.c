/*
 * parts/lcp_save.c -- shared body; LCP_ORG links it in save.o,
 * LCP_STX at 0x1481c, the last function in the 0xdece object.
 * Files under parts/ are never compiled standalone.
 */
/* addr: lcp_save() */
void
lcp_save(filename, size, addr)
char *  filename;
short   size;
void *  addr;
{
        short   filehandle;
        long    lVar1;

        crFile(filename);

        for (;;) {
                filehandle = Fopen(filename, 1L);
                if (filehandle >= 0)
                        break;
                er_write();
        }

        for (;;) {
                lVar1 = Fwrite(filehandle, (long) size, addr);
                /* ROM evaluates the size cast first. */
                if ((long) size == lVar1)
                        break;
                er_write();
        }

        Fclose(filehandle);
}
