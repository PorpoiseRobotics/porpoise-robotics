"""
content_advanced.py - the five advanced lessons, built around
Pathfinder_Op_Program12.

This course assumes the student has already driven a Pathfinder and can read
C++. It is about how a real embedded program is organised, and about the
handful of ideas that separate a sketch from a product: subsystem separation,
non-blocking state machines, persistent settings, hardware self-test, and
knowing why each bug in the previous version was a bug.
"""

import os

import diagrams
from slidelib import Deck, Placeholder

IMAGES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "images")


def img(name):
    return os.path.normpath(os.path.join(IMAGES, name))


BYLINE = [
    "PORPOISE ROBOTICS  -  Precision Oceanographic Program On and In the Sea Environment",
    "porpoiserobotics.org",
    "",
    "Kevin Bowen, President  -  kbowen6@icloud.com",
    "Malcom Graham, VP Education      Louis Parker, VP Technology",
    "Valen Farre, System Engineer      Gary Howland, Animation",
    "Eddie Revollo, Artificial Intelligence",
    "Interns: Oscar Canazales, Krishnansh Vemulapalli, Soham Gangal, Erik Olsen",
]

LOGO = img("porpoise-logo.png")

TRACK_LABEL = "Pathfinder Advanced - Op Program 12"

SRC = "ground-vehicle/src/lessons/advanced"


# The stages each lesson moves through, used by the progress markers.
STAGES = {
    "lesson1": [
        "Architecture",
        "Tabs and Config.h",
        "The console",
    ],
    "lesson2": [
        "The LEDC peripheral",
        "Resolution",
        "Decay and ramping",
    ],
    "lesson3": [
        "Callbacks and allowlist",
        "Locking a board",
        "EEPROM",
    ],
    "lesson4": [
        "State machines",
        "Measuring a redraw",
        "Turn signal geometry",
    ],
    "lesson5": [
        "I2C",
        "Finding the sensor",
        "Measuring a motor",
    ],
}

# ===================================================================
# LESSON 1
# ===================================================================

def lesson1(deck):
    deck.title_slide(
        "How a real embedded program is put together: tabs, headers, "
        "capability flags, and a console you can talk to.",
        BYLINE + ["", "Three hours.  Assumes you have driven a Pathfinder before."],
        hero_image=img("vehicle-top-plate-esp32.jpg"), logo=LOGO,
        speaker=[
            "Check the room: everybody here should have driven a Pathfinder "
            "and be comfortable reading C++. If somebody is not, pair them "
            "up now.",
            "Have Op Program 12 open on the projector before you start. "
            "They will be looking at it all day.",
            "Set expectations in the first minute: no new features. This is "
            "about how the program is built.",
        ])

    deck.objectives([
        "Explain how the Arduino IDE assembles a multi-tab sketch, and "
         "why a type has to live in a header",
        "Split a program by subsystem without breaking it",
        "Say why the program detects its hardware rather than being "
         "told about it",
        "Write a serial console that does not block the vehicle",
        "Read the boot sequence of Op Program 12 and justify its order",
    ],
        speaker=[
            "Objective one is the one that will bite in the activity. The "
            "others are conceptual and get earned across the lesson.",
        ])

    deck.bullets(
        "Who this course is for, and what it is not",
        [("You have driven a Pathfinder. You can read C++: functions, loops, "
          "structs, pointers, and you are not frightened of a header file.", 0),
         ("", 0),
         ("This course is NOT more features. The vehicle does the same things "
          "it did on the beginner programs.", 0),
         ("", 0),
         ("It is about how the program is BUILT: how you split 1300 lines so "
          "that somebody else can find things in it, how you make animations "
          "that never block the motors, how you store settings that survive a "
          "power cycle, and how you make a machine test its own hardware.", 0),
         ("", 0),
         ("Along the way you will meet six real bugs from the previous version "
          "and see exactly why each one was wrong.", 0)],
        note="Every idea here transfers. None of it is specific to robots - it "
             "is how embedded software is written everywhere.",
        speaker=[
            "Be blunt about what this course is not. Some people arrive "
            "expecting more capability and are disappointed until you "
            "reframe it.",
            "The six real bugs are the hook. Real code, real mistakes, and "
            "the reasons each one was wrong.",
            "The note matters: none of this is robot-specific. It is how "
            "embedded software is written everywhere, and that is why it is "
            "worth three hours.",
        ])

    deck.table(
        "Where this course goes",
        ["Lesson", "Subject", "The idea underneath it"],
        [["1", "Architecture and the toolchain", "Separate subsystems, and a way to talk to them"],
         ["2", "Motors, resolution and ramping", "Control quality, and integer maths that lies"],
         ["3", "Bluetooth and persistent storage", "Identity that survives a power cycle"],
         ["4", "The lighting state machine", "Never block, and never redraw for nothing"],
         ["5", "Sensors, self-test, diagnostics", "A machine that can check itself"]],
        col_widths=[0.9, 4.3, 5.8],
        speaker=[
            "The right-hand column is the point. Each lesson has one "
            "transferable idea underneath the vehicle-specific detail.",
        ])

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "What changed from Op 11.2, and why"],
         ["0:30", "One binary, two vehicles: capability flags"],
         ["0:50", "How Arduino tabs really work, and why Config.h is a header"],
         ["1:15", "BREAK  (10 minutes)"],
         ["1:25", "Upload a1a_tabs_and_config. Break it on purpose"],
         ["1:55", "A serial console: reading without blocking, and parsing"],
         ["2:15", "BREAK  (10 minutes)"],
         ["2:25", "Upload a1b_serial_console"],
         ["2:45", "The boot sequence of Op Program 12, line by line"]],
        col_widths=[1, 9],
        speaker=[
            "Two breaks. The tabs activity at 1:25 is the one that overruns "
            "- it is worth the time, so take it out of the boot-sequence "
            "walkthrough if you have to.",
        ])

    deck.table(
        "Op Program 11.2 to 12  -  what changed",
        ["Change", "Why"],
        [["FastLED replaced with Adafruit NeoPixel",
          "FastLED 3.10.5 auto-enables an ESP-DSP FFT backend that does not "
          "build against the bluepad32 4.1.0 SDK. 11.2 stopped compiling."],
         ["Servo travel corrected to 1.0 - 2.0 ms",
          "11.2 used 0.5 - 2.5 ms, which can drive a servo into its end stops."],
         ["Paired address saved to EEPROM",
          "11.2 only added to the allowlist at runtime, so a reboot could leave "
          "a vehicle refusing its own controller."],
         ["Addresses print most significant byte first",
          "11.2 printed them backwards, so the address it told you to use was "
          "the reverse of the real one."],
         ["Speed ramping always reaches its target",
          "Integer truncation stalled the old ramp one count short, forever."],
         ["Self-test compares against the measured baseline",
          "11.2 measured a baseline, printed it, then used fixed absolutes."],
         ["Standby lighting redrawn only when it changes",
          "11.2 pushed 32 LEDs on every pass of loop()."],
         ["One 1292-line file split into eight",
          "So that \"where does pairing happen\" has a one-word answer."]],
        col_widths=[3.6, 7.4],
        size=12,
        note="Read that list again at the end of the course. Most of them are "
             "mistakes that are easy to make and hard to see.",
        speaker=[
            "This is a real change log from a real project. Treat it as a "
            "case study, not a list.",
            "The FastLED row is worth dwelling on: a library minor version "
            "broke the build through a dependency nobody chose. That "
            "happens constantly in the field.",
            "The servo travel row is a safety bug - 0.5 to 2.5 ms can drive "
            "a servo into its end stops and cook it.",
            "The address-printing bug is the cruellest kind: the program "
            "told you the wrong answer confidently.",
            "Ask which of these they would have caught in review. Most "
            "people say the servo one and miss the integer truncation.",
        ])

    deck.two_columns(
        "One binary, two vehicles",
        "The problem",
        [("Gen 2 vehicles have no current sensor. Gen 3 vehicles do.", 0),
         ("Two separate builds means two things to keep in step, and somebody "
          "eventually flashes the wrong one.", 0),
         ("", 0),
         ("The answer: DETECT the hardware at boot and switch behaviour on what "
          "is actually there.", 0)],
        "How Op 12 does it",
        [("Wire.begin(SDA, SCL);", 1),
         ("if (is_ina219_present()) {", 1),
         ("  capabilities = CAP_SELF_TEST | CAP_CURRENT_MON;", 1),
         ("} else {", 1),
         ("  capabilities = 0;", 1),
         ("}", 1),
         ("", 0),
         ("Bit FLAGS, not booleans, so one byte can carry several independent "
          "capabilities and a test is a single & operation.", 0),
         ("Every feature that needs the sensor checks its flag first and is a "
          "no-op without it.", 0)],
        size=14,
        note="Detect what is there rather than being told what is there. A "
             "configuration that can disagree with the hardware eventually will.",
        speaker=[
            "Two vehicles, one binary. Set the problem up before showing "
            "the solution.",
            "Two separate builds is the obvious answer and the wrong one. "
            "Ask why - somebody eventually flashes the wrong image.",
            "Detect what is there rather than being told what is there. "
            "That sentence is the transferable idea.",
            "Bit flags rather than booleans: one byte carries several "
            "independent capabilities and a test is a single & operation. "
            "Show the arithmetic if anybody is shaky on bitwise operators.",
        ])

    deck.bullets(
        "How Arduino tabs actually work",
        [("Every .ino file in the sketch folder is a tab in the IDE.", 0),
         ("Before compiling, the IDE CONCATENATES them into one file - the one "
          "named after the folder first, then the rest alphabetically - and "
          "generates function prototypes at the top.", 0),
         ("", 0),
         ("Two consequences you have to know:", 0),
         ("", 0),
         ("Functions and globals are shared across tabs with NO header needed. "
          "Lighting.ino can call strip and read NUM_LEDS with no include.", 1),
         ("", 0),
         ("TYPES are not. The generated prototypes go ABOVE your code, so a "
          "prototype mentioning an enum or struct you declared in a tab will "
          "not compile - the type is not defined yet at that point.", 1),
         ("", 0),
         ("That is precisely why Config.h is a real header rather than a ninth "
          "tab. A header is #included by the main sketch, so everything in it "
          "is defined before the generated prototypes appear.", 0)],
        lead="Convenient, and exactly one trap",
        note="Op 12's Config.h opens with a paragraph saying this. Now you know "
             "what it is warning you about.",
        speaker=[
            "This is the slide the activity is built on. Go slowly.",
            "The IDE concatenates the tabs and generates prototypes ABOVE "
            "your code. Draw that on the board - the ordering is the whole "
            "explanation.",
            "Functions and globals cross tabs freely. Types do not, because "
            "the generated prototype mentions a type that is not defined "
            "yet at that point.",
            "That is precisely why Config.h is a real header. Not style - "
            "necessity.",
        ])

    deck.table(
        "The eight files, and what each one owns",
        ["File", "Owns"],
        [["Pathfinder_Op_Program12.ino", "setup(), loop(), the hardware objects, the shared state"],
         ["Config.h", "Every tunable number and every custom type"],
         ["Motors.ino", "PWM setup, drive modes, ramping, servos, throttle modes"],
         ["Lighting.ino", "Colour helpers, every animation, the lighting state machine"],
         ["Bluetooth.ino", "Connection callbacks, the allowlist, the pairing workflow"],
         ["Console.ino", "The serial command interface, EEPROM settings, diagnostics"],
         ["Sensors.ino", "The INA219 driver"],
         ["SelfTest.ino", "The powered motor check"]],
        lead="One subsystem per file. The rule that makes it work: each tab owns "
             "its own state, and other tabs go through its functions.",
        col_widths=[3.4, 7.6],
        size=13,
        note="Nothing in the compiler enforces that. Every tab can see every "
             "global. It is a convention, and it is the only thing standing "
             "between you and 1292 lines again.",
        speaker=[
            "Eight files, one subsystem each. Read the table, then say the "
            "rule underneath it: each tab owns its own state and others go "
            "through its functions.",
            "The note is the honest part: nothing in the compiler enforces "
            "this. Every tab can see every global. It is a convention held "
            "by discipline.",
            "Ask what happens when somebody breaks the convention. The "
            "answer is that the program still compiles, which is exactly "
            "why it is dangerous.",
        ])

    deck.progress(STAGES["lesson1"], 1,
        speaker=[
            "Twenty seconds. First activity.",
        ])

    deck.activity(
        "Do it now  -  three files, one program",
        "a1a_tabs_and_config",
        [("1.  Open it. Look at the tab bar: the sketch, Config.h, Blinker.ino.", 0),
         ("2.  Upload it. It blinks and reports, and swaps speed every 5 s.", 0),
         ("3.  Change BLINK_SLOW_MS in Config.h. Notice you touched neither "
          ".ino file.", 0),
         ("4.  Add a fourth file, Reporter.ino, with a function in it. Call it "
          "from loop(). Notice you declared it nowhere.", 0),
         ("5.  Now MOVE the BlinkMode enum out of Config.h and into "
          "Blinker.ino. Compile. READ THE ERROR CAREFULLY.", 0),
         ("6.  Put it back.", 0)],
        expect=[("Step 5 fails with an error about BlinkMode not naming a type, "
                 "pointing at a line you did not write.", 0)],
        questions=[("Whose line is the error actually on?", 0),
                   ("Why does moving a FUNCTION between tabs work fine, but "
                    "moving a TYPE does not?", 0)],
        minutes=30,
        speaker=[
            "Step 5 is the whole point of this activity. Do not warn them "
            "what the error will look like.",
            "The error points at a line they did not write, in generated "
            "code. That experience is worth more than the explanation on "
            "the previous slide.",
            "Answer to the first question: the error is on a generated "
            "prototype line, not on any line in their file.",
            "Answer to the second: functions are only NAMED in the "
            "prototype, but a type has to be DEFINED before the prototype "
            "that mentions it.",
            "Thirty minutes, and it is worth every one of them.",
        ])

    deck.bullets(
        "Why a console changes what a vehicle is",
        [("A compiled binary is opaque. You can see what it does, not what it "
          "thinks.", 0),
         ("", 0),
         ("A serial console turns it into something you can interrogate:", 0),
         ("help      what can this thing do", 1),
         ("diag      what does it think is happening right now", 1),
         ("pair      change its behaviour without a laptop full of toolchains", 1),
         ("dynamics  retune the deadzones between races", 1),
         ("", 0),
         ("The practical difference: at a track event, tuning a vehicle stops "
          "being an edit-compile-upload cycle and becomes a conversation. On "
          "Op 12 those settings then go to EEPROM, so the tuning survives.", 0),
         ("", 0),
         ("This is not a toy idea. Industrial controllers, network switches and "
          "flight computers all have one, for exactly this reason.", 0)],
        lead="From opaque binary to something you can ask questions",
        speaker=[
            "A compiled binary is opaque. You can see what it does, not "
            "what it thinks. That is the framing.",
            "The four commands on the slide are the four things people "
            "actually want at a track event.",
            "Tuning stops being an edit-compile-upload cycle and becomes a "
            "conversation. That is a real productivity difference, not a "
            "nicety.",
            "Industrial controllers, network switches and flight computers "
            "all have one. This is standard practice, not a toy.",
        ])

    deck.code(
        "Reading a line without stopping the vehicle",
        ["void handleSerial() {",
         "  while (Serial.available() > 0) {",
         "    char c = (char)Serial.read();",
         "",
         "    if (c == '\\n' || c == '\\r') {      // accept both",
         "      if (input_line.length() > 0) {",
         "        runCommand(input_line);",
         "        input_line = \"\";",
         "      }",
         "      continue;",
         "    }",
         "",
         "    if (c >= 32 && c < 127 && input_line.length() < 80) {",
         "      input_line += c;",
         "    }",
         "  }",
         "}"],
        filename="a1b_serial_console.ino",
        notes=[("Serial.readStringUntil() would WAIT for a newline. While it "
                "waits the vehicle is not driving. Never use it on a moving "
                "machine.", 0),
               ("This takes whatever bytes have arrived and returns "
                "immediately, every time.", 0),
               ("The Serial Monitor sends whatever its line-ending dropdown "
                "says: nothing, LF, CR, or both. Handle all four or the console "
                "looks broken to whoever set theirs differently.", 0),
               ("The length cap matters. Without it a stuck sender grows the "
                "String until the ESP32 runs out of heap.", 0)],
        size=13, highlight={1, 4, 12},
        speaker=[
            "Serial.readStringUntil() WAITS. On a moving vehicle that is "
            "unacceptable. Say it plainly.",
            "This version takes whatever bytes have arrived and returns "
            "immediately, every time. That is the non-blocking pattern from "
            "beginner Lesson 5, applied to input.",
            "The line-ending handling looks like paranoia until you watch a "
            "console appear broken because the Serial Monitor dropdown was "
            "set differently.",
            "The printable-character and length checks are the input "
            "validation. Worth pointing out that they are not optional.",
        ])

    deck.progress(STAGES["lesson1"], 2,
        speaker=[
            "Twenty seconds. Second activity.",
        ])

    deck.activity(
        "Do it now  -  a console",
        "a1b_serial_console",
        [("1.  Upload it, open the Serial Monitor, type help.", 0),
         ("2.  Try every command. Then type something that is not a command.", 0),
         ("3.  Type  set rate abc. What happens? Why does the range check "
          "catch it?", 0),
         ("4.  Change your Serial Monitor line-ending setting to each of its "
          "four options. Does the console still work in all four?", 0),
         ("5.  Add a command of your own: toggle, that flips a bool.", 0),
         ("6.  Open Console.ino in Op Program 12 and find the same three "
          "problems being solved there.", 0)],
        expect=[("A prompt, echoed typing, working backspace, and commands that "
                 "run while the LED keeps blinking on its own schedule.", 0)],
        questions=[("toInt() returns 0 for anything it cannot parse. Why does "
                    "that make range checking essential rather than optional?", 0)],
        minutes=25,
        speaker=[
            "Step 4 is the one people skip. Make them try all four "
            "line-ending settings - it is the fastest way to appreciate the "
            "previous slide.",
            "Step 5 is the first thing they write themselves today.",
            "Step 6 sends them into the real program to find the same three "
            "problems solved there. That is the bridge to the rest of the "
            "course.",
            "The answer to the question: toInt() returns 0 for anything "
            "unparseable, so a typo silently becomes a valid-looking zero. "
            "Range checking is what catches it.",
        ])

    deck.code(
        "The boot sequence of Op Program 12",
        ["void setup() {",
         "  Serial.begin(115200);",
         "",
         "  EEPROM.begin(EEPROM_SIZE);",
         "  loadSettings();              // tuning, range checked",
         "",
         "  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);",
         "  if (is_ina219_present()) {   // which vehicle am I?",
         "    vehicle_config.capabilities = CAP_SELF_TEST | CAP_CURRENT_MON;",
         "    ina219_init();",
         "  } else {",
         "    vehicle_config.capabilities = 0;",
         "    test_state = TEST_DISABLED;",
         "  }",
         "",
         "  setup_motors();              // outputs safe FIRST",
         "  setup_servos();",
         "",
         "  strip.begin();               // then the lights",
         "  set_disconnected_lighting();",
         "",
         "  setup_bluetooth();           // then the radio",
         "",
         "  pinMode(PAIR_TRIGGER_PIN, INPUT_PULLUP);",
         "  pinMode(TEST_PIN, INPUT);",
         "}"],
        filename="Pathfinder_Op_Program12.ino",
        notes=[("The ORDER is deliberate.", 0),
               ("Settings first, because everything else may depend on them.", 0),
               ("Hardware detection next, because it decides what the rest of "
                "setup is even allowed to do.", 0),
               ("Motors before anything slow. An uninitialised H-bridge pin "
                "floats, and a floating pin can twitch a motor. Get the "
                "outputs into a known safe state as early as you can.", 0),
               ("Radio last. It is the slowest thing to come up and nothing "
                "else waits on it.", 0)],
        size=12, highlight={15, 16},
        speaker=[
            "The order is the lesson, not the code. Walk down it and ask "
            "why each step is where it is.",
            "Settings first, because everything after may depend on them. "
            "Then detect the hardware, because behaviour branches on it.",
            "Motors before lights, because getting the outputs into a safe "
            "state matters more than anything pretty.",
            "Radio last, because a controller connecting to a "
            "half-initialised vehicle is the worst possible moment.",
            "Ask them to justify a different order. Making them defend it "
            "is better than telling them.",
        ])

    diagrams.system_block(deck, controller="Bluetooth gamepad")

    deck.table(
        "The hardware this program is talking to",
        ["Part", "Where", "What Op 12 does with it"],
        [["ESP32 DevKitC, 38 pin", "-", "240 MHz dual core, Bluetooth, 16 PWM channels"],
         ["4 x DRV8871 H-bridge", "12/13, 18/19, 22/23, 16/17", "10-bit PWM at 30 kHz, hybrid drive"],
         ["32 x WS2812B", "GPIO 5", "One loop: 0-15 front, 16-31 rear"],
         ["4 x servo header", "25, 26, 27, 14", "50 Hz, 1.0 to 2.0 ms, one per stick direction"],
         ["INA219 current sensor", "I2C on 32 / 33", "Gen 3 only. Detected at boot."],
         ["BOOT button", "GPIO 0", "Held low to enter pairing mode"],
         ["Self-test jumper", "GPIO 34", "Input-only pin, external pull-down"],
         ["4S LiPo, 3300 mAh", "-", "About 16 V charged"]],
        lead="Nothing here is new to you. What is new is that one binary has to "
             "cope with two different versions of it.",
        col_widths=[3, 3, 5],
        size=12,
        note="GPIO 34-39 are INPUT ONLY on the ESP32 and have no internal pull "
             "resistors. That is why the self-test pin needs one on the board.",
        note_kind="warn",
        speaker=[
            "This is reference, not teaching. Point at it and move on.",
            "The note is the one thing to say out loud: GPIO 34 to 39 are "
            "input only and have no internal pull-up, which is why the "
            "self-test jumper needs an external pull-down.",
            "Worth printing for the rest of the course.",
        ])

    deck.bullets(
        "Reading a program you did not write",
        [("You are about to be handed 1300 lines across eight files. Reading it "
          "front to back is the slowest possible way in.", 0),
         ("", 0),
         ("A better order:", 0),
         ("1.  The header comment. It tells you the hardware, the controls, "
          "and - in this case - the change log.", 1),
         ("2.  Config.h. Every number the program cares about, in one place. "
          "You learn what it can do from what it lets you tune.", 1),
         ("3.  setup(). What has to be true before anything runs.", 1),
         ("4.  loop(). The whole shape of the program in one screen.", 1),
         ("5.  Only then, the subsystem that interests you.", 1),
         ("", 0),
         ("Notice what that order gives you: the vocabulary first, then the "
          "skeleton, then the detail. Going the other way means reading "
          "Lighting.ino with no idea what led_mode is or who sets it.", 0),
         ("", 0),
         ("EXERCISE, ten minutes. Open Pathfinder_Op_Program12, read in that "
          "order, and write down three questions. We will answer them over the "
          "next four lessons.", 0)],
        lead="Header, then Config.h, then setup(), then loop()",
        speaker=[
            "This is the most useful slide in the lesson for anybody who "
            "will read somebody else's code again - which is all of them.",
            "Vocabulary first, then skeleton, then detail. Say why: reading "
            "Lighting.ino with no idea what led_mode is or who sets it is "
            "wasted time.",
            "Run the ten-minute exercise properly. Time it, and get answers "
            "from three different groups.",
            "Point out that Config.h tells you what the program can DO, "
            "because it tells you what somebody thought was worth tuning.",
        ])

    deck.quiz(
        "Check yourself",
        [("1.  Why is Config.h a header file rather than a ninth tab?", 0),
         ("2.  In what order does the Arduino IDE concatenate your tabs?", 0),
         ("3.  Why does Op 12 detect the current sensor rather than being told "
          "which generation it is running on?", 0),
         ("4.  Why are capabilities bit flags rather than separate booleans?", 0),
         ("5.  Name two things wrong with Serial.readStringUntil() on a moving "
          "vehicle.", 0),
         ("6.  Why does setup() initialise the motors before the LEDs?", 0),
         ("7.  Every tab can see every global. Why is that both the convenience "
          "and the danger?", 0)],
        lead="Next lesson: motors, and integer maths that quietly lies to you.",
        speaker=[
            "Conversation, not a test.",
            "Answers: Config.h is a header because generated prototypes sit "
            "above the code and a type must already be defined; tabs "
            "concatenate with the folder-named file first and the rest "
            "alphabetically; detection avoids a configuration that can "
            "disagree with reality.",
            "Bit flags let one byte carry several independent capabilities "
            "and make a test a single & operation.",
            "readStringUntil blocks, and it also depends on a timeout you "
            "did not choose.",
            "Motors before LEDs so the outputs are safe before anything "
            "else happens.",
            "The last question has no clean answer, and that is the point - "
            "shared globals are convenient exactly because nothing stops "
            "you.",
        ])

    return deck


