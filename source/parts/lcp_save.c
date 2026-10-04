/*
 * The last function of the stx_u2 object.
 *
 * Included by stx_u2.c; never compiled on its own.
 */
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
                filehandle = Fopen(filename, RMODE_WR);
                if (filehandle >= 0)
                        break;
                er_write();
        }

        for (;;) {
                lVar1 = Fwrite(filehandle, (long) size, addr);
                /* The size cast stays on the left on purpose: swapping
                   the operands changes the compiled code. */
                if ((long) size == lVar1)
                        break;
                er_write();
        }

        Fclose(filehandle);
}
