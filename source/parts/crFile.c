/*
 * Included by stx_u2.c; never compiled on its own.
 */
/* Make sure the save file `filename' exists before lcp_save writes
   it: if access() finds it nothing happens, otherwise it is created
   empty and closed.  A failed Fcreate shows er_write's alert (e.g. a
   write-protected disk) and tries again until it succeeds. */
void
crFile(filename)
char *  filename;
{
        /* The create attribute goes through a third local and the
           retry is a goto loop, as in the original. */
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
