/* Sets up the off-screen picture buffer at boot: housePtr becomes
   houseBuf rounded up to a 512-byte boundary, houseMfdb describes it as a
   320x200 (scaled by screenScale) bitmap, screenMfdb's NULL address names the
   physical screen, and copyScreen copies what is on screen into the new
   buffer. */
void
initHouseBuf()
{
        /* The buffer size goes through a local that both arms of a
           vestigial if/else set to the same value, and the pointer is
           aligned in the global itself.  The unused locals set the frame.
           All of this is the original's shape and must stay. */
        unsigned short  size;
        short           unused1;
        short           unused2;
        short           unused3;

        if (screenScale == 1)
                size = 0xE800;
        else
                size = 0xE800;
        /* fd_addr = NULL, written as its two halves; copyScreen passes the
           same address.  It is the NULL address that means "the device
           screen" to the VDI. */
        screenMfdb[0] = 0;
        screenMfdb[1] = 0;
        housePtr = (void *) houseBuf;
        housePtr = (void *) (((long) housePtr + 0x200L) & ~0x1FFL);
        initMfdb((long) (size >> 3), &houseMfdb, housePtr,
                screenScale * 0x140, screenScale * 200);
        copyScreen(vdiHandle, &houseMfdb);
}
