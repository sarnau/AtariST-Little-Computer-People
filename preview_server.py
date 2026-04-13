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

    # Draw dog bowl
    bowl_colors = {0: (100, 100, 100), 1: (160, 120, 60), 2: (200, 150, 80)}
    bowl_color = bowl_colors.get(gs.dog_bowl_status, (100, 100, 100))
    draw.rectangle([5, 190, 14, 195], fill=bowl_color, outline=(80, 80, 80))

    # Draw fire if active
    if gs.fire_active_flag:
        fire_colors = [(255, 100, 0), (255, 180, 0), (255, 60, 0), (255, 200, 50)]
        fc = fire_colors[gs.fire_animation_frame % 4]
        draw.rectangle([257, 162, 275, 175], fill=fc)

    return img


def _nearest_house_pos(tx: int, ty: int) -> str:
    """Find the HOUSE_POS name closest to the given coordinates."""
    if tx == 0 and ty == 0:
        return ''
    best_dist = 999999
    best_name = ''
    for member in HOUSE_POS:
        x, y = house_get_position_xy(member.value)
        d = abs(x - tx) + abs(y - ty)
        if d < best_dist:
            best_dist = d
            best_name = member.name
    return best_name if best_dist < 20 else f'({tx},{ty})'


def get_state_json() -> dict:
    """Return game state as JSON-serializable dict."""
    try:
        lcp_state_name = PLAYER_STATE(gs.lcp_state).name
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
    """Handle a player command from the web UI."""
    cmd_map = {
        'water':    'ctrl_w_water_flag',
        'food':     'ctrl_f_food_flag',
        'dog_food': 'ctrl_d_dogfood_flag',
        'pet':      'ctrl_p_pet_flag',
        'alarm':    'ctrl_a_alarm_pressed_flag',
        'call':     'ctrl_c_phone_flag',
        'record':   'ctrl_r_record_flag',
        'book':     'ctrl_b_book_flag',
    }
    flag = cmd_map.get(cmd)
    if flag and hasattr(gs, flag):
        setattr(gs, flag, 1)


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
