/*
 * render.c -- VDI palette and screen refresh (stubs for now).
 *
 * screen_render_8hz() copies the pending sprite buffers onto the visible
 * screen at ~8 Hz.  lcp_update_palette_colors() reloads the 16-entry
 * palette from lcp_current_palette[] via Setpalette(); called after
 * sickness onset, sickness recovery, and TV toggle.
 *
 * All hardware calls (Setpalette, VDI vs_color) will be reintroduced
 * when the render pipeline is ported for real; today these are stubs so
 * everything upstream links.
 *
 * addr: screen_render_8hz(), lcp_update_palette_colors()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     soundeffect_select();
extern void     play_soundeffect_tv_click();
extern void     tv_draw_static_line();
extern void     lcp_idle_look_left();
extern void     screen_set_draw_to_backbuffer();
extern void     screen_set_draw_to_frontbuffer();
extern void     vsl_color();
extern void     v_pline();
extern void     screen_fill_row_white();

/* lcp_update_palette_colors -> render_extra.c */

/* screen_render_8hz -> render_frame.c */

/* clock_redraw_hands: erase the previous minute/hour hand pair (paint
   in white), then draw the new pair (paint in grey).  Compares
   time_minutes against a cached clock_minute to skip work when the
   clock hasn't advanced yet.
   addr: clock_redraw_hands() */

extern void     clock_draw_hands();

void
clock_redraw_hands()
{
        if (clock_minute == time_minutes)
                return;
        clock_draw_hands(clock_minute, clock_hour, COLOR_white);
        clock_minute = time_minutes;
        clock_hour   = time_hours;
        clock_draw_hands(time_minutes, time_hours, COLOR_grey);
}

/* object_draw: blit a pre-loaded background object at (x, y) via
   vdi_copy_rect (VRO copy S_ONLY = replace, no transparency).  Each
   object has its width/height stored in object_tab_width/height[] and
   its MFDB source rect in object_tab_mfdb offset by object_index.
   addr: object_draw() */

extern void     vdi_copy_rect();

void
object_draw(object_index, x, y)
short   object_index;
short   x;
short   y;
{
        vdi_copy_rect(
                vdihandle, S_ONLY,
                (MFDB *) ((char *) object_tab_mfdb + object_index),
                &MFDB_screen_ptr,
                0, 0,
                object_tab_width[object_index]  - 1,
                object_tab_height[object_index] - 1,
                x, y,
                x + object_tab_width[object_index]  - 1,
                y + object_tab_height[object_index] - 1);
}

/* fill_top_rect_with_background: clear the top text strip (rows 0
   through maxY-1).  Uses a solid white fill for the letter-typing
   pane (maxY < 70) and a striped house-background fill for the
   larger clear cases.  The last row before maxY is painted black to
   form a separator.
   addr: fill_top_rect_with_background() */

extern void     screen_fill_row_striped();
extern void     screen_fill_row_black();

void
fill_top_rect_with_background(max_y)
short   max_y;
{
        short   y;

        /* dest_screenbase_ptr is the 8-row-tall top strip within
           dest_scr_buffer -- offset 0x7f words (=254 bytes) forward
           to hit the row-0 baseline. */
        dest_screenbase_ptr = (void *) (dest_scr_buffer + 0x7f);

        for (y = 0; y < max_y - 1; y = y + 1) {
                if (max_y < 70)
                        screen_fill_row_white(dest_screenbase_ptr, y);
                else
                        screen_fill_row_striped(dest_screenbase_ptr, y);
        }
        screen_fill_row_black(dest_screenbase_ptr, max_y - 1);
}

/* screen_scroll_text_down, tv_draw_static_noise -> render_extra.c */

/* record_player_animate_needle -> render_extra.c */

/* ---- TV toggle ------------------------------------------------------- */

/* tv_turn_on: walk to the living room, do an idle look-left, set the
   flag and play the click SFX.  Returns -1 on walk failure, 0 otherwise.
   addr: tv_turn_on() */