# ===================================================================
# LESSON 2
# ===================================================================

def lesson2(deck):
    deck.title_slide(
        "Resolution against frequency, four states of an H-bridge, fast "
        "decay, and a ramp that used to stall one count short.",
        BYLINE + ["", "Three hours.  Wheels off the ground throughout."],
        hero_image=img("vehicle-rear-battery.jpg"), logo=LOGO,
        speaker=[
            "Blocks on every desk. Everything today runs a motor at up to "
            "full power and none of it should touch the floor.",
            "Have a multimeter to hand for the voltage column in the first "
            "activity.",
        ])

    deck.objectives([
        "Work out the maximum PWM frequency for a given resolution, and "
         "say what more resolution buys you",
        "Describe the four states of an H-bridge and what the motor "
         "does in each",
        "Explain why fast decay holds speed better under load than slow "
         "decay",
        "Spot the integer-truncation bug in a ramp, and fix it",
        "Explain why remap_axis constrains before it maps",
    ],
        speaker=[
            "Objective four - spotting the truncation bug - is the one they "
            "will remember. It is a real bug from a real program.",
        ])

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. The LEDC peripheral and what it can actually do"],
         ["0:25", "Resolution against frequency. Upload a2a_pwm_resolution"],
         ["1:00", "BREAK  (10 minutes)"],
         ["1:10", "Coast, brake, and the two decay modes"],
         ["1:40", "Upload a2b_coast_brake_hybrid. Feel the difference"],
         ["2:10", "BREAK  (10 minutes)"],
         ["2:20", "Speed ramping, and the truncation bug"],
         ["2:35", "remap_axis: deadzones, scale, and per-axis tuning"],
         ["2:50", "Four servos on one thumbstick"]],
        col_widths=[1, 9],
        speaker=[
            "Two breaks. The first activity is thirty-five minutes and it "
            "earns them; do not compress it.",
        ])

    deck.bullets(
        "The LEDC peripheral, and the trade you cannot avoid",
        [("The ESP32 generates PWM in hardware. Sixteen channels, driven from "
          "an 80 MHz clock.", 0),
         ("", 0),
         ("At N bits of resolution the duty range is 0 to 2^N - 1:", 0),
         ("8 bits   ->  0..255      256 steps    0.39% per step", 1),
         ("10 bits  ->  0..1023    1024 steps    0.098% per step", 1),
         ("12 bits  ->  0..4095    4096 steps    0.024% per step", 1),
         ("", 0),
         ("More bits means finer control near the BOTTOM of the range, which is "
          "exactly where a motor is hardest to drive smoothly. On a vehicle "
          "that ramps its speed, that is the difference between a smooth start "
          "and a visible staircase.", 0),
         ("", 0),
         ("It is not free. The counter has to run 2^N times per cycle:", 0),
         ("max frequency = 80,000,000 / 2^bits", 1),
         ("8 bits -> 312 kHz     10 bits -> 78 kHz     12 bits -> 19.5 kHz", 1),
         ("", 0),
         ("Op 12 runs 10 bits at 30 kHz. Inside the limit, above hearing, and "
          "four times the resolution of the beginner programs.", 0)],
        lead="Bits cost frequency",
        note="ledcSetup() returns the frequency it managed to configure, or 0 if "
             "the combination is impossible. Check it. Silence is otherwise very "
             "hard to debug.",
        note_kind="warn",
        speaker=[
            "This is a genuine engineering trade-off, so present it as one "
            "rather than as a fact to memorise.",
            "More bits buys finer control at the BOTTOM of the range, which "
            "is exactly where a motor is hardest to drive smoothly.",
            "The cost is frequency: the counter has to run 2^N times per "
            "cycle. Do the division on the board.",
            "Op 12 sits at 10 bits and 30 kHz. Ask them to check that "
            "against the formula before you confirm it.",
            "Connect it back: the beginner course ran 8 bits, and this is "
            "what the extra two bits are for.",
        ])

    deck.progress(STAGES["lesson2"], 1,
        speaker=[
            "Twenty seconds. First activity.",
        ])

    deck.activity(
        "Do it now  -  resolution you can hear",
        "a2a_pwm_resolution",
        [("1.  Wheels off the ground. Upload. Type help.", 0),
         ("2.  Run  step  at 8 bits. Watch the volts column and LISTEN at the "
          "very bottom of the range.", 0),
         ("3.  bits 10, then  step  again. Compare.", 0),
         ("4.  Try  bits 12  then  freq 30000. Read what the console tells you.", 0),
         ("5.  Find the lowest  pct  at which the wheel turns at all. Is it the "
          "same at 8 bits and at 10?", 0),
         ("6.  freq 300, listen, then freq 20000.", 0)],
        expect=[("A duty value, a percentage, and an approximate voltage for "
                 "each step of the sweep.", 0)],
        questions=[("Why does 12 bits at 30 kHz get refused?", 0),
                   ("Does more resolution change the lowest speed the motor "
                    "will turn at, or only how finely you can approach it?", 0)],
        safety="Wheels off the ground. This reaches full power.",
        minutes=35,
        speaker=[
            "The console is the tool for the whole activity. Make sure "
            "everybody has typed help and got a response before moving on.",
            "Step 2 and 3 are a listening exercise as much as a reading "
            "one. The bottom of the range is where the difference lives.",
            "Step 4 is refused by the console on purpose. 12 bits caps out "
            "at 19.5 kHz, so 30 kHz is impossible - and the console says so "
            "rather than silently doing something else.",
            "The second question is the subtle one: more resolution does "
            "NOT lower the speed at which the motor starts turning. It only "
            "lets you approach it more finely.",
        ])

    diagrams.h_bridge(deck)

    deck.two_columns(
        "Slow decay against fast decay",
        "Sign-magnitude  (the beginner programs)",
        [("Pulse IN1 at the duty you want. Hold IN2 low.", 0),
         ("The motor alternates between DRIVEN and COAST.", 0),
         ("During the off part nothing controls the motor at all, so the "
          "current in the windings decays slowly and the actual speed depends "
          "heavily on the load.", 0),
         ("", 0),
         ("Simple to read, and perfectly good for learning.", 0),
         ("", 0),
         ("This is SLOW DECAY.", 0)],
        "Hybrid drive  (Op Program 12)",
        [("Hold IN1 HIGH. Pulse IN2 at the inverse of the duty.", 0),
         ("The motor alternates between DRIVEN and BRAKE.", 0),
         ("The braking part actively pulls the current down, so the average is "
          "much closer to what you asked for, and speed holds up under load.", 0),
         ("", 0),
         ("Noticeably better control at low duty - exactly where you need it.", 0),
         ("", 0),
         ("This is FAST DECAY.", 0)],
        note="Watch the zero case. In hybrid, duty 0 puts PWM_MAX on BOTH pins, "
             "which is a hard brake, not a coast. Op 12 tests for zero "
             "separately and calls coast_all(). Miss that and the vehicle will "
             "not roll when you let go.",
        note_kind="warn",
        speaker=[
            "Both columns describe the same H-bridge. What changes is what "
            "happens during the off part of each cycle.",
            "Sign-magnitude alternates driven and coast. Hybrid alternates "
            "driven and brake, which actively pulls the current down.",
            "The practical consequence: hybrid holds the speed you asked "
            "for when the load changes. They will feel this in the "
            "activity.",
            "Sign-magnitude is not wrong. It is simpler to read, and that "
            "is why the beginner programs use it.",
        ])

    deck.code(
        "Hybrid drive, and the trap in it",
        ["void set_motor_hybrid(uint8_t in1_ch, uint8_t in2_ch, int duty) {",
         "  duty = constrain(duty, -PWM_MAX, PWM_MAX);",
         "",
         "  if (duty == 0) {           // WITHOUT THIS, ZERO IS A BRAKE",
         "    ledcWrite(in1_ch, 0);",
         "    ledcWrite(in2_ch, 0);",
         "    return;",
         "  }",
         "",
         "  int inverse = PWM_MAX - abs(duty);",
         "  if (duty > 0) {",
         "    ledcWrite(in1_ch, PWM_MAX);",
         "    ledcWrite(in2_ch, inverse);",
         "  } else {",
         "    ledcWrite(in1_ch, inverse);",
         "    ledcWrite(in2_ch, PWM_MAX);",
         "  }",
         "}"],
        filename="Motors.ino",
        notes=[("Full duty forward: IN1 = 1023, IN2 = 0. Driven hard.", 0),
               ("Half duty forward: IN1 = 1023, IN2 = 512. Half driven, half "
                "braked.", 0),
               ("Zero without the guard: IN1 = 1023, IN2 = 1023. Both high. "
                "That is a brake.", 0),
               ("", 0),
               ("A vehicle that will not coast, and cannot be pushed by hand "
                "when it is switched on, has this bug.", 0)],
        size=13, highlight={3, 4, 5, 6, 7},
        speaker=[
            "Walk the three cases on the right-hand notes before reading "
            "the code.",
            "The guard clause is the whole slide. Without it, duty zero "
            "puts both pins high, and both pins high is a brake.",
            "Ask what that feels like: a vehicle that will not coast and "
            "cannot be pushed by hand when switched on. That is a real "
            "symptom somebody would report as a mechanical fault.",
            "This is a good example of a bug that hides in the ONE case the "
            "arithmetic happens to handle differently.",
        ])

    deck.code(
        "The ramp, and the bug that hid in it for a year",
        ["// Op 11.2 wrote this:",
         "current += (target - current) * RAMP_FACTOR;",
         "",
         "// With RAMP_FACTOR = 0.5 and a gap of 1:",
         "//    1 * 0.5 = 0.5,  truncated to 0",
         "//    -> current never changes again",
         "//    -> the ramp stalls ONE COUNT short, forever",
         "",
         "// Op 12:",
         "int ramp_towards(int current, int target) {",
         "  int gap = target - current;",
         "  if (gap == 0) return current;",
         "",
         "  int step = (int)(gap * RAMP_FACTOR);",
         "  if (step == 0) step = (gap > 0) ? 1 : -1;   // the fix",
         "",
         "  return current + step;",
         "}"],
        filename="Motors.ino",
        notes=[("Ramping stops the vehicle lurching and stops it yanking a "
                "large current out of the battery on every stick movement.", 0),
               ("Closing a FRACTION of the gap each step gives a smooth "
                "exponential approach - fast at first, gentle at the end.", 0),
               ("The bug is invisible. One count out of 1023 is 0.1% of full "
                "power. Nobody would ever see it on a wheel.", 0),
               ("It matters because comparisons against the target then never "
                "become true, and anything waiting for \"ramp finished\" waits "
                "forever.", 0)],
        size=13, highlight={1, 14},
        speaker=[
            "Set the scene: this bug lived for a year in shipping code.",
            "Do the arithmetic out loud. Gap of 1, factor of 0.5, gives "
            "0.5, which truncates to 0 - so current never changes again.",
            "The ramp stalls one count short, forever. On a 1024-count "
            "scale that is 0.1%, which nobody can see on a wheel.",
            "Ask why it still matters. Because a value that never reaches "
            "its target breaks anything that tests for equality, and "
            "because a bug you cannot see is worse than one you can.",
            "The fix is one line: if the step rounds to nothing, take one "
            "count.",
        ])

    deck.progress(STAGES["lesson2"], 2,
        speaker=[
            "Twenty seconds. Second activity.",
        ])

    deck.activity(
        "Do it now  -  decay modes and ramping",
        "a2b_coast_brake_hybrid",
        [("1.  sign, then  go 100. Now pinch the tyre gently.", 0),
         ("2.  hybrid, then  go 100. Pinch it again. Which holds its speed?", 0),
         ("3.  Find the lowest  go  value that turns the wheel in each mode.", 0),
         ("4.  ramp off, then  go 1023  from a standstill. Then  ramp on  and "
          "do it again. Listen to the difference.", 0),
         ("5.  Set RAMP_FACTOR to 0.9, then 0.1. What does it change?", 0),
         ("6.  DELETE the minimum-step line in ramp_towards, upload, and run "
          "go 500. Watch where the printout stops.", 0)],
        expect=[("Step 6: the ramp prints its way down to one count short of "
                 "the target and then stops printing.", 0)],
        questions=[("Why does fast decay hold speed better under load?", 0),
                   ("Why would a 0.1% speed error still be a real bug?", 0)],
        safety="Wheels off the ground. One motor, up to full power.",
        minutes=35,
        speaker=[
            "Steps 1 and 2 are a feel test. Pinching the tyre gently is the "
            "load - warn them to be gentle and to keep fingers clear of the "
            "spokes.",
            "Step 6 recreates the bug deliberately. Watching the printout "
            "stop one count short is the memorable part of this lesson.",
            "Answer to the first question: fast decay actively pulls the "
            "winding current down during the off part, so the average "
            "tracks the command.",
            "Answer to the second: a 0.1% error is invisible but it means a "
            "value that never reaches its target, and anything comparing "
            "for equality never fires.",
        ])

    diagrams.pwm_duty(deck)

    deck.code(
        "remap_axis: deadzone, scale, and range, in one function",
        ["int remap_axis(int raw_value, int specific_dz, float scale_factor,",
         "               int in_max, int out_max) {",
         "  int dz = get_effective_deadzone(specific_dz);",
         "  if (abs(raw_value) < dz) return 0;",
         "",
         "  int sign   = (raw_value > 0) ? 1 : -1;",
         "  int scaled = lround(abs(raw_value) * scale_factor);",
         "  scaled = constrain(scaled, dz, in_max);",
         "",
         "  return sign * map(scaled, dz, in_max, 0, out_max);",
         "}"],
        filename="Motors.ino",
        notes=[("A per-axis deadzone of -1 means \"use the global one\" - a "
                "default and an override, with no second flag.", 0),
               ("lround(), not a cast, so 0.5 rounds up rather than "
                "truncating. The same class of bug as the ramp.", 0),
               ("constrain() BEFORE map(): map() does not clamp.", 0),
               ("Sign comes off at the start and goes back on at the end.", 0)],
        size=13, highlight={6, 7},
        speaker=[
            "Four ideas in ten lines. Take them one at a time.",
            "The -1 sentinel for a per-axis deadzone gives you a default "
            "and an override with no second flag. That is a pattern worth "
            "naming.",
            "lround() rather than a cast, so 0.5 rounds up. Point out that "
            "this is the SAME class of bug as the ramp, avoided this time.",
            "constrain() before map(), because map() does not clamp. Ask "
            "what happens if you do it the other way round.",
            "Sign comes off at the start and goes back on at the end, so "
            "the maths in the middle only ever deals with positive numbers.",
        ])

    deck.two_columns(
        "Throttle modes, and four servos on one stick",
        "Throttle modes  (L3 toggles)",
        [("NORMAL - full forward speed, steering scaled to 0.5.", 0),
         ("FAST   - full forward speed, full steering authority.", 0),
         ("", 0),
         ("Only the STEERING scale changes. Top speed is the same in both.", 0),
         ("", 0),
         ("This matters: it was the steering that needed softening, not the "
          "speed. A global speed cap makes a vehicle feel broken; a steering "
          "cap makes it feel controllable.", 0)],
        "Four servos, one right stick",
        [("Each servo owns one DIRECTION of the stick and sweeps its full "
          "travel across half the range:", 0),
         ("servo 1  -  stick up", 1),
         ("servo 3  -  stick down", 1),
         ("servo 2  -  stick left", 1),
         ("servo 4  -  stick right", 1),
         ("", 0),
         ("Each sits centred whenever the stick is not pushed its way, so one "
          "stick aims four independent things.", 0),
         ("", 0),
         ("Travel is 1.0 to 2.0 ms. 11.2 used 0.5 to 2.5, which can drive a "
          "servo into its mechanical end stops.", 0)],
        size=14,
        speaker=[
            "The left column is a design argument, not a feature list. Only "
            "the steering scale changes between the modes.",
            "The reasoning is worth hearing: a global speed cap makes a "
            "vehicle feel broken, a steering cap makes it feel "
            "controllable. Ask whether they agree.",
            "The four-servo scheme surprises people. One stick, four "
            "servos, each owning one direction and centred otherwise.",
            "1.0 to 2.0 ms is the corrected travel. Point back to the "
            "change log in Lesson 1 - the old range could drive a servo "
            "into its stops.",
        ])

    diagrams.servo_pulse(deck)

    deck.quiz(
        "Check yourself",
        [("1.  Why can you not have 12-bit resolution at 30 kHz on this chip?", 0),
         ("2.  What does more PWM resolution actually buy you, and where?", 0),
         ("3.  Describe the four states of an H-bridge and what the motor does "
          "in each.", 0),
         ("4.  Why does hybrid drive hold speed better under load?", 0),
         ("5.  In hybrid drive, what happens at duty 0 without the guard clause?", 0),
         ("6.  Explain the ramp truncation bug, and why nobody would see it on "
          "a wheel.", 0),
         ("7.  Why does remap_axis call constrain() before map() rather than "
          "after?", 0),
         ("8.  Why does NORMAL mode scale steering rather than top speed?", 0)],
        lead="Next lesson: identity that survives a power cycle.",
        speaker=[
            "Conversation, not a test.",
            "Answers: 12 bits caps at 19.5 kHz because 80 MHz divided by "
            "4096 is 19,531; resolution buys fine control at the bottom of "
            "the range; the four states are forward, reverse, coast and "
            "brake.",
            "Hybrid drive holds speed because the braking part actively "
            "pulls current down. At duty 0 without the guard, both pins go "
            "high and the motor brakes.",
            "The truncation bug leaves the ramp one count short, which is "
            "0.1% and invisible on a wheel.",
            "constrain before map because map does not clamp. NORMAL scales "
            "steering because steering authority was the problem, not top "
            "speed.",
        ])

    return deck


