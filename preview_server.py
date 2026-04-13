"""
Web preview server for Little Computer People (Atari ST).
Loads game assets, runs the simulation, and serves an HTML5 Canvas visualization.
Uses Server-Sent Events (SSE) to push frame updates to the browser.
"""

import io
import json
import base64
import time
import threading
from http.server import HTTPServer, SimpleHTTPRequestHandler
from pathlib import Path

from PIL import Image

# ---------------------------------------------------------------------------
# Game state initialization
# ---------------------------------------------------------------------------
from lcp.state import GameState
from lcp.assets import load_all_assets
from lcp.enums import (
    PLAYER_STATE, FACING_DIR, DOG_BOWL_STATUS, ACTION_ID,
    HOUSE_POS, SPRITE_ID,
)
from lcp.constants import house_get_position_xy, FLOOR_BASELINE_Y

DATA_DIR = Path(__file__).parent / 'DATA'
PORT = 8111

# Friendly display names for PLAYER_STATE (shown in HUD)
PLAYER_STATE_DISPLAY = {
    PLAYER_STATE.STATE_WALK_FRAME_0: 'Walk 0',
    PLAYER_STATE.STATE_WALK_FRAME_1: 'Walk 1',
    PLAYER_STATE.STATE_WALK_FRAME_2: 'Walk 2',
    PLAYER_STATE.STATE_WALK_FRAME_3: 'Walk 3',
    PLAYER_STATE.STATE_WALK_FRAME_4: 'Walk 4',
    PLAYER_STATE.STATE_WALK_FRAME_5: 'Walk 5',
    PLAYER_STATE.STATE_WALK_FRAME_6: 'Walk 6',
    PLAYER_STATE.STATE_WALK_FRAME_7: 'Walk 7',
    PLAYER_STATE.STATE_STAND_IDLE: 'Stand Idle',
    PLAYER_STATE.STATE_STAIR_UP_0: 'Stair Up 0',
    PLAYER_STATE.STATE_STAIR_UP_1: 'Stair Up 1',
    PLAYER_STATE.STATE_STAIR_UP_2: 'Stair Up 2',
    PLAYER_STATE.STATE_STAIR_UP_3: 'Stair Up 3',
    PLAYER_STATE.STATE_STAIR_TOP_0: 'Stair Top 0',
    PLAYER_STATE.STATE_STAIR_TOP_1: 'Stair Top 1',
    PLAYER_STATE.STATE_STAIR_TOP_2: 'Stair Top 2',
    PLAYER_STATE.STATE_STAIR_TOP_3: 'Stair Top 3',
    PLAYER_STATE.STATE_STAIR_DOWN_0: 'Stair Down 0',
    PLAYER_STATE.STATE_STAIR_DOWN_1: 'Stair Down 1',
    PLAYER_STATE.STATE_STAIR_DOWN_2: 'Stair Down 2',
    PLAYER_STATE.STATE_STAIR_DOWN_3: 'Stair Down 3',
    PLAYER_STATE.STATE_STAIR_BTM_0: 'Stair Btm 0',
    PLAYER_STATE.STATE_STAIR_BTM_1: 'Stair Btm 1',
    PLAYER_STATE.STATE_STAIR_BTM_2: 'Stair Btm 2',
    PLAYER_STATE.STATE_STAIR_BTM_3: 'Stair Btm 3',
    PLAYER_STATE.STATE_STAND_FACING_SCREEN: 'Facing Screen',
    PLAYER_STATE.STATE_STAND_SIDE_VIEW: 'Side View',
    PLAYER_STATE.STATE_SIT_CHAIR: 'Sit Chair',
    PLAYER_STATE.STATE_SIT_COUCH: 'Sit Couch',
    PLAYER_STATE.STATE_SIT_DESK: 'Sit Desk',
    PLAYER_STATE.STATE_TYPE_LEFT: 'Type Left',
    PLAYER_STATE.STATE_TYPE_RIGHT: 'Type Right',
    PLAYER_STATE.STATE_EAT_BITE: 'Eat',
    PLAYER_STATE.STATE_DRINK_GLASS: 'Drink',
    PLAYER_STATE.STATE_EXERCISE_ARMS_UP: 'Exercise Up',
    PLAYER_STATE.STATE_EXERCISE_CROUCH: 'Exercise Crouch',
    PLAYER_STATE.STATE_SLEEP_IN_BED: 'Sleep In Bed',
    PLAYER_STATE.STATE_SLEEP_LYING: 'Sleep Lying',
    PLAYER_STATE.STATE_SHOWER_1: 'Shower 1',
    PLAYER_STATE.STATE_SHOWER_2: 'Shower 2',
    PLAYER_STATE.STATE_SHOWER_3: 'Shower 3',
    PLAYER_STATE.STATE_SHOWER_4: 'Shower 4',
    PLAYER_STATE.STATE_SHOWER_5: 'Shower 5',
    PLAYER_STATE.STATE_BRUSH_TEETH: 'Brush Teeth',
    PLAYER_STATE.STATE_WASH_HANDS: 'Wash Hands',
    PLAYER_STATE.STATE_USE_TOILET: 'Use Toilet',
    PLAYER_STATE.STATE_PLAY_PIANO_1: 'Piano 1',
    PLAYER_STATE.STATE_PLAY_PIANO_2: 'Piano 2',
    PLAYER_STATE.STATE_DANCE_LEFT: 'Dance Left',
    PLAYER_STATE.STATE_DANCE_RIGHT: 'Dance Right',
    PLAYER_STATE.STATE_READ_NEWSPAPER: 'Read Paper',
    PLAYER_STATE.STATE_WRITE_LETTER: 'Write Letter',
    PLAYER_STATE.STATE_SIT_EXERCISE_1: 'Sit Exercise 1',
    PLAYER_STATE.STATE_SIT_EXERCISE_2: 'Sit Exercise 2',
    PLAYER_STATE.STATE_YAWN: 'Yawn',
    PLAYER_STATE.STATE_STRETCH: 'Stretch',
    PLAYER_STATE.STATE_WANDER_LOOK: 'Wander Look',
    PLAYER_STATE.STATE_PLAY_COMPUTER: 'Play Computer',
    PLAYER_STATE.STATE_PEEK_AROUND: 'Peek Around',
    PLAYER_STATE.STATE_NOD_HEAD: 'Nod Head',
    PLAYER_STATE.STATE_HELLO: 'Hello',
    PLAYER_STATE.STATE_GET_IN_BED: 'Get In Bed',
    PLAYER_STATE.STATE_GET_OUT_BED: 'Get Out Bed',
    PLAYER_STATE.STATE_OPEN_CLOSET: 'Open Closet',
    PLAYER_STATE.STATE_CLOSE_CLOSET: 'Close Closet',
    PLAYER_STATE.STATE_CARRY_OBJECT: 'Carry Object',
    PLAYER_STATE.STATE_PUT_DOWN_OBJECT: 'Put Down',
    PLAYER_STATE.STATE_PICK_UP_OBJECT: 'Pick Up',
    PLAYER_STATE.STATE_FEED_DOG: 'Feed Dog',
    PLAYER_STATE.STATE_PET_DOG_1: 'Pet Dog 1',
    PLAYER_STATE.STATE_PET_DOG_2: 'Pet Dog 2',
    PLAYER_STATE.STATE_SIT_ON_COUCH_DOG: 'Sit w/ Dog',
    PLAYER_STATE.STATE_LIGHT_FIRE_1: 'Light Fire 1',
    PLAYER_STATE.STATE_LIGHT_FIRE_2: 'Light Fire 2',
    PLAYER_STATE.STATE_PHONE_ANSWER: 'Phone Answer',
    PLAYER_STATE.STATE_PHONE_TALK: 'Phone Talk',
    PLAYER_STATE.STATE_PHONE_HANG_UP: 'Phone Hang Up',
    PLAYER_STATE.STATE_PLAY_RECORD_1: 'Play Record 1',
    PLAYER_STATE.STATE_PLAY_RECORD_2: 'Play Record 2',
    PLAYER_STATE.STATE_WATCH_TV: 'Watch TV',
    PLAYER_STATE.STATE_PLAY_GAME_SIT: 'Play Game',
    PLAYER_STATE.STATE_PACE_1: 'Pace 1',
    PLAYER_STATE.STATE_PACE_2: 'Pace 2',
    PLAYER_STATE.STATE_WAKE_FROM_ALARM: 'Wake Alarm',
    PLAYER_STATE.STATE_STRETCH_WAKE: 'Stretch Wake',
    PLAYER_STATE.STATE_SNIFF: 'Sniff',
    PLAYER_STATE.STATE_DRESSED_STAND: 'Dressed',
    PLAYER_STATE.STATE_TIDY_1: 'Tidy 1',
    PLAYER_STATE.STATE_TIDY_2: 'Tidy 2',
    PLAYER_STATE.STATE_CLEAN_1: 'Clean 1',
    PLAYER_STATE.STATE_CLEAN_2: 'Clean 2',
}

