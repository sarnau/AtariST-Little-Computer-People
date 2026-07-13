/*
 * games.c -- mini-game entry points + shared setup helpers.
 *
 * Fully-ported helpers:
 *   minigame_setup_screen -- 5-tick pause, top-strip clear, freeze the
 *                            text scroll pane, disable keyboard input.
 *   play_erase_rect       -- v_bar-based rectangular clear at (x1,y1)-(x2,y2)
 *                            with VDI init/exit brackets.
 *
 * Skeleton-ported game mains:
 *   Each of the 5 game mains has its *outer flow* ported for real:
 *     1. Allocate the game-specific data buffer via GEMDOS_Malloc
 *        (sizes verified from Ghidra: 10000 for anagram, 2000 for
 *        word puzzle, 10400 for poker/war, 0x28a0 = 10432 for
 *        blackjack).  On OOM, error_not_enough_memory (infinite alert
 *        loop on ST; exit(1) on host).
 *     2. Load the required data file with file_read_compressed
 *        ("words" for anagram, "wordpz.txt" for word puzzle).  Card
 *        games load their graphics via poker_load_card_graphics.
 *     3. Call minigame_setup_screen.
 *     4. Print the game title.
 *     5. Enter a key-poll loop that terminates on F10.
 *     6. Free the buffer via GEMDOS_Mfree, restore text_scroll_timer,
 *        clear disable_key_input_flag, return.
 *
 * The *inner* game logic (word scrambling, poker hand evaluation,
 * card-shuffle algorithms, ~50 subsystem helpers per game) is
 * intentionally deferred to per-game batches -- each of the 5 games
 * is a self-contained 100..600-line subsystem worth its own file.
 * With this skeleton, pressing F10 (or the outer game menu timing
 * out) cleanly returns to the house so the overall port continues
 * to link and run to completion.
 *
 * addr: minigame_setup_screen(), play_erase_rect(),
 *       anagram_main(), word_puzzle_main(), poker_main(),
 *       poker_war_main(), poker_blackjack_main()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern short    get_pressed_key();
extern void     string_print();
extern void     fill_top_rect_with_background();
extern void     file_read_compressed();
extern void     error_not_enough_memory();
extern void     init_vdi_and_screen();
extern void     exit_vdi_and_screen();
extern void     v_bar();
extern void     poker_load_card_graphics();

/* KEY_F10 already defined in enums.h as 0x144 in our compact encoding. */

/* ---- Real helpers ---------------------------------------------------- */

/* minigame_setup_screen: prep the top status strip for the game menu.
   Fills 0x4d = 77 rows (the full text pane) with the house background,
   then freezes the text-scroll pane (`text_scroll_timer = -1`) and
   disables keyboard input from the game-command dispatcher so keys
   don't leak into the parser while a mini-game is running.
   addr: minigame_setup_screen() */

void
minigame_setup_screen()
{
        game_tick_and_animate(5);
        fill_top_rect_with_background(0x4d);
        text_scroll_timer      = -1;
        disable_key_input_flag = YES;
}

/* play_erase_rect: clear a rectangle via VDI v_bar.  Bracketed by
   init_vdi_and_screen / exit_vdi_and_screen which are the mini-game-
   specific VDI setup helpers (deferred stubs for now).
   addr: play_erase_rect() */

void
play_erase_rect(x1, y1, x2, y2)
short   x1;
short   y1;
short   x2;
short   y2;
{
        short   rect[4];

        rect[0] = x1;
        rect[1] = y1;
        rect[2] = x2;
        rect[3] = y2;
        init_vdi_and_screen();
        v_bar(vdihandle, rect);
        exit_vdi_and_screen();
}

/* ---- Mini-game skeletons -------------------------------------------- */

/* Shared cleanup at exit from any game: restore text scroll, free the
   game's data buffer if allocated, re-enable keyboard input. */

static void
game_cleanup(buffer)
void *  buffer;
{
        text_scroll_timer      = 0;
        disable_key_input_flag = NO;
        if (buffer != (void *) 0)
                _gemdos(GEMDOS_Mfree, (long) buffer, 0L, 0L);
}

/* Poll-loop skeleton: tick + read key + return true if user quit. */

static short
game_poll_wait_for_quit()
{
        short   key;
        for (;;) {
                key = get_pressed_key();
                game_tick_and_animate(0);
                if (key == KEY_F10)
                        return 1;
                if (triggered_event_list[0] != ACTION_NONE)
                        return 1;
        }
}

/* anagram_main: outer flow verified; word-selection / scrambling /
   input-buffer inner loops are deferred.
   addr: anagram_main() */

void
anagram_main()
{
        anagram_words_buffer =
                (char *) _gemdos(GEMDOS_Malloc, 10000L, 0L, 0L);
        if (anagram_words_buffer == (char *) 0)
                error_not_enough_memory();
        file_read_compressed("words",
                             (unsigned char *) anagram_words_buffer,
                             10000);

        minigame_setup_screen();
        string_print("***ANAGRAMS***", 5, 8, COLOR_black);
        /* anagram_show_intro_text, anagram_select_and_scramble_word,
           the guess/clue loop -- deferred. */
        game_poll_wait_for_quit();
        game_cleanup(anagram_words_buffer);
        anagram_words_buffer = (char *) 0;
}

