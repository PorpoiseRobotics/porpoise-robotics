# Pathfinder Explorers — instructor guide

**DRAFT.** Twelve one-hour lessons for students aged **9 to 13** (upper
elementary and middle school), designed to run once a week for twelve weeks.
Nintendo Switch track. Nobody has taught from it yet.

Explorers is the Pathfinder Beginner course rebuilt for younger students. It
uses the same vehicle, the same board package and the same slide engine, with
these changes:

- **One hour at a time.** Each lesson is about 20 slides instead of 85.
- **Short programs.** Every sketch carries a helper tab, `explorer.h`, that
  hides the motor, light and Bluetooth plumbing. A student's own program fits
  on one screen, and they still write real Arduino C++.
- **An unplugged game before most new ideas.** Students play the robot, the
  light strip or the tank before they program it.
- **Three levels on every mission.** A nine-year-old and a thirteen-year-old
  can share a vehicle and both be stretched.
- **Speed is earned.** Every moving sketch starts at a learner speed limit, and
  students raise it themselves when they pass a driving test.
- **A Mission Log.** Each student gets a printed workbook with a page per
  lesson, a badge to earn each time, and a driver's license.
- **Three questionnaires.** At the start, halfway and the end of every
  cohort, so the team can see how students are growing, hear what's landing,
  and keep refining the lessons.

If a student finishes Explorers and wants more, the Beginner course
([`../beginner-switch/`](../beginner-switch/)) is the next step. It covers the
same ground again, three hours at a time and line by line, with no helper
file.

### How the lessons teach

The same few methods run through every session, because they are what makes
an idea land for this age group:

- **Do it with your body, then with the robot.** Most new ideas start as a
  game: students program a classmate as a robot (instructions), stand in a row
  as the light strip (loops), walk as a two-person tank (steering), relay
  messages to "Mars" (why rovers need a plan), and play Simon Says (if/else).
- **Predict, test, explain.** Students write a guess in the Mission Log before
  they upload: what color, which wheels, where the square ends. Being wrong
  and finding out why is the lesson.
- **See it drawn.** Every lesson has figures made for this age: the robot as a
  body, pedaling and coasting for motor speed, the light loop, a stick as a
  number line, a nap against a glance at the clock.
- **One idea and one short program a session.** The helper tab keeps each
  program to a screen, so the new idea is the only new thing on it.
- **Everyone works at their own level.** Three levels on every mission, and
  three rotating team jobs, so younger and older students both stay busy.
- **Progress students can see.** Badges, a driver's license that unlocks
  speed, and a Mission Day to show families. Nothing is a competition.
- **Look back every time.** Each session ends with a Mission Log page and a
  few check-yourself questions. The cohort as a whole is checked three times
  by the questionnaires.

---

## What's here

