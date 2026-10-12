"""
build_cheatsheet.py - the coding cheat sheet for the beginner course, one per
track.

    python build_cheatsheet.py                    both tracks
    python build_cheatsheet.py beginner-ps3       just that one
    python build_cheatsheet.py --compile          ...and compile every snippet

Four pages, printed double-sided on two sheets: two pages of the C++ the course
uses, two pages about this vehicle. Students get it in Lesson 1 and keep it on
the desk. It is written to cover every coding challenge in the course, the
Lesson 4 pattern and the Lesson 5 projects included, so "what does your cheat
sheet say?" is always a fair answer to a question.

Output, for each track:
    <track>/handouts/coding_cheat_sheet.docx    editable
    <track>/handouts/coding_cheat_sheet.pdf     what you print

The same two guards as the decks:

  * Numbers that live in the sketches - the motor pins, the LED pin, the stick
    range, the deadzone - are read out of them with srcfacts, never typed.

  * --compile pastes every code block on the sheet into a test sketch, with
    the same setup the course programs use, and builds it with arduino-cli for
    the track's board package. A snippet with a typo stops the build. Blocks
    that are a list rather than a program (the operators, the button names)
    are marked as not compiled. The PS3 track needs "Fork of PS3 Controller
    Host" installed and the original removed; point ARDUINO_DIRECTORIES_USER at
    a folder that has the right libraries if your own does not.

The PDF is made with LibreOffice, and the build checks it comes out at exactly
four pages - a fifth page is a cheat sheet nobody prints double-sided.
"""

import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import content_beginner                       # noqa: E402
import srcfacts                               # noqa: E402
from docxlite import (Document, Span, run,    # noqa: E402
                      NAVY, TEAL, GREY, LIGHT_TEAL)

COURSES = os.path.normpath(os.path.join(HERE, ".."))

TEXT_PT = 8.5
CODE_PT = 8
CODE_FONT = "Consolas"
CODE_FILL = "F2F4F7"

SOFFICE = [
    os.environ.get("SOFFICE"),
    r"C:\Program Files\LibreOffice\program\soffice.com",
    r"C:\Program Files (x86)\LibreOffice\program\soffice.com",
    "/Applications/LibreOffice.app/Contents/MacOS/soffice",
    shutil.which("soffice"),
]
ARDUINO_CLI = [
    os.environ.get("ARDUINO_CLI"),
    r"C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe",
    shutil.which("arduino-cli"),
]


def first_existing(paths):
    for path in paths:
        if path and os.path.exists(path):
            return path
    return None


# ===================================================================
# Cell helpers
# ===================================================================

class Code:
    """
    A block of code on the sheet.

    ctx says how --compile uses it:
        ("top", group)   file-level code: variables, functions, #includes
        ("body", group)  statements, compiled inside a function
        None             a list rather than a program - shown, not compiled
    Blocks with the same group share a namespace, so they can use each
    other's names. Different groups can reuse a name without clashing.
    """

    def __init__(self, lines, ctx=None):
        self.lines = lines
        self.ctx = ctx


class Codes(list):
    """Several Code blocks in one cell, shown one after another."""


def code_cell(block):
    blocks = block if isinstance(block, Codes) else [block]
    return [[run(line if line else " ", font=CODE_FONT, size=CODE_PT)]
            for one in blocks for line in one.lines]


def text_cell(*paragraphs):
    out = []
    for p in paragraphs:
        if isinstance(p, str):
            out.append([run(p, size=TEXT_PT)])
        else:
            out.append(p)
    return out


def b(text):
    return run(text, bold=True, size=TEXT_PT)


def t(text):
    return run(text, size=TEXT_PT)


def c(text):
    return run(text, font=CODE_FONT, size=CODE_PT)


# ===================================================================
# The content
# ===================================================================

def _pad(code, column):
    """Pad a line of code so its comment starts at `column`."""
    return code + " " * max(1, column - len(code))


def is_ps3(T):
    return T["key"].endswith("ps3")


