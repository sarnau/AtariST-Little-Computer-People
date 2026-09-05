# Makefile for the C reconstruction of LCP.PRG.
#
# Two build modes:
#
#   make            -- HOST build: compile every .c with the host cc as a
#                      syntax / semantic sanity check.  Produces object
#                      files under build/host/ but does not link a
#                      runnable binary (the code targets ST hardware).
#
#   make alcyon     -- ATARI ST build: drive Alcyon C 4.14 (via wrappers
#                      run under Hatari or a native cross-toolchain
#                      shim) to produce LCP.PRG.  Requires ALCYON_BIN
#                      to be set to the directory holding CP68/C068/
#                      AS68/LO68.
#
#   make clean      -- remove all build outputs.
#
# Add new source files to the SOURCES list.  Header dependencies are
# tracked with -MMD so incremental rebuilds are correct.
#
# NOTE: files under source/parts/ are never listed -- they are bodies
# #included by the unity units (see tools/stx_units.txt), and the
# units' constituents are not listed either.

# ---- Source list ---------------------------------------------------------
# Derived, not hand-maintained: every .c in this directory EXCEPT the
# unity-unit constituents, which are #included by their unit and must
# not also be compiled on their own (that is what put midi_seq.c in the
# link twice, once standalone and once inside globals.c).  The skip
# list is columns 2+ of tools/stx_units.txt -- the same file
# alcyon_build.sh reads, so the two builds cannot drift apart.  (\043 is
# awk's '#': a literal one would start a make comment and swallow the
# closing paren of $(shell ...).)
#
# savehost.c and hostasm.c come in through the wildcard and are
# HOST-ONLY: stdio stand-ins for the GEMDOS/BIOS calls and for the five
# hand-written .s files.  alcyon_build.sh skips both by name, so they
# never reach the shipped binary.
UNIT_PARTS      := $(shell awk '$$1 !~ /^\043/ { for (i = 2; i <= NF; i++) print $$i }' tools/stx_units.txt 2>/dev/null)
SOURCES         := $(filter-out $(UNIT_PARTS),$(notdir $(wildcard *.c)))

# ---- Directory layout ----------------------------------------------------
INCDIR          := include
HOSTOBJDIR      := build/host
ALCYONOBJDIR    := build/alcyon

HOSTOBJS        := $(patsubst %.c,$(HOSTOBJDIR)/%.o,$(SOURCES))
ALCYONOBJS      := $(patsubst %.c,$(ALCYONOBJDIR)/%.o,$(SOURCES))

# ---- Host toolchain (syntax check only) ----------------------------------
# K&R-style extern decls with empty parens trigger -Wdeprecated-non-prototype
# on modern clang -- that's the Alcyon convention we're targeting, so silence
# the warning rather than "fixing" the code.
#
# The same goes for the three flags added below.  Alcyon is a K&R
# compiler: a call to an undeclared function is legal and returns int,
# a declaration with no type IS int, and a function may fall off its
# end leaving the last computed value in d0 -- which the port relies on
# deliberately (pk_cace and chk_timA both do it, see CLAUDE.md) -- and
# Super()'s void * drops straight into a long.  Clang makes all four
# hard errors by default.  Turning them back into what
# they are on the target keeps this build a check on the code we ship
# rather than a demand that the code stop being faithful.
CC              ?= cc
HOST_CFLAGS     := -I$(INCDIR) \
                   -DHOST \
                   -Wall -Wextra \
                   -Wno-deprecated-non-prototype \
                   -Wno-strict-prototypes \
                   -Wno-unused-parameter \
                   -Wno-implicit-function-declaration \
                   -Wno-implicit-int \
                   -Wno-return-type \
                   -Wno-int-conversion \
                   -MMD -MP \
                   -std=c89 -pedantic

# ---- Alcyon toolchain (real target) --------------------------------------
# Override on the command line, e.g.
#     make alcyon ALCYON_BIN=/opt/alcyon/bin
# The wrappers should invoke Alcyon's CP68 -> C068 -> AS68 -> LO68 chain.
ALCYON_BIN      ?= /opt/alcyon/bin
ALCYON_CC       := $(ALCYON_BIN)/cc
ALCYON_LD       := $(ALCYON_BIN)/lo68
ALCYON_CFLAGS   := -I$(INCDIR) -O
ALCYON_LDFLAGS  :=
LCP_PRG         := LCP.PRG

# ---- Default target: host syntax check -----------------------------------
.PHONY: all
all: host

.PHONY: host
host: $(HOSTOBJS)
	@echo "host build OK -- $(words $(HOSTOBJS)) object(s)"

$(HOSTOBJDIR)/%.o: %.c | $(HOSTOBJDIR)
	@echo "  CC  $<"
	@$(CC) $(HOST_CFLAGS) -c -o $@ $<

$(HOSTOBJDIR):
	@mkdir -p $@

# ---- Alcyon build (LCP.PRG) ----------------------------------------------
# Drives Alcyon C 4.14 headless via Hatari.  See tools/alcyon_build.sh
# for the design rationale (2-phase compile per file to survive c068
# crashes).  All .c/.h basenames are already 8.3-safe so nothing here
# needs aliasing.
#
# Uses EmuTOS bundled with Homebrew Hatari by default.  Override with:
#   make alcyon HATARI=/path/to/hatari TOS_IMG=/path/to/tos.img
.PHONY: alcyon
alcyon: alcyon-objs alcyon-link

