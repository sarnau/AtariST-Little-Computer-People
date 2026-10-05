/* Writes size bytes from addr to filename (the HYBER save file),
   creating it first if needed (ensureFile).  A failed open or write
   shows the RETRY-only alert (writeErrorAlert) and tries again, until
   it succeeds. */
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
