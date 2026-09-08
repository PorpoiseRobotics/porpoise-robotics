# Take it further — the project sketches

Lesson 5 ends with a slide called **"Take it further — projects that fit on
this vehicle"**. These are those projects, built and working, one folder each.

They are **worked answers, for instructors**. A student who picks one of these
up should be building it themselves, from the lesson sketches they already
have. These exist so that when a student gets stuck at 2:40 on a Saturday
afternoon, the person teaching has already seen where the problem is.

An instructor who does not know how to help is the failure this folder is
here to prevent.

---

## The five

| Folder | From the slide | New hardware |
|---|---|---|
| `p1_low_battery_warning/` | Flash yellow below 12 V and red below 10 V | Two resistors |
| `p2_collision_warning/` | Stop, flash red, wait three seconds, carry on | HC-SR04 + two resistors |
| `p3_automatic_lights/` | Reversing beeps, hazard lights, headlights that come on only when moving | A passive piezo buzzer |
| `p4_waypoint_navigation/` | Give the program a LIST of moves instead of one hard-coded square | None |
| `p5_servo_pan_tilt/` | Two servos and a bracket, aimed with the right stick | Two servos and a bracket |

Two of the five need nothing bought: `p4` needs no hardware at all, and `p5`
uses the servo headers that are already on the board. Those are the two to
suggest first.

---

## Which folder

Same rule as the lesson sketches, for the same reason.

| Folder | Course | Board package |
|---|---|---|
| [`beginner_ps3/`](beginner_ps3/) | Pathfinder Beginner, PS3 track | `esp32` by Espressif Systems, **3.0.7** |
| [`beginner_switch/`](beginner_switch/) | Pathfinder Beginner, Switch track | `esp32_bluepad32` by Ricardo Quesada, **4.1.0** |

The two packages are **mutually exclusive** and both add an entry called "ESP32
Dev Module" to the board menu. A pile of errors that make no sense is almost
always the wrong one selected.

On the Switch track, fill in `MY_CONTROLLER` before you upload, the same way
you did for the lesson sketches. Run `l3a_controller_check` and paste the line
it prints.

---

## How they are built

**Every one of these is `l5c_drive_with_lights` with one thing added.**

That is deliberate. `l5c` is the last program students write themselves, so it
is the one they know best. Open a project next to it in two editor windows and
the difference between them is the project — nothing else has moved, nothing
has been tidied, and no clever refactor is hiding the part that matters.

So the way to teach one of these is not to hand it over. It is to open both
files, scroll to the part that differs, and ask what it does.

Each file's header comment carries:

- what it does, and what you have to wire up, with the resistor values worked
  out rather than asserted
- what happens if you have not wired it up yet — every one of them is safe to
  upload before the hardware exists, and says on the serial monitor that the
  hardware is missing rather than behaving strangely
- **THE IDEA** — the one thing worth understanding, which is usually not the
  feature. `p2` is really about hysteresis; `p4` is really about the difference
  between putting a route in code and putting it in data; `p5` is really about
  position control against rate control.
- **WHAT TO TRY** — things to change and find out, the same as the lesson
  sketches, ending with folding the project into a copy of the full program.

---

## Keeping the two tracks in step

The PS3 and Switch copies of each project are the same program. They differ
only where the hardware forces them to: the controller API, the PWM API, the
stick range, and the button names.

**Change one and change the other.** They were written together and a diff
between the pair should only ever show those four things. The same discipline
the lesson sketches need, for the same reason: a student who has done one
track should be able to read the other.

---

## What these are not

They are not operating programs. Nothing here has been through a season of
Saturdays with fourteen-year-olds and a flat battery, which is what
`pathfinder_ps3` and `Pathfinder_Op_Program12` have.

They compile, and the logic is right. `p4` in particular will drive off on its
own the moment you press the button, so read it before you run it, and keep
the wheels off the ground until you know what it does.

If one of these turns out to be worth having on every vehicle, it belongs in
the operating program, not here.
