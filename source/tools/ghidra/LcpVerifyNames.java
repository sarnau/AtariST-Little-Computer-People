// LcpVerifyNames.java -- read-only check that Ghidra carries the port's
// names.  Reports, never writes.
//
// The expectation list is GENERATED from the repo by
// tools/gen_ghidra_verify.py and handed to this script as its first
// argument:
//
//   analyzeHeadless <projdir> LCP -process -noanalysis -readOnly \
//       -scriptPath ~/ghidra_scripts \
//       -postScript LcpVerifyNames.java <path-to-lcp_verify.tsv>
//
// It used to read a hand-written ~/ghidra_scripts/lcp_verify.tsv that was
// not version-controlled, so a port rename made this report a FALSE
// mismatch until someone edited that file too.  That path is still
// accepted as a fallback, but sync_ghidra_names.sh always passes the
// generated one.  Every row is now a name the repo owns, so a mismatch
// here is a real disagreement rather than known noise.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class LcpVerifyNames extends GhidraScript {
    public void run() throws Exception {
        String[] argv = getScriptArgs();
        File tsv = (argv.length > 0 && argv[0].length() > 0)
                 ? new File(argv[0])
                 : new File(System.getProperty("user.home"),
                            "ghidra_scripts/lcp_verify.tsv");
        if (!tsv.exists()) {
            println("VERIFY ERROR: missing " + tsv
                    + " -- run tools/gen_ghidra_verify.py");
            return;
        }
        println("VERIFY reading " + tsv);

        SymbolTable st = currentProgram.getSymbolTable();
        int ok = 0, bad = 0, miss = 0;
        BufferedReader r = new BufferedReader(new FileReader(tsv));
        String line;
        while ((line = r.readLine()) != null) {
            if (line.startsWith("#")) continue;
            String[] c = line.split("\t");
            if (c.length != 3) continue;
            Address ad;
            try { ad = currentProgram.getAddressFactory().getAddress(c[1]); }
            catch (Exception e) { miss++; continue; }
            String want = c[2], got = null;
            if (c[0].equals("F")) {
                Function f = getFunctionAt(ad);
                if (f != null) got = f.getName();
            }
            if (got == null) {
                // Not every TEXT symbol is a function: LCP_STX keeps
                // mi_dwrm, mi_rlock, g_mtpre, g_msmsa and psg_ntAc in the
                // text segment behind mq_tick.  Fall back to the label so
                // those are checked rather than counted as missing.
                Symbol p = st.getPrimarySymbol(ad);
                if (p == null) { miss++; println("VERIFY NO SYMBOL " + c[1]
                                                 + " want=" + want); continue; }
                got = p.getName();
            }
            if (got.equals(want)) ok++;
            else { bad++; println("VERIFY MISMATCH " + c[1]
                                  + " want=" + want + " got=" + got); }
        }
        r.close();
        println("VERIFY ok=" + ok + " mismatched=" + bad
                + " no_symbol=" + miss);
    }
}
