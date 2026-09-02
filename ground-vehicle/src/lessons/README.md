# Lesson sketches

Small programs students upload during the lessons, one idea each.

These are teaching programs, not operating programs. Each one does one thing,
says at the top exactly what it does and why, and ends with a **WHAT TO TRY**
list — because the point is not to run them, it is to change them and find out
what happens.

The slides that go with them are in
[`../../docs/courses/`](../../docs/courses/).

---

## Which folder

| Folder | Course | Builds against |
|---|---|---|
| [`beginner_ps3/`](beginner_ps3/) | Pathfinder Beginner, PS3 track | `pathfinder_ps3` |
| [`beginner_switch/`](beginner_switch/) | Pathfinder Beginner, Switch track | `pathfinder_nintendoswitch` |
| [`advanced/`](advanced/) | Pathfinder Advanced | `Pathfinder_Op_Program12` |

**Every sketch uses the same Arduino IDE settings as the full program for its
track.** That is the whole reason the beginner sketches exist twice: a student
sets their board package up once in Lesson 1 and never touches it again.

| Track | Board package | Board menu | Extra libraries |
|---|---|---|---|
| `beginner_ps3` | esp32 by Espressif Systems, **3.0.7** | Tools > Board > ESP32 Arduino > ESP32 Dev Module | PS3 Controller Host, Adafruit NeoPixel |
| `beginner_switch` | esp32_bluepad32 by Ricardo Quesada, **4.1.0** | Tools > Board > esp32_bluepad32 > ESP32 Dev Module | Adafruit NeoPixel |
| `advanced` | esp32_bluepad32 by Ricardo Quesada, **4.1.0** | Tools > Board > esp32_bluepad32 > ESP32 Dev Module | Adafruit NeoPixel |

The two board packages are **mutually exclusive** and both add an entry called
"ESP32 Dev Module" to the board menu. A pile of errors that make no sense is
almost always the wrong one being selected. Each sketch names the package it
needs in its header comment.

The two packages also use incompatible PWM APIs, which is why the motor
sketches differ between tracks and cannot be copied across:

| | PS3 track (esp32 3.0.7) | Switch and advanced (bluepad32 4.1.0) |
|---|---|---|
| Set up | `ledcAttach(pin, freq, bits)` | `ledcSetup(ch, freq, bits)` then `ledcAttachPin(pin, ch)` |
| Write | `ledcWrite(pin, duty)` | `ledcWrite(ch, duty)` |

---

## The beginner sketches

Same fifteen names in both tracks.

| Sketch | Lesson | The one idea |
|---|---|---|
| `l1a_blink` | 1 | setup, loop, pinMode, digitalWrite, delay |
| `l1b_serial_monitor` | 1 | Serial output, variables and types, integer division |
| `l1c_first_pixel` | 1 | Libraries, one NeoPixel, and why `show()` matters |
| `l2a_one_motor` | 2 | Two pins per motor, PWM, direction and speed |
| `l2b_speed_ramp` | 2 | Duty cycle against actual wheel speed |
| `l2c_maneuver_square` | 2 | Dead reckoning: distance = speed x time |
| `l3a_controller_check` | 3 | What the controller actually sends |
| `l3b_deadzone_and_map` | 3 | Deadzone and `map()`, printed, nothing moving |
| `l3c_tank_drive` | 3 | Mixing forward with turn, and the failsafe |
| `l4a_all_one_colour` | 4 | `for` loops over the strip, and the power budget |
| `l4b_led_map` | 4 | Where each of the 32 LED numbers physically is |
| `l4c_patterns` | 4 | Functions: colour wipe, chase, rainbow, scanner |
| `l5a_millis_not_delay` | 5 | Two jobs at two rates without blocking |
| `l5b_button_toggle` | 5 | Edge detection, and the bug you get without it |
| `l5c_drive_with_lights` | 5 | Everything together, one step short of the full program |

`l5b_button_toggle` contains a **deliberate bug**, clearly labelled. One button
is written correctly and one is not, so students can watch a single press
register hundreds of times. Do not "fix" it.

## The advanced sketches

| Sketch | Lesson | The one idea |
|---|---|---|
| `a1a_tabs_and_config` | 1 | Multi-file sketches, and why `Config.h` is a header |
| `a1b_serial_console` | 1 | Reading a command line without blocking |
| `a2a_pwm_resolution` | 2 | Resolution against frequency on the LEDC peripheral |
| `a2b_coast_brake_hybrid` | 2 | Coast, brake, slow and fast decay, and the ramp bug |
| `a3a_controller_address` | 3 | The Bluetooth allowlist, live from the console |
| `a3b_eeprom_settings` | 3 | Settings that survive a power cycle |
| `a4a_led_state_machine` | 4 | One enum, six modes, no `delay()` |
| `a4b_turn_signal_larson` | 4 | The turn signal geometry, one frame at a time |
| `a5a_i2c_scan` | 5 | Finding out what is on the bus |
| `a5b_ina219_current` | 5 | Shunt resistors, and what a motor costs |

`a1a_tabs_and_config` is three files on purpose — the sketch, `Config.h` and
`Blinker.ino` — so students can see the tab mechanism and break it.

---

## Safety

Anything that drives a motor says so in its header. The rule in class:

- **Wheels off the ground** the first time any changed program is uploaded.
- `l2c_maneuver_square` drives on the floor from the moment it is powered up.
  Clear the space and know where the power switch is before uploading it.
- Never leave a lithium pack charging unattended.

---

## Building and checking them

All forty sketch folders compile clean. To verify after a change, using
the `arduino-cli` bundled with the Arduino IDE:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all beginner_ps3/l2a_one_motor
```

```bash
arduino-cli compile --fqbn esp32-bluepad32:esp32:esp32 advanced/a5b_ina219_current
```

Note the core IDs: `esp32:esp32` for the PS3 track and `esp32-bluepad32:esp32`
— with a **hyphen** — for the other two. `esp32_bluepad32` with an underscore
is the board-menu name and `arduino-cli` rejects it.

Add `--libraries <your Arduino libraries folder>` if the CLI cannot find
Adafruit NeoPixel or the PS3 library.