# Compile every .c to build/alcyon/*.o (headless Hatari).
.PHONY: alcyon-objs
alcyon-objs:
	@tools/alcyon_build.sh

# Archive + doindex + aln -> build/alcyon/LCP.PRG.
.PHONY: alcyon-link
alcyon-link:
	@tools/alcyon_link.sh

# ---- CI: build + three regression checks -----------------------------
# Runs after every alcyon build to answer three questions before you
# eyeball a screenshot:
#   1. did the port boot?                            -> tools/run_hatari.sh
#   2. did it render identically to the last golden? -> tools/frame_hash.sh
#   3. does every C name link uniquely under         -> tools/check_truncation.py
#      Alcyon's 8-char external limit?
# Any failure fails the target.
# CI_VBLS: 15000 VBLs runs cleanly on the current tip -- the previous
# v_gtext crash at VBL 2361 was resolved incidentally by the initBRev
# fix (rev_tab was BSS-zero, sprite mirroring smeared garbage into
# BSS regions that overlapped TOS's font table pointers).  Bump higher
# if you want more coverage; a 30000-VBL run also runs clean.
CI_VBLS ?= 15000
.PHONY: ci
ci: alcyon
	@echo ""
	@echo "==== 1/3 truncation lint ===="
	@tools/check_truncation.py
	@echo ""
	@echo "==== 2/3 boot smoke test ($(CI_VBLS) VBLs) ===="
	@VBLS=$(CI_VBLS) tools/run_hatari.sh
	@echo ""
	@echo "==== 3/3 frame-hash regression ===="
	@tools/frame_hash.sh
	@echo ""
	@echo "ci: all checks passed"

# ---- Housekeeping --------------------------------------------------------
.PHONY: linktest
# The port's own entry point is lcp_main() on the host (parts/main.c
# renames it so the unit tests can supply their own main), so this
# still needs a tiny one of its own.
linktest: host
	@echo "  LINK  build/host/lcp_linktest"
	@printf 'int main(void){return 0;}\n' > build/host/_tinymain.c
	@$(CC) -o build/host/lcp_linktest build/host/_tinymain.c $(HOSTOBJS)
	@rm -f build/host/_tinymain.c
	@echo "  link OK -- $(words $(HOSTOBJS)) object(s) plus tiny main"

.PHONY: hyber_test
hyber_test: host
	@echo "  CC    tests/t_hyber.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/hyber_test tests/t_hyber.c $(HOSTOBJS)
	@echo "  RUN   build/host/hyber_test"
	@cd build/host && ./hyber_test

.PHONY: letter_test
letter_test: host
	@echo "  CC    tests/t_letter.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/letter_test tests/t_letter.c $(HOSTOBJS)
	@echo "  RUN   build/host/letter_test"
	@cd build/host && ./letter_test

.PHONY: parser_test
parser_test: host
	@echo "  CC    tests/t_parser.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/parser_test tests/t_parser.c $(HOSTOBJS)
	@echo "  RUN   build/host/parser_test"
	@cd build/host && ./parser_test

.PHONY: vdi_pb_test
vdi_pb_test: host
	@echo "  CC    tests/vdi_pb.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/vdi_pb_test tests/vdi_pb.c $(HOSTOBJS)
	@echo "  RUN   build/host/vdi_pb_test"
	@cd build/host && ./vdi_pb_test

.PHONY: assets_test
assets_test: host
	@echo "  CC    tests/t_assets.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/assets_test tests/t_assets.c $(HOSTOBJS)
	@echo "  RUN   build/host/assets_test"
	@cd build/host && ./assets_test

.PHONY: scn_test
scn_test: host
	@echo "  CC    tests/t_scn.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/scn_test tests/t_scn.c $(HOSTOBJS)
	@echo "  RUN   build/host/scn_test"
	@cd build/host && ./scn_test

.PHONY: sounds_test
sounds_test: host
	@echo "  CC    tests/t_sounds.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/sounds_test tests/t_sounds.c $(HOSTOBJS)
	@echo "  RUN   build/host/sounds_test"
	@cd build/host && ./sounds_test

.PHONY: sim_test
sim_test: host
	@echo "  CC    tests/sim_tick.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/sim_test tests/sim_tick.c $(HOSTOBJS)
	@echo "  RUN   build/host/sim_test"
	@cd build/host && ./sim_test

.PHONY: sprite_test
sprite_test: host
	@echo "  CC    tests/t_sprite.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/sprite_test tests/t_sprite.c $(HOSTOBJS)
	@echo "  RUN   build/host/sprite_test"
	@cd build/host && ./sprite_test

.PHONY: sprite_golden_test
sprite_golden_test: host
	@echo "  CC    tests/t_sprgld.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/sprite_golden_test tests/t_sprgld.c $(HOSTOBJS)
	@echo "  RUN   build/host/sprite_golden_test"
	@cd build/host && ./sprite_golden_test

.PHONY: test
test: linktest hyber_test letter_test parser_test vdi_pb_test \
      assets_test scn_test sounds_test sim_test sprite_test \
      sprite_golden_test
	@echo "  all tests OK"

.PHONY: clean
clean:
	@rm -rf build $(LCP_PRG)
	@echo "  cleaned"

.PHONY: sources
sources:
	@echo $(SOURCES)

# ---- Auto-generated header dependencies ----------------------------------
-include $(HOSTOBJS:.o=.d)