# Friendly display names for HOUSE_POS (shown in HUD)
# Format: "floor: label" where floor is 3=top, 2=mid, 1=btm
HOUSE_POS_DISPLAY = {
    HOUSE_POS.POS_TOP_0: '3: Left Edge',
    HOUSE_POS.POS_TOP_ARMCHAIR: '3: Armchair',
    HOUSE_POS.POS_TOP_GAME_TABLE: '3: Game Table',
    HOUSE_POS.POS_TOP_DANCE_FLOOR: '3: Dance Floor',
    HOUSE_POS.POS_TOP_FIREPLACE: '3: Fireplace',
    HOUSE_POS.POS_TOP_LOG_AREA: '3: Log Area',
    HOUSE_POS.POS_TOP_6: '3: Hallway',
    HOUSE_POS.POS_TOP_STUDY_DOOR: '3: Study Door',
    HOUSE_POS.POS_TOP_8: '3: Study 8',
    HOUSE_POS.POS_TOP_FILING_CAB: '3: Filing Cabinet',
    HOUSE_POS.POS_TOP_DESK_LAMP: '3: Desk Lamp',
    HOUSE_POS.POS_TOP_11: '3: Study 11',
    HOUSE_POS.POS_TOP_RECORD_SHELF: '3: Record Shelf',
    HOUSE_POS.POS_TOP_13: '3: Living 13',
    HOUSE_POS.POS_TOP_14: '3: Study 14',
    HOUSE_POS.POS_TOP_15: '3: Right Edge',
    HOUSE_POS.POS_MID_0: '2: Left Edge',
    HOUSE_POS.POS_MID_BED: '2: Bed',
    HOUSE_POS.POS_MID_DRESSER: '2: Dresser',
    HOUSE_POS.POS_MID_CLOSET: '2: Closet',
    HOUSE_POS.POS_MID_COUCH: '2: Couch',
    HOUSE_POS.POS_MID_SINK: '2: Bathroom Sink',
    HOUSE_POS.POS_MID_TOILET: '2: Toilet',
    HOUSE_POS.POS_MID_SHOWER: '2: Shower Door',
    HOUSE_POS.POS_MID_24: '2: Pos 24',
    HOUSE_POS.POS_MID_PIANO: '2: Piano',
    HOUSE_POS.POS_MID_SHOWER_INSIDE: '2: Shower Inside',
    HOUSE_POS.POS_MID_27: '2: Pos 27',
    HOUSE_POS.POS_MID_28: '2: Pos 28',
    HOUSE_POS.POS_MID_COMPUTER: '2: Computer',
    HOUSE_POS.POS_MID_30: '2: Right Edge',
    HOUSE_POS.POS_MID_31: '2: Stairwell',
    HOUSE_POS.POS_BTM_0: '1: Left Edge',
    HOUSE_POS.POS_BTM_SINK: '1: Kitchen Sink',
    HOUSE_POS.POS_BTM_STOVE: '1: Stove',
    HOUSE_POS.POS_BTM_FRIDGE: '1: Fridge',
    HOUSE_POS.POS_BTM_CABINET: '1: Cabinet',
    HOUSE_POS.POS_BTM_TABLE: '1: Table',
    HOUSE_POS.POS_BTM_DOG_BOWL: '1: Dog Bowl',
    HOUSE_POS.POS_BTM_39: '1: Pos 39',
    HOUSE_POS.POS_BTM_40: '1: Fireplace',
    HOUSE_POS.POS_BTM_41: '1: Pos 41',
    HOUSE_POS.POS_BTM_42: '1: Pos 42',
    HOUSE_POS.POS_BTM_43: '1: Pos 43',
    HOUSE_POS.POS_BTM_44: '1: Pos 44',
    HOUSE_POS.POS_BTM_45: '1: Pos 45',
    HOUSE_POS.POS_BTM_FRONT_DOOR: '1: Front Door',
    HOUSE_POS.POS_BTM_SCREEN_EDGE: '1: Right Edge',
}