def content(T):
    """
    The four pages, as a list of ("page", title) markers and sections.
    A section is (heading, widths, rows, note). Each row is a list of cells:
    a string, a list of paragraphs, or a Code block.
    """
    ps3 = is_ps3(T)
    half = T["stick_max"] // 2
    mapped = half * 255 // T["stick_max"]
    pins = {label: (a, b_) for label, a, b_ in T["motor_pins"]}
    W = [1.95, 5.65]          # the usual two columns: what it is, how to write it

    pages = []

    # ---------------------------------------------------------------- page 1
    pages.append(("page", "The C++ you need  (1 of 2)"))

    pages.append(("1   The shape of every program", W, [
        [text_cell([b("INITIALIZATION: "), t("the top of the file. Libraries, "
                   "settings and variables. Read once, before anything runs.")]),
         Code(["#include <Adafruit_NeoPixel.h>   // pull in a library",
               "const int LED_PIN = 2;           // a setting: never changes",
               "int loopCount = 0;               // a variable: can change"],
              ("top", "shape"))],
        [text_cell([b("setup() "), t("runs ONCE, at power-up or reset. Get "
                   "the hardware ready.")]),
         Code(["void setup() {",
               "  pinMode(LED_PIN, OUTPUT);",
               "  Serial.begin(115200);",
               "}"], ("top", "shape"))],
        [text_cell([b("loop() "), t("runs over and over, as fast as it can, "
                   "until the power goes off.")]),
         Code(["void loop() {",
               "  loopCount = loopCount + 1;",
               "  digitalWrite(LED_PIN, HIGH);",
               "  delay(1000);",
               "}"], ("top", "shape"))],
    ], [b("The rules that catch everybody.  "), t("Every statement ends with  "),
        c(";"), t(".  Every  "), c("{"), t("  has a matching  "), c("}"),
        t(".  "), c("//"), t("  starts a comment the computer ignores.  "
        "Capitals matter:  "), c("Serial"), t("  is not  "), c("serial"),
        t(".  A name is one word - letters, digits and _ - and cannot start "
          "with a digit.")]))

    controller_row = (
        [c("const char *"), "A piece of text",
         Code(['const char *PS3_MAC_ADDRESS = "02:02:03:04:05:07";'],
              ("top", "types"))]
        if ps3 else
        [c("uint8_t x[6]"), "A list of six numbers, 0 to 255",
         Code(["const uint8_t MY_CONTROLLER[6] = { 0x58, 0x2F, 0x40, 0x11, 0x22, 0x33 };"],
              ("top", "types"))])

    pages.append(("2   Variables: named boxes that hold a value", [1.15, 2.85, 3.6], [
        [b("Type"), b("Holds"), b("Example")],
        [c("int"), "A whole number, up to about 2 billion either way",
         Code(["int speed = 200;"], ("top", "types"))],
        [c("float"), "A number with a decimal point",
         Code(["float volts = 16.8;"], ("top", "types"))],
        [c("bool"), "Only true or false",
         Code(["bool lightsOn = true;"], ("top", "types"))],
        [c("unsigned long"), "A big whole number, never negative - for millis()",
         Code(["unsigned long lastBlink = 0;"], ("top", "types"))],
        [c("uint32_t"), "A NeoPixel color, packed into one number",
         Code(["uint32_t red = strip.Color(255, 0, 0);"], ("top", "types"))],
        [c("const"), "In front of any type: it can never change",
         Code(["const int MOTOR_MAX = 255;"], ("top", "types"))],
        controller_row,
    ], [t("Make a variable ONCE, with its type; after that use the name on its "
          "own:  "), c("speed = 150;"), t("  A variable made inside  "),
        c("{ }"), t("  disappears at the  "), c("}"), t(". If loop() must "
        "remember it from one pass to the next, make it at the top of the "
        "file.")]))

    pages.append(("3   Math, and the traps in it", W, [
        [text_cell([b("Arithmetic. "), t("Whole numbers give whole answers: "
                   "the fraction is thrown away, never rounded.")]),
         Code(["int total    = 7 + 2;     // 9",
               "int half     = 7 / 2;     // 3, NOT 3.5",
               "float exact  = 7.0 / 2;   // 3.5 - one decimal point does it",
               "int leftOver = 7 % 2;     // 1, the remainder",
               "int none     = 9 / 10;    // 0, and it does not round up"],
              ("body", "math"))],
        [text_cell([b("Changing a variable")]),
         Code(["count = count + 1;   count++;     // both add one",
               "count += 5;          count--;     // add five, take one away"],
              ("body", "math"))],
        [text_cell([b("Multiply BEFORE you divide, "), t("or it comes out 0.")]),
         Code(["int percent = duty * 100 / 255;   // duty 128 gives 50",
               "int wrong   = duty / 255 * 100;   // duty 128 gives 0"],
              ("body", "math"))],
        [text_cell([b("Handy functions")]),
         Code([_pad("int size  = abs(-40);", 40) + "// 40: no sign",
               _pad("int safe  = constrain(300, -255, 255);", 40) + "// 255: kept in range",
               _pad("int speed = map({}, 0, {}, 0, 255);".format(
                   half, T["stick_max"]), 40) + "// {}: proportion".format(mapped),
               _pad("int small = min(3, 8);", 40) + "// 3  (and max() gives the larger)",
               _pad("int pick  = random(0, 32);", 40) + "// 0 to 31: the top is left out"],
              ("body", "math"))],
    ], None))

    pages.append(("4   Comparing, and combining tests", W, [
        [text_cell([b("Comparisons "), t("come out true or false. TWO = signs "
                   "to compare: one = SETS the value instead.")]),
         Code(["a == b  equal         a != b  not equal",
               "a <  b  less than     a <= b  less than or equal",
               "a >  b  more than     a >= b  more than or equal",
               "if (speed = 0)   // WRONG: sets speed to 0, never compares"])],
        [text_cell([b("Combine tests: "), t("AND, OR, NOT.")]),
         Code(["if (speed > 0 && lightsOn) { }        // && both must be true",
               "if (stickX < -20 || stickX > 20) { }  // || either one will do",
               "if (!lightsOn) { }                    // !  flips true and false"],
              ("body", "logic"))],
    ], None))

    # ---------------------------------------------------------------- page 2
    pages.append(("page", "The C++ you need  (2 of 2)"))

    pages.append(("5   Making decisions: if, else if, else", W, [
        [text_cell([b("if "), t("runs its block only when the test is "
                   "true; "), b("else "), t("when it is not - never both. "),
                    b("else if "), t("makes a chain: the FIRST true test wins, "
                    "so put the narrowest test first.")],
                   [c("test ? a : b"), t("  is a quick either/or for one "
                    "value: a if the test is true, b if not.")]),
         Codes([Code(["if (duty > 200) {",
                      '  Serial.println("fast");',
                      "} else if (duty > 60) {",
                      '  Serial.println("moving");',
                      "} else {",
                      '  Serial.println("stopped");',
                      "}"], ("body", "decide")),
                Code(["int forwardDuty = (speed > 0) ? speed : 0;",
                      'Serial.println(lightsOn ? "Lights ON" : "Lights OFF");'],
                     ("body", "decide"))])],
    ], None))

    pages.append(("6   Loops: doing something many times", W, [
        [text_cell([b("for "), t("a known number of times: start here; keep "
                   "going while this is true; change this each time round.")]),
         Code(["for (int i = 0; i < 32; i++) {",
               "  strip.setPixelColor(i, strip.Color(0, 0, 255));",
               "}",
               "for (int i = 15; i >= 0; i--)  { strip.setPixelColor(i, 0); }  // down",
               "for (int i = 0; i < 32; i += 2) { strip.setPixelColor(i, 0); }  // in twos"],
              ("body", "loops"))],
        [text_cell([b("A run of LEDs: "), t("use  "), c("<="), t("  when the "
                   "last number counts, or you miss it - off by one.")]),
         Code(["for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {",
               "  strip.setPixelColor(i, strip.Color(255, 255, 255));",
               "}"], ("body", "loops"))],
        [text_cell([b("while "), t("goes on as long as the test stays true, "
                   "and might not run at all.  "), c("break;"),
                    t("  leaves any loop early.")]),
         Code(["while (countdown > 0) {   // countdown is an int",
               "  countdown--;",
               "}"], ("body", "loops"))],
    ], None))

    pages.append(("7   Functions: give a job a name, then call it", W, [
        [text_cell([b("void "), t("means it hands nothing back. "),
                    b("Parameters "), t("pass information in. Write it outside "
                    "every other function, then "), b("call it "),
                    t("from loop().")]),
         Codes([Code(["void lightRange(int first, int last, uint32_t color) {",
                      "  for (int i = first; i <= last; i++) {",
                      "    strip.setPixelColor(i, color);",
                      "  }",
                      "}"], ("top", "func")),
                Code(["lightRange(0, 15, strip.Color(255, 0, 0));   // call it: front bar red"],
                     ("body", "func"))])],
        [text_cell([b("return "), t("hands a value back: its type goes "
                   "where void was.")]),
         Code(["int percentOf(int duty) {",
               "  return duty * 100 / 255;      // percentOf(128) is 50",
               "}"], ("top", "func"))],
    ], None))

    pages.append(("8   The Serial Monitor: how the program talks to you", W, [
        [text_cell([b("Tools > Serial Monitor. "), t("Nonsense characters: "
                   "the baud box is not 115200. ")],
                   [b("Stuck? "), t("Print the numbers you are working "
                    "from.")]),
         Code([_pad("Serial.begin(115200);", 30) + "// in setup()",
               _pad('Serial.print("Speed: ");', 30) + "// print stays on the line",
               _pad("Serial.println(speed);", 30) + "// println ends it",
               _pad("Serial.println(volts, 2);", 30) + "// two decimal places",
               _pad('Serial.print("\\t");', 30) + "// a tab, to line up columns"],
              ("body", "serial"))],
    ], None))

    pages.append(("9   Time: delay() stops everything, millis() does not", W, [
        [text_cell([b("delay(1000) "), t("waits a second and NOTHING else "
                   "happens, so never in a vehicle program. "), b("millis() "),
                    t("is how long the board has been on, in ms. Check it "
                      "every pass; act when enough time has gone by. Every job "
                      "gets its OWN last-time variable.")]),
         Code(["unsigned long lastBlink = 0;",
               "void loop() {",
               "  if (millis() - lastBlink >= 500) {      // every half second",
               "    lastBlink = millis();",
               "    blinkOn = !blinkOn;      // ...do the job...",
               "  }                          // everything else keeps running",
               "}"], ("top", "timer"))],
    ], None))

    pages.append(("10   Named states, lists, and reading sensors", W, [
        [text_cell([b("enum "), t("names the states something can be in. An "),
                    b("array "), t("is a list:  "), c("[i]"), t("  picks one, "
                    "counting from 0. "), b("Sensors "), t("for the projects "
                    "are read like the last two lines.")]),
         Codes([Code(["enum LightPattern { LIGHTS_STOPPED, LIGHTS_FORWARD, LIGHTS_REVERSE };",
                      "LightPattern pattern = LIGHTS_STOPPED;       // one named state",
                      "const int CORNERS[] = { 0, 15, 16, 31 };     // CORNERS[0] is 0",
                      "const int HOW_MANY = sizeof(CORNERS) / sizeof(CORNERS[0]);   // 4"],
                     ("top", "lists")),
                Code(["int millivolts = analogReadMilliVolts(34);   // 0 to about 3300",
                      "long echo = pulseIn(ECHO_PIN, HIGH, 30000);  // microseconds, 0 if none"],
                     ("body", "lists"))])],
    ], None))

    # ---------------------------------------------------------------- page 3
    pages.append(("page", "This vehicle  (1 of 2)"))

    pages.append(("11   The pins this vehicle uses", [1.9, 2.2, 3.5], [
        [b("What"), b("GPIO pins"), b("Notes")],
        ["Left motors",
         "front {}, {}    rear {}, {}".format(*(pins["Front left"] + pins["Rear left"])),
         "Two pins each: the first drives forward,"],
        ["Right motors",
         "front {}, {}    rear {}, {}".format(*(pins["Front right"] + pins["Rear right"])),
         "the second reverse"],
        ["The 32 LEDs", str(T["led_pin"]), "One data wire drives all of them"],
        ["Servos 1 to 4", "25, 26, 27, 14", "On the main board.  Breadboard LED: 2"],
        ["I2C sensors", "SDA 32, SCL 33", "GPIO 34 to 39 are INPUT only"],
    ], None))

    if ps3:
        setup_motor = Code(["ledcAttach(FRONT_LEFT_A, 20000, 8);   // pin, 20 kHz, 8 bits"],
                           ("body", "motors"))
        write_motor = Code(["ledcWrite(FRONT_LEFT_A, 200);   // forward, about 78%",
                            "ledcWrite(FRONT_LEFT_B, 0);     // B off"],
                           ("body", "motors"))
    else:
        setup_motor = Code(["ledcSetup(FRONT_LEFT_CH_A, 20000, 8);   // channel, 20 kHz, 8 bits",
                            "ledcAttachPin(FRONT_LEFT_PIN_A, FRONT_LEFT_CH_A);"],
                           ("body", "motors"))
        write_motor = Code(["ledcWrite(FRONT_LEFT_CH_A, 200);   // forward, about 78%",
                            "ledcWrite(FRONT_LEFT_CH_B, 0);     // B off"],
                           ("body", "motors"))

    pages.append(("12   Motors", W, [
        [text_cell([b("Set each pin up once, "), t("in setup().")]), setup_motor],
        [text_cell([b("Duty 0 to 255 "), t("is the power. A for forward, B for "
                   "reverse; both 0 coasts.")]),
         write_motor],
        [text_cell([b("drive(left, right) "), t("does all four motors: -255 to "
                   "+255, minus is reverse.")]),
         Code(["drive(200, 200);    // straight ahead",
               "drive(150, -150);   // spin right on the spot",
               "drive(0, 0);        // stop"], ("body", "motors"))],
        [text_cell([b("Tank drive: "), t("one stick, two sides. constrain() "
                   "stops 200 + 60 asking for 260.")]),
         Code(["int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);",
               "int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);",
               "drive(leftSpeed, rightSpeed);"], ("body", "motors"))],
    ], [b("A wheel turns the wrong way? "), t("Swap that motor's two pin "
        "numbers in the sketch - never rewire.  "), b("Wheels off the ground "),
        t("the first time you upload any change that drives.")]))

    pages.append(("13   The 32 LEDs  (Adafruit NeoPixel)", W, [
        [text_cell([b("Make the strip once, "), t("at the top of the file.")]),
         Code(["#include <Adafruit_NeoPixel.h>",
               "const int LED_PIN   = {};".format(T["led_pin"]),
               "const int LED_COUNT = {};".format(T["led_count"]),
               "Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);"],
              ("top", "strip"))],
        [text_cell([b("In setup()")]),
         Code(["strip.begin();",
               "strip.setBrightness(60);   // 0 to 255, set once"],
              ("body", "leds"))],
        [text_cell([b("Paint, then show. "), t("These only change memory: "
                   "NOTHING lights up until  "), c("strip.show()"), t(".")]),
         Code([_pad("strip.setPixelColor(4, strip.Color(255, 0, 0));", 50) + "// one LED",
               _pad("strip.fill(strip.Color(0, 255, 0));", 50) + "// all of them",
               _pad("strip.clear();", 50) + "// all off",
               _pad("strip.show();", 50) + "// now send it!"],
              ("body", "leds"))],
        [text_cell([b("Rainbows "), t("use a HUE, 0 to 65535, round the "
                   "color wheel.")]),
         Code(["strip.setPixelColor(0, strip.gamma32(strip.ColorHSV(21845)));",
               "int howMany = strip.numPixels();   // 32"], ("body", "leds"))],
    ], None))

    pages.append(("led-map", None, None, None))

    pages.append(("Colors:  each one is  strip.Color(red, green, blue)", [0.85, 1.7, 0.85, 1.7, 0.85, 1.65], [
        ["Red", Code(["Color(255, 0, 0)"]), "Yellow", Code(["Color(255, 255, 0)"]),
         "Purple", Code(["Color(128, 0, 255)"])],
        ["Green", Code(["Color(0, 255, 0)"]), "Cyan", Code(["Color(0, 255, 255)"]),
         "White", Code(["Color(255, 255, 255)"])],
        ["Blue", Code(["Color(0, 0, 255)"]), "Magenta", Code(["Color(255, 0, 255)"]),
         "Warm white", Code(["Color(255, 160, 80)"])],
        ["Orange", Code(["Color(255, 100, 0)"]), "Pink", Code(["Color(255, 100, 100)"]),
         "Off", Code(["Color(0, 0, 0)"])],
    ], [b("Power: "), t("each LED at full white draws about 60 mA, so all 32 "
        "draw nearly 2 amps. Red lights one of the three LEDs inside a pixel; "
        "white lights all three, and costs three times as much.")]))

    # ---------------------------------------------------------------- page 4
    pages.append(("page", "This vehicle  (2 of 2)"))

    if ps3:
        pad_rows = [
            [text_cell([b("At the top: "), t("the library, the line that "
                       "keeps Bluetooth on, and YOUR controller's address. "
                       "The number on its sticker replaces the 00.")]),
             Code(["#include <Ps3Controller.h>",
                   '#include "esp32-hal-alloc-bt-classic-mem.h"   // KEEP: Bluetooth',
                   'const char *PS3_MAC_ADDRESS = "02:02:03:04:05:00";   // CHANGE 00'],
                  ("top", "pad"))],
            [text_cell([b("In setup()")]),
             Code(["Ps3.begin(PS3_MAC_ADDRESS);"], ("body", "pad"))],
            [text_cell([b("Sticks: "), t("{}. Pushing UP gives a NEGATIVE "
                       "number.".format(T["stick_range"]))]),
             Code(["int leftX  = Ps3.data.analog.stick.lx;   // left and right",
                   "int leftY  = Ps3.data.analog.stick.ly;   // UP is negative",
                   "int rightX = Ps3.data.analog.stick.rx;   // .ry for up and down"],
                  ("body", "pad"))],
            [text_cell([b("Buttons "), t("are true while held down - use one "
                       "like a bool.  "), b("Pressure, "), t("0 to 255, from  "),
                        c(".analog.button")]),
             Code(["Ps3.data.button.cross  .circle  .square  .triangle",
                   "Ps3.data.button.up  .down  .left  .right   (the D-pad)",
                   "Ps3.data.button.l1  .r1  .l2  .r2  .l3  .r3  .select  .start  .ps",
                   "int squeeze = Ps3.data.analog.button.l2;"])],
        ]
    else:
        pad_rows = [
            [text_cell([b("At the top: "), t("the library, and YOUR "
                       "controller's address - paste the line "
                       "l3a_controller_check prints.")]),
             Code(["#include <Bluepad32.h>",
                   "const uint8_t MY_CONTROLLER[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };",
                   "ControllerPtr myController = nullptr;"], ("top", "pad"))],
            [text_cell([b("Setting up "), t("takes two callbacks and an "
                       "allowlist: copy them from l3c_tank_drive.")]),
             Code(["BP32.update();   // the first line of loop(), every time"],
                  ("body", "pad"))],
            [text_cell([b("Sticks: "), t("{}. Pushing UP gives a NEGATIVE "
                       "number.".format(T["stick_range"]))]),
             Code(["int leftX  = myController->axisX();    // left and right",
                   "int leftY  = myController->axisY();    // UP is negative",
                   "int rightX = myController->axisRX();   // axisRY() for up and down"],
                  ("body", "pad"))],
            [text_cell([b("Buttons "), t("are named by POSITION, not by the "
                       "letter printed on the pad. True while held down.")]),
             Code(["a() bottom (B)   b() right (A)   x() left (Y)   y() top (X)",
                   "l1()  r1()  l2()  r2()   shoulders and triggers",
                   "thumbL()  thumbR()       click a stick in",
                   "bool upPressed = myController->dpad() & DPAD_UP;"])],
        ]

    pages.append(("14   The controller", W, pad_rows, None))

    pages.append(("15   The patterns the full program is built from", W, [
        [text_cell([b("FAILSAFE. "), t("No controller, no driving: the very "
                   "first lines of loop().")]),
         Code(["if ({}) {{".format(T["guard"]),
               "  drive(0, 0);",
               "  return;      // skip the rest of loop()",
               "}"], ("body", "patterns"))],
        [text_cell([b("DEADZONE AND MAP. "), t("Nearly centered counts as "
                   "centered; the rest of the stick is stretched across the "
                   "whole speed range.")]),
         Code(["int stickToSpeed(int stickValue, int maxSpeed) {",
               "  if (abs(stickValue) < STICK_DEADZONE) {",
               "    return 0;",
               "  }",
               "  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, maxSpeed);",
               "  size = constrain(size, MOTOR_MIN, maxSpeed);",
               "  return (stickValue > 0) ? size : -size;",
               "}"], ("top", "patterns"))],
        [text_cell([b("EDGE DETECTION. "), t("One press, one action, however "
                   "long the finger stays down. One wasDown per button.")]),
         Code(["bool justPressed(bool isDown, bool &wasDown) {",
               "  bool isNewPress = isDown && !wasDown;",
               "  wasDown = isDown;",
               "  return isNewPress;",
               "}"], ("top", "patterns"))],
        [text_cell([b("TOGGLE "), t("on each new press, and "), b("DIRTY "
                   "FLAG: "), t("redraw only when something changed.")]),
         Code(["if (justPressed({}, lightsWasDown)) {{".format(T["read_button"]),
               "  lightsOn = !lightsOn;",
               "  lightsChanged = true;",
               "}",
               "if (lightsChanged) {",
               "  lightsChanged = false;",
               "  showDrivingLights();",
               "}"], ("body", "patterns"))],
    ], None))

    if ps3:
        library_rows = [
            [Code(["Ps3Controller.h: No such file or directory"]),
             'Install "Fork of PS3 Controller Host".'],
            [Code(["implicit declaration of ... esp_base_mac_addr_set"]),
             "The ORIGINAL PS3 library is installed. Remove it; keep the Fork."],
            ["The controller will not connect",
             "PS3_MAC_ADDRESS still ends 00, or is not your number. Pad charged?"],
        ]
    else:
        library_rows = [
            [Code(["Bluepad32.h: No such file or directory"]),
             "Wrong board: pick the esp32_bluepad32 ESP32 Dev Module."],
            [Code(["MY_CONTROLLER has not been filled in"]),
             "Run l3a_controller_check and paste the line it prints."],
            ["The controller will not connect",
             "MY_CONTROLLER is not this pad. Is it charged? Hold SYNC."],
        ]

    pages.append(("16   When it will not compile, or will not work", [3.45, 4.15], [
        [b("What you see"), b("What it usually means")],
        [Code(["'speed' was not declared in this scope"]),
         "Spelled differently (capitals count), made inside other { }, or a "
         "missing #include."],
        [Code(["expected ';' before ..."]),
         "A missing ; - usually at the end of the line ABOVE."],
        [Code(["expected '}' at end of input"]), "A { with no matching }."],
        ] + library_rows + [
        [Code(["Failed to connect to ESP32"]),
         "Charge-only cable, wrong port, or the vehicle is off."],
        ["Serial Monitor shows nonsense", "The baud box is not 115200."],
        ["The LEDs do not change", "No strip.show()."],
        ["It creeps on its own", "STICK_DEADZONE is too small."],
    ], None))
    return pages


