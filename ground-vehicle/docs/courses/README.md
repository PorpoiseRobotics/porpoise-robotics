# Pathfinder course slides

Three courses, five three-hour lessons each, as editable PowerPoint files.

Each course teaches one of the Pathfinder operating programs in
[`../../src/`](../../src/). The follow-along sketches students upload during
the lessons live in [`../../src/lessons/`](../../src/lessons/).

---

## Which course to teach

| Folder | Course | Teaches | For |
|---|---|---|---|
| [`beginner-ps3/`](beginner-ps3/) | Pathfinder Beginner, PS3 | `pathfinder_ps3` | New to robotics, driving with a PS3 controller |
| [`beginner-switch/`](beginner-switch/) | Pathfinder Beginner, Switch | `pathfinder_nintendoswitch` | New to robotics, driving with a Nintendo Switch controller |
| [`advanced/`](advanced/) | Pathfinder Advanced | `Pathfinder_Op_Program12` | Students who have finished a beginner course and can read C++ |

The two beginner courses are the **same course**. They differ only where the
hardware forces them to: the controller, the board package, the stick range,
and the way a vehicle is locked to one controller. A student who has done one
can read the other.

**The board packages are mutually exclusive in the Arduino IDE.** Do not try to
run both beginner tracks on one laptop in the same session without warning
students about the two identical "ESP32 Dev Module" entries in the board menu.

---

## The lessons

### Beginner (either track)

| Deck | Lesson | Sketches uploaded |
|---|---|---|
| `l1_introduction.pptx` | Mechatronics, the vehicle, the Arduino IDE, first programs | `l1a_blink`, `l1b_serial_monitor`, `l1c_first_pixel` |
| `l2_motor_control.pptx` | H-bridges, PWM, tank drive, dead reckoning | `l2a_one_motor`, `l2b_speed_ramp`, `l2c_maneuver_square` |
| `l3_controller_programming.pptx` | Bluetooth, deadzones, mapping, mixing, failsafe | `l3a_controller_check`, `l3b_deadzone_and_map`, `l3c_tank_drive` |
| `l4_neopixel_leds.pptx` | Addressable LEDs, colour, power budget, patterns | `l4a_all_one_colour`, `l4b_led_map`, `l4c_patterns` |
| `l5_maneuvers_with_lights.pptx` | Non-blocking timing, edge detection, the full program | `l5a_millis_not_delay`, `l5b_button_toggle`, `l5c_drive_with_lights` |

### Advanced

| Deck | Lesson | Sketches uploaded |
|---|---|---|
| `l1_architecture_and_toolchain.pptx` | Tabs, headers, capability flags, a serial console | `a1a_tabs_and_config`, `a1b_serial_console` |
| `l2_motors_and_ramping.pptx` | PWM resolution, decay modes, speed ramping | `a2a_pwm_resolution`, `a2b_coast_brake_hybrid` |
| `l3_bluetooth_and_storage.pptx` | Callbacks, the allowlist, EEPROM that is not EEPROM | `a3a_controller_address`, `a3b_eeprom_settings` |
| `l4_lighting_state_machine.pptx` | State machines, non-blocking animation, dirty flags | `a4a_led_state_machine`, `a4b_turn_signal_larson` |
| `l5_sensors_and_self_test.pptx` | I2C, current sensing, self-test, diagnostics | `a5a_i2c_scan`, `a5b_ina219_current` |

Every lesson ends with a short quiz slide. Every hands-on block is a
distinctively formatted "Do it now" slide naming the sketch, the steps, what
students should see, and the questions to answer.

---

## Editing the decks

Open them in PowerPoint and edit them. That is what they are for.

Everything is a real PowerPoint object — real title placeholders so the outline
view works, real text frames, real tables, real autoshapes. Nothing is a
picture of a slide, and every diagram is built from native shapes so it stays
editable and prints sharp.

They are 16:9 (13.333 x 7.5 inches), matching the existing Porpoise Robotics
decks, and designed to print: white background, dark text, no full-bleed dark
fills, and nothing that needs colour to make sense in greyscale.

---

## Regenerating them

**Do not, unless you mean it.** The `.pptx` files are the deliverable, and once
anybody has edited one in PowerPoint it is the source of truth. Re-running the
generator overwrites them and hand edits go with them.

The generator exists so that the first version was reproducible, and so a
change affecting all fifteen decks — a corrected pin number, a new house
colour — can be made in one place before anybody has started editing.

```bash
python ground-vehicle/docs/courses/generator/build_decks.py
```

It needs `python-pptx` (`pip install python-pptx`). Files in
[`generator/`](generator/):

| File | What it is |
|---|---|
| `build_decks.py` | Runs everything. Start here. |
| `slidelib.py` | The slide types: bullets, tables, code, activities, quizzes |
| `diagrams.py` | The fourteen drawn figures, as native shapes |
| `content_beginner.py` | The five beginner lessons, parameterised by track |
| `content_advanced.py` | The five advanced lessons |
| `lint_decks.py` | Checks every deck for text overflow and off-slide shapes |
| `render_decks.py` | Renders the decks to PDF, and optionally to contact sheets |

Two checks, and both are worth running after any content change.

`lint_decks.py` is fast and needs nothing installed. It estimates how tall each
block of text will be once it wraps, and reports anything that will not fit its
box or that runs off the slide. It should report zero issues.

```bash
python ground-vehicle/docs/courses/generator/lint_decks.py
```

`render_decks.py` needs LibreOffice and actually draws the slides, so you can
look at them. It also produces the print-ready PDFs.

```bash
python ground-vehicle/docs/courses/generator/render_decks.py --sheets
```

PDFs land in `docs/courses/pdf/`, which is gitignored — it is a build output. A
PDF prints identically everywhere, whether or not the machine has Calibri and
Consolas installed, so hand those out rather than the `.pptx` when you only
need paper. `--sheets` also writes one PNG per deck showing every slide at
thumbnail size, which is the quickest way to spot a broken layout.

---

## Images

[`images/`](images/) holds the photographs and diagrams the decks use. They
were taken from the previous Porpoise Robotics lesson PDFs so the new decks
keep continuity with the old ones.

Only material Porpoise Robotics owns was carried over — vehicle and control
board photographs, the engineering-to-mechatronics chart, the house logo, and
the annotated PS3 and Switch controller maps. Generic technical figures that
came from third parties in the old decks were **redrawn** as native PowerPoint
shapes instead, because this repository is public. That is why the duty-cycle,
H-bridge, colour-mixing, servo-timing and Ohm's-law figures look different from
the versions in the old PDFs.

Four images came from the **P3 Gen 3 decks**: the Porpoise Robotics logo, the
labelled Switch controller, a second Switch controller shot, and the vehicle
with a robotic arm fitted.

---

## Still to do

- Photographs of a **Gen 3 vehicle**, ideally showing the INA219 current
  sensor. Every vehicle photograph here is Gen 2, so the advanced course has no
  picture of the sensor it spends a whole lesson on. The P3 decks did not have
  one either.
- The two **controller images** are vendor product shots rather than Porpoise
  photography. They were already in the P2 and P3 decks, but this repository is
  public — worth a decision.
- Nobody has taught from these yet. Timings on the agenda slides are estimates.