# ===================================================================
# LESSON 3
# ===================================================================

def lesson3(deck):
    deck.title_slide(
        "Allowlists, callbacks, EEPROM that is not EEPROM, and making a "
        "vehicle remember which controller is its own.",
        BYLINE + ["", "Three hours.  Bring two controllers per group if you can."],
        hero_image=img("control-board-bare.jpg"), logo=LOGO,
        speaker=[
            "Two controllers per group makes the allowlist activity far "
            "better. If you only have one each, pair groups up for that "
            "section.",
            "Charge every pad before the class. A flat controller looks "
            "exactly like a refused connection.",
        ])

    deck.objectives([
        "Explain what a callback is and when it runs",
        "Configure a Bluetooth allowlist correctly, in the right order",
        "Say what EEPROM.commit() actually does on an ESP32, and why it "
         "must not be called from loop()",
        "Design a storage map with a validity marker, and load it "
         "defensively",
        "Walk through the full pairing workflow end to end",
    ],
        speaker=[
            "Objective three - what commit() actually does - is the one "
            "that prevents a real hardware failure later. Do not let it "
            "slide by.",
        ])

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. Bluepad32, callbacks, and who calls whom"],
         ["0:25", "The allowlist, and the order the calls must go in"],
         ["0:50", "Upload a3a_controller_address. Lock and unlock a board"],
         ["1:20", "BREAK  (10 minutes)"],
         ["1:30", "There is no EEPROM on an ESP32"],
         ["1:55", "Magic bytes, storage maps, and never trusting storage"],
         ["2:15", "BREAK  (10 minutes)"],
         ["2:25", "Upload a3b_eeprom_settings. Pull the cable out"],
         ["2:50", "The full pairing workflow in Op 12"]],
        col_widths=[1, 9],
        speaker=[
            "Two breaks. Both activities are long and both are worth their "
            "time.",
        ])

    deck.bullets(
        "Callbacks: functions you write and never call",
        [("BP32.setup(&onConnectedController, &onDisconnectedController);", 1),
         ("", 0),
         ("You hand Bluepad32 the ADDRESSES of two of your functions. It calls "
          "them when something happens. You never call them yourself.", 0),
         ("", 0),
         ("That inversion is worth naming. Normally your code calls a library. "
          "With a callback, the library calls your code. It is how every event "
          "system works, from button handlers to network stacks.", 0),
         ("", 0),
         ("The practical question is always: WHEN does it run, and what is safe "
          "to do in there?", 0),
         ("", 0),
         ("Here, both callbacks run from inside BP32.update(), which you call "
          "yourself from loop(). So they run on your thread, at a moment you "
          "chose, and it is safe to print and to touch program state.", 0),
         ("", 0),
         ("If they ran from an interrupt, neither of those would be true.", 0)],
        lead="Inversion of control",
        speaker=[
            "Name the inversion out loud: normally your code calls a "
            "library, and with a callback the library calls your code.",
            "The practical question is always WHEN it runs and what is safe "
            "to do there. Ask that before you answer it.",
            "Here both callbacks run inside BP32.update(), which they call "
            "themselves from loop(). So it is their thread, at a moment "
            "they chose, and printing is safe.",
            "The last line is the important caveat. From an interrupt, "
            "neither of those would be true - and that is a whole category "
            "of embedded bug.",
        ])

    deck.bullets(
        "The allowlist, and the two ways to get it wrong",
        [("A Switch pad in pairing mode connects to whichever host answers "
          "first. Pairing alone does not give you one-vehicle-one-controller.", 0),
         ("", 0),
         ("The allowlist is a guest list checked BEFORE a connection is "
          "accepted:", 0),
         ("uni_bt_allowlist_remove_all();", 1),
         ("uni_bt_allowlist_add_addr(address);", 1),
         ("uni_bt_allowlist_set_enabled(true);", 1),
         ("", 0),
         ("MISTAKE ONE: calling them before BP32.setup(). Setup is what brings "
          "the Bluetooth stack up. Before that there is no list to add to, and "
          "the calls do nothing at all - silently.", 0),
         ("", 0),
         ("MISTAKE TWO: adding to the list instead of rebuilding it. Rebuild "
          "from scratch every boot and the sketch is always the single source "
          "of truth. Add, and stale entries accumulate where nobody can see "
          "them.", 0),
         ("", 0),
         ("Note that enableNewBluetoothConnections stays TRUE. The allowlist "
          "keeps other people out; leaving new connections on means your own "
          "pad can always get back in even if the bonding keys are lost.", 0)],
        lead="A guest list, checked at the door",
        note="11.2 added addresses at runtime only. If those entries did not "
             "survive a reboot, a vehicle could come back up with an empty "
             "allowlist AND new connections disabled - and then refuse its own "
             "controller with no way in.",
        note_kind="warn",
        speaker=[
            "Set the problem up first: pairing alone does not give you one "
            "vehicle per controller.",
            "Mistake one is the cruel one. Called before setup(), the "
            "allowlist calls do nothing AT ALL, and they do it silently. No "
            "error, no warning.",
            "Mistake two is the one that rots slowly: add rather than "
            "rebuild, and stale entries accumulate where nobody can see "
            "them.",
            "Rebuild from scratch every boot, and the sketch is the single "
            "source of truth. That principle is worth stating generally.",
        ])

    deck.progress(STAGES["lesson3"], 1,
        speaker=[
            "Twenty seconds. First activity.",
        ])

    deck.activity(
        "Do it now  -  lock a board to one pad",
        "a3a_controller_address",
        [("1.  Upload. The allowlist starts OFF. Connect a controller.", 0),
         ("2.  Type  lock. Disconnect, and connect a DIFFERENT controller.", 0),
         ("3.  Watch the refusal in the Serial Monitor.", 0),
         ("4.  Type  open  and try the second one again.", 0),
         ("5.  Type  lockaddr 00:11:22:33:44:55  and try to connect anything.", 0),
         ("6.  Type  whoami. Compare with what a Bluetooth scanner app on your "
          "phone shows for this board.", 0)],
        expect=[("Every connection attempt prints an address. With the lock on, "
                 "everything except the locked address is turned away.", 0)],
        questions=[("Where does this sketch keep its locked address, and what "
                    "happens to it at the next reset?", 0),
                   ("Why does the sketch call applyAllowlist() AFTER "
                    "BP32.setup()?", 0)],
        minutes=35,
        speaker=[
            "Step 3 is the moment it becomes real: a second controller "
            "turned away with a printed reason.",
            "Step 5 locks to an address that does not exist, so nothing can "
            "connect. Good for showing that the mechanism really is doing "
            "the work.",
            "Step 6 cross-checks against a phone scanner app. Worth doing - "
            "it connects the number on screen to something outside the "
            "room.",
            "Answers: this sketch keeps the address in RAM only, so it is "
            "gone at reset; and applyAllowlist() must come after "
            "BP32.setup() because setup is what brings the stack up.",
        ])

    deck.bullets(
        "There is no EEPROM on an ESP32",
        [("The EEPROM library is an EMULATION. It reserves a slice of the flash "
          "chip, keeps a copy in RAM, and only writes the flash when you call "
          "commit().", 0),
         ("", 0),
         ("Three consequences:", 0),
         ("EEPROM.begin(size) must come first, before any get or put.", 1),
         ("Nothing is stored until EEPROM.commit(). Forgetting it is the single "
          "most common cause of \"my settings did not save\".", 1),
         ("Flash WEARS OUT - roughly a hundred thousand erase cycles per "
          "sector. Plenty for saving when a human asks. Nowhere near enough to "
          "commit() inside loop().", 1),
         ("", 0),
         ("Never write on a schedule. Write when something actually changed, "
          "and only when a person asked for it.", 0)],
        lead="It is flash, pretending",
        note="A commit() in loop() at 30 Hz burns through a hundred thousand "
             "cycles in under an hour.",
        note_kind="warn",
        speaker=[
            "Lead with the headline: there is no EEPROM on this chip. The "
            "library is an emulation over flash.",
            "The three consequences are all practical. begin() first, "
            "nothing saved without commit(), and flash wears out.",
            "The warning panel has the arithmetic: a commit() in loop() at "
            "30 Hz burns a hundred thousand cycles in under an hour. Say "
            "that number out loud.",
            "The rule to take away: write when something changed and a "
            "person asked, never on a schedule.",
        ])

    deck.code(
        "The storage map, and why the addresses are spaced out",
        ["const int ADDR_DEADZONE       =  0;   // int,   4 bytes",
         "const int ADDR_DEADZONE_LS_X  =  4;",
         "const int ADDR_DEADZONE_LS_Y  =  8;",
         "const int ADDR_DEADZONE_RS_X  = 12;",
         "const int ADDR_DEADZONE_RS_Y  = 16;",
         "const int ADDR_LS_Y_SC        = 20;   // float, 4 bytes",
         "const int ADDR_LS_X_SC        = 24;",
         "const int ADDR_RS_Y_SC        = 28;",
         "const int ADDR_RS_X_SC        = 32;",
         "const int ADDR_PAIRED_VALID   = 36;   // 1 byte:  0xA5 when valid",
         "const int ADDR_PAIRED_ADDR    = 37;   // 6 bytes: the address",
         "",
         "const uint8_t PAIRED_VALID_MAGIC = 0xA5;"],
        filename="Config.h",
        notes=[("EEPROM is a flat array of bytes. YOU choose where each value "
                "lives, and nothing stops two values overlapping.", 0),
               ("An int is 4 bytes and a float is 4 bytes on this chip, so each "
                "slot gets 4.", 0),
               ("Laying them out as one ADDR_ constant per value, in one place, "
                "is what makes an overlap visible when you read the file.", 0),
               ("Get this wrong and one setting silently corrupts another. It "
                "will look like a hardware fault.", 0)],
        size=13, highlight={9, 10, 12},
        speaker=[
            "EEPROM is a flat array of bytes and YOU choose the layout. "
            "Nothing stops two values overlapping - the compiler cannot "
            "help you here.",
            "Four bytes per slot because an int and a float are both four "
            "bytes on this chip.",
            "Ask why the addresses are spaced out and written down in one "
            "place. The answer is that a map you can read is a map you can "
            "extend safely.",
            "They break this deliberately in step 6 of the activity, so "
            "plant the idea now.",
        ])

    deck.bullets(
        "The magic byte, and never trusting storage",
        [("A brand new ESP32 has flash full of 0xFF. Read an int out of that "
          "and you get -1. Read a float and you get something meaningless.", 0),
         ("", 0),
         ("So the first byte of the block is a MARKER, written only after a "
          "successful save. If it is not 0xA5, the data is not ours and we use "
          "the compiled-in defaults.", 0),
         ("", 0),
         ("But that is not enough on its own. Even WITH the marker, every value "
          "is range checked as it loads:", 0),
         ("if (saved_dz >= 0 && saved_dz <= DEADZONE_MAX)  INPUT_DEADZONE = saved_dz;", 1),
         ("", 0),
         ("Why? Because a value saved by an OLDER version of the program is "
          "perfectly valid data and still wrong for this one. The marker tells "
          "you the bytes are yours. It does not tell you they still make sense.", 0),
         ("", 0),
         ("Defensive loading is not paranoia. It is the difference between a "
          "vehicle that boots with odd settings and one that will not boot.", 0)],
        lead="Two separate checks, for two separate problems",
        speaker=[
            "Start with the fact: new flash is full of 0xFF. Read an int "
            "out of that and you get -1.",
            "The magic byte says the bytes are OURS. It does not say they "
            "still make sense.",
            "That distinction is the whole slide. A value written by an "
            "older version of the program is perfectly valid data and still "
            "wrong for this one.",
            "Defensive loading is not paranoia - it is the difference "
            "between a vehicle that boots with odd settings and one that "
            "will not boot at all.",
        ])

    deck.progress(STAGES["lesson3"], 2,
        speaker=[
            "Twenty seconds. Second activity.",
        ])

    deck.activity(
        "Do it now  -  settings that survive",
        "a3b_eeprom_settings",
        [("1.  set deadzone 100,  save. Now UNPLUG the board and plug it back "
          "in. Type  show.", 0),
         ("2.  set deadzone 200  and do NOT save. Reset the board. What "
          "happened, and why?", 0),
         ("3.  erase, then reset. Where did the values come from this time?", 0),
         ("4.  dump  before and after a save. Find the magic byte.", 0),
         ("5.  Comment out the EEPROM.commit() in saveSettings. Save, reset, "
          "and see the bug you will one day write for real.", 0),
         ("6.  Deliberately overlap two ADDR_ constants and watch one setting "
          "eat the other.", 0)],
        expect=[("Step 1: the value is still 100 after a power cycle. Step 2: it "
                 "is back to 100, not 200.", 0)],
        questions=[("What is at address 0 in the dump, and what does it mean?", 0),
                   ("Why is range checking needed even when the magic byte is "
                    "correct?", 0)],
        minutes=30,
        speaker=[
            "Step 1 involves physically unplugging the board. Make sure "
            "nobody is mid-upload when they do it.",
            "Step 5 is the deliberate bug: comment out commit(), save, "
            "reset, and watch the setting vanish. That is the mistake they "
            "will one day make for real.",
            "Step 6 overlaps two addresses on purpose and one setting eats "
            "the other. It makes the flat-array point concrete.",
            "Answers: address 0 holds the deadzone int; and range checking "
            "is needed even with a correct magic byte because an older "
            "version could have written a value this version cannot use.",
        ])

    deck.table(
        "Where each program keeps its controller address",
        ["Program", "Where the address lives", "What that costs you"],
        [["pathfinder_ps3",
          "In the CONTROLLER, written by SixaxisPairTool",
          "Needs a Windows PC and a cable to change"],
         ["pathfinder_nintendoswitch",
          "A constant in the source, MY_CONTROLLER",
          "Recompile to change it"],
         ["a3a_controller_address",
          "RAM only",
          "Gone at the next reset - fine for a demonstration"],
         ["Pathfinder_Op_Program12",
          "EEPROM, written the first time it pairs",
          "Nothing. Press BOOT and pair again."]],
        lead="Four programs in this repository, four different answers, each "
             "right for what it is for.",
        col_widths=[3.4, 4.2, 3.4],
        size=13,
        note="A field-serviceable machine should never need a laptop to change "
             "who is allowed to drive it. That is the whole argument for the "
             "EEPROM version.",
        speaker=[
            "Four programs, four answers, and each is right for what it is "
            "for. Do not let this become a ranking.",
            "The cost column is where the engineering judgement lives. Read "
            "that rather than the middle column.",
            "The note is the argument for the EEPROM version: a "
            "field-serviceable machine should never need a laptop to change "
            "who is allowed to drive it.",
        ])

    deck.code(
        "The pairing workflow, end to end",
        ["// Two ways in, both do the same thing:",
         "//   - press the BOOT button (GPIO 0, reads LOW when pressed)",
         "//   - type 'pair' on the serial console",
         "",
         "void enterPairingMode() {",
         "  // drop whatever is connected",
         "  BP32.forgetBluetoothKeys();",
         "  clearPairedAddress();          // clears the EEPROM marker",
         "  applyAllowlist();              // now empty, so disabled",
         "  BP32.enableNewBluetoothConnections(true);",
         "  led_mode = PAIRING;            // LEDs breathe blue",
         "}",
         "",
         "// Then, in the connect callback:",
         "if (!paired_addr_valid) {",
         "  savePairedAddress(props.btaddr);   // straight to EEPROM",
         "  applyAllowlist();                  // rebuild, now enabled",
         "}"],
        filename="Bluetooth.ino",
        notes=[("The first controller to connect during pairing becomes this "
                "vehicle's own, and is written down immediately.", 0),
               ("applyAllowlist() is called again straight away, so the window "
                "where anything can connect closes the moment it is filled.", 0),
               ("forget  reopens it deliberately.", 0),
               ("The LEDs breathing blue is the only feedback a student gets "
                "with no laptop attached. That is a deliberate design decision, "
                "not decoration.", 0)],
        size=13, highlight={12, 13, 14},
        speaker=[
            "Two ways in, one code path. That is a design choice worth "
            "pointing at - a button and a console command that do exactly "
            "the same thing.",
            "The first controller to connect during pairing becomes the "
            "vehicle's own and is written down immediately, not at "
            "shutdown.",
            "Ask why it is written immediately. Because a vehicle that "
            "loses power before saving would come back not knowing who it "
            "belongs to.",
            "Walk the order: forget keys, clear the marker, rebuild the "
            "empty allowlist, enable new connections, show it on the LEDs.",
        ])

    deck.bullets(
        "One more bug worth knowing about",
        [("btaddr is a uint8_t[6], already in PRINTED order. Byte 0 prints "
          "first.", 0),
         ("", 0),
         ("Op 11.2 printed it by counting DOWN from byte 5 to byte 0. So the "
          "address it showed you was the reverse of the real one.", 0),
         ("", 0),
         ("The symptom: you write down the address it printed, paste it into "
          "another program, and nothing ever connects. Everything looks "
          "correct. The address is simply backwards.", 0),
         ("", 0),
         ("Two lessons:", 0),
         ("When an address will not work, print it at BOTH ends and compare "
          "them character by character.", 1),
         ("Byte order is a real category of bug. It is why network protocols "
          "specify it explicitly, and why the phrase big-endian exists.", 1)],
        lead="Endianness, and an afternoon you will not get back",
        note="Op 12 prints most significant byte first, which matches every "
             "Bluetooth scanner you will compare against.",
        speaker=[
            "This is the most human bug in the course. Everything looks "
            "correct and the address is simply backwards.",
            "Describe the symptom before the cause: you write down what it "
            "printed, paste it somewhere else, and nothing ever connects.",
            "The first lesson is a debugging habit: print it at both ends "
            "and compare character by character.",
            "The second is a real category. Byte order is why network "
            "protocols specify it and why the word big-endian exists.",
            "Op 12 now prints most significant byte first, matching every "
            "Bluetooth scanner they will ever use.",
        ])

    deck.quiz(
        "Check yourself",
        [("1.  What is a callback, and who calls it?", 0),
         ("2.  Why must the allowlist calls come after BP32.setup()?", 0),
         ("3.  Why rebuild the allowlist every boot instead of adding to it?", 0),
         ("4.  Why does Op 12 leave new Bluetooth connections enabled?", 0),
         ("5.  What actually happens when you call EEPROM.put()? When does the "
          "flash get written?", 0),
         ("6.  Why must you never call EEPROM.commit() from loop()?", 0),
         ("7.  What is the magic byte for, and why is it not sufficient on its "
          "own?", 0),
         ("8.  A vehicle refuses its own controller after a power cycle. Give "
          "two possible causes.", 0)],
        lead="Next lesson: never block, and never redraw for nothing.",
        speaker=[
            "Conversation, not a test.",
            "Answers: a callback is a function you supply and the library "
            "calls; the allowlist needs the stack up first, which is what "
            "BP32.setup() does; rebuilding keeps the sketch as the single "
            "source of truth.",
            "New connections stay enabled so a vehicle can be re-paired "
            "without a recompile.",
            "put() only writes the RAM copy; the flash is written by "
            "commit(). Never from loop, because flash wears out.",
            "The magic byte says the data is ours, not that it is still "
            "valid for this version.",
            "For question 8: the allowlist was applied before setup, or the "
            "paired address was never committed to EEPROM.",
        ])

    return deck