# ===================================================================
# Building the document
# ===================================================================

def led_map(doc, T):
    """The LED loop as two rows of sixteen: front on top, rear underneath,
    so the LED behind each front one is directly below it."""
    doc.para([b("The LED map. "), t("Front bar 0 (left) to 15 (right); the "
              "rear bar runs back, 16 (right) to 31 (left). The LED behind "
              "front LED p is  "), c("31 - p"), t(".  LEFT side: 0-7 and "
              "24-31.  RIGHT side: 8-15 and 16-23.  Corners: 0, 15, 16, 31.")],
             after=2, keep_next=True)
    width = 7.6
    side = 0.72
    cell = (width - side) / 16
    left_fill, right_fill = LIGHT_TEAL, "FBE3C8"
    sides = ["", Span(8, [[b("LEFT side of the vehicle")]]),
             Span(8, [[b("RIGHT side of the vehicle")]])]
    front = [[[b("FRONT")]]] + [str(i) for i in range(16)]
    rear = [[[b("REAR")]]] + [str(31 - i) for i in range(16)]
    fills = {(0, 1): left_fill, (0, 2): right_fill}
    for col in range(1, 17):
        fills[(1, col)] = fills[(2, col)] = left_fill if col <= 8 else right_fill
    doc.table([sides, front, rear], [side] + [cell] * 16,
              size=TEXT_PT, align="center", fills=fills, borders="all",
              para_after=0, line_height=1.0)