| Where | What |
|---|---|
| This folder | The twelve lesson decks, `l01_meet_pathfinder.pptx` to `l12_mission_day.pptx`. Every slide has speaker notes. |
| [`handouts/mission_log.docx`](handouts/mission_log.docx) | The student workbook. Print one per student, double-sided. 22 pages. |
| [`handouts/letter_to_families.docx`](handouts/letter_to_families.docx) | A letter home, with a photo-permission slip. Fill in the highlighted **[BRACKETS]**. |
| [`handouts/certificate.pptx`](handouts/certificate.pptx) | The certificate of completion. Type each name over **[Student name]**, or generate a whole class at once (see [Rebuilding](#rebuilding)). |
| [`handouts/questionnaire_1_starting_line.docx`](handouts/questionnaire_1_starting_line.docx) | Questionnaire 1 of 3, Lesson 1. One sheet, double-sided. |
| [`handouts/questionnaire_2_halfway.docx`](handouts/questionnaire_2_halfway.docx) | Questionnaire 2 of 3, Lesson 7. Three pages. |
| [`handouts/questionnaire_3_finish_line.docx`](handouts/questionnaire_3_finish_line.docx) | Questionnaire 3 of 3, Lesson 12. Three pages. |
| [`handouts/questionnaire_tracker.xlsx`](handouts/questionnaire_tracker.xlsx) | Where the answers go. Works out class averages and each student's growth. One copy per cohort. |
| [`../../../src/lessons/explorers/`](../../../src/lessons/explorers/) | The fifteen sketches, and the master copy of `explorer.h`. |

### The lessons

| Deck | Lesson | Sketches | Unplugged game | Badge |
|---|---|---|---|---|
| `l01_meet_pathfinder` | What a robot is, the vehicle, the Explorer Code, first drive | `e01a_learner_drive` (preloaded) | Robot or not? | Learner Permit |
| `l02_talking_to_robots` | Instructions, `setup()` and `loop()`, uploading, bugs | `e02a_hello_rover`, `e02b_bug_hunt` | Program a Human Robot | Bug Hunter |
| `l03_color_lab` | Mixing light, `mixColor()`, the light map | `e03a_color_lab`, `e03b_light_map` | — | Color Scientist |
| `l04_loops_and_light_shows` | `for` loops, animation, designing a pattern | `e04a_light_show` | The Human Light Strip | Light Show Designer |
| `l05_make_it_move` | Motors, half speed, tank steering — wheels up | `e05a_motor_lab` | The Human Tank | Motor Mechanic |
| `l06_drive_by_code` | Autopilot moves, milliseconds, tuning | `e06a_drive_a_square` | — | Autopilot |
| `l07_mission_to_mars` | Why rovers drive themselves, functions, route planning | `e07a_mission_planner` | The Mars Delay Game | Mission Planner |
| `l08_wireless_control` | Bluetooth, `if`/`else`, held and tapped buttons | `e08a_button_lab` | Simon Says, if/else edition | Radio Operator |
| `l09_joystick_driving` | Stick numbers, the wobble zone, mixing, the driving test | `e09a_joystick_drive` | — | Licensed Driver |
| `l10_smart_lights` | Car-light language, `else if`, blinking without `delay()` | `e10a_smart_lights` | Nap vs. clock (in the notes) | Signal Expert |
| `l11_design_challenge` | The design process, planning and building an upgrade | `e11a_my_upgrade` | — | Design Engineer |
| `l12_mission_day` | The four-station course, showcase, certificates | `e12a_mission_day` | — | Pathfinder Explorer |

Every lesson follows the same shape: today's mission, a recap ("Last time"),
the agenda, then two to four stages, each opened by a divider that shows where
the class is in the hour. It ends with a "Check yourself" quiz (answers in
the notes) and a "Mission Log, then pack up" slide.

Lessons 1, 7 and 12 also hand out a questionnaire, with its own CHECK-IN slide
and time on the agenda. See [Questionnaires](#questionnaires).

---

## Before the course

### Per team (three students)

- One Pathfinder vehicle with a charged battery
- One Nintendo Switch-style controller, charged
- One **USB data cable**. A charge-only cable looks identical and is the most
  common reason an upload fails, so keep two known-good spares in your pocket.
- One laptop with the Arduino IDE (setup below)
- One wheels-up stand: a wooden block, a small sturdy box, or a stack of books
  that holds all four wheels off the table
- A pair of matching number stickers, one for the vehicle and one for its
  controller

Teams of two work fine (one student is Crew Chief and Pilot). In teams of four,
add a Navigator who reads the steps and keeps the Mission Log.

### For the room

| Item | Used in |
|---|---|
| Masking tape (a lot) | The arena, the square, the Mars grid, the Mission Day course |
| 4 to 8 plastic cups | The driving course, Mission Day |
| Sticky notes | Lesson 1, each student's Explorer code. Lesson 6, marking where each run ends. |
| 8 sheets of colored paper | Lesson 4, the Human Light Strip |
| A pool noodle or a meter stick | Lesson 5, the Human Tank |
| A hula hoop or paper plates | Lesson 7, the crater |
| A magnifying glass (optional) | Lesson 3, the three tiny lights inside each LED |
| A rubber stamp or stickers | Badges and licenses, every lesson |
| A printed Mission Log per student | Every lesson |
| Printed questionnaires, one per student | Lessons 1, 7 and 12 |

### Set up each laptop, once

Students should never be installing anything. Do this before Lesson 1:

1. Install **Arduino IDE 2**.
2. *File > Preferences > Additional boards manager URLs*, add
   `https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json`
3. *Tools > Board > Boards Manager*, search **bluepad32**, install
   **esp32_bluepad32 by Ricardo Quesada, version 4.1.0**.
4. *Tools > Manage Libraries*, install **Adafruit NeoPixel** by Adafruit.
5. Copy the whole [`src/lessons/explorers/`](../../../src/lessons/explorers/)
   folder into the Arduino sketchbook (normally `Documents/Arduino/`), so
   students find every sketch under *File > Sketchbook > explorers*.
6. Choose *Tools > Board > esp32_bluepad32 > ESP32 Dev Module*, and leave
   *Tools > Erase All Flash Before Sketch Upload* at **Disabled**.
7. Plug in a vehicle once and pick its port under *Tools > Port*.

If the same laptops are used for the PS3 Beginner course, note that both board
packages add an entry called "ESP32 Dev Module". Explorers needs the one under
**esp32_bluepad32**.

### Set up each vehicle, once

Each vehicle must be told which controller is its own. Explorers stores that in
the vehicle's own memory, so students never type a Bluetooth address.

1. Switch **off** every other controller in the room.
2. Upload **`e00_claim_controller`** to the vehicle, wheels up, and open the
   Serial Monitor at 115200 baud. The lights blink blue.
3. Hold the small round SYNC button on the controller until its lights run back
   and forth. When it connects, the vehicle turns solid blue.
4. Press **A** on that controller. The vehicle flashes green three times and the
   controller buzzes. The claim is saved.
5. Press the other buttons. The Serial Monitor names each one, which checks
   that this controller's printed letters match what the course expects.
6. Put matching number stickers on the vehicle and the controller.
7. Upload **`e01a_learner_drive`**, ready for Lesson 1.

Uploading other programs does not erase the claim. Running
`e00_claim_controller` again replaces it. If a vehicle's lights ever **blink
red**, it has no claim (usually because somebody enabled *Erase All Flash*).
Run `e00` again.

Two other lights to know: **blinking green** means the vehicle is waiting for its
controller, so press a button on it. **Solid lights, nothing moving** means it's
connected and the program is running.

### The arena

Tape a rectangle on the floor, about **10 by 12 feet**, with a START box, four
cups in a line, and a GARAGE box (a taped rectangle a little bigger than a
vehicle). The same course is the Lesson 1 first drive, the Lesson 9 driving test
and the Lesson 10 Expert test. Lesson 1 slide 21 has a drawing of it.

---

## Before each lesson

| Lesson | Prepare |
|---|---|
| 1 | Every vehicle claimed and running `e01a_learner_drive`. The arena taped. Mission Logs and Starting Line questionnaires printed. An Explorer code for each student on a sticky note, and your own list matching codes to names. A copy of `questionnaire_tracker.xlsx` for this cohort. |
| 2 | Laptops set up. A cable per team, plus spares. |
| 3 | A dim corner helps for judging colors. Magnifying glass, if you have one. |
| 4 | Eight sheets of colored paper. Clear space at the front of the room. |
| 5 | Check every wheels-up stand. Pool noodle or meter stick. Remind students: hair tied back. |
| 6 | One tape square per team, about 4 feet on a side, START marked. Sticky notes. |
| 7 | Halfway Check-in questionnaires printed. The Mars course: START, a crater, a SAMPLE zone, on a grid of one-foot squares. |
| 8 | Every controller charged. Check each team has its own controller. |
| 9 | The driving course set up. Fully charged batteries. License stamps. |
| 10 | The driving course again, for Expert License attempts. |
| 11 | Nothing special. Expect it to be loud. |
| 12 | The Mission Day course. All batteries and controllers charged. Finish Line questionnaires printed. Certificates printed with names. A spare vehicle running `e12a_mission_day`. Chairs for families. |

---

## How the course works

### Team jobs

Every team has a **Pilot** (drives, runs the tests), a **Coder** (at the
laptop), and a **Crew Chief** (in charge of the vehicle: stand, power switch,
cable; reads the steps out loud; calls STOP). Jobs rotate every lesson. There
is a rotation table in the Mission Log, and the opening slide of Lessons 2 to
10 says "Switch jobs!" as a reminder.

### The three levels

Every MISSION slide has Level 1, Level 2 and Level 3 steps. Level 1 is what
every team should finish. Level 2 is where most teams land. Level 3 is for the
team that's flying. Answers and hints for every level are in the speaker notes,
not on the slides.

### Speed limits and the driver's license

Every sketch that moves the wheels has this line near the top:

```cpp
const int SPEED_LIMIT = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100
```

| License | `SPEED_LIMIT` | Earned |
|---|---|---|
| Learner Permit | 50 | Lesson 1: knows the Explorer Code and finishes the driving course |
| Driver's License | 75 | Lesson 9: passes the driving test with no cups knocked over |
| Expert License | 100 | Lesson 10 on: the test course at 75, signaling every turn |

`SPEED_LIMIT` is a percent of full power. At 50, a full push on the stick
gives half power, and every other speed scales with it, so steering still
works at full stick. When a team has earned a level, **the students change the
number themselves**. Changing a number to change the robot is the whole idea of
the course. You decide when a team goes up a level, and you can send a careless
team back down.

This limit belongs to the Explorers sketches only. The full operating programs,
`pathfinder_nintendoswitch` and `pathfinder_ps3`, are unchanged and still run
at full speed. The deck build stops if any Explorers sketch ships with a limit
above the Learner level.

### Badges and the Mission Log

Every lesson's final slide names a badge. Stamp it in the box at the top of
that lesson's Mission Log page. Stamp the three licenses on the license page.
The glossary at the back of the Mission Log is built from the decks' "New
words" slides, so the workbook and the slides use the same words.

---

## Questionnaires

Eddie asked for a questionnaire at the beginning, the middle and the end of
each cohort, "so we can see how students are growing, hear what's resonating,
and keep refining the modules together". There are three, and the slides hand
each one out:

| Form | When | Time | Length |
|---|---|---|---|
| **Starting Line** | Lesson 1, before anything is taught | 6 minutes | One sheet, double-sided |
| **Halfway Check-in** | Start of Lesson 7, after six lessons | 7 minutes | Three pages |
| **Finish Line** | Lesson 12, straight after the mission | 7 minutes | Three pages |

### What's on them

- **Part 1, About me.** The same eight sentences every time, on the same
  four-point scale, from "Not true for me" to "Very true": confidence (three
  sentences), keeping going when it's hard (one), teamwork (two) and interest
  (two). This is growth in how students see themselves.
- **Part 2, What do you know?** The same eight questions every time, one from
  each big idea of the course: what a robot is, `loop()`, mixing light, tank
  steering, milliseconds, functions, `if`/`else`, and why `delay()` can't
  drive. Every question has an "I don't know yet" box, so students don't
  guess. At the Starting Line most of Part 2 *should* be "I don't know yet";
  that's the baseline the other two are measured against.
- **Parts 3 and 4 change each time.** The Starting Line asks what students have
  done before, how they learn best, and what they're excited or worried
  about. The Halfway Check-in and Finish Line ask students to rate each lesson
  they've had (did you like it? how hard was it?), plus the pace, whether the
  team shares jobs fairly, and whether they feel safe in the class. The
  Finish Line adds whether they'd recommend the course and whether they want
  to keep going. Each ends with open questions in the students' own words.

The wording lives in one place, `generator/questionnaires.py`, and the three
forms and the tracker all read from it. **Don't reword a Part 1 or Part 2
item partway through a cohort.** If you do, the growth numbers for that item
stop meaning anything.

### Explorer codes, not names

In Lesson 1, give each student a code: their team number plus a letter (3A,
3B, 3C), on a sticky note. They copy it onto the Mission Log cover (there's a
line for it) and onto every questionnaire. Keep the only list that matches
codes to names yourself. Codes let you follow each student from start to
finish, while students can answer honestly knowing their name isn't on the
page. The letter to families explains this.

### Giving it

- Read every question aloud to the whole room, at a steady pace. You're
  measuring what students know and think, not how fast they read.
- Don't explain or hint at Part 2. The answer to every question is "if you're
  not sure, tick I don't know yet".
- Collect the forms straight away, and don't read them in front of the class.
- A student who is away that day fills it in at the start of the next lesson,
  with the real date on it.
- Any **No** to "I feel safe in this class" deserves a quiet follow-up within
  the week. Use your code list.

### Entering the answers

Make a copy of [`handouts/questionnaire_tracker.xlsx`](handouts/questionnaire_tracker.xlsx)
for each cohort. Type each form into the **Answers** sheet as one row; the
**Start here** sheet says how (Part 1 is 1 to 4, Part 2 is 1 for right and 0
for wrong or "I don't know yet", and so on), and has the Part 2 answer key.
It takes about two minutes a form. Yellow cells are for typing; everything
else works itself out:

- **Summary** shows the whole class at Start, Halfway and Finish: the average
  for every Part 1 sentence and group, the share answering every Part 2
  question right, each lesson's "liked it" and "how hard" averages with a flag
  (*Too hard?*, *Too easy?*, *Not landing?*), and the pace, fairness, safety
  and recommendation answers. Two charts show Part 1 and Part 2 by checkpoint.
- **Growth** shows each student: type each Explorer code once in column A,
  and it fills in their Part 1 average and Part 2 score at each checkpoint,
  and how much each changed.

Copy any Part 4 answer worth talking about into the **NOTES** column, word for
word. The grey EXAMPLE rows show the format and are left out of the Summary.

### The cohort review: refining the lessons together

Twice a cohort, the week after the Halfway Check-in and the week after Mission
Day, whoever teaches the course meets for half an hour with the Summary sheet
open:

1. **How many forms came back?** If it's well under the class size, go easy on
   the numbers.
2. **Part 2.** Which questions are most students still getting wrong? At the
   Finish, each one points at a lesson to strengthen (its lesson number is
   beside it).
3. **Part 1.** What moved and what didn't? Falling interest or flat confidence
   is worth a closer look.
4. **Lessons.** Which are flagged *Too hard?*, *Too easy?* or *Not landing?*
5. **Notes.** Read the students' own words out loud. They are often the most
   useful thing in the workbook.
6. **Decide on no more than three changes,** and write them in the change log
   below, with what in the questionnaires led to each.

The Halfway review is the one that can still help this cohort. Pick one change
you can make in Lessons 8 to 12, make it, and tell the class it came from
their answers.

With a class of twelve, every number is a conversation starter rather than a
statistic: look for big changes and for things many students agree on. These
are our own questions, written for this course, not a validated research
instrument.

### Course change log

| Date | Cohort | What we changed | Why: what the questionnaires showed |
|---|---|---|---|
| | | | |

---

## Safety

The **Explorer Code** (Lesson 1, and the Mission Log) is seven rules, and every
student signs it before driving:

1. Wheels up whenever the cable is plugged in.
2. Hands, hair and sleeves away from the wheels while the power is on.
3. Power off before picking it up; carry it by the bottom plate, never the wires.
4. Drive on the floor, inside the arena, never on a table.
5. Batteries and chargers are for grown-ups only.
6. A puffy, hot or damaged battery, or a burning smell: power off, step back,
   tell a grown-up.
7. Don't stare into the lights.

The Crew Chief can call STOP at any time.

For you, beyond the Code:

- The battery is a **4S lithium polymer pack, about 16 volts**. Only adults
  charge, connect or swap batteries. Charge on a non-flammable surface,
  supervised, never overnight. Retire any pack that is swollen, dented or hot.
- Every Explorers program stops the motors the moment the controller
  disconnects.
- In the autopilot programs, **holding B stops the vehicle** and cancels the
  rest of the route. B is for Brake. Lesson 12 teaches it, and it is worth
  showing earlier if a team's route is going wrong.
- The engine room caps LED brightness well below what the lights can do, to
  protect eyes and the battery.

---

## The engine room: `explorer.h`

Every sketch folder has a copy of `explorer.h`, which opens as a second tab in
the IDE. Its header comment lists everything a student can call, in plain
language, and it's written to be read by a curious student. What it does for
you:

- **Motors.** `drive(left, right)` in percent, the autopilot moves
  (`forward(ms)`, `spinRight(ms)` and so on), `countdown()`, and the speed
  limit.
- **Lights.** `setLight()`, `setFrontLights()`, `setLeftLights()`, named colors,
  `mixColor()`, `rainbowColor()`, `behind()`. Students never call `show()`.
  The engine room sends a picture to the strip only once the program has
  finished drawing it, which keeps the lights from flickering and keeps the
  Bluetooth radio fed.
- **The controller.** `controllerReady()` at the top of `loop()` connects to
  the claimed controller, stops the motors and blinks green when the controller
  isn't there, and works out which buttons were tapped. Buttons are named by
  the letter **printed on a Switch pad** (`buttonA()` is the right-hand
  button), not by Bluepad32's Xbox-style positions.
- **Timing.** `blinkIsOn()` and `timeToPrint()`, so students can blink and
  print without `delay()`.

**There are fifteen copies, and they must agree.** Edit the master,
[`src/lessons/explorers/explorer.h`](../../../src/lessons/explorers/explorer.h),
then run `sync_explorer_h.py` to copy it into every sketch. The deck build
refuses to run while any copy differs.

---

## When something goes wrong

| What you see | Most likely | Do this |
|---|---|---|
| Lights blink **red** | The vehicle has no claimed controller | Run `e00_claim_controller` on it |
| Lights blink **green** and never stop | Its controller is off, flat, or somebody else's | Press a button on the right controller. Check the stickers. Charge it. |
| No port in *Tools > Port* | Vehicle switched off, or a charge-only cable | Switch it on. Swap the cable. |
| Upload stuck on `Connecting...` | Some ESP32 boards need a nudge | Hold the ESP32's BOOT button until the percentage starts |
| Nonsense on the Serial Monitor | Wrong speed | Set the monitor to 115200 baud |
| A pile of errors that make no sense | The wrong board package | *Tools > Board > esp32_bluepad32 > ESP32 Dev Module* |
| The wrong button does something | A controller that prints its letters differently | Run `e00_claim_controller` and press each button; it names them |
| X never buzzes | Some controllers have no rumble motor | Nothing to fix; use lights instead |
| First upload takes a minute | It compiles the Bluetooth library | Warn the class. It's faster after that. |
| `e02b_bug_hunt` won't compile | That's the lesson | Three deliberate bugs. Don't fix it for them. |

---

## Photos and screenshots still needed

Each of these appears on its slide as a dashed **PLACEHOLDER** box saying what
belongs there, so they are visible on screen and on paper until they're
supplied. Save the picture in [`../images/`](../images/) and replace the
`Placeholder(...)` in `generator/content_explorers.py` with
`img("your-file.jpg")`. Phone photographs need cropping first: the phone shoots
square, and uncropped they are mostly floor.

| # | Deck, slide | What to take |
|---|---|---|
| 1 | L01 s10, L07 s9 | **Download** a NASA photograph of the Sojourner rover on Mars (1997). NASA images are generally free to use in teaching. Search "Sojourner" at photojournal.jpl.nasa.gov. |
| 2 | L01 s15, L08 s6 | **Photo:** a vehicle and its controller with matching number stickers, close enough to read both numbers |
| 3 | L01 s19 | **Photo:** a vehicle on its wheels-up stand, from the side, so the gap under the wheels shows |
| 4 | L01 s19 | **Photo:** the vehicle's power switch, close up, with ON marked |
| 5 | L01 s22 | **Photo:** the driving arena on the floor, from a chair, the whole course in frame |
| 6 | L02 s9 | **Screenshot:** the Arduino IDE with `e02a_hello_rover` open, Verify, Upload, the Serial Monitor button and the `explorer.h` tab circled |
| 7 | L02 s15 | **Screenshot:** the Serial Monitor at 115200 baud, showing three or four "Hello! I am a Pathfinder rover." lines |
| 8 | L02 s17 | **Screenshot:** the IDE after clicking Verify on `e02b_bug_hunt`, the first error message visible |
| 9 | L03 s14 | **Photo:** lights 0 to 6 running `e03a_color_lab`, in a dim room |
| 10 | L06 s12 | **Photo:** the tape square on the floor, START marked with which way the vehicle faces |
| 11 | L07 s14 | **Photo:** the Mission to Mars course: START, crater, SAMPLE zone, grid |
| 12 | L12 s7 | **Photo:** the Mission Day course from above. Take it on the day if there is no earlier chance. |

No photographs of students are needed anywhere in the course.

---

## Rebuilding

**The `.pptx`, `.docx` and `.xlsx` files are the deliverable.** Once anybody
has edited one in PowerPoint, Word or Excel, that file is the source of truth,
and rebuilding overwrites it. Rebuilding never touches a cohort's own copy of
the tracker, as long as that copy lives somewhere other than `handouts/`. The generator is here so the first version is reproducible.

From `ground-vehicle/docs/courses/generator/`:

```bash
python build_decks.py explorers
```

Builds only the Explorers decks. **Name the course.** With no argument the
script rebuilds every course, overwriting hand edits in the other folders.

```bash
python build_handouts.py
```

Builds the Mission Log, the family letter, the blank certificate, the three
questionnaires and the questionnaire tracker. It reads badge names and the
glossary out of the built decks, so build the decks first. The questionnaire
wording comes from `questionnaires.py`.
For a whole class of certificates, put one name per line in a text file:

```bash
python build_handouts.py --names names.txt
```

After editing `explorer.h`:

```bash
python sync_explorer_h.py
```

The usual checks cover this course too: `lint_decks.py` (text overflow),
`check_code.py` (every code line on a slide exists in a sketch), and
`render_decks.py --sheets explorers` (PDFs and contact sheets to look at).

To compile every Explorers sketch with the `arduino-cli` bundled in the Arduino
IDE:

```bash
arduino-cli compile --fqbn esp32-bluepad32:esp32:esp32 --warnings all ../../../src/lessons/explorers/e05a_motor_lab
```

Add `--libraries <your Arduino libraries folder>` if the CLI cannot find
Adafruit NeoPixel.

---

## Status

- **Compiled, not yet run on a vehicle.** All fourteen teaching sketches and
  `e00_claim_controller` compile with no warnings against esp32_bluepad32
  4.1.0; `e02b_bug_hunt` fails with exactly its three intended bugs. The engine
  room is new code. Its background light sender, the saved controller claim,
  and B-to-cancel autopilot all need a hardware check before the first class.
- **The button letters assume a Nintendo-layout pad** (A on the right, B at the
  bottom). That matches `pathfinder_nintendoswitch`. `e00_claim_controller`
  prints each button's name so you can check a box of controllers in a minute.
- **Every timing is an estimate.** One hour is tight for Lessons 2, 7 and 11.
  If something has to give, give the quiz, never the pack-up.
- **The questionnaires have not been used with students yet.** The tracker's
  formulas were checked against a set of made-up answers worked out by hand,
  recalculated in LibreOffice. They have not been opened in Excel or Google
  Sheets yet.
- **Twelve pictures are outstanding** (table above).
- The letter to families has blanks for dates, place and contacts, and a
  photo-permission slip to delete if your organization has its own.