# ===================================================================
# LESSON 4
# ===================================================================

def lesson4(deck):
    deck.title_slide(
        "Six modes, one variable, no delay() anywhere, and geometry that is "
        "not symmetric.",
        BYLINE + ["", "Three hours.  Nothing moves today."],
        hero_image=img("vehicle-green-leds.jpg"), logo=LOGO,
        speaker=[
            "Nothing moves today, so no blocks needed. Dim the room if you "
            "can.",
            "Have a vehicle in front of you with the LED map from the "
            "beginner course to hand. The geometry section needs it.",
        ])

    deck.objectives([
        "Say why a state machine beats a set of booleans for lighting",
        "Write a non-blocking animation that draws one frame per pass",
        "Explain what a dirty flag saves, and measure it",
        "Work out the LED indices for a turn signal on either side",
        "Add a new lighting mode without touching the existing ones",
    ],
        speaker=[
            "The last objective is the assessment: adding a mode without "
            "touching the existing ones is what a good architecture buys "
            "you.",
        ])

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. What a state machine buys you"],
         ["0:25", "The six modes, and how control passes between them"],
         ["0:50", "Non-blocking animation as a discipline"],
         ["1:10", "Upload a4a_led_state_machine. Measure a redraw"],
         ["1:40", "BREAK  (10 minutes)"],
         ["1:50", "Colour helpers, and why they are written out longhand"],
         ["2:10", "Turn signal geometry. Upload a4b_turn_signal_larson"],
         ["2:35", "BREAK  (10 minutes)"],
         ["2:45", "Add a mode of your own"]],
        col_widths=[1, 9],
        speaker=[
            "Two breaks. The final thirty minutes is a build exercise - "
            "protect it.",
        ])

    deck.bullets(
        "Why lighting needs a state machine and not a pile of booleans",
        [("Suppose you track lighting with flags: lights_on, turning_left, "
          "turning_right, scanning, pairing, disconnected.", 0),
         ("", 0),
         ("Six booleans is 64 combinations. Most are nonsense - turning left "
          "AND right, pairing AND driving - and every one of them is reachable "
          "by a bug.", 0),
         ("", 0),
         ("A STATE MACHINE says: the vehicle is in exactly ONE mode. One "
          "variable holds it. Each mode knows how to draw itself and when to "
          "hand over.", 0),
         ("", 0),
         ("Two things fall out:", 0),
         ("Two modes can never be half-on at once, because there is one "
          "variable and it holds one value.", 1),
         ("Adding a mode is one enum value and one branch - not a new "
          "combination to test against every existing flag.", 1),
         ("", 0),
         ("enum LEDMode { DISCONNECTED, STANDBY, TURN_LEFT, TURN_RIGHT,", 1),
         ("               PAIRING, KITT_SCANNER };", 1)],
        lead="Six booleans is 64 combinations. One enum is six.",
        speaker=[
            "Do the arithmetic on the board: six booleans is 64 "
            "combinations, and most of them are nonsense that a bug can "
            "still reach.",
            "Turning left AND right at the same time is the example that "
            "lands. Ask what the lights would actually do.",
            "One variable, one value, so two modes can never be half-on at "
            "once. That is not discipline, it is structure.",
            "The second consequence is the one they will test at the end of "
            "the lesson: adding a mode is one enum value and one branch.",
        ])

    diagrams.state_machine(deck)

    deck.bullets(
        "Non-blocking, as a rule with no exceptions",
        [("Not one animation function in Op 12 calls delay(). Each checks the "
          "clock, returns immediately if it is not time, and draws exactly ONE "
          "frame when it is:", 0),
         ("", 0),
         ("if (now - last_update < INTERVAL) return;", 1),
         ("last_update = now;", 1),
         ("// ...draw one frame...", 1),
         ("", 0),
         ("That is what lets the vehicle drive, read its controller, watch its "
          "current and animate all at the same time.", 0),
         ("", 0),
         ("The old System Test program did the opposite: a light pattern with "
          "delay() in it froze the vehicle for two seconds, still holding "
          "whatever motor command was last set. A vehicle that keeps driving "
          "while it ignores you is not a cosmetic problem.", 0),
         ("", 0),
         ("The one legitimate delay() in the whole program is the startup light "
          "show, because nothing else needs to happen yet.", 0)],
        lead="One frame per pass, never a loop that waits",
        note="\"Draw one frame and return\" is the same discipline a game engine "
             "uses. Once you see it, you will see it everywhere.",
        speaker=[
            "This is the beginner-course millis() pattern, promoted to a "
            "rule with no exceptions.",
            "Three lines: is it time, remember when, draw ONE frame. Get "
            "them to say it back.",
            "The System Test story is the counterexample. A vehicle that "
            "keeps driving while it ignores you is a safety problem, not a "
            "cosmetic one.",
            "The one legitimate delay() is the startup show, because "
            "nothing else needs to happen yet. Being able to say exactly "
            "why the exception is allowed is the mark of understanding the "
            "rule.",
        ])

    deck.code(
        "Dirty flags: not redrawing for nothing",
        ["// STANDBY is not an animation. It is ONE PICTURE that changes",
         "// only when the headlights or running lights change.",
         "",
         "void set_standby_lighting() {",
         "  strip.clear();",
         "  if (running_lights_on) {",
         "    uint8_t level = full_headlights ? HEADLIGHT_BRIGHT",
         "                                    : HEADLIGHT_DIM;",
         "    fill_range(LEFT_FRONT_START, CORNER_LEN * 2, rgb(level, level, level));",
         "    fill_range(RIGHT_REAR_START, CORNER_LEN * 2, rgb(TAILLIGHT_LEVEL, 0, 0));",
         "  }",
         "  strip.show();",
         "  standby_dirty = false;        // <- the whole point",
         "}",
         "",
         "// ...and the last branch of update_lighting():",
         "  if (led_mode == STANDBY && standby_dirty) {",
         "    set_standby_lighting();",
         "  }"],
        filename="Lighting.ino",
        notes=[("Pushing 32 WS2812B pixels takes about 1 ms of tightly timed "
                "bit-banging. loop() runs tens of thousands of times a second.", 0),
               ("11.2 pushed standby lighting on EVERY pass. That is most of "
                "the vehicle's attention, spent redrawing an identical picture.", 0),
               ("Anything that alters the picture sets standby_dirty = true. "
                "The state machine clears it.", 0),
               ("Graphics engines, spreadsheets and browsers all do this. "
                "Anywhere redrawing is expensive, something is tracking what "
                "actually changed.", 0)],
        size=12, highlight={12},
        speaker=[
            "STANDBY is not an animation. It is one picture that changes "
            "only when something changes it. Make that distinction clearly.",
            "One line - clearing the flag - is what turns a redraw every "
            "pass into a redraw when it matters.",
            "They measure exactly what that line is worth in the next "
            "activity, so do not give them the number now.",
            "Pushing 32 pixels is about a millisecond of tightly timed "
            "bit-banging. That is why it is expensive.",
        ])

    deck.progress(STAGES["lesson4"], 1,
        speaker=[
            "Twenty seconds. First activity.",
        ])

    deck.activity(
        "Do it now  -  measure the state machine",
        "a4a_led_state_machine",
        [("1.  Upload. Switch between the four modes from the console.", 0),
         ("2.  Type  load  in each mode. Which mode costs the most time per "
          "pass? Why?", 0),
         ("3.  In  standby, type  bright  and  dim. Watch WHEN the redraw "
          "happens.", 0),
         ("4.  Comment out the  standby_dirty = false  at the end of "
          "setStandbyLighting. Upload. Check  load  in standby again.", 0),
         ("5.  Write down both numbers. That is what one line was worth.", 0),
         ("6.  Add a fifth mode: ALARM, a slow red pulse.", 0)],
        expect=[("A large loop count in standby, and a much smaller one once "
                 "the dirty flag is removed.", 0)],
        questions=[("How many passes per second in standby with the flag? "
                    "Without it?", 0),
                   ("What did you have to change to add ALARM? How many places?", 0)],
        minutes=35,
        speaker=[
            "The load command is the instrument for this whole activity. "
            "Make sure everybody gets a reading before step 4.",
            "Step 4 removes the dirty flag on purpose, and step 5 makes "
            "them write down both numbers. That comparison IS the lesson.",
            "Step 6 - adding an ALARM mode - is a rehearsal for the "
            "exercise at the end. Note how few places they had to touch.",
            "Thirty-five minutes. If a group finishes early, ask them to "
            "add a second mode and check the cost is the same.",
        ])

    deck.two_columns(
        "Colour helpers, written out longhand",
        "What 11.2 got from FastLED",
        [("CRGB, fill_solid(), nscale8(), sin8() - all free.", 0),
         ("", 0),
         ("Op 12 cannot use FastLED: version 3.10.5 auto-enables an ESP-DSP FFT "
          "backend, and the bluepad32 SDK ships that header without the include "
          "directory it needs. There is no per-sketch opt-out.", 0),
         ("", 0),
         ("So the handful of helpers we actually relied on are written out in "
          "Lighting.ino.", 0)],
        "Why that turned out to be a good thing",
        [("scale_colour() unpacks a 32-bit colour into three bytes, scales "
          "each, and repacks it. Four lines, and now the bit-shifting is "
          "visible instead of magic.", 0),
         ("", 0),
         ("sine8() maps 0..255 of angle onto 0..255 of amplitude with 128 as "
          "the midpoint. Two lines of floating point.", 0),
         ("", 0),
         ("A dependency you do not understand is a dependency that will "
          "eventually break in a way you cannot fix. Forty lines of your own "
          "code is often the cheaper option.", 0)],
        note="It also means the whole course - beginner and advanced - now uses "
             "one LED library instead of two.",
        speaker=[
            "The FastLED story is a real dependency failure and worth "
            "telling properly: a minor version auto-enabled a backend that "
            "would not build against the SDK, with no per-sketch opt-out.",
            "Then make the constructive point. Writing the four helpers out "
            "by hand made the bit-shifting visible instead of magic.",
            "The line to land: a dependency you do not understand is a "
            "dependency that will eventually break in a way you cannot fix.",
            "That is not an argument against libraries. It is an argument "
            "for knowing what the ones you rely on actually do.",
        ])

    deck.bullets(
        "Turn signal geometry, which is the genuinely hard part",
        [("The strip is one loop, so the two halves of each side run in "
          "OPPOSITE directions and the arithmetic is not symmetric:", 0),
         ("front, left to right :   0 .. 15    left half 0-7,  right half 8-15", 1),
         ("rear,  right to left :  16 .. 31    right half 16-23, left half 24-31", 1),
         ("", 0),
         ("The bar has to grow OUTWARD from the centre of the vehicle towards "
          "the corner that is turning, on both bars at once.", 0),
         ("", 0),
         ("LEFT signal:", 0),
         ("front:  RIGHT_FRONT_START - 1 - i      (counts DOWN from 7)", 1),
         ("rear:   LEFT_REAR_START + i            (counts UP from 24)", 1),
         ("", 0),
         ("RIGHT signal:", 0),
         ("front:  RIGHT_FRONT_START + i          (counts UP from 8)", 1),
         ("rear:   LEFT_REAR_START - 1 - i        (counts DOWN from 23)", 1),
         ("", 0),
         ("Do not try to derive this. Step through it and read the indices.", 0)],
        lead="Two bars, four directions, one loop of wire",
        speaker=[
            "This is the hardest slide in the lesson. Have a real vehicle "
            "out and point at LEDs while you talk.",
            "The strip is ONE loop, so the two halves of each side run in "
            "opposite directions. That is the whole reason the arithmetic "
            "is asymmetric.",
            "The bar grows outward from the centre towards the turning "
            "corner, on both bars at once. Trace it with a finger before "
            "showing the indices.",
            "Take the advice on the slide seriously: do not derive it. Step "
            "through it and read the numbers off a real vehicle. They do "
            "exactly that in the activity.",
        ])

    deck.code(
        "Two details that are easy to miss",
        ["// 1. Repaint the BASE lighting first, every frame.",
         "strip.clear();",
         "if (running_lights_on) {",
         "  fill_range(LEFT_FRONT_START, CORNER_LEN * 2, headlights);",
         "  fill_range(RIGHT_REAR_START, CORNER_LEN * 2, taillights);",
         "}",
         "// ...then blank the signalling side, then draw the amber over it.",
         "// Painting only the amber would leave the last frame underneath.",
         "",
         "// 2. The animation finishes its own cycles.",
         "bool still_held = (led_mode == TURN_LEFT  && (dpad & DPAD_LEFT)) ||",
         "                  (led_mode == TURN_RIGHT && (dpad & DPAD_RIGHT));",
         "if (!still_held && turn_signal_cycles >= TURN_CYCLES) {",
         "  led_mode = STANDBY;",
         "  standby_dirty = true;",
         "}"],
        filename="Lighting.ino",
        notes=[("The D-pad only ever STARTS the animation. It does not hold it "
                "on and it does not stop it.", 0),
               ("A real indicator does not stop mid-blink when you let go of "
                "the stalk. It finishes.", 0),
               ("Holding the D-pad keeps resetting the count, so it carries on "
                "for as long as you hold it and then finishes cleanly.", 0),
               ("This is a small thing that makes a machine feel considered "
                "rather than twitchy.", 0)],
        size=12, highlight={10, 11, 12},
        speaker=[
            "Two details, both of which look like paranoia until you skip "
            "them.",
            "Repainting the base first means the previous frame is not left "
            "underneath. Ask what a signal painted over a stale frame would "
            "look like.",
            "The D-pad only STARTS the animation. It does not hold it on "
            "and it does not stop it.",
            "That matches a real indicator, which finishes its blink after "
            "you let go of the stalk. Small detail, and it is what makes it "
            "feel right.",
        ])

    deck.progress(STAGES["lesson4"], 2,
        speaker=[
            "Twenty seconds. Second activity.",
        ])

    deck.activity(
        "Do it now  -  step through the geometry",
        "a4b_turn_signal_larson",
        [("1.  Upload. Type  left, then  right.", 0),
         ("2.  Type  speed 0  to freeze it, then  step  repeatedly. READ the "
          "indices it prints and check them against the vehicle.", 0),
         ("3.  hold left, wait, then  release. Where exactly does it stop?", 0),
         ("4.  speed 200  and watch the bar build slowly.", 0),
         ("5.  Change the LEFT branch to use LEFT_FRONT_START + i and see "
          "what breaks. Explain it.", 0),
         ("6.  Make the trailing edge FADE rather than switch off abruptly.", 0)],
        expect=[("Amber growing outward from the centre towards the turning "
                 "corner, front and rear mirrored.", 0)],
        questions=[("Why does the front index count down for a left signal but "
                    "up for a right one?", 0),
                   ("What would happen if the base lighting were not repainted "
                    "each frame?", 0)],
        minutes=35,
        speaker=[
            "Step 2 is the point of the whole activity: freeze it, step it, "
            "and read the indices against a real vehicle.",
            "Step 5 breaks the left branch on purpose. The bar grows the "
            "wrong way, inward instead of outward, and they should be able "
            "to say why.",
            "Step 6 is open-ended. A fading trailing edge needs "
            "scale_colour(), which they met two slides ago.",
            "Answers: the front counts down for a left signal because the "
            "left half of the front bar is indices 0 to 7 and the growth is "
            "outward from 8; without the base repaint you would see the "
            "previous frame left underneath.",
        ])

    deck.code(
        "The state machine itself",
        ["void update_lighting(unsigned long now, uint8_t dpad) {",
         "",
         "  if (led_mode == PAIRING) {",
         "    update_pairing_breathing(now);",
         "    return;",
         "  }",
         "",
         "  if (led_mode == TURN_LEFT || led_mode == TURN_RIGHT) {",
         "    if (now - larson_time < LARSON_DELAY_MS) return;",
         "    larson_time = now;",
         "    // ...advance one phase, count cycles, maybe hand back...",
         "    update_larson_scanner();",
         "    return;",
         "  }",
         "",
         "  if (led_mode == KITT_SCANNER) {",
         "    if (now - kitt_time < (unsigned long)KITT_SPEED_MS) return;",
         "    kitt_time = now;",
         "    // ...draw one frame, step the dot, maybe hand back...",
         "    return;",
         "  }",
         "",
         "  if (led_mode == STANDBY && standby_dirty) {",
         "    set_standby_lighting();",
         "  }",
         "}"],
        filename="Lighting.ino",
        notes=[("Every branch does the same three things: is it my turn, is it "
                "time yet, draw ONE frame.", 0),
               ("Every branch returns. Exactly one mode runs per pass, and "
                "none of them can run into another.", 0),
               ("The early return on the timer is what makes this cheap. Most "
                "passes through this function do almost nothing at all.", 0),
               ("Note that STANDBY is last and has no timer - it is the "
                "fall-through, and it only acts when the dirty flag says so.", 0)],
        size=12, highlight={8, 16, 23},
        speaker=[
            "This is the whole state machine on one screen. Let them look "
            "at it in silence for thirty seconds before you say anything.",
            "Every branch does the same three things: is it my turn, is it "
            "time yet, draw one frame. Get them to notice the repetition "
            "themselves.",
            "The early returns are what keeps it flat and readable. Ask "
            "what this would look like as nested ifs.",
            "Point out that STANDBY is last and is the only branch guarded "
            "by a flag rather than a clock.",
        ])

    deck.bullets(
        "Breathing, and an 8-bit sine",
        [("The pairing animation fades the whole strip up and down smoothly. "
          "Doing that with a straight line looks wrong - it appears to hover "
          "at the ends and rush through the middle.", 0),
         ("", 0),
         ("A sine wave is what looks natural, because it slows at the "
          "extremes. FastLED gave 11.2 a sin8() for this. Op 12 writes it out:", 0),
         ("uint8_t sine8(uint8_t theta) {", 1),
         ("  float radians = (theta / 256.0f) * 2.0f * PI;", 1),
         ("  return (uint8_t)(128.0f + 127.0f * sinf(radians));", 1),
         ("}", 1),
         ("", 0),
         ("Feed it 0..255 for one full turn, get 0..255 back with 128 as the "
          "midpoint. No angles in degrees, no floating point in the caller.", 0),
         ("", 0),
         ("Then the phase comes from the clock rather than from a counter:", 0),
         ("phase = (now % BREATHE_PERIOD_MS) * 255 / BREATHE_PERIOD_MS;", 1),
         ("", 0),
         ("Deriving the phase from millis() rather than incrementing a variable "
          "means the animation runs at the same speed no matter how often the "
          "function gets called, and it cannot drift.", 0)],
        lead="Why the fade is a sine and not a ramp",
        note="Compute animation state FROM the clock rather than stepping a "
             "counter. It is self-correcting, and it survives a slow pass of "
             "loop() without stuttering.",
        speaker=[
            "Start with why a linear fade looks wrong: it appears to hover "
            "at the ends and rush through the middle.",
            "A sine slows at the extremes, which is what your eye expects. "
            "That is a perception point, like the colour lesson in the "
            "beginner course.",
            "The 0..255 in, 0..255 out convention keeps floating point out "
            "of the caller entirely. Worth noticing as an interface design "
            "choice.",
            "Deriving the phase from millis() rather than a counter means "
            "the animation keeps correct time even if a frame is missed. "
            "That is the transferable idea.",
        ])

    deck.bullets(
        "Add a mode  -  thirty minutes, in your groups",
        [("Add a HAZARD mode to a4a_led_state_machine: all four corners "
          "flashing amber together, running until it is switched off.", 0),
         ("", 0),
         ("Do it the way the program is built:", 0),
         ("1.  Add HAZARD to the LEDMode enum.", 1),
         ("2.  Add its timing constants to the configuration block at the top - "
          "not buried in the function.", 1),
         ("3.  Write update_hazard(now) so it draws ONE frame and returns.", 1),
         ("4.  Add one branch to the state machine.", 1),
         ("5.  Add a console command to enter and leave it.", 1),
         ("", 0),
         ("Then answer these:", 0),
         ("How many existing lines did you have to change?", 1),
         ("Could HAZARD and TURN_LEFT ever be on at the same time? Why not?", 1),
         ("What does  load  say while it is running?", 1),
         ("", 0),
         ("The point of the exercise is the first question. If a new mode costs "
          "you one enum value and one branch, the design is doing its job.", 0)],
        lead="The real test of a design is what it costs to extend",
        speaker=[
            "Thirty minutes, hands off. Circulate but do not solve.",
            "The five steps are the architecture of the program, restated "
            "as a recipe. Insist on step 2 - constants at the top, not "
            "buried in the function.",
            "The first question is the real assessment. Count the lines "
            "they had to change in existing code.",
            "HAZARD and TURN_LEFT can never both be on, because there is "
            "one variable holding one value. Make sure somebody says that "
            "out loud.",
            "Leave five minutes to demonstrate. Every group shows theirs.",
        ])

    deck.quiz(
        "Check yourself",
        [("1.  Six booleans give how many combinations? One six-value enum?", 0),
         ("2.  Write the three-line non-blocking animation pattern.", 0),
         ("3.  What is a dirty flag, and roughly what did it save on this "
          "vehicle?", 0),
         ("4.  Why can Op 12 not use FastLED?", 0),
         ("5.  Why does each turn-signal frame repaint the headlights?", 0),
         ("6.  Why does the front index count down for a LEFT signal?", 0),
         ("7.  Why does the turn signal finish its cycles instead of stopping "
          "when the D-pad is released?", 0),
         ("8.  You want to add a hazard-lights mode. What exactly do you have "
          "to change?", 0)],
        lead="Next lesson: a machine that tests its own hardware.",
        speaker=[
            "Conversation, not a test.",
            "Answers: 64 combinations against 6; the three-line pattern "
            "should be written out; a dirty flag skips a redraw when "
            "nothing changed, and they have their own measured numbers for "
            "what it saved.",
            "FastLED 3.10.5 auto-enables an ESP-DSP backend that will not "
            "build against the bluepad32 SDK.",
            "Each frame repaints the headlights so the previous frame is "
            "not left underneath.",
            "The front counts down for a left signal because the growth is "
            "outward from the centre of the bar.",
            "The signal finishes its cycles because that is what a real "
            "indicator does.",
            "For question 8: one enum value, one set of constants, one "
            "update function, one branch, one console command.",
        ])

    return deck