# Global game state
gs = GameState()
gs.copyprot_check_return = 1
_game_thread = None
_running = False


def _init_game():
    """Load assets and set up initial state."""
    load_all_assets(gs, DATA_DIR)
    # If no save file, create a random LCP
    if not gs.lcp_loaded:
        from lcp.structs import LCP
        gs.lcp = LCP.create_random()
        gs.lcp_loaded = 1
    # Place LCP at study door
    x, y = house_get_position_xy(HOUSE_POS.POS_TOP_STUDY_DOOR)
    gs.lcp_x = x - 10
    gs.lcp_y = y - 3
    gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
    gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
    # Place dog at bottom floor
    gs.dog_x = 8
    gs.dog_y = 190
    gs.game_speed_counter = 5
    gs.init_clock_from_system()


def _game_loop():
    """Run the game simulation in background.

    Timing: screen_render_8hz_headless() sleeps to maintain ~8 Hz when
    gs._realtime is True, matching the original Atari ST hardware timer.
    1 game-second = 8 render frames × 125ms = 1 real second.
    """
    global _running
    from lcp.simulation import game_simulate_one_second
    from lcp.ai import check_for_any_action_triggers
    from lcp.render import screen_render_8hz_headless
    from lcp.sprites import sprite_update_body, sprite_lcp_head_animate, sprite_lcp_head_update

    gs._realtime = True  # Enable 8 Hz rate limiting in headless renderer
    _running = True
    frame = 0
    while _running:
        try:
            screen_render_8hz_headless(gs)
            gs.sub_animation_frame_counter += 1
            game_simulate_one_second(gs)
            sprite_update_body(gs)
            sprite_lcp_head_animate(gs)
            sprite_lcp_head_update(gs)
            if frame % 8 == 0:
                check_for_any_action_triggers(gs)
            frame += 1
        except Exception as e:
            print(f"Game loop error: {e}")