def section(doc, heading, widths, rows, note):
    doc.para([run(heading, bold=True, color=TEAL, size=11)], before=3, after=2,
             keep_next=True)
    if rows:
        header = bool(rows) and all(isinstance(cell, dict) for cell in rows[0])
        table_rows = []
        fills = {}
        for r_index, row in enumerate(rows):
            out_row = []
            for c_index, cell in enumerate(row):
                if isinstance(cell, (Code, Codes)):
                    out_row.append(code_cell(cell))
                    fills[(r_index, c_index)] = CODE_FILL
                elif isinstance(cell, dict):
                    out_row.append([[cell]])
                elif isinstance(cell, str):
                    out_row.append([[t(cell)]])
                else:
                    out_row.append(cell)
            table_rows.append(out_row)
        doc.table(table_rows, widths, header=header, size=TEXT_PT,
                  fills=fills, borders="lines", para_after=0, line_height=1.0)
    if note:
        doc.para(note, after=4)


def build_doc(T):
    doc = Document(margins=0.45, base_size=TEXT_PT,
                   footer="{}  |  Coding cheat sheet".format(T["track_label"]))
    first = True
    for item in content(T):
        if item[0] == "page":
            # The break goes ON the page's first line rather than in a
            # paragraph of its own: a page that is exactly full would push
            # a separate break paragraph over and leave a blank page.
            doc.para([run(T["track_label"].upper() + "   ", bold=True,
                          color=GREY, size=9),
                      run("CODING CHEAT SHEET", bold=True, color=GREY, size=9)],
                     after=0, page_break_before=not first)
            first = False
            doc.para([run(item[1], bold=True, color=NAVY, size=16)], after=2)
            if item[1].startswith("The C++ you need  (1"):
                doc.para([t("Everything you need to write the code in this "
                            "course. Stuck? Look here first, then ask your "
                            "group, then an instructor.")], after=2)
            continue
        if item[0] == "led-map":
            led_map(doc, T)
            continue
        heading, widths, rows, note = item
        section(doc, heading, widths, rows, note)
    return doc