short
tv_turn_on()
{
        short   result;

        if (lcp_tv_on != NO)
                return 0;

        house_get_position_xy(POS_TOP_LIVING_ROOM,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return -1;

        game_tick_and_animate(2);
        lcp_idle_look_left();
        lcp_tv_on = YES;
        play_soundeffect_tv_click();
        return 0;
}

/* tv_turn_off: same walk, clear the flag, redraw the antenna in the
   off (static-line) state.  Note: no SFX_TV_CLICK on off in the 1985
   binary -- preserved verbatim.
   addr: tv_turn_off() */

short
tv_turn_off()
{
        short   result;

        if (lcp_tv_on == NO)
                return 0;

        house_get_position_xy(POS_TOP_LIVING_ROOM,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return -1;

        game_tick_and_animate(2);
        lcp_idle_look_left();
        lcp_tv_on = NO;
        tv_draw_static_line(COLOR_white);
        return 0;
}

/* ---- Kitchen food-cabinet overlay ----------------------------------- */

/* screen_draw_food_cabinet: paint the food-count marker sprites in the
   4 slots of the open kitchen cabinet.  Count comes from bits 9..11 of
   door_states_and_flags (0..4 packs).
     1 item -> (50, 159)   3 items -> +(50, 151)
     2      -> +(58, 159)  4       -> +(58, 151)
   No-op when the cabinet is closed.
   addr: screen_draw_food_cabinet() */

void
screen_draw_food_cabinet()
{
        unsigned short  cabinet_content;

        if (lcp_cabinet_open == NO)
                return;

        cabinet_content = (lcp.door_states_and_flags >> 9) & 7;
        object_draw(object_id_cabinet_open_2, 46, 140);

        if (cabinet_content > 0) object_draw(object_id_blue_green, 50, 159);
        if (cabinet_content > 1) object_draw(object_id_blue_green, 58, 159);
        if (cabinet_content > 2) object_draw(object_id_blue_green, 50, 151);
        if (cabinet_content > 3) object_draw(object_id_blue_green, 58, 151);
}

/* ---- Water tank level bar (VDI polylines) ---------------------------- */

/* update_water_level_bar: repaint or animate the water tank indicator
   at x=146..159, y=165..174.
     val == 0 : full redraw at current lcp_water_level
     val <  0 : drain `-val` steps down, one game-tick each
     val >  0 : fill `val` steps up
   Each level is one 14px-wide horizontal polyline; drawn in colour 0x0D
   when filled and 0x0C (empty background) otherwise.  VDI operations
   go to the backbuffer so the animation isn't torn by the next 8Hz
   render.
   addr: update_water_level_bar() */

void
update_water_level_bar(val)
short   val;
{
        short   y;
        RECT16  rect;

        rect.x1 = 146;
        rect.y1 = 174;
        rect.x2 = 159;
        rect.y2 = 174;

        if (val == 0) {
                /* Draw filled portion (colour 0x0D). */
                y = lcp_water_level;
                screen_set_draw_to_backbuffer();
                while (y != 0) {
                        rect.y1 = 174 - (y - 1);
                        rect.y2 = rect.y1;
                        vsl_color(vdihandle, _vdi_color_table[0xd]);
                        v_pline(vdihandle, 2, &rect.x1);
                        y = y - 1;
                }
                screen_set_draw_to_frontbuffer();

                /* Draw empty portion (colour 0x0C). */
                y = lcp_water_level;
                screen_set_draw_to_backbuffer();
                while (y < 10) {
                        rect.y1 = 174 - y;
                        rect.y2 = rect.y1;
                        vsl_color(vdihandle, _vdi_color_table[0xc]);
                        v_pline(vdihandle, 2, &rect.x1);
                        y = y + 1;
                }
                screen_set_draw_to_frontbuffer();
                return;
        }

        if (val < 0) {
                /* Drain -val steps. */
                while (lcp_water_level != 0 && val != 0) {
                        rect.y1 = 174 - (lcp_water_level - 1);
                        rect.y2 = rect.y1;
                        screen_set_draw_to_backbuffer();
                        vsl_color(vdihandle, _vdi_color_table[0xc]);
                        v_pline(vdihandle, 2, &rect.x1);
                        screen_set_draw_to_frontbuffer();
                        game_tick_and_animate(4);
                        lcp_water_level = lcp_water_level - 1;
                        val = val + 1;
                }
                return;
        }

        /* Fill val steps (capped at 10). */
        while (val != 0 && lcp_water_level < 11 &&
               (lcp_water_level + 1) < 11) {
                rect.y1 = 174 - lcp_water_level;
                lcp_water_level = lcp_water_level + 1;
                rect.y2 = rect.y1;
                screen_set_draw_to_backbuffer();
                vsl_color(vdihandle, _vdi_color_table[0xd]);
                v_pline(vdihandle, 2, &rect.x1);
                screen_set_draw_to_frontbuffer();
                val = val - 1;
        }
}