# ---------------------------------------------------------------------------
# Render game state to an image
# ---------------------------------------------------------------------------

# Atari ST 3-bit RGB palette (0-7 per channel -> 0-255)
def _st_color(val: int) -> tuple[int, int, int]:
    """Convert Atari ST 0x0RGB color to (r, g, b) tuple."""
    r = ((val >> 8) & 0x7) * 36
    g = ((val >> 4) & 0x7) * 36
    b = (val & 0x7) * 36
    return (r, g, b)


def render_frame() -> Image.Image:
    """Render current game state to a PIL Image."""
    # Start with house background or blank
    if hasattr(gs, '_house_background') and gs._house_background is not None:
        img = gs._house_background.copy()
    else:
        img = Image.new('RGB', (320, 200), (40, 60, 40))

    # Draw LCP position indicator
    from PIL import ImageDraw
    draw = ImageDraw.Draw(img)

    lcp_x, lcp_y = gs.lcp_x, gs.lcp_y

    # Draw LCP body sprite (slot 3) using computed frame index
    from lcp.constants import BODY_SPRITE_FRAME_TABLE, CARRY_BODY_FRAME_TABLE, BODY_Y_OFFSET_PER_STATE
    from lcp.constants import HEAD_X_OFFSET_PER_STATE, HEAD_HEIGHT_PER_STATE, HAPPINESS_HEAD_FRAME_OFFSET

    lcp_drawn = False
    body_frames = getattr(gs, '_body_frames', None)
    if body_frames:
        state = gs.lcp_state
        if state < 0 or state >= len(BODY_SPRITE_FRAME_TABLE):
            state = 0
        frame_idx = BODY_SPRITE_FRAME_TABLE[state]
        if gs.lcp_carrying_object_flag and state < len(CARRY_BODY_FRAME_TABLE):
            frame_idx = CARRY_BODY_FRAME_TABLE[state]

        if 0 <= frame_idx < len(body_frames):
            body = body_frames[frame_idx]
            # Position: X = lcp_x - 4 (right) or -14 (left); Y = lcp_y + offset - 21
            # Sprites are 32px wide (matching original Atari ST buffers).
            if gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT:
                bx = lcp_x - 4
            else:
                bx = lcp_x - 14
                body = body.transpose(Image.FLIP_LEFT_RIGHT)
            y_off = BODY_Y_OFFSET_PER_STATE[state] if state < len(BODY_Y_OFFSET_PER_STATE) else 0
            by = lcp_y + y_off - 21
            try:
                if body.mode == 'RGBA':
                    img.paste(body, (int(bx), int(by)), body)
                else:
                    img.paste(body, (int(bx), int(by)))
                lcp_drawn = True
            except Exception:
                pass

    # Draw LCP head sprite (slot 4)
    head_frames_dict = getattr(gs, '_head_frames', {})
    char_variant = gs.lcp.character_sprite_id
    head_frames = head_frames_dict.get(char_variant, None)
    if head_frames and lcp_drawn:
        happiness = gs.lcp.happiness
        if happiness < 0 or happiness >= len(HAPPINESS_HEAD_FRAME_OFFSET):
            happiness = 0
        head_idx = HAPPINESS_HEAD_FRAME_OFFSET[happiness] + (gs.head_sprite_frame & 0x7F)
        if 0 <= head_idx < len(head_frames):
            head = head_frames[head_idx]
            # Position head relative to body
            state = gs.lcp_state
            if state < 0 or state >= len(HEAD_X_OFFSET_PER_STATE):
                state = 0
            x_off = HEAD_X_OFFSET_PER_STATE[state]
            y_off = BODY_Y_OFFSET_PER_STATE[state] if state < len(BODY_Y_OFFSET_PER_STATE) else 0
            h_height = HEAD_HEIGHT_PER_STATE[state] if state < len(HEAD_HEIGHT_PER_STATE) else 21
            mirror = getattr(gs, 'head_sprite_mirror_flag', 0)
            if mirror:
                hx = lcp_x + x_off - 14
                head = head.transpose(Image.FLIP_LEFT_RIGHT)
            else:
                hx = lcp_x + x_off - 4
            hy = lcp_y + y_off - (h_height + 21)
            # Carry on stairs adjustment
            if gs.lcp_carrying_object_flag and 12 < state < 17:
                hy += 1
            try:
                if head.mode == 'RGBA':
                    img.paste(head, (int(hx), int(hy)), head)
                else:
                    img.paste(head, (int(hx), int(hy)))
            except Exception:
                pass

    if not lcp_drawn:
        # Fallback: draw a colored rectangle for the LCP
        color = _st_color(0x0060)  # green-ish
        draw.rectangle(
            [lcp_x - 4, lcp_y - 20, lcp_x + 4, lcp_y],
            fill=color, outline=(255, 255, 255)
        )
        draw.ellipse(
            [lcp_x - 3, lcp_y - 26, lcp_x + 3, lcp_y - 20],
            fill=(220, 180, 140), outline=(180, 140, 100)
        )

    # Draw dog — always draw a marker + sprite if available
    dog_x, dog_y = gs.dog_x, gs.dog_y
    dog_sid = gs.dog_sprite_id
    dog_flip = getattr(gs, '_dog_flip', 0)
    sprite_images = getattr(gs, '_sprite_images', {})

    # Always draw dog marker dot
    if dog_x > 0 or dog_y > 0:
        if dog_sid in sprite_images:
            dog_img = sprite_images[dog_sid].copy()
            if dog_flip:
                dog_img = dog_img.transpose(Image.FLIP_LEFT_RIGHT)
            dx = dog_x - dog_img.width // 2
            dy = dog_y - 17
            try:
                img.paste(dog_img, (int(dx), int(dy)), dog_img)
            except Exception as e:
                # Draw red X on error
                draw.line([dog_x-5, dog_y-5, dog_x+5, dog_y+5], fill=(255,0,0), width=2)
                draw.line([dog_x-5, dog_y+5, dog_x+5, dog_y-5], fill=(255,0,0), width=2)
        else:
            # Sprite not found — draw yellow dot
            draw.ellipse(
                [dog_x - 4, dog_y - 4, dog_x + 4, dog_y + 4],
                fill=(255, 255, 0), outline=(255, 0, 0)
            )

    # Draw fire / fireplace object
    # addr: object_draw(_object_fire_animation[...], 257, 170)
    # _object_fire_animation = [32, 33, 34, 35], object_id_fire_off = 31
    obj_imgs = getattr(gs, '_object_images', {})
    if gs.fire_active_flag:
        fire_obj_ids = [32, 33, 34, 35]
        fire_obj_id = fire_obj_ids[gs.fire_animation_frame % 4]
        fire_img = obj_imgs.get(fire_obj_id)
        if fire_img:
            img.paste(fire_img, (257, 170))
        else:
            fire_colors = [(255, 100, 0), (255, 180, 0), (255, 60, 0), (255, 200, 50)]
            draw.rectangle([257, 170, 280, 186], fill=fire_colors[gs.fire_animation_frame % 4])
    else:
        fire_off_img = obj_imgs.get(31)
        if fire_off_img:
            img.paste(fire_off_img, (257, 170))

    # Draw clock pendulum animation
    # addr: object_draw(_object_clock_animation[...], 271, 92)
    # _object_clock_animation = [13, 14, 13, 15]
    clock_obj_ids = [13, 14, 13, 15]
    clock_frame = (gs.sub_animation_frame_counter >> 2) & 3
    clock_img = obj_imgs.get(clock_obj_ids[clock_frame])
    if clock_img:
        img.paste(clock_img, (271, 92))

    # Draw dog bowl object
    # addr: object_draw(_object_dog_eating_animation[...], 8, 190)
    # _object_dog_eating_animation = [51, 50, 49] (full=0→51, half=1→50, empty=2→49)
    dog_bowl_obj_ids = [51, 50, 49]
    bowl_idx = min(gs.dog_bowl_status, 2)
    bowl_img = obj_imgs.get(dog_bowl_obj_ids[bowl_idx])
    if bowl_img:
        img.paste(bowl_img, (8, 190))

    # Draw alarm animation
    # addr: object_draw(_object_alarm_animation[...], 53, 102)
    # _object_alarm_animation = [3, 4]
    if gs.ctrl_a_alarm_pressed_flag:
        alarm_frame = gs.sub_animation_frame_counter & 1
        alarm_img = obj_imgs.get(3 + alarm_frame)
        if alarm_img:
            img.paste(alarm_img, (53, 102))

    return img


