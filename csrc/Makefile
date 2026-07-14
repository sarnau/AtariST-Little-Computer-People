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

# ---- Source list ---------------------------------------------------------
SOURCES         := globals.c \
                   sprite_globals.c \
                   tables.c \
                   sim.c \
                   calendar.c \
                   random.c \
                   events.c \
                   health.c \
                   ai.c \
                   movement.c \
                   walk.c \
                   sprites.c \
                   sprites_head.c \
                   dog.c \
                   actions.c \
                   action_stubs.c \
                   action_simple.c \
                   actions_idle.c \
                   actions_house.c \
                   actions_food.c \
                   actions_bathroom.c \
                   actions_leisure.c \
                   actions_letter.c \
                   actions_doors.c \
                   letter_load.c \
                   render_extra.c \
                   gfx_prim.c \
                   clock.c \
                   keyboard.c \
                   render_frame.c \
                   parser.c \
                   actions_games.c \
                   sprite_render.c \
                   tv_animate.c \
                   games.c \
                   cards.c \
                   midi_seq.c \
                   psg_io.c \
                   sfx_irq.c \
                   vdi.c \
                   vocab_data.c \
                   psg_freq_data.c \
                   assets.c \
                   alerts.c \
                   deliveries.c \
                   ai_random.c \
                   save.c \
                   save_host.c \
                   render.c \
                   sound.c \
                   tick.c \
                   stubs.c \
                   main.c

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
CC              ?= cc
HOST_CFLAGS     := -I$(INCDIR) \
                   -DHOST \
                   -Wall -Wextra \
                   -Wno-deprecated-non-prototype \
                   -Wno-strict-prototypes \
                   -Wno-unused-parameter \
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
# crashes, 8.3 filename aliasing via tools/filename_map.txt).
#
# Uses EmuTOS bundled with Homebrew Hatari by default.  Override with:
#   make alcyon HATARI=/path/to/hatari TOS_IMG=/path/to/tos.img
.PHONY: alcyon
alcyon:
	@tools/alcyon_build.sh

# Direct .o-only target: run alcyon_build.sh without linking.  Useful
# for iterating during Alcyon fidelity work.
.PHONY: alcyon-objs
alcyon-objs: alcyon

# ---- Housekeeping --------------------------------------------------------
.PHONY: linktest
linktest: host
	@echo "  LINK  build/host/lcp_linktest"
	@printf 'extern void endless_game_loop();\nint main(void){return 0;}\n' > build/host/_tinymain.c
	@$(CC) -o build/host/lcp_linktest build/host/_tinymain.c $(HOSTOBJS)
	@rm -f build/host/_tinymain.c
	@echo "  link OK -- $(words $(HOSTOBJS)) object(s) plus tiny main"

.PHONY: hyber_test
hyber_test: host
	@echo "  CC    tests/hyber_roundtrip.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/hyber_test tests/hyber_roundtrip.c $(HOSTOBJS)
	@echo "  RUN   build/host/hyber_test"
	@cd build/host && ./hyber_test

.PHONY: letter_test
letter_test: host
	@echo "  CC    tests/letter_load.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/letter_test tests/letter_load.c $(HOSTOBJS)
	@echo "  RUN   build/host/letter_test"
	@cd build/host && ./letter_test

.PHONY: parser_test
parser_test: host
	@echo "  CC    tests/parser_smoke.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/parser_test tests/parser_smoke.c $(HOSTOBJS)
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
	@echo "  CC    tests/assets_load.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/assets_test tests/assets_load.c $(HOSTOBJS)
	@echo "  RUN   build/host/assets_test"
	@cd build/host && ./assets_test

.PHONY: scn_test
scn_test: host
	@echo "  CC    tests/scn_decode.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/scn_test tests/scn_decode.c $(HOSTOBJS)
	@echo "  RUN   build/host/scn_test"
	@cd build/host && ./scn_test

.PHONY: sounds_test
sounds_test: host
	@echo "  CC    tests/sounds_load.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/sounds_test tests/sounds_load.c $(HOSTOBJS)
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
	@echo "  CC    tests/sprite_compose.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/sprite_test tests/sprite_compose.c $(HOSTOBJS)
	@echo "  RUN   build/host/sprite_test"
	@cd build/host && ./sprite_test

.PHONY: sprite_golden_test
sprite_golden_test: host
	@echo "  CC    tests/sprite_golden.c"
	@$(CC) $(HOST_CFLAGS) -o build/host/sprite_golden_test tests/sprite_golden.c $(HOSTOBJS)
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
