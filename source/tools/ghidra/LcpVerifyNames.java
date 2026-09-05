// LcpVerifyNames.java -- read-only check that Ghidra carries the port's
// names.  Reports, never writes.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class LcpVerifyNames extends GhidraScript {
    public void run() throws Exception {
        File tsv = new File(System.getProperty("user.home"),
                            "ghidra_scripts/lcp_verify.tsv");
        SymbolTable st = currentProgram.getSymbolTable();
        int ok=0, bad=0, miss=0;
        BufferedReader r = new BufferedReader(new FileReader(tsv));
        String line;
        while ((line = r.readLine()) != null) {
            String[] c = line.split("\t");
            if (c.length != 3) continue;
            Address ad = currentProgram.getAddressFactory().getAddress(c[1]);
            String want = c[2], got;
            if (c[0].equals("F")) {
                Function f = getFunctionAt(ad);
                if (f == null) { miss++; continue; }
                got = f.getName();
            } else {
                Symbol p = st.getPrimarySymbol(ad);
                if (p == null) { miss++; continue; }
                got = p.getName();
            }
            if (got.equals(want)) ok++;
            else { bad++; println("VERIFY MISMATCH " + c[1] + " want=" + want + " got=" + got); }
        }
        r.close();
        println("VERIFY ok=" + ok + " mismatched=" + bad + " no_symbol=" + miss);
    }
}