def _nearest_house_pos(tx: int, ty: int) -> str:
    """Find the HOUSE_POS name closest to the given coordinates."""
    if tx == 0 and ty == 0:
        return ''
    best_dist = 999999
    best_member = None
    for member in HOUSE_POS:
        x, y = house_get_position_xy(member.value)
        d = abs(x - tx) + abs(y - ty)
        if d < best_dist:
            best_dist = d
            best_member = member
    if best_dist < 20 and best_member is not None:
        return HOUSE_POS_DISPLAY.get(best_member, best_member.name)
    return f'({tx},{ty})'


def get_state_json() -> dict:
    """Return game state as JSON-serializable dict."""
    try:
        ps = PLAYER_STATE(gs.lcp_state)
        lcp_state_name = PLAYER_STATE_DISPLAY.get(ps, ps.name)
    except ValueError:
        lcp_state_name = str(gs.lcp_state)
    try:
        dog_sprite_name = SPRITE_ID(gs.dog_sprite_id).name
    except ValueError:
        dog_sprite_name = str(gs.dog_sprite_id)
    return {
        'lcp': {
            'x': gs.lcp_x,
            'y': gs.lcp_y,
            'state': gs.lcp_state,
            'state_name': lcp_state_name,
            'facing': gs.lcp_facing_direction,
            'name': gs.lcp.name_str,
            'sleeping': gs.lcp.is_sleeping,
            'happiness': int(gs.lcp.happiness),
            'hunger': int(gs.lcp.hunger_level),
            'thirst': int(gs.lcp.thirst_level),
            'sickness': int(gs.lcp.sickness_level),
            'target_x': gs.walk_target_x,
            'target_y': gs.walk_target_y,
            'target_name': _nearest_house_pos(gs.walk_target_x, gs.walk_target_y),
            'waypoint_x': gs.walk_waypoint_x,
            'waypoint_y': gs.walk_waypoint_y,
            'on_stairs': gs.lcp_on_stairs_flag,
        },
        'dog': {
            'x': gs.dog_x,
            'y': gs.dog_y,
            'sprite_name': dog_sprite_name,
            'eating': gs.dog_eating_active,
            'idle_countdown': gs.dog_idle_countdown,
            'bowl_status': gs.dog_bowl_status,
            'target_x': gs.dog_target_x,
            'target_y': gs.dog_target_y,
            'target_name': _nearest_house_pos(gs.dog_target_x, gs.dog_target_y),
            'waypoint_x': gs.dog_waypoint_x,
            'waypoint_y': gs.dog_waypoint_y,
            'on_stairs': gs.dog_on_stairs_flag,
        },
        'world': {
            'date_day': gs.date_day,
            'date_month': gs.date_month,
            'date_year': gs.date_year,
            'game_hour': gs.time_hours,
            'game_minute': gs.time_minutes,
            'fire_active': gs.fire_active_flag,
            'tv_on': getattr(gs, 'tv_on', 0),
            'action': gs.last_action,
            'tick': gs.animation_tick_counter,
        },
    }


