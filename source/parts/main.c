/*
 * parts/main.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x5546, in the 0x400c object between fr_reac and dg_ipos). Files
 * under parts/ are never compiled standalone.
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
        /* STX's frame is -42.  ct_clrB, the .SCN file handling,
           al_locs, al_loot/al_lost and sp_reglp's loop are all INLINED
           here, which is why this function is 1370 bytes. */
        short   i;              /* -2 */
        short * p;              /* -6 */
        short * q;              /* -10 */
        short   w;              /* -12 */
        short   h;              /* -14 */
        short   wpr;            /* -16 */
        short   pad1;           /* -18 */
        short   pad2;           /* -20 */
        short   fhandle;        /* -22 */
        char *  conterm;        /* -26 */
        long    ssp;            /* -30 */
        short   r[4];           /* -32 .. -38 */

        mq_intim();
        aes_init();

        /* ct_clrB inlined: clear bits 0..2 of TOS's `conterm`. */
        conterm = (char *) 0x484L;
        ssp = Super(0L);
        *conterm = *conterm & 0xf8;
        Super(ssp);

        /* STX's data files live in a DATA subdirectory. */
        Dsetpath("data");

        vdi_init();
        stpScrB();
        initBRev();
        cntSong();
        g_lcldd = lc_load();
        st_titl();

        /* The .SCN file handling is inlined here; only the nibble
           decoder is a function.  Note the handle is never closed. */
        fhandle = fOpen("house.scn", 0);
        fr_read(fhandle, 2L, &scn_siz);
        scn_buf = (char *) Malloc((long) (scn_siz - 32));
        if (scn_buf == (char *) 0)
                er_nomem();
        fr_read(fhandle, 30L, scn_dic);
        fr_read(fhandle, (long) (scn_siz - 32), scn_buf);
        scn_dec(scn_buf, g_srptr, 16000);
        Mfree(scn_buf);

        /* Ghidra step 14 */  fillTopR(27);
        /* Ghidra step 15 */  cl_drini();                /* clock_draw_initial */

        /* al_locs inlined: body.lcp loads FIRST, then lcp_crnd for a
           new game, then the PEx filename is patched and loaded. */
        al_loal("body.lcp", (unsigned char *) body_ptr);
        if (g_lcldd == 0)
                lcp_crnd();
        pex_name[2] = lcp.character_sprite_id + '0';
        al_loal(pex_name, (unsigned char *) pex_ptr);

        sp_lbal();

        /* al_loot / al_lost / sp_reglp are all inlined here: a fixed
           56- and 50-iteration walk with no zero-record or size check
           and no return value. */
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
        if (lcp_bwlS == 0)
                od_draw(51, 8, 190);
        if (lcp_bwlS == 1)
                od_draw(50, 8, 190);
        if (lcp_bwlS == 2)
                od_draw(49, 8, 190);

        /* Ghidra step 34 */  sc_drfc();                /* screen_draw_food_cabinet */
        /* Ghidra step 35 */  daily_rs();
        /* Ghidra step 36 */  pa_cloc();                /* palette_apply_clothing_colors */
        /* Ghidra step 37 */
#ifdef SKIP_COPYPROT
        /* Test builds only.  cp_main drives the 1772 directly to read
           the protected track, and no emulator here satisfies it: it
           returns 0, cs_mvIn parks the resident in
           `while (1) a_sleep(-1);` -- which re-runs lcp_hwt() every
           iteration, so it stands and waves for ever -- and gameLoop
           does the same.  A non-zero cprot_r is all either test wants;
           the real routine ORs 0xf0000000 into its count.  Skipping
           the CALL rather than the check also avoids the FDC wait,
           which never terminates when the program was launched from a
           drive that is not the floppy.

           This is NOT part of the shipped configuration: the default
           build must stay byte-identical to DATA/LCP_STX.PRG. */
        cprot_r = 0xf000000aL;
#else
        cprot_r = cp_main();  /* copyprot_main_check */
#endif
        /* Ghidra step 38 */  sp_imfs();                /* sprite_init_MFDBs */
        /* Ghidra step 39 */
        if (g_lcldd == 0)
                cs_mvIn();        /* cutscene_new_lcp_move_in */

        /* gameLoop never returns; there is no Pterm here. */
        gameLoop();
}
