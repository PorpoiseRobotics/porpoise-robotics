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

**Every slide has speaker notes.** They are the teacher's script, not a copy of
the slide: what to say, what to ask before you explain, which mistake the room
is about to make, and the answers to every question the slide poses. Open
Presenter View, or print the notes pages. Students never see them.

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
| `diagrams.py` | The nineteen drawn figures, as native shapes |
| `content_beginner.py` | The five beginner lessons, parameterised by track |
| `content_advanced.py` | The five advanced lessons |
| `srcfacts.py` | Reads constants out of the `.ino` files so slides cannot quote stale numbers |
| `lint_decks.py` | Checks every deck for text overflow, overlapping text and off-slide shapes |
| `check_code.py` | Checks every code listing still matches the source it came from |
| `render_decks.py` | Renders the decks to PDF, and optionally to contact sheets |

Three checks. Run all of them after any change, to the decks or to the code.

`lint_decks.py` is fast and needs nothing installed. It estimates how tall each
block of text will be once it wraps, and reports anything that will not fit its
box, runs off the slide, or is drawn on top of another block of text. It should
report zero issues.

That last check earns its keep: a diagram whose caption sits under the note
panel at the bottom of the slide fits its own box perfectly and is still
invisible, which is exactly the failure the height estimate cannot see.

```bash
python ground-vehicle/docs/courses/generator/lint_decks.py
```

`check_code.py` is the one that matters most. Every code listing on a slide is
supposed to be a real excerpt from a real file under `src/`, and this proves it
still is — it pulls every line out of every code panel and looks for it in the
sources. Illustrative lines (generic C, the deliberately-broken examples) are
listed in `ALLOWED` inside that file. It exits non-zero on a mismatch, so it
can gate a commit.

```bash
python ground-vehicle/docs/courses/generator/check_code.py
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

## Keeping the slides and the code together

This is the failure mode that got the old decks: somebody improves a function,
and a slide somewhere quietly goes on teaching last month's version. Two
mechanisms stop it happening here.

**Numbers are read, not retyped.** The deadzone, the stick range, the PWM
frequency and resolution, the motor pin assignments and the LED count are all
pulled out of the `.ino` files at build time by `srcfacts.py`. Change
`STICK_DEADZONE` in a sketch, rebuild, and the slide that quotes it changes
with it — including the percentage worked out from it. If a constant is
renamed, the build stops rather than printing a stale figure.

**Code listings are verified.** `check_code.py` confirms every line in every
code panel appears verbatim in a source file. A listing that has drifted is a
build failure, not something you have to notice by eye.

Neither can check prose. If you rewrite a paragraph describing what a program
does, or a speaker note explaining it, that is still on you.

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
with a robotic arm fitted. Two of Kevin's own labelled figures were carried
over as crops rather than redrawn, because he drew them and they are ours: the
board layout seen from above, and the annotated top plate.

### Pictures we have not taken yet

Where a photograph is wanted and does not exist, the slide carries a **dashed,
labelled placeholder box** in the space the picture will occupy, saying what
belongs there. Flip through a deck and every outstanding photograph is visible
in one pass; they show on the printed handout too.

Three are outstanding:

| Deck | What is wanted |
|---|---|
| Beginner L1 | Our own breadboard wired to GPIO 2 and GND, LED lit. The photographs we have use a 9 V battery. |
| Beginner L1 | A Gen 3 driving upside down, taken from floor level. |
| Advanced L5 | The INA219 on a Gen 3 board, close enough to read the part. |

To fill one in, drop the photograph in `images/` and swap the `Placeholder(...)`
in the content module for `img("your-file.jpg")`. Anything else that resolves
to a missing file also draws a placeholder rather than silently leaving a hole,
so a mistyped filename is visible instead of invisible.

---

## Still to do

- The three **photographs listed above**. Each one has a labelled placeholder
  sitting on the slide until it arrives.
- The two **controller images** are vendor product shots rather than Porpoise
  photography. They were already in the P2 and P3 decks, but this repository is
  public — worth a decision.
- Kevin's P3 L1 carries a full **ESP32-WROOM-32E pinout** picture. It is a
  third-party figure, so this course draws its own page of just the pins this
  vehicle uses instead. If we ever want the complete pinout, it needs redrawing
  or licensing.
- Kevin's P3 L5 carries the **Gen 3 kit parts list and assembly procedure**.
  That is build documentation rather than course material, so it was left out
  of the decks — but it should live somewhere, probably in `hardware/`.
- Nobody has taught from these yet. Timings on the agenda slides are estimates,
  and so is every "about N minutes" in the speaker notes.
