#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t pins[16];
  uint16_t pin_state;
  uint8_t buffer[2];
  uint8_t bytes_received;
  uint32_t address_attr;
  uint32_t address;
  int magnet_position;
  int steps_per_rot;
  int step_count;
  uint8_t last_coil_state;
  int charset_size;
} chip_state_t;

static const char charset[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789':/?!.->$#%";
static char display_board[9] = "        ";

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

    // 1. Update outputs for all pins except the Hall sensor pin P17 (pins[15])
    for (int i = 0; i < 16; i++) {
      if (i == 15) continue;
      if ((new_state & (1 << i)) == 0) {
        pin_write(chip->pins[i], LOW);
        pin_mode(chip->pins[i], OUTPUT);
      } else {
        pin_mode(chip->pins[i], INPUT_PULLUP);
      }
    }

    // 2. Track Stepper Coils (mapped to P1, P2, P3, P4 on bits 1..4)
    uint8_t coils = (new_state >> 1) & 0x0F;
    if (coils != chip->last_coil_state && coils != 15 && chip->last_coil_state != 15) {
      int step_diff = 0;
      switch (chip->last_coil_state) {
        case 3: // 0b0011
          if (coils == 9) step_diff = 1;
          else if (coils == 6) step_diff = -1;
          break;
        case 9: // 0b1001
          if (coils == 12) step_diff = 1;
          else if (coils == 3) step_diff = -1;
          break;
        case 12: // 0b1100
          if (coils == 6) step_diff = 1;
          else if (coils == 9) step_diff = -1;
          break;
        case 6: // 0b0110
          if (coils == 3) step_diff = 1;
          else if (coils == 12) step_diff = -1;
          break;
      }

      if (step_diff != 0) {
        chip->step_count = (chip->step_count + step_diff + chip->steps_per_rot) % chip->steps_per_rot;

        // Drive P17 (pins[15]) LOW if it reaches the magnet position
        if (chip->step_count == chip->magnet_position) {
          pin_write(chip->pins[15], LOW);
          pin_mode(chip->pins[15], OUTPUT);
          printf("[PCF8575 @ 0x%02X] Magnet Sensed at step %d!\n", chip->address, chip->step_count);
        } else {
          pin_mode(chip->pins[15], INPUT_PULLUP);
        }

        // Update display character
        float stepSize = (float)chip->steps_per_rot / (float)chip->charset_size;
        int char_idx = (int)((chip->step_count + (stepSize / 2.0)) / stepSize) % chip->charset_size;
        char new_char = charset[char_idx];

        int idx = chip->address - 0x20;
        if (idx >= 0 && idx < 8) {
          if (display_board[idx] != new_char) {
            display_board[idx] = new_char;
            printf("Display Board: |%s|\n", display_board);
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

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->pin_state = 0xFFFF;
  chip->bytes_received = 0;
  chip->address_attr = attr_init("address", 0x20);
  chip->address = attr_read(chip->address_attr);

  uint32_t magnet_attr = attr_init("magnet_position", 730);
  chip->magnet_position = attr_read(magnet_attr);

  uint32_t steps_attr = attr_init("steps_per_rot", 2048);
  chip->steps_per_rot = attr_read(steps_attr);

  uint32_t charset_attr = attr_init("charset", 48);
  chip->charset_size = attr_read(charset_attr);

  chip->step_count = 0;
  chip->last_coil_state = 15;

  char pin_name[8];
  for (int i = 0; i < 8; i++) {
    sprintf(pin_name, "P%d", i);
    chip->pins[i] = pin_init(pin_name, INPUT_PULLUP);
  }
  for (int i = 0; i < 8; i++) {
    sprintf(pin_name, "P1%d", i);
    chip->pins[8 + i] = pin_init(pin_name, INPUT_PULLUP);
  }

  // Setup initial state for Hall sensor pin P17 (pins[15])
  if (chip->step_count == chip->magnet_position) {
    pin_write(chip->pins[15], LOW);
    pin_mode(chip->pins[15], OUTPUT);
  } else {
    pin_mode(chip->pins[15], INPUT_PULLUP);
  }

  // Initial char setup on the board
  int idx = chip->address - 0x20;
  if (idx >= 0 && idx < 8) {
    float stepSize = (float)chip->steps_per_rot / (float)chip->charset_size;
    int char_idx = (int)((chip->step_count + (stepSize / 2.0)) / stepSize) % chip->charset_size;
    display_board[idx] = charset[char_idx];
  }

  const i2c_config_t i2c_config = {
    .address = chip->address,
    .sda = pin_init("SDA", INPUT),
    .scl = pin_init("SCL", INPUT),
    .connect = on_i2c_connect,
    .user_data = chip,
    .write = on_i2c_write,
    .read = on_i2c_read,
    .disconnect = on_i2c_disconnect
  };
  i2c_init(&i2c_config);
}