# ---------------------------------------------------------------------------
# HTTP Handler
# ---------------------------------------------------------------------------

HTML_PAGE = r"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Little Computer People - Preview</title>
<style>
  * { margin: 0; padding: 0; box-sizing: border-box; }
  body {
    background: #1a1a2e;
    color: #e0e0e0;
    font-family: 'Courier New', monospace;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 16px;
  }
  h1 {
    font-size: 18px;
    color: #7ec8e3;
    margin-bottom: 8px;
  }
  #game-container {
    position: relative;
    border: 2px solid #444;
    border-radius: 4px;
    overflow: hidden;
    image-rendering: pixelated;
  }
  #game-canvas {
    display: block;
    image-rendering: pixelated;
    width: 640px;
    height: 400px;
  }
  #hud {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr;
    gap: 12px;
    margin-top: 12px;
    width: 640px;
    font-size: 12px;
  }
  .hud-panel {
    background: #16213e;
    border: 1px solid #333;
    border-radius: 4px;
    padding: 8px;
  }
  .hud-panel h2 {
    font-size: 13px;
    color: #7ec8e3;
    margin-bottom: 4px;
    border-bottom: 1px solid #333;
    padding-bottom: 2px;
  }
  .hud-row { display: flex; justify-content: space-between; margin: 2px 0; }
  .hud-label { color: #888; }
  .hud-value { color: #e0e0e0; font-weight: bold; }
  #controls {
    margin-top: 10px;
    width: 640px;
    display: flex;
    gap: 8px;
    flex-wrap: wrap;
  }
  button {
    background: #16213e;
    color: #7ec8e3;
    border: 1px solid #555;
    border-radius: 4px;
    padding: 6px 14px;
    cursor: pointer;
    font-family: inherit;
    font-size: 12px;
  }
  button:hover { background: #1a3a5c; border-color: #7ec8e3; }
  button.active { background: #2a5a8c; border-color: #7ec8e3; }
  #status { color: #888; font-size: 11px; margin-top: 6px; }
</style>
</head>
<body>
<h1>Little Computer People (Atari ST) - Live Preview</h1>
<div id="game-container">
  <canvas id="game-canvas" width="320" height="200"></canvas>
</div>
<div id="hud">
  <div class="hud-panel">
    <h2>Character</h2>
    <div class="hud-row"><span class="hud-label">Name:</span><span class="hud-value" id="hud-name">--</span></div>
    <div class="hud-row"><span class="hud-label">Position:</span><span class="hud-value" id="hud-pos">--</span></div>
    <div class="hud-row"><span class="hud-label">State:</span><span class="hud-value" id="hud-state">--</span></div>
    <div class="hud-row"><span class="hud-label">Target:</span><span class="hud-value" id="hud-target">--</span></div>
    <div class="hud-row"><span class="hud-label">Happiness:</span><span class="hud-value" id="hud-happy">--</span></div>
  </div>
  <div class="hud-panel">
    <h2>Needs</h2>
    <div class="hud-row"><span class="hud-label">Hunger:</span><span class="hud-value" id="hud-hunger">--</span></div>
    <div class="hud-row"><span class="hud-label">Thirst:</span><span class="hud-value" id="hud-thirst">--</span></div>
    <div class="hud-row"><span class="hud-label">Sickness:</span><span class="hud-value" id="hud-sick">--</span></div>
    <div class="hud-row"><span class="hud-label">Sleeping:</span><span class="hud-value" id="hud-sleep">--</span></div>
  </div>
  <div class="hud-panel">
    <h2>World</h2>
    <div class="hud-row"><span class="hud-label">Date:</span><span class="hud-value" id="hud-date">--</span></div>
    <div class="hud-row"><span class="hud-label">Time:</span><span class="hud-value" id="hud-time">--</span></div>
    <div class="hud-row"><span class="hud-label">Dog:</span><span class="hud-value" id="hud-dog">--</span></div>
    <div class="hud-row"><span class="hud-label">Dog Target:</span><span class="hud-value" id="hud-dog-target">--</span></div>
    <div class="hud-row"><span class="hud-label">Tick:</span><span class="hud-value" id="hud-tick">--</span></div>
  </div>
</div>
<div id="controls">
  <button onclick="sendCmd('water')">Water (Ctrl+W)</button>
  <button onclick="sendCmd('food')">Food (Ctrl+F)</button>
  <button onclick="sendCmd('dog_food')">Dog Food (Ctrl+D)</button>
  <button onclick="sendCmd('pet')">Pet (Ctrl+P)</button>
  <button onclick="sendCmd('alarm')">Alarm (Ctrl+A)</button>
  <button onclick="sendCmd('call')">Call (Ctrl+C)</button>
  <button onclick="sendCmd('record')">Record (Ctrl+R)</button>
  <button onclick="sendCmd('book')">Book (Ctrl+B)</button>
</div>
<div id="status">Connecting...</div>

<script>
const canvas = document.getElementById('game-canvas');
const ctx = canvas.getContext('2d');
ctx.imageSmoothingEnabled = false;

const NEED_NAMES = ['Satisfied', 'Moderate', 'High', 'Critical'];
const HAPPY_NAMES = ['Happy', 'Content', 'Sad'];
const BOWL_NAMES = ['Empty', 'Half', 'Full'];

let frameImg = new window.Image();
frameImg.onload = () => {
  ctx.drawImage(frameImg, 0, 0, 320, 200);
};

function updateHud(state) {
  const l = state.lcp;
  const w = state.world;
  const d = state.dog;
  document.getElementById('hud-name').textContent = l.name || '--';
  document.getElementById('hud-pos').textContent = `${l.x}, ${l.y}`;
  document.getElementById('hud-state').textContent = l.state_name;
  if (l.target_x || l.target_y) {
    let t = l.target_name || `(${l.target_x},${l.target_y})`;
    if (l.on_stairs) t += ' [stairs]';
    document.getElementById('hud-target').textContent = t;
  } else {
    document.getElementById('hud-target').textContent = 'idle';
  }
  document.getElementById('hud-happy').textContent = HAPPY_NAMES[l.happiness] || l.happiness;
  document.getElementById('hud-hunger').textContent = NEED_NAMES[l.hunger] || l.hunger;
  document.getElementById('hud-thirst').textContent = NEED_NAMES[l.thirst] || l.thirst;
  document.getElementById('hud-sick').textContent = l.sickness > 0 ? `Level ${l.sickness}` : 'Healthy';
  document.getElementById('hud-sleep').textContent = l.sleeping ? 'Zzz...' : 'Awake';
  document.getElementById('hud-date').textContent =
    `${w.date_day}/${w.date_month}/${w.date_year}`;
  document.getElementById('hud-time').textContent =
    `${String(w.game_hour).padStart(2,'0')}:${String(w.game_minute).padStart(2,'0')}`;
  document.getElementById('hud-dog').textContent =
    `(${d.x},${d.y}) ${d.sprite_name} Bowl:${BOWL_NAMES[d.bowl_status]||d.bowl_status}`;
  if (d.target_x || d.target_y) {
    let t = d.target_name || `(${d.target_x},${d.target_y})`;
    if (d.on_stairs) t += ' [stairs]';
    document.getElementById('hud-dog-target').textContent = t;
  } else {
    document.getElementById('hud-dog-target').textContent = 'idle';
  }
  document.getElementById('hud-tick').textContent = w.tick;
}

function poll() {
  fetch('/api/frame')
    .then(r => r.json())
    .then(data => {
      if (data.error) { document.getElementById('status').textContent = 'Server: ' + data.error; return; }
      if (data.image) frameImg.src = 'data:image/png;base64,' + data.image;
      if (data.state) { updateHud(data.state);
        document.getElementById('status').textContent =
          `Frame ${data.state.world.tick} | ${new Date().toLocaleTimeString()}`; }
    })
    .catch(e => {
      document.getElementById('status').textContent = 'Error: ' + e;
    });
}

function sendCmd(cmd) {
  fetch('/api/command', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({command: cmd})
  });
}

// Poll at ~4 fps
setInterval(poll, 250);
poll();
</script>
</body>
</html>"""


class PreviewHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(HTML_PAGE.encode('utf-8'))

        elif self.path == '/api/frame':
            try:
                img = render_frame()
                buf = io.BytesIO()
                img.save(buf, format='PNG')
                img_b64 = base64.b64encode(buf.getvalue()).decode('ascii')
                state = get_state_json()
                payload = json.dumps({'image': img_b64, 'state': state})
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Cache-Control', 'no-cache')
                self.end_headers()
                self.wfile.write(payload.encode('utf-8'))
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({'error': str(e)}).encode())

        elif self.path == '/api/state':
            state = get_state_json()
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(state).encode('utf-8'))

        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        if self.path == '/api/command':
            length = int(self.headers.get('Content-Length', 0))
            body = json.loads(self.rfile.read(length)) if length > 0 else {}
            cmd = body.get('command', '')
            _handle_command(cmd)
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({'ok': True}).encode())
        else:
            self.send_response(404)
            self.end_headers()

    def log_message(self, format, *args):
        pass  # Suppress request logs


def _handle_command(cmd: str):
    """Handle a player command from the web UI.
    addr: deal_with_keycode() — processes Ctrl key combos
    """
    from lcp.simulation import put_event_to_list
    from lcp.sound import soundeffect_select
    from lcp.enums import SOUND_EFFECT_ID

    if cmd == 'alarm':
        # Ctrl+A: toggle alarm clock
        gs.ctrl_a_alarm_pressed_flag = 1 if not gs.ctrl_a_alarm_pressed_flag else 0

    elif cmd == 'book':
        # Ctrl+B: book delivery (doorbell + event)
        soundeffect_select(gs, SOUND_EFFECT_ID.SFX_DOORBELL, 6)
        put_event_to_list(gs, ACTION_ID.ACTION_EVENT_BOOK_DELIVERY)

    elif cmd == 'call':
        # Ctrl+C: phone call
        if not gs.phone_answered_flag:
            gs.phone_call_active_flag = 1
            put_event_to_list(gs, ACTION_ID.ACTION_EVENT_PHONE_CALL)

    elif cmd == 'dog_food':
        # Ctrl+D: dog food delivery (doorbell + event)
        soundeffect_select(gs, SOUND_EFFECT_ID.SFX_DOORBELL, 6)
        put_event_to_list(gs, ACTION_ID.ACTION_EVENT_DOG_FOOD)

    elif cmd == 'food':
        # Ctrl+F: food delivery (doorbell + event)
        soundeffect_select(gs, SOUND_EFFECT_ID.SFX_DOORBELL, 6)
        put_event_to_list(gs, ACTION_ID.ACTION_EVENT_FOOD_DELIVERY)

    elif cmd == 'pet':
        # Ctrl+P: pet the dog
        if gs.dog_pettable_flag and not gs.petting_dog_active:
            gs.petting_anim_frame = 0
            gs.petting_dog_active = 1

    elif cmd == 'record':
        # Ctrl+R: record delivery (doorbell + event)
        soundeffect_select(gs, SOUND_EFFECT_ID.SFX_DOORBELL, 6)
        put_event_to_list(gs, ACTION_ID.ACTION_EVENT_RECORD_DELIVERY)

    elif cmd == 'water':
        # Ctrl+W: add water
        if getattr(gs, 'lcp_water_level', 0) < 10:
            gs.lcp_water_level = getattr(gs, 'lcp_water_level', 0) + 1
            soundeffect_select(gs, SOUND_EFFECT_ID.SFX_WATER_TAP, -1)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    global _game_thread

    print(f"Loading assets from {DATA_DIR}...")
    _init_game()
    print(f"LCP: {gs.lcp.name_str}")

    # Start game loop in background
    _game_thread = threading.Thread(target=_game_loop, daemon=True)
    _game_thread.start()

    print(f"Preview server running at http://localhost:{PORT}")
    server = HTTPServer(('0.0.0.0', PORT), PreviewHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        global _running
        _running = False
        print("\nShutting down.")
        server.shutdown()


if __name__ == '__main__':
    main()
