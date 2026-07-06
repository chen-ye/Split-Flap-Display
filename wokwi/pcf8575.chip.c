#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ── Framebuffer layout ────────────────────────────────────────────────────────
// Each PCF8575 chip renders one split-flap panel: 48×64 RGBA pixels
// The 5×7 dot-matrix font is scaled up 5× → 25×35 px, centered on the panel
#define FB_W     48
#define FB_H     64
#define FSCALE    5   // font scale factor: 5×7 dots → 25×35 px
#define CHAR_X  ((FB_W - 5 * FSCALE) / 2)   // 11 px left offset
#define CHAR_Y  ((FB_H - 7 * FSCALE) / 2)   // 14 px top offset
#define SPLIT_Y (FB_H / 2)                   // horizontal split line at y=32

// ── Pixel type (R, G, B, A in memory order) ──────────────────────────────────
typedef struct { uint8_t r, g, b, a; } px_t;

// Palette – classic departure board aesthetic
static const px_t COL_BG_T  = {0x10, 0x10, 0x22, 0xFF}; // deep navy  (top half)
static const px_t COL_BG_B  = {0x18, 0x18, 0x32, 0xFF}; // slightly lighter (bottom half)
static const px_t COL_AMBER = {0xFF, 0xB3, 0x47, 0xFF}; // warm amber character
static const px_t COL_SPLIT = {0x00, 0x00, 0x00, 0xFF}; // black split crease
static const px_t COL_EDGE  = {0x08, 0x08, 0x18, 0xFF}; // dark border

