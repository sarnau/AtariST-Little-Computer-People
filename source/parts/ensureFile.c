/*
 * Included by stx_u2.c; never compiled on its own.
 */
/* Make sure the save file `filename' exists before saveFile writes
   it: if access() finds it nothing happens, otherwise it is created
   empty and closed.  A failed Fcreate shows writeErrorAlert's alert (e.g. a
   write-protected disk) and tries again until it succeeds. */
void
ensureFile(filename)
char *  filename;
{
        /* The create attribute goes through a third local and the
           retry is a goto loop, as in the original. */
        short   rval;
        short   fhnd;
        short   attr;

        rval = access(filename, 4);
        if (rval == 0)
                return;

again:
        attr  = 0;
        fhnd = Fcreate(filename, attr);
        if (fhnd < 0) {
                writeErrorAlert();
                goto again;
        }
        Fclose(fhnd);
}
