/*
 * Included by stx_u1.c; never compiled on its own.
 */
int
#ifdef HOST
/* The host unit tests each supply their own main(), and this one now
   lives inside stx_u1.o where it cannot be left out of the link.
   Renaming it for the host keeps both linkable; the Atari build is
   untouched. */
lcp_main(argc, argv)
#else
main(argc, argv)
#endif
int     argc;
char ** argv;
{
        /* The conterm clear, the .SCN file handling, and the object and
           sprite loading are all written out inline here, which is why
           this function is so long.  pad1/pad2 are unused but must
           stay, and the declaration order is the original's frame
           layout. */
        short   i;
        short * p;
        short * q;
        short   w;
        short   h;
        short   wpr;
        short   pad1;
        short   pad2;
        short   fhandle;
        char *  conterm;
        long    ssp;
        short   r[4];

        mq_intim();
        aes_init();

        /* Clear bits 0..2 of TOS's `conterm` (key click, key repeat,
           bell) at 0x484, which needs supervisor mode. */
        conterm = (char *) 0x484L;
        ssp = Super(0L);
        *conterm = *conterm & 0xf8;
        Super(ssp);

        /* The data files live in a DATA subdirectory. */
        Dsetpath("data");

        vdi_init();
        stpScrB();
        initBRev();
        cntSong();
        g_lcldd = lc_load();
        st_titl();

        /* The .SCN file handling is inlined here; only the nibble
           decoder is a function.  Note the handle is never closed. */
        fhandle = fOpen("house.scn", RMODE_RD);
        fr_read(fhandle, 2L, &scn_siz);
        scn_buf = (char *) Malloc((long) (scn_siz - 32));
        if (scn_buf == (char *) 0)
                er_nomem();
        fr_read(fhandle, 30L, scn_dic);
        fr_read(fhandle, (long) (scn_siz - 32), scn_buf);
        scn_dec(scn_buf, g_srptr, 16000);
        Mfree(scn_buf);

        fillTopR(27);
        cl_drini();

        /* body.lcp loads FIRST, then lcp_crnd for a
           new game, then the PEx filename is patched and loaded. */
        al_loal("body.lcp", (unsigned char *) body_ptr);
        if (g_lcldd == 0)
                lcp_crnd();
        pex_name[2] = lcp.character_sprite_id + '0';
        al_loal(pex_name, (unsigned char *) pex_ptr);

        sp_lbal();

        /* Object and sprite tables: a fixed 56- and 50-iteration walk
           with no zero-record or size check. */
        ldObj();
        p = (short *) obj_file;
        for (i = 0; i < 56; i++) {
                h = *p;
                p++;
                g_obtah[i] = h;
                w = *p;
                g_obtaw[i] = w;
                p++;
                wpr = w / 16;
                if (w % 16)
                        wpr++;
                sp_iniM(0L, &g_obtmt[i], p, wpr << 4, h);
#ifdef HOST
                /* Alcyon takes a cast as an lvalue and the compound form is what
                   emits `add.l d0,mem`; clang cannot parse it at all.  Same
                   arithmetic, spelled for the host.  See CLAUDE.md. */
                p = (void *) ((char *) p + ((wpr * h) << 3));
#else
                (char *) p += (wpr * h) << 3;
#endif
        }

        ldSpr();
        p = (short *) spr_file;
        q = (short *) sp_mbuf;
        for (i = 0; i < 50; i++) {
                h = *p;
                p++;
                w = *p;
                p++;
                wpr = w / 16;
                if (w % 16)
                        wpr++;
                sp_regs(sp_fidx[i], p, q, h, wpr << 4);
#ifdef HOST
                /* Alcyon takes a cast as an lvalue and the compound form is what
                   emits `add.l d0,mem`; clang cannot parse it at all.  Same
                   arithmetic, spelled for the host.  See CLAUDE.md. */
                p = (void *) ((char *) p + ((wpr * h) << 3));
#else
                (char *) p += (wpr * h) << 3;
#endif
#ifdef HOST
                /* Alcyon takes a cast as an lvalue and the compound form is what
                   emits `add.l d0,mem`; clang cannot parse it at all.  Same
                   arithmetic, spelled for the host.  See CLAUDE.md. */
                q = (void *) ((char *) q + ((wpr * h) << 3));
#else
                (char *) q += (wpr * h) << 3;
#endif
        }

        sf_sl();
        dg_ipos();
        if (g_lcldd == 0)
                sp_spud(-1, 1, NO);
        updWtLv(0);

        /* Water pipe polyline (147..158, 175): the second point is
           written as offsets from the first. */
        sc_sdtb();
        r[0] = 147;
        r[1] = 175;
        r[2] = r[0] + 11;
        r[3] = r[1];
        vsl_color(vdihnd, vdi_colt[0xb]);
        v_pline(vdihnd, 2, r);
        sc_sdtf();

        /* Door / cabinet draws: each is a full if/else with the whole
           od_draw call duplicated, not a ternary in the argument. */
        if (lcp_cabO == NO)
                od_draw(od_cbcl, 46, 140);
        else
                od_draw(od_cbo2, 46, 140);
        if (lcp_frdO != NO)
                od_draw(od_fro2, 294, 151);
        else
                od_draw(od_frcl, 294, 151);
        if (lcp_drsO != NO)
                od_draw(od_dro2, 97, 115);
        else
                od_draw(od_drcl, 97, 115);
        if (lcp_clsO != NO)
                od_draw(od_clo2, 75, 87);
        else
                od_draw(od_clcl, 75, 87);
        if (studyDrO != NO)
                od_draw(od_sto2, 178, 23);
        else
                od_draw(od_stcl, 178, 23);
        if (lcp_toiO != NO)
                od_draw(od_too2, 187, 87);
        else
                od_draw(od_tocl, 187, 87);
        if (lcp_flcO != NO)
                od_draw(od_fio2, 258, 47);
        else
                od_draw(od_ficl, 258, 47);

        /* Dog bowl: three explicit state tests with literal frame
           ids, not an index into g_obdea. */
        if (lcp_bwlS == BOWL_EMPTY)
                od_draw(OBJ_DOG_FOOD_BOWL_3, 8, 190);
        if (lcp_bwlS == BOWL_HALF)
                od_draw(OBJ_DOG_FOOD_BOWL_2, 8, 190);
        if (lcp_bwlS == BOWL_FULL)
                od_draw(OBJ_DOG_FOOD_BOWL_1, 8, 190);

        sc_drfc();
        daily_rs();
        pa_cloc();
#ifdef SKIP_COPYPROT
        /* Test builds only.  cp_main drives the 1772 directly to read
           the protected track, and no emulator here satisfies it: it
           returns 0, cs_mvIn parks the resident in
           `while (1) a_sleep(SLEEP_RANDOM);` -- which re-runs lcp_hwt() every
           iteration, so it stands and waves for ever -- and gameLoop
           does the same.  A non-zero cprot_r is all either test wants;
           the real routine ORs 0xf0000000 into its count.  Skipping
           the CALL rather than the check also avoids the FDC wait,
           which never terminates when the program was launched from a
           drive that is not the floppy.

           This is NOT part of the shipped configuration: the default
           build must stay byte-identical to the original. */
        cprot_r = 0xf000000aL;
#else
        cprot_r = cp_main();  /* copy-protection check */
#endif
        sp_imfs();
        if (g_lcldd == 0)
                cs_mvIn();        /* new game: move-in cutscene */

        /* gameLoop never returns; there is no Pterm here. */
        gameLoop();
}
