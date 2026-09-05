// LcpSyncNames.java -- push the port's names into the Ghidra project.
//
// Replaces the RenameLcpGlobals.java + :8089 HTTP pipeline, which needs
// scripts and a server that are not installed.  Runs headless instead:
//
//   analyzeHeadless <projdir> LCP -process -noanalysis \
//       -scriptPath ~/ghidra_scripts -postScript LcpSyncNames.java
//
// Reads ~/ghidra_scripts/lcp_sync.tsv, one row per rename:
//   D <TAB> current_ghidra_name <TAB> port_name     (data symbol, by name)
//   F <TAB> 0xADDRESS           <TAB> port_name     (function, by address)
//
// Data rows go by NAME because the map is name-to-name; function rows go
// by ADDRESS because that is what the port's symbol table gives us
// (Ghidra address = link address + 0x10000).  Rows already carrying the
// port name are counted as ok and skipped, so the script is idempotent.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.*;
import ghidra.util.exception.DuplicateNameException;
import java.io.*;
import java.util.*;

public class LcpSyncNames extends GhidraScript {
    public void run() throws Exception {
        File tsv = new File(System.getProperty("user.home"),
                            "ghidra_scripts/lcp_sync.tsv");
        if (!tsv.exists()) { println("SYNC ERROR: missing " + tsv); return; }

        SymbolTable st = currentProgram.getSymbolTable();
        int dOk=0, dSkip=0, dMiss=0, dFail=0;
        int fOk=0, fSkip=0, fMiss=0, fFail=0;
        List<String> notes = new ArrayList<String>();

        BufferedReader r = new BufferedReader(new FileReader(tsv));
        String line;
        while ((line = r.readLine()) != null) {
            String[] c = line.split("\t");
            if (c.length != 3) continue;

            if (c[0].equals("D")) {
                String from = c[1], to = c[2];
                if (!st.getGlobalSymbols(to).isEmpty()) { dSkip++; continue; }
                List<Symbol> hits = st.getGlobalSymbols(from);
                if (hits.isEmpty()) { dMiss++; notes.add("D miss " + from); continue; }
                try {
                    hits.get(0).setName(to, SourceType.USER_DEFINED);
                    dOk++;
                } catch (Exception e) {
                    dFail++; notes.add("D fail " + from + " -> " + to + ": " + e.getMessage());
                }
            } else if (c[0].equals("A")) {
                // Rename (or create) the primary label AT an address.
                // Used where the port's own name changed, so a
                // name-to-name row would chase a symbol that is no
                // longer there -- and where two names were SWAPPED, in
                // which case going by name could collide.
                Address ad;
                try { ad = currentProgram.getAddressFactory().getAddress(c[1]); }
                catch (Exception e) { dMiss++; continue; }
                Symbol pri = st.getPrimarySymbol(ad);
                try {
                    if (pri == null) {
                        st.createLabel(ad, c[2], SourceType.USER_DEFINED);
                        dOk++;
                    } else if (pri.getName().equals(c[2])) {
                        dSkip++;
                    } else {
                        notes.add("A " + c[1] + " " + pri.getName() + " -> " + c[2]);
                        pri.setName(c[2], SourceType.USER_DEFINED);
                        dOk++;
                    }
                } catch (Exception e) {
                    dFail++; notes.add("A fail " + c[1] + " -> " + c[2] + ": " + e.getMessage());
                }
            } else if (c[0].equals("F")) {
                Address ad;
                try { ad = currentProgram.getAddressFactory().getAddress(c[1]); }
                catch (Exception e) { fMiss++; continue; }
                Function fn = getFunctionAt(ad);
                if (fn == null) { fMiss++; notes.add("F miss " + c[1] + " " + c[2]); continue; }
                if (fn.getName().equals(c[2])) { fSkip++; continue; }
                try {
                    fn.setName(c[2], SourceType.USER_DEFINED);
                    fOk++;
                } catch (Exception e) {
                    fFail++; notes.add("F fail " + c[1] + " -> " + c[2] + ": " + e.getMessage());
                }
            }
        }
        r.close();

        for (String n : notes) println("SYNC NOTE " + n);
        println("SYNC data renamed=" + dOk + " already=" + dSkip
                + " missing=" + dMiss + " failed=" + dFail);
        println("SYNC func renamed=" + fOk + " already=" + fSkip
                + " missing=" + fMiss + " failed=" + fFail);
    }
}