# ===================================================================
# LESSON 5
# ===================================================================

def lesson5(deck):
    deck.title_slide(
        "I2C, shunt resistors, current signatures, and a vehicle that can "
        "tell you which motor is unplugged.",
        BYLINE + ["", "Three hours.  Gen 3 vehicles for the sensor work."],
        hero_image=img("vehicle-with-rangefinder.jpg"), logo=LOGO,
        speaker=[
            "You need at least one Gen 3 vehicle per two groups for the "
            "sensor work. A Gen 2 is still useful - the second step of the "
            "first activity needs one.",
            "Blocks out again. The current measurements need a motor "
            "spinning freely and then held.",
            "This is the last lesson. Leave time at the end for the "
            "capstone slide - it is what they take away.",
        ])

    deck.objectives([
        "Test whether a device is present on an I2C bus, and diagnose "
         "an empty scan",
        "Explain how a shunt resistor measures current, and why it is "
         "small",
        "Read a chip register from its datasheet with no library",
        "Recognise the current signature of a healthy, disconnected and "
         "jammed motor",
        "Say why self-test thresholds are measured against a baseline",
        "Add a subsystem to Op Program 12 without making it worse",
    ],
        speaker=[
            "Objective six is the real one. Everything before it is "
            "groundwork for being able to extend the program without making "
            "it worse.",
        ])

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. I2C addressing, acknowledgement, pull-ups"],
         ["0:25", "Upload a5a_i2c_scan. Find the sensor, then lose it"],
         ["0:50", "Measuring current without interrupting it"],
         ["1:15", "BREAK  (10 minutes)"],
         ["1:25", "Driving a chip from its datasheet, with no library"],
         ["1:50", "Upload a5b_ina219_current. Measure a motor"],
         ["2:20", "BREAK  (10 minutes)"],
         ["2:30", "Current signatures, and the self-test state machine"],
         ["2:50", "Adding a subsystem of your own, properly"]],
        col_widths=[1, 9],
        speaker=[
            "Two breaks. The capstone discussion at 2:50 will overrun if "
            "you let the measurement activity run long.",
        ])

    deck.bullets(
        "I2C in ten lines",
        [("Two wires, SDA and SCL, shared by every device on the bus.", 0),
         ("Each device answers to a 7-bit address. 0x00-0x07 and 0x78-0x7F are "
          "reserved, leaving 0x08 to 0x77 to scan.", 0),
         ("", 0),
         ("To TEST an address you start a transmission to it and end it "
          "immediately, sending no data. If a device is there it pulls SDA low "
          "to acknowledge:", 0),
         ("0  acknowledged - something is there", 1),
         ("2  no acknowledge for the address - nothing there", 1),
         ("", 0),
         ("PULL-UP RESISTORS. I2C devices can only pull a line DOWN. Something "
          "has to pull it back up, and that is a pair of resistors to 3.3 V, "
          "usually 4.7k. On the Pathfinder they are on the control board.", 0),
         ("", 0),
         ("A scan that returns absolutely nothing on EVERY address is the "
          "classic symptom of a missing pull-up. That matters the moment you "
          "add your own sensor on the breadboard.", 0)],
        lead="Two wires, 120 possible devices",
        note="On this board I2C is on GPIO 32 and 33, not the ESP32 defaults. "
             "GPIO 34-39 are input-only and cannot drive a bus at all.",
        speaker=[
            "Ten lines, and it covers what they need. Do not turn this into "
            "a bus protocol lecture.",
            "The acknowledge mechanism is the clever bit: address it, send "
            "nothing, and see whether anything pulls SDA low.",
            "The pull-up point is the one that will save them an afternoon. "
            "Devices can only pull a line DOWN, so something has to pull it "
            "back up.",
            "Nothing on every address is the classic missing-pull-up "
            "symptom. It matters the moment they wire their own sensor on "
            "the breadboard.",
        ])

    deck.progress(STAGES["lesson5"], 1,
        speaker=[
            "Twenty seconds. First activity.",
        ])

    deck.activity(
        "Do it now  -  find the sensor",
        "a5a_i2c_scan",
        [("1.  Upload it to a Gen 3 vehicle. You should find 0x40.", 0),
         ("2.  Upload it to a Gen 2 vehicle. You should find nothing, and that "
          "is the CORRECT answer.", 0),
         ("3.  Type  watch, then unplug the sensor. How quickly does it "
          "notice?", 0),
         ("4.  Try  pins 21 22, the ESP32 defaults. Does the sensor still "
          "answer? Why not?", 0),
         ("5.  Try  pins 34 35. Read what the sketch tells you.", 0),
         ("6.  If you have a spare I2C sensor, wire it on the breadboard and "
          "find its address.", 0)],
        expect=[("Gen 3: one device at 0x40, named. Gen 2: nothing, with an "
                 "explanation of why that is fine.", 0)],
        questions=[("This is exactly what is_ina219_present() does in Op 12. "
                    "What decision does the program make from the answer?", 0)],
        minutes=25,
        speaker=[
            "Step 2 is the important one and it is counter-intuitive: "
            "finding nothing on a Gen 2 is the CORRECT answer, not a "
            "failure.",
            "Step 4 uses the ESP32 default pins, which are not the ones "
            "this board wires the sensor to. Nothing answers, and that is "
            "the point.",
            "Step 5 tries input-only pins. The sketch explains why that "
            "cannot work.",
            "The question ties it back: this is exactly what "
            "is_ina219_present() does in Op 12, and the decision it drives "
            "is which capability flags get set.",
        ])

    deck.bullets(
        "Measuring current without getting in the way of it",
        [("You cannot measure current directly. What the INA219 does is put a "
          "very small known resistor - a SHUNT - in the path and measure the "
          "voltage across it. Ohm's law does the rest:", 0),
         ("I = V / R", 1),
         ("", 0),
         ("The shunt on this board is 0.0025 ohms. So two amps drops:", 0),
         ("V = 2 A x 0.0025 ohm = 0.005 V,  five millivolts", 1),
         ("", 0),
         ("That is deliberate. A bigger resistor would be easier to read, but "
          "it would waste power and steal voltage from the motors:", 0),
         ("P = I^2 x R = 4 x 0.0025 = 0.01 W  at 0.0025 ohm", 1),
         ("P = I^2 x R = 4 x 0.1    = 0.4 W   at 0.1 ohm", 1),
         ("", 0),
         ("Reading five millivolts accurately is the sensor's entire job. The "
          "shunt register reports in units of 10 microvolts, which is where the "
          "0.01 in the maths comes from.", 0)],
        lead="A tiny resistor, and Ohm's law",
        speaker=[
            "You cannot measure current directly. Say that first - it is "
            "the reason the whole shunt idea exists.",
            "Do both power calculations on the board. 0.01 W against 0.4 W "
            "is the whole argument for a tiny resistor.",
            "Five millivolts at two amps. Reading that accurately is the "
            "sensor's entire job, and it is why an INA219 exists rather "
            "than an analog input.",
            "This is Ohm's law from the beginner course doing real work. "
            "Point back at it.",
        ])

    diagrams.ohms_and_power_law(
        deck, title="The two relationships the whole lesson rests on")

    deck.code(
        "Driving a chip with no library",
        ["uint16_t ina219_read16(uint8_t reg) {",
         "  Wire.beginTransmission(INA219_I2C_ADDR);",
         "  Wire.write(reg);",
         "  Wire.endTransmission(false);   // REPEATED START, keep the bus",
         "  Wire.requestFrom((uint8_t)INA219_I2C_ADDR, (uint8_t)2);",
         "  return Wire.available() >= 2 ? (Wire.read() << 8) | Wire.read() : 0;",
         "}",
         "",
         "float readCurrentAmps() {",
         "  int16_t raw = (int16_t)ina219_read16(0x01);   // shunt reg, SIGNED",
         "  return (raw * 0.01f) / (SHUNT_RESISTOR_OHMS * 1000.0f);",
         "}",
         "",
         "float readBusVolts() {",
         "  uint16_t value = ina219_read16(0x02);",
         "  return (float)((value >> 3) * 4) * 0.001f;   // top 13 bits, 4 mV/count",
         "}"],
        filename="Sensors.ino",
        notes=[("endTransmission(FALSE) is a repeated start. A full stop "
                "would let another master grab the bus mid-read.", 0),
               ("The shunt register is SIGNED. Read it unsigned and every "
                "braking current becomes a huge positive number.", 0),
               ("Bus voltage sits in the top 13 bits, so it is shifted down.", 0),
               ("Forty lines of your own beats a library you cannot read.", 0)],
        size=12, highlight={3, 9},
        speaker=[
            "No library. Two functions and a datasheet. That is the point "
            "of the slide.",
            "The repeated start matters: a full stop would let another "
            "master grab the bus between writing the register number and "
            "reading the answer.",
            "The signed read is the trap. Read the shunt register unsigned "
            "and every regenerative spike comes back as a huge positive "
            "number.",
            "The bus voltage register keeps flags in the bottom three bits, "
            "which is why it shifts right by three. That is normal for "
            "register maps, and reading a datasheet is a skill worth "
            "naming.",
        ])

    deck.progress(STAGES["lesson5"], 2,
        speaker=[
            "Twenty seconds. Second activity.",
        ])

    deck.activity(
        "Do it now  -  what a motor costs",
        "a5b_ina219_current",
        [("1.  baseline  with everything off. Now switch the headlights on and "
          "take another. Where did the difference go?", 0),
         ("2.  spin 0  with the wheel free. Then HOLD the wheel and do it "
          "again. Compare peak and average.", 0),
         ("3.  Unplug one motor and run  test. Which numbers give it away?", 0),
         ("4.  regs  - read the raw registers and check them against the "
          "datasheet values in the comments.", 0),
         ("5.  Work out how long a 3300 mAh pack lasts at the average you "
          "measured while driving.", 0),
         ("6.  Compare that with the LED power budget from the beginner course. "
          "Which dominates?", 0)],
        expect=[("A clear spike then a settle for a healthy motor. Almost "
                 "nothing for a disconnected one. A high draw that never "
                 "settles for a jammed one.", 0)],
        questions=[("What is your vehicle's resting current?", 0),
                   ("How long will your battery last at your measured average?", 0)],
        safety="Wheels off the ground. The test drives every motor at full power.",
        minutes=35,
        speaker=[
            "Step 2 needs care: hold the wheel firmly but keep fingers out "
            "of the spokes, and do not hold a stalled motor for more than a "
            "second or two.",
            "Step 3 is the diagnostic payoff - an unplugged motor draws "
            "almost nothing, and that is unmistakable in the numbers.",
            "Steps 5 and 6 connect this to the beginner course. Get them to "
            "compare the motor draw with the LED power budget and say which "
            "dominates.",
            "Collect resting currents from a few groups. They vary, and the "
            "variation is itself interesting.",
        ])

    deck.image_pair(
        "Where the current sensor actually is",
        img("vehicle-gen3-top.jpg"),
        "A Gen 3, looking down through the lexan top plate",
        Placeholder(
            "PHOTO: the INA219 on a Gen 3 board, close up",
            "Close enough to read the part, with the shunt and the two I2C "
            "wires to GPIO 32 and 33 visible."),
        "The INA219, and the shunt it measures across",
        lead="Everything in this lesson depends on one small part that only "
             "Gen 3 vehicles carry. Find it before you write code that "
             "assumes it.",
        speaker=[
            "Have a Gen 3 open on the desk and let people look at the actual "
            "part before the theory starts.",
            "The shunt is the small, low-value resistor next to it. That is "
            "what the whole measurement happens across.",
            "Two wires to GPIO 32 and 33 is the entire electrical interface. "
            "Everything else is I2C protocol.",
            "The close-up photograph is still outstanding for this deck. "
            "Until it arrives, pass a board round.",
        ])

    diagrams.current_signature(deck)

    deck.bullets(
        "The self-test as a state machine",
        [("Jumper GPIO 34 high at boot. Each motor is driven forward then "
          "reverse while the current sensor watches.", 0),
         ("", 0),
         ("CHECK_TRIGGER   wait for the pin to settle, then decide", 1),
         ("BASELINE_CURRENT  measure the resting draw, motors off", 1),
         ("RUNNING_TESTS     eight steps: four motors, two directions", 1),
         ("TEST_RESULT       green blinks for pass, red for fail", 1),
         ("TEST_DISABLED     hand over to normal driving", 1),
         ("", 0),
         ("It is a state machine driven from loop(), not a blocking routine. So "
          "the LEDs keep animating and the serial console keeps responding "
          "throughout - and any button press clears the result screen.", 0),
         ("", 0),
         ("Note GPIO 34 is input-only and has NO internal pull resistors. The "
          "board carries an external pull-down. Read a floating input-only pin "
          "and you get whatever the room's electrical noise suggests.", 0)],
        lead="Five states, no blocking",
        note="A test that freezes the machine while it runs is a test nobody "
             "will leave enabled.",
        speaker=[
            "Five states, and it is driven from loop() rather than "
            "blocking. That is the same discipline as the lighting lesson.",
            "So the LEDs keep animating and the console keeps responding "
            "while the test runs. Ask why that matters - because a machine "
            "that goes deaf during a self-test cannot be interrupted.",
            "The GPIO 34 note is a real hardware trap. Input-only, no "
            "internal pulls, so the board carries an external pull-down.",
            "A floating input-only pin reads whatever the room's electrical "
            "noise says. Worth saying, because somebody will try this on a "
            "breadboard.",
        ])

    deck.two_columns(
        "Diagnostics, and the battery maths you can now do",
        "What  diag  gives you",
        [("Every five seconds, while it is switched on:", 0),
         ("VBat   - bus voltage, straight from the sensor", 1),
         ("I-Avg  - mean current since the last report", 1),
         ("I-Max  - the largest positive spike", 1),
         ("I-Min  - the largest negative excursion", 1),
         ("", 0),
         ("Negative current is real. When you cut the throttle, a spinning "
          "motor becomes a generator and pushes current BACK. That is why the "
          "shunt register is signed.", 0),
         ("", 0),
         ("The tracking is reset after each report, so each line describes the "
          "last five seconds rather than all of time.", 0)],
        "Work it out for your vehicle",
        [("A 4S 3300 mAh pack holds 3.3 amp hours.", 0),
         ("", 0),
         ("runtime (hours) = 3.3 / average current in amps", 1),
         ("", 0),
         ("So at a measured 2.0 A average:", 0),
         ("3.3 / 2.0 = 1.65 hours, about 99 minutes", 1),
         ("", 0),
         ("At 6.0 A of hard driving:", 0),
         ("3.3 / 6.0 = 0.55 hours, about 33 minutes", 1),
         ("", 0),
         ("Now add the LEDs. 32 pixels at full white is nearly 2 A on its own - "
          "and that is a constant drain whether you are moving or not.", 0),
         ("", 0),
         ("Measure yours. Then decide what LED brightness you can actually "
          "afford for a two-hour class.", 0)],
        size=14,
        note="Never run a lithium pack flat. Below about 3.0 V per cell - 12 V "
             "for a 4S - you are damaging it. The vehicle knows its own voltage, "
             "so a low-battery warning is a short job and a good exercise.",
        note_kind="safety",
        speaker=[
            "The negative current is the interesting column. A spinning "
            "motor with the throttle cut becomes a generator and pushes "
            "current back.",
            "That is why the shunt register is signed, and it links "
            "straight back to the previous code slide.",
            "The right column is arithmetic they can do for their own "
            "vehicle. Get everybody to compute a runtime from their own "
            "measured average.",
            "Point out that the tracking resets after each report, so each "
            "line describes the last five seconds, not the whole session.",
        ])

    deck.bullets(
        "Adding a subsystem of your own",
        [("Say you want a range finder that stops the vehicle before it hits "
          "something. Do it the way the program is already built:", 0),
         ("", 0),
         ("1.  Every tunable number goes in Config.h. Pins, thresholds, "
          "timings. Nothing hard-coded in the logic.", 0),
         ("2.  One new tab, RangeFinder.ino, owning all of it.", 0),
         ("3.  A setup_rangefinder() called from setup(), after the motors.", 0),
         ("4.  An update_rangefinder(now) called every pass, that returns "
          "immediately if it is not time. No delay(), ever.", 0),
         ("5.  A capability flag, so a vehicle without the sensor still runs "
          "this binary.", 0),
         ("6.  A console command, so it can be tested and tuned without "
          "recompiling.", 0),
         ("7.  Keep its state in static file-scope variables. Expose functions, "
          "not variables.", 0),
         ("", 0),
         ("If you can do those seven things, you can extend this program "
          "without making it worse. That is the actual skill.", 0)],
        lead="The seven things that keep a program from rotting",
        speaker=[
            "Seven rules, and they are the summary of the whole course. "
            "Read them slowly.",
            "Every one of them is something Op 12 already does, so they can "
            "check each against the real program.",
            "Rule seven is the one people skip: expose functions, not "
            "variables. Ask what goes wrong when another tab writes your "
            "state directly.",
            "If they can do these seven things, they can extend this "
            "program without making it worse. That is the whole point of "
            "five lessons.",
        ])

    deck.two_columns(
        "Where this goes",
        "On the vehicle",
        [("Obstacle avoidance from the range finder.", 0),
         ("Line following.", 0),
         ("Battery protection - the current sensor already knows the voltage, "
          "so flashing amber below 12 V and red below 10 V is a short job.", 0),
         ("Stall detection while driving, using the same current signature "
          "logic as the self-test.", 0),
         ("ESP-NOW, so vehicles talk to each other with no router.", 0)],
        "Beyond it",
        [("Everything here is how embedded software is written generally: "
          "subsystem separation, non-blocking state machines, defensive "
          "loading, capability detection, self-test, and a console.", 0),
         ("", 0),
         ("The same patterns run cars, medical devices, satellites and "
          "industrial controllers.", 0),
         ("", 0),
         ("Six of the ideas in this course exist because somebody shipped the "
          "bug first. That is also how it works in industry.", 0)],
        note="Read the change log at the top of Pathfinder_Op_Program12.ino "
             "again now. It should read very differently than it did in "
             "Lesson 1.",
        speaker=[
            "The left column is concrete next steps. The right column is "
            "why any of it mattered.",
            "Say the last line plainly: six of the ideas in this course "
            "exist because somebody shipped the bug first. That is also how "
            "it works in industry, and it is not a shameful thing.",
            "The same patterns run cars, medical devices, satellites and "
            "industrial controllers. That is not hyperbole.",
        ])

    deck.two_columns(
        "The same test, done twice: on the bench and in software",
        "The factory bench test",
        [("Before a control board leaves the factory:", 0),
         ("Check 15 V, 5 V and 3.3 V rails", 1),
         ("Break the motor 1 high side into an ammeter", 1),
         ("Issue a full throttle 255 command", 1),
         ("Measure current at no load, then at stall", 1),
         ("Repeat for motors 2, 3 and 4, then reconnect", 1),
         ("Calibrate and test the I2C devices", 1),
         ("", 0),
         ("Accurate, and it needs a meter, a bench and a person.", 0)],
        "The self-test in the program",
        [("Jumper GPIO 34 high at boot and the vehicle does the same thing to "
          "itself:", 0),
         ("Measure the resting current with the motors off", 1),
         ("Drive each motor full throttle, both ways", 1),
         ("Watch the peak and the average against that baseline", 1),
         ("Blink green for pass, red for fail", 1),
         ("", 0),
         ("Less accurate than a meter. But it runs in ten seconds, in a "
          "classroom, with no equipment and nobody who knows how to use a "
          "multimeter.", 0)],
        note="That is usually the trade with built-in test: you give up "
             "precision to get a check that will actually be run. A test that "
             "needs a bench is a test that happens once.",
        speaker=[
            "Two versions of the same test, and the comparison is the "
            "lesson.",
            "The bench test is more accurate and needs a meter, a bench and "
            "a person. The self-test is less accurate and runs in ten "
            "seconds, anywhere, on every vehicle.",
            "Ask which one you would want in a classroom of thirty "
            "vehicles. Then ask which one you would want before a board "
            "ships.",
            "The answer is both, for different reasons. That is a real "
            "engineering judgement rather than a right answer.",
        ])

    deck.bullets(
        "Capstone  -  choose one, and build it properly",
        [("The rest of the course is yours. Pick one and apply the seven rules "
          "from the previous slide.", 0),
         ("", 0),
         ("LOW BATTERY WARNING. The vehicle already knows its voltage. Flash "
          "amber below 12 V, red below 10 V, and make it a lighting mode rather "
          "than something bolted on the side.", 0),
         ("", 0),
         ("STALL DETECTION. You have the current signature logic from the "
          "self-test. Apply it while driving: if a motor draws a jammed "
          "signature for more than half a second, cut it and say so.", 0),
         ("", 0),
         ("OBSTACLE STOP. An ultrasonic range finder, a threshold in Config.h, "
          "and a rule that overrides the throttle. Think hard about what "
          "happens when the sensor gives one bad reading.", 0),
         ("", 0),
         ("TELEMETRY. Stream voltage, current and speed over ESP-NOW to a "
          "second ESP32 with its own LED bar.", 0),
         ("", 0),
         ("You will be marked on whether somebody else can find your code, "
          "tune it without recompiling, and run the binary on a vehicle that "
          "does not have your hardware fitted.", 0)],
        lead="One subsystem, done to the standard of the rest of the program",
        speaker=[
            "This is the last teaching slide. Give it proper time.",
            "Insist on the seven rules from the earlier slide. A capstone "
            "built badly teaches the wrong thing.",
            "The obstacle-stop option has the best hidden question in it: "
            "what happens when the sensor gives one bad reading? Push "
            "whoever picks it.",
            "If anybody is continuing after the course, get them to commit "
            "to one today while the enthusiasm is still there.",
        ])

    deck.quiz(
        "Check yourself",
        [("1.  How do you test whether an I2C device exists at an address?", 0),
         ("2.  A scan finds nothing on any address. What is the classic cause?", 0),
         ("3.  Why is the shunt resistor 0.0025 ohms and not 0.1?", 0),
         ("4.  Why must the shunt register be read as SIGNED?", 0),
         ("5.  What is a repeated start, and why not just stop and start again?", 0),
         ("6.  Describe the current signature of a healthy motor, a "
          "disconnected one, and a jammed one.", 0),
         ("7.  Why are the self-test thresholds measured above the baseline "
          "rather than as absolute amps?", 0),
         ("8.  Why is the self-test a state machine rather than a blocking "
          "routine?", 0),
         ("9.  List the seven things you would do to add a range finder.", 0)],
        lead="That is the course. Go and add something.",
        speaker=[
            "Conversation, not a test. This is the last thing they do.",
            "Answers: address it and end the transmission with no data, "
            "then check the acknowledgement; nothing on any address means a "
            "missing pull-up; a small shunt wastes less power and steals "
            "less voltage from the motors.",
            "Signed, because a motor pushes current back when the throttle "
            "is cut.",
            "A repeated start keeps the bus so no other master can take it "
            "mid-read.",
            "Healthy: a spike then a settle. Disconnected: almost nothing. "
            "Jammed: high and never settling.",
            "Thresholds sit above a measured baseline because the resting "
            "draw differs between vehicles and with battery charge.",
            "The self-test is a state machine so the LEDs and console keep "
            "working.",
            "Question 9 is the seven rules. If they can list them, the "
            "course did its job.",
        ])

    return deck


# ===================================================================
# BUILD
# ===================================================================

LESSONS = [
    ("l1_architecture_and_toolchain.pptx", "Lesson 1",
     "Architecture and Toolchain", lesson1),
    ("l2_motors_and_ramping.pptx", "Lesson 2",
     "Motors, Resolution and Ramping", lesson2),
    ("l3_bluetooth_and_storage.pptx", "Lesson 3",
     "Bluetooth and Persistent Storage", lesson3),
    ("l4_lighting_state_machine.pptx", "Lesson 4",
     "The Lighting State Machine", lesson4),
    ("l5_sensors_and_self_test.pptx", "Lesson 5",
     "Sensors, Self-Test and Diagnostics", lesson5),
]


def build(out_dir):
    made = []
    for filename, label, title, builder in LESSONS:
        deck = Deck(filename, title, label, TRACK_LABEL)
        builder(deck)
        made.append(deck.save(out_dir))
    return made