/* word_puzzle_main: outer flow verified; per-puzzle parse + fill-in-
   the-blank dispatch is deferred.  Loads 66-line wordpz.txt into
   letter_line_ptr entries 0..0x41 via the same line-indexing pattern
   as file_load_letter_template.
   addr: word_puzzle_main() */

void
word_puzzle_main()
{
        char *  parse_ptr;
        short   line_index;

        word_puzzle_data_buffer =
                (char *) _gemdos(GEMDOS_Malloc, 2000L, 0L, 0L);
        if (word_puzzle_data_buffer == (char *) 0)
                error_not_enough_memory();

        minigame_setup_screen();
        file_read_compressed("wordpz.txt",
                             (unsigned char *) word_puzzle_data_buffer,
                             1536);

        /* Index the 66 lines (33 puzzles * 2 lines each). */
        parse_ptr = word_puzzle_data_buffer;
        for (line_index = 0; line_index < 0x42;
             line_index = line_index + 1) {
                letter_line_ptr[line_index] = parse_ptr;
                do {
                        parse_ptr = parse_ptr + 1;
                } while ((unsigned char) *parse_ptr > 31);
                while ((unsigned char) *parse_ptr < ' ')
                        parse_ptr = parse_ptr + 1;
        }

        word_puzzle_current_index = 0;
        string_print("**WORD PUZZLE #  **", 8, 8, COLOR_black);
        /* The per-puzzle "choose then solve" loop -- deferred. */
        game_poll_wait_for_quit();
        game_cleanup(word_puzzle_data_buffer);
        word_puzzle_data_buffer = (char *) 0;
}

/* poker_main: outer flow verified; 5-card-draw round logic (ante, deal,
   draw, showdown, computer AI) is deferred.
   addr: poker_main() */

void
poker_main()
{
        cards_data = (short *) _gemdos(GEMDOS_Malloc, 10400L, 0L, 0L);
        if (cards_data == (short *) 0)
                error_not_enough_memory();
        poker_load_card_graphics();
        minigame_setup_screen();

        _poker_round_count    = 0;
        poker_quit_flag       = NO;
        poker_computer_money  = 400;
        poker_player_money    = 400;
        poker_pot_amount      = 0;

        string_print("***POKER***", 5, 8, COLOR_black);
        /* Round loop with ante/deal/bet/draw/showdown phases -- deferred. */
        game_poll_wait_for_quit();
        game_cleanup(cards_data);
        cards_data = (short *) 0;
}

/* poker_war_main: outer flow verified; card-shuffling is real (52-card
   deck initialized 0..51, then 400 random-swap iterations, then split
   into two 26-card piles).  The head-to-head reveal + score-tracking
   loop is deferred.
   addr: poker_war_main() */

void
poker_war_main()
{
        short   input_key;
        short   card_index;
        short   temp;
        short   i;
        short   j;
        short   k;

        cards_data = (short *) _gemdos(GEMDOS_Malloc, 10400L, 0L, 0L);
        if (cards_data == (short *) 0)
                error_not_enough_memory();
        poker_load_card_graphics();
        minigame_setup_screen();

        poker_computer_money = 26;
        poker_player_money   = 26;
        poker_pot_amount     = 0;

        /* Deck initialization: 52 cards indexed 0..51. */
        for (i = 0; i < 52; i = i + 1)
                poker_draw_discard_flags[i] = i;

        /* Fisher-Yates-lite shuffle: 400 random-pair swaps.  Sufficient
           over a 52-element array to fully randomise the deck. */
        j = 400;
        while (j != 0) {
                input_key = randomRange(0, 51);
                do {
                        card_index = randomRange(0, 51);
                } while (input_key == card_index);
                temp = poker_draw_discard_flags[card_index];
                poker_draw_discard_flags[card_index] =
                        poker_draw_discard_flags[input_key];
                poker_draw_discard_flags[input_key] = temp;
                j = j - 1;
        }

        /* Split the shuffled deck into two 26-card piles. */
        k = 0;
        for (i = 0; i < 52; i = i + 2) {
                poker_computer_draw_pile[k] = poker_draw_discard_flags[i];
                poker_player_draw_pile[k]   = poker_draw_discard_flags[i + 1];
                k = k + 1;
        }

        string_print("***WAR***", 5, 8, COLOR_black);
        /* Reveal/compare loop -- deferred. */
        game_poll_wait_for_quit();
        game_cleanup(cards_data);
        cards_data = (short *) 0;
}

/* poker_blackjack_main: outer flow verified; hit/stand/double logic
   and dealer AI are deferred.
   addr: poker_blackjack_main() */

void
poker_blackjack_main()
{
        cards_data = (short *) _gemdos(GEMDOS_Malloc, 0x28a0L, 0L, 0L);
        if (cards_data == (short *) 0)
                error_not_enough_memory();
        poker_load_card_graphics();
        minigame_setup_screen();

        poker_computer_money = 400;
        poker_player_money   = 400;

        string_print("***BLACKJACK***", 5, 8, COLOR_black);
        /* Round loop with bet/deal/hit/stand/dealer/settle -- deferred. */
        game_poll_wait_for_quit();
        game_cleanup(cards_data);
        cards_data = (short *) 0;
}
