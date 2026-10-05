/*
 * The last function of the stx_u2 object.
 *
 * Included by stx_u2.c; never compiled on its own.
 */
void
saveFile(filename, size, addr)
char *  filename;
short   size;
void *  addr;
{
        short   filehandle;
        long    written;

        ensureFile(filename);

        for (;;) {
                filehandle = Fopen(filename, RMODE_WR);
                if (filehandle >= 0)
                        break;
                writeErrorAlert();
        }

        for (;;) {
                written = Fwrite(filehandle, (long) size, addr);
                /* The size cast stays on the left on purpose: swapping
                   the operands changes the compiled code. */
                if ((long) size == written)
                        break;
                writeErrorAlert();
        }

        Fclose(filehandle);
}
