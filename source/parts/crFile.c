/*
 * Included by stx_u2.c; never compiled on its own.
 */
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