// ── 5×7 bitmap font ───────────────────────────────────────────────────────────
// 7 rows per glyph; each row is 5 bits wide: bit4=leftmost, bit0=rightmost.
// Charset index matches: " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789':/?!.->#$%"
static const uint8_t FONT[48][7] = {
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //  (space)
  {0x0E,0x11,0x11,0x1F,0x11,0x11,0x00}, // A
  {0x1E,0x11,0x11,0x1E,0x11,0x1E,0x00}, // B
  {0x0E,0x11,0x10,0x10,0x11,0x0E,0x00}, // C
  {0x1C,0x12,0x11,0x11,0x12,0x1C,0x00}, // D
  {0x1F,0x10,0x10,0x1E,0x10,0x1F,0x00}, // E
  {0x1F,0x10,0x10,0x1E,0x10,0x10,0x00}, // F
  {0x0E,0x10,0x10,0x13,0x11,0x0F,0x00}, // G
  {0x11,0x11,0x11,0x1F,0x11,0x11,0x00}, // H
  {0x0E,0x04,0x04,0x04,0x04,0x0E,0x00}, // I
  {0x1F,0x02,0x02,0x02,0x12,0x0C,0x00}, // J
  {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // K
  {0x10,0x10,0x10,0x10,0x10,0x1F,0x00}, // L
  {0x11,0x1B,0x15,0x11,0x11,0x11,0x00}, // M
  {0x11,0x19,0x15,0x13,0x11,0x11,0x00}, // N
  {0x0E,0x11,0x11,0x11,0x11,0x0E,0x00}, // O
  {0x1E,0x11,0x11,0x1E,0x10,0x10,0x00}, // P
  {0x0E,0x11,0x11,0x15,0x12,0x0D,0x00}, // Q
  {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // R
  {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E}, // S
  {0x1F,0x04,0x04,0x04,0x04,0x04,0x00}, // T
  {0x11,0x11,0x11,0x11,0x11,0x0E,0x00}, // U
  {0x11,0x11,0x11,0x0A,0x0A,0x04,0x00}, // V
  {0x11,0x11,0x15,0x15,0x1B,0x11,0x00}, // W
  {0x11,0x0A,0x04,0x04,0x0A,0x11,0x00}, // X
  {0x11,0x11,0x0A,0x04,0x04,0x04,0x00}, // Y
  {0x1F,0x01,0x02,0x04,0x08,0x1F,0x00}, // Z
  {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, // 0
  {0x04,0x0C,0x04,0x04,0x04,0x0E,0x00}, // 1
  {0x0E,0x11,0x01,0x06,0x08,0x1F,0x00}, // 2
  {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E}, // 3
  {0x02,0x06,0x0A,0x12,0x1F,0x02,0x00}, // 4
  {0x1F,0x10,0x1E,0x01,0x11,0x0E,0x00}, // 5
  {0x0E,0x10,0x10,0x1E,0x11,0x11,0x0E}, // 6
  {0x1F,0x01,0x02,0x04,0x04,0x04,0x00}, // 7
  {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, // 8
  {0x0E,0x11,0x11,0x0F,0x01,0x11,0x0E}, // 9
  {0x04,0x04,0x00,0x00,0x00,0x00,0x00}, // '
  {0x00,0x04,0x00,0x00,0x04,0x00,0x00}, // :
  {0x01,0x02,0x02,0x04,0x08,0x08,0x10}, // /
  {0x0E,0x11,0x01,0x02,0x04,0x00,0x04}, // ?
  {0x04,0x04,0x04,0x04,0x00,0x04,0x00}, // !
  {0x00,0x00,0x00,0x00,0x00,0x04,0x00}, // .
  {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}, // -
  {0x10,0x08,0x04,0x02,0x04,0x08,0x10}, // >
  {0x0A,0x0A,0x1F,0x0A,0x1F,0x0A,0x00}, // #
  {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04}, // $
  {0x18,0x19,0x02,0x04,0x08,0x13,0x03}, // %
};

// ── Chip state ────────────────────────────────────────────────────────────────
typedef struct {
  pin_t    pins[16];
  uint16_t pin_state;
  uint8_t  buffer[2];
  uint8_t  bytes_received;
  uint32_t address;
  int      magnet_position;
  int      steps_per_rot;
  int      step_count;
  uint8_t  last_coil_state;
  int      charset_size;
  buffer_t fb;
  int      current_char_idx;
  timer_t  render_timer;
  bool     dirty;           // framebuffer needs a redraw
} chip_state_t;

static const char CHARSET[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789':/?!.->#$%";

// Forward declaration (defined below)
static void render_display(chip_state_t *chip);

// ── Render timer callback (~30 fps cap) ──────────────────────────────────────
// Called by Wokwi's timer infrastructure; never blocks the I2C path.
static void on_render_timer(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  chip->dirty = false;
  render_display(chip);
}

// ── Framebuffer renderer ──────────────────────────────────────────────────────
static void render_display(chip_state_t *chip) {
  // Static so it lives in WASM linear memory, not the tiny stack
  static px_t pixels[FB_W * FB_H];

  // Fill background: two subtly different shades for top/bottom halves
  for (int y = 0; y < FB_H; y++) {
    px_t bg = (y < SPLIT_Y) ? COL_BG_T : COL_BG_B;
    for (int x = 0; x < FB_W; x++) {
      pixels[y * FB_W + x] = bg;
    }
  }

  // 1-pixel dark border
  for (int x = 0; x < FB_W; x++) {
    pixels[0             * FB_W + x] = COL_EDGE;
    pixels[(FB_H - 1)    * FB_W + x] = COL_EDGE;
  }
  for (int y = 0; y < FB_H; y++) {
    pixels[y * FB_W + 0]          = COL_EDGE;
    pixels[y * FB_W + (FB_W - 1)] = COL_EDGE;
  }

  // 2-pixel black split crease (the characteristic flip line)
  for (int x = 0; x < FB_W; x++) {
    pixels[(SPLIT_Y - 1) * FB_W + x] = COL_SPLIT;
    pixels[SPLIT_Y       * FB_W + x] = COL_SPLIT;
  }

  // Render the glyph in amber
  int cidx = chip->current_char_idx;
  if (cidx >= 0 && cidx < 48) {
    for (int row = 0; row < 7; row++) {
      for (int col = 0; col < 5; col++) {
        if (FONT[cidx][row] & (0x10 >> col)) {
          // Each font "dot" becomes a FSCALE×FSCALE block of amber pixels
          for (int dy = 0; dy < FSCALE; dy++) {
            for (int dx = 0; dx < FSCALE; dx++) {
              int px = CHAR_X + col * FSCALE + dx;
              int py = CHAR_Y + row * FSCALE + dy;
              if (px >= 0 && px < FB_W && py >= 0 && py < FB_H) {
                pixels[py * FB_W + px] = COL_AMBER;
              }
            }
          }
        }
      }
    }
  }

  buffer_write(chip->fb, 0, (uint8_t *)pixels, sizeof(pixels));
}

// ── I2C callbacks ─────────────────────────────────────────────────────────────
static bool on_i2c_connect(void *user_data, uint32_t address, bool connect) {
  chip_state_t *chip = (chip_state_t *)user_data;
  if (connect) {
    chip->bytes_received = 0; // reset buffer at start of each transaction
  }
  return true; // ACK
}

static bool on_i2c_write(void *user_data, uint8_t data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  if (chip->bytes_received < 2) {
    chip->buffer[chip->bytes_received] = data;
    chip->bytes_received++;
  }
  if (chip->bytes_received == 2) {
    uint16_t new_state = chip->buffer[0] | (chip->buffer[1] << 8);
    chip->pin_state = new_state;

    // Update output pins (skip Hall sensor pin P17 = pins[15])
    for (int i = 0; i < 16; i++) {
      if (i == 15) continue;
      if ((new_state & (1 << i)) == 0) {
        pin_write(chip->pins[i], LOW);
        pin_mode(chip->pins[i], OUTPUT);
      } else {
        pin_mode(chip->pins[i], INPUT_PULLUP);
      }
    }

    // Track stepper coils on bits 1..4 (P1–P4)
    uint8_t coils = (new_state >> 1) & 0x0F;
    if (coils != chip->last_coil_state && coils != 15 && chip->last_coil_state != 15) {
      int step_diff = 0;
      switch (chip->last_coil_state) {
        case 3:  if (coils == 9)  step_diff =  1; else if (coils == 6)  step_diff = -1; break;
        case 9:  if (coils == 12) step_diff =  1; else if (coils == 3)  step_diff = -1; break;
        case 12: if (coils == 6)  step_diff =  1; else if (coils == 9)  step_diff = -1; break;
        case 6:  if (coils == 3)  step_diff =  1; else if (coils == 12) step_diff = -1; break;
      }

      if (step_diff != 0) {
        chip->step_count = (chip->step_count + step_diff + chip->steps_per_rot) % chip->steps_per_rot;

        // Drive Hall sensor (P17 / pins[15]) LOW when passing the magnet
        if (chip->step_count == chip->magnet_position) {
          pin_write(chip->pins[15], LOW);
          pin_mode(chip->pins[15], OUTPUT);
          printf("[PCF8575 @ 0x%02X] Magnet at step %d\n", chip->address, chip->step_count);
        } else {
          pin_mode(chip->pins[15], INPUT_PULLUP);
        }

        // Derive the visible character and re-render if it changed
        float step_size = (float)chip->steps_per_rot / (float)chip->charset_size;
        int char_idx = (int)((chip->step_count + (step_size / 2.0f)) / step_size) % chip->charset_size;
        if (char_idx != chip->current_char_idx) {
          chip->current_char_idx = char_idx;
          // Only schedule a redraw if one isn't already pending
          if (!chip->dirty) {
            chip->dirty = true;
            timer_start(chip->render_timer, 33333, false); // ~30 fps
          }
        }
      }
    }

    if (coils != 15) {
      chip->last_coil_state = coils;
    }
  }
  return true; // ACK
}

static uint8_t on_i2c_read(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  uint16_t input_state = 0;
  for (int i = 0; i < 16; i++) {
    if (pin_read(chip->pins[i]) == HIGH) {
      input_state |= (1 << i);
    }
  }
  if (chip->bytes_received == 0) {
    chip->bytes_received = 1;
    return input_state & 0xFF;
  } else {
    chip->bytes_received = 0;
    return (input_state >> 8) & 0xFF;
  }
}

static void on_i2c_disconnect(void *user_data) {
  (void)user_data; // bytes_received is reset in on_i2c_connect
}

// ── chip_init ────────────────────────────────────────────────────────────────
void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->pin_state       = 0xFFFF;
  chip->bytes_received  = 0;
  chip->last_coil_state = 15;
  chip->step_count      = 0;
  chip->current_char_idx = 0;

  // Read diagram.json attrs (decimal addresses: 32–39 for 0x20–0x27)
  uint32_t addr_attr    = attr_init("address",        32);
  chip->address         = attr_read(addr_attr);

  uint32_t mag_attr     = attr_init("magnet_position", 730);
  chip->magnet_position = (int)attr_read(mag_attr);

  uint32_t steps_attr   = attr_init("steps_per_rot",  2048);
  chip->steps_per_rot   = (int)attr_read(steps_attr);

  uint32_t cs_attr      = attr_init("charset",        48);
  chip->charset_size    = (int)attr_read(cs_attr);

  // Initialise GPIO pins: P0–P7, P10–P17
  char pin_name[8];
  for (int i = 0; i < 8; i++) {
    sprintf(pin_name, "P%d", i);
    chip->pins[i] = pin_init(pin_name, INPUT_PULLUP);
  }
  for (int i = 0; i < 8; i++) {
    sprintf(pin_name, "P1%d", i);
    chip->pins[8 + i] = pin_init(pin_name, INPUT_PULLUP);
  }

  // Hall sensor starts HIGH (no magnet) unless step_count == magnet_position
  if (chip->step_count == chip->magnet_position) {
    pin_write(chip->pins[15], LOW);
    pin_mode(chip->pins[15], OUTPUT);
  } else {
    pin_mode(chip->pins[15], INPUT_PULLUP);
  }

  // Framebuffer: must be called from chip_init only
  uint32_t fw = FB_W, fh = FB_H;
  chip->fb = framebuffer_init(&fw, &fh);
  chip->dirty = false;

  // Render timer – fires once per dirty cycle, capping renders at ~30 fps
  const timer_config_t timer_cfg = {
    .callback  = on_render_timer,
    .user_data = chip,
  };
  chip->render_timer = timer_init(&timer_cfg);

  render_display(chip); // draw initial blank panel

  // Set up I2C slave
  const i2c_config_t i2c_cfg = {
    .address    = chip->address,
    .scl        = pin_init("SCL", INPUT),
    .sda        = pin_init("SDA", INPUT),
    .connect    = on_i2c_connect,
    .read       = on_i2c_read,
    .write      = on_i2c_write,
    .disconnect = on_i2c_disconnect,
    .user_data  = chip,
  };
  i2c_init(&i2c_cfg);

  printf("[PCF8575 @ 0x%02X] initialised (magnet=%d, steps=%d)\n",
         chip->address, chip->magnet_position, chip->steps_per_rot);
}