def to_pdf(docx_path):
    soffice = first_existing(SOFFICE)
    if not soffice:
        print("  LibreOffice not found - skipping the PDF")
        return None
    out_dir = os.path.dirname(docx_path)
    subprocess.run([soffice, "--headless", "--convert-to", "pdf", "--outdir",
                    out_dir, docx_path], check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return os.path.splitext(docx_path)[0] + ".pdf"


def page_count(pdf_path):
    try:
        import pymupdf
    except ImportError:
        import fitz as pymupdf
    with pymupdf.open(pdf_path) as pdf:
        return len(pdf)


# ===================================================================
# --compile: every snippet through the real compiler
# ===================================================================

PRELUDE = {
    "ps3": """
#include <Arduino.h>
#include <Ps3Controller.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip(32, 5, NEO_GRB + NEO_KHZ800);
const int FRONT_FIRST = 0, FRONT_LAST = 15, REAR_FIRST = 16, REAR_LAST = 31;
const int MOTOR_MAX = 255, MOTOR_MIN = 0;
const int STICK_MAX = 127, STICK_DEADZONE = 20;
const int FRONT_LEFT_A = 12, FRONT_LEFT_B = 13;
void drive(int leftSpeed, int rightSpeed) { (void)leftSpeed; (void)rightSpeed; }
""",
    "switch": """
#include <Arduino.h>
#include <Bluepad32.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip(32, 5, NEO_GRB + NEO_KHZ800);
const int FRONT_FIRST = 0, FRONT_LAST = 15, REAR_FIRST = 16, REAR_LAST = 31;
const int MOTOR_MAX = 255, MOTOR_MIN = 0;
const int STICK_MAX = 511, STICK_DEADZONE = 60;
const int FRONT_LEFT_PIN_A = 12, FRONT_LEFT_CH_A = 0;
const int FRONT_LEFT_PIN_B = 13, FRONT_LEFT_CH_B = 1;
ControllerPtr myController = nullptr;
void drive(int leftSpeed, int rightSpeed) { (void)leftSpeed; (void)rightSpeed; }
""",
}

# Names a group's snippets use without declaring - the sheet shows the
# interesting line, not the boilerplate around it.
HIDDEN = {
    "math": "int count = 0;\nint duty = 128;\n",
    "loops": "int countdown = 3;\n",
    "lists": "const int ECHO_PIN = 35;\n",
    "logic": "int speed = 0;\nbool lightsOn = false;\nint stickX = 0;\n",
    "decide": "int speed = 300;\nbool lightsOn = true;\nint duty = 120;\n",
    "serial": "int speed = 200;\nfloat volts = 16.8;\nint forward = 0;\n",
    "timer": "bool blinkOn = false;\n",
    "motors": "int forward = 200;\nint turn = 60;\n",
    "patterns": "bool lightsWasDown = false;\nbool lightsOn = false;\n"
                "bool lightsChanged = false;\nvoid showDrivingLights() {}\n"
                "bool justPressed(bool isDown, bool &wasDown);\n",
}


def selftest_source(T):
    """One .cpp holding every compiled snippet, each group in a namespace."""
    track = "ps3" if is_ps3(T) else "switch"
    groups = {}
    includes = []
    for item in content(T):
        if item[0] in ("page", "led-map"):
            continue
        rows = item[2] or []
        for row in rows:
            for cell in row:
                blocks = cell if isinstance(cell, Codes) else [cell]
                for block in blocks:
                    if not isinstance(block, Code) or block.ctx is None:
                        continue
                    kind, group = block.ctx
                    lines = []
                    for line in block.lines:
                        if line.startswith("#include"):
                            includes.append(line)
                        else:
                            lines.append(line)
                    groups.setdefault(group, []).append((kind, lines))

    out = [PRELUDE[track]] + sorted(set(includes))
    for group, blocks in groups.items():
        out.append("namespace cheat_{} {{".format(group))
        out.append(HIDDEN.get(group, ""))
        for index, (kind, lines) in enumerate(blocks):
            if kind == "top":
                out.extend(lines)
            else:
                out.append("void body_{}() {{".format(index))
                out.extend("  " + line for line in lines)
                out.append("}")
        out.append("}  // namespace cheat_" + group)
    return "\n".join(out) + "\n"


def selftest(T):
    cli = first_existing(ARDUINO_CLI)
    if not cli:
        raise SystemExit("--compile needs arduino-cli; set ARDUINO_CLI")
    fqbn = "esp32:esp32:esp32" if is_ps3(T) else "esp32-bluepad32:esp32:esp32"
    work = tempfile.mkdtemp(prefix="cheatsheet_")
    sketch = os.path.join(work, "cheatsheet_selftest")
    os.makedirs(sketch)
    with open(os.path.join(sketch, "cheatsheet_selftest.ino"), "w") as handle:
        handle.write("// Every snippet is in snippets.cpp.\n"
                     "void setup() {}\nvoid loop() {}\n")
    with open(os.path.join(sketch, "snippets.cpp"), "w") as handle:
        handle.write(selftest_source(T))
    result = subprocess.run([cli, "compile", "--fqbn", fqbn, sketch],
                            capture_output=True, text=True)
    if result.returncode != 0:
        print(result.stdout[-4000:])
        print(result.stderr[-4000:])
        raise SystemExit("cheat sheet snippets do not compile for %s (kept in %s)"
                         % (T["key"], sketch))
    shutil.rmtree(work, ignore_errors=True)
    print("  every snippet compiles for", fqbn)


# ===================================================================

TRACKS = {"beginner-ps3": content_beginner.PS3,
          "beginner-switch": content_beginner.SWITCH}


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    compile_too = "--compile" in sys.argv
    wanted = args or list(TRACKS)
    for name in wanted:
        if name not in TRACKS:
            raise SystemExit("unknown track %s; choose from %s"
                             % (name, ", ".join(TRACKS)))
        T = content_beginner.enrich(TRACKS[name])
        out = os.path.join(COURSES, name, "handouts", "coding_cheat_sheet.docx")
        build_doc(T).save(out)
        print(name)
        print("  " + os.path.relpath(out, COURSES))
        pdf = to_pdf(out)
        if pdf:
            pages = page_count(pdf)
            print("  " + os.path.relpath(pdf, COURSES), "-", pages, "pages")
            if pages != 4:
                raise SystemExit("the cheat sheet must be exactly 4 pages, "
                                 "and %s is %d" % (name, pages))
        if compile_too:
            selftest(T)


if __name__ == "__main__":
    main()
