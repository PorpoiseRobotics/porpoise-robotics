"""
content_beginner.py - the five beginner lessons.

The PS3 track and the Nintendo Switch track teach the same course. They differ
in the controller, the board package, the stick range, and the way a vehicle is
locked to one controller - so the lessons are written once here and the
differences come from the TRACK dictionary passed in.

Each lesson is a function that takes a Deck and a track, and fills it.
"""

import os

import diagrams
import srcfacts
from slidelib import Deck

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

# ===================================================================
# TRACKS
# ===================================================================

PS3 = {
    "key": "beginner-ps3",
    "src": "ground-vehicle/src/lessons/beginner_ps3",
    "track_label": "Pathfinder Beginner - PS3",
    "pad": "Sony PS3 controller",
    "pad_short": "PS3 controller",
    "full_program": "pathfinder_ps3",
    "board_pkg": "esp32  by Espressif Systems,  version 3.0.7",
    "board_menu": "Tools > Board > ESP32 Arduino > \"ESP32 Dev Module\"",
    "boards_url": None,
    "extra_lib": "\"PS3 Controller Host\" by Jeffrey van Pernis",
    "extra_lib_note": "Searching Library Manager for \"Ps3Controller\" finds nothing. "
                      "The display name is the longer one.",
    "lib_short": "PS3 Controller Host",
    "lib_and": " + PS3 Controller Host",
    "drive_sketch": "lessons/beginner_ps3/l3c_tank_drive/l3c_tank_drive.ino",
    "led_sketch": "lessons/beginner_ps3/l4a_all_one_colour/l4a_all_one_colour.ino",
    "pin_style": "pin",
    "stick_range": "-128 to +127",
    "deadzone_pct": "about 16%",
    "attach": "ledcAttach(pin, freq, bits);",
    "write": "ledcWrite(pin, duty);",
    "pwm_story": "This board package lets you talk straight to the pin.",
    "hero": img("vehicle-with-ps3-controller.jpg"),
    "pad_image": img("ps3-button-map.png"),
    "lock_story": "the CONTROLLER is told which Bluetooth address to talk to",
    "lock_tool": "SixaxisPairTool, on a Windows PC, over a USB cable",
    "btn_scanner": "TRIANGLE",
    "btn_lights": "SQUARE",
    "btn_stickclick": "L3 (click the left stick in)",
    "read_x": "Ps3.data.analog.stick.lx",
    "read_y": "Ps3.data.analog.stick.ly",
    "read_button": "Ps3.data.button.square",
    "connected": "Ps3.isConnected()",
    "guard": "!Ps3.isConnected()",
}

SWITCH = {
    "key": "beginner-switch",
    "src": "ground-vehicle/src/lessons/beginner_switch",
    "track_label": "Pathfinder Beginner - Nintendo Switch",
    "pad": "Nintendo Switch style controller",
    "pad_short": "Switch controller",
    "full_program": "pathfinder_nintendoswitch",
    "board_pkg": "esp32_bluepad32  by Ricardo Quesada,  version 4.1.0",
    "board_menu": "Tools > Board > esp32_bluepad32 > \"ESP32 Dev Module\"",
    "boards_url": "https://raw.githubusercontent.com/ricardoquesada/"
                  "esp32-arduino-lib-builder/master/bluepad32_files/"
                  "package_esp32_bluepad32_index.json",
    "extra_lib": "none - Bluepad32 arrives with the board package",
    "extra_lib_note": "You still need \"Adafruit NeoPixel\" by Adafruit for the "
                      "lighting lessons.",
    "lib_short": "none (Bluepad32 ships with the board package)",
    "lib_and": "",
    "drive_sketch": "lessons/beginner_switch/l3c_tank_drive/l3c_tank_drive.ino",
    "led_sketch": "lessons/beginner_switch/l4a_all_one_colour/l4a_all_one_colour.ino",
    "pin_style": "channel",
    "stick_range": "-512 to +511",
    "deadzone_pct": "about 12%",
    "attach": "ledcSetup(channel, freq, bits);\nledcAttachPin(pin, channel);",
    "write": "ledcWrite(channel, duty);",
    "pwm_story": "This board package is built on an older ESP32 core, so you "
                 "work through numbered PWM channels.",
    "hero": img("vehicle-front-blue-leds.jpg"),
    "pad_image": img("switch-controller-labelled.jpg"),
    "lock_story": "the VEHICLE is told which controller address it will accept",
    "lock_tool": "l3a_controller_check, which prints the address for you",
    "btn_scanner": "the TOP face button (marked X on most Switch pads)",
    "btn_lights": "the LEFT face button (marked Y on most Switch pads)",
    "btn_stickclick": "clicking the left stick in",
    "read_x": "myController->axisX()",
    "read_y": "myController->axisY()",
    "read_button": "myController->x()",
    "connected": "myController->isConnected()",
    "guard": "myController == nullptr || !myController->isConnected()",
}


# The stages each lesson moves through, used by the progress markers.
STAGES = {
    "lesson1": [
        "The vehicle",
        "Setting up the IDE",
        "Your first circuit and program",
        "Talking back",
        "Your first light",
    ],
    "lesson2": [
        "H-bridges and PWM",
        "One motor",
        "Duty against speed",
        "Ohm's law and circuits",
        "Driving a square",
    ],
    "lesson3": [
        "Bluetooth and pairing",
        "What the pad sends",
        "Deadzone and map",
        "Driving it",
    ],
    "lesson4": [
        "Addressable LEDs",
        "The power budget",
        "The LED map",
        "Patterns",
    ],
    "lesson5": [
        "Why delay() has to go",
        "Two speeds at once",
        "Edge detection",
        "Lights from state",
        "The full program",
    ],
}

# ===================================================================
# LESSON 1
# ===================================================================

def lesson1(deck, T):
    deck.title_slide(
        "Mechatronics, the Pathfinder vehicle, the Arduino IDE, and your "
        "first three programs.",
        BYLINE + ["", "Three hours.  Work in groups of three."],
        hero_image=T["hero"], logo=LOGO)

    deck.objectives([
        "Say what mechatronics is, and point to three parts of the "
         "Pathfinder that are mechanical, electrical and software",
        "Set up the Arduino IDE, the board package and the libraries, "
         "and get the course files into your Arduino folder",
        "Name the three parts of every Arduino program and say when "
         "each runs",
        "Build a working LED circuit on a breadboard, and say why the "
         "resistor is there",
        "Upload a program to the vehicle and change it",
        "Read what the vehicle prints back on the Serial Monitor",
        "Light any one of the 32 LEDs, in any colour",
    ])

    deck.bullets(
        "Welcome",
        [("This is a five-lesson course. Each lesson is three hours, and each one "
          "ends with your vehicle doing something it could not do at the start.", 0),
         ("Please make it interactive. Questions, comments and better ideas are all "
          "welcome, and the best ones usually come from somebody who has just "
          "broken something.", 0),
         ("You work in groups of three and share a vehicle. There are more vehicles "
          "if your group wants one each.", 0),
         ("You will read programs, change them, upload them, and see what happens. "
          "Modifying a working program is the fastest way to learn to write one.", 0),
         ("Very often the vehicle will do exactly what you told it to do, and not "
          "at all what you meant. That is not a setback. That is the job.", 0)],
        note="By the end of Lesson 5 you will be driving a vehicle you understand "
             "line by line, and racing it.")

    deck.table(
        "Where this course goes",
        ["Lesson", "Subject", "What your vehicle can do by the end"],
        [["1", "Introduction and the toolchain", "Blink an LED, print to the screen, light one pixel"],
         ["2", "Motor control", "Drive itself round a square you calculated"],
         ["3", "Controller programming", "Be driven by you, with a thumbstick"],
         ["4", "NeoPixel LEDs", "Run headlights, signals and light patterns"],
         ["5", "Maneuvers with lights", "Everything at once - and race"]],
        col_widths=[0.8, 4, 6],
        note="Every lesson builds directly on the one before it. Nothing gets "
             "thrown away.")

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "What mechatronics is, and what a Pathfinder is"],
         ["0:30", "A tour of the vehicle: computer, motors, lights, battery"],
         ["0:55", "BREAK  (10 minutes)"],
         ["1:05", "Install the Arduino IDE, the board package and the libraries"],
         ["1:40", "The three parts of every program.  Upload l1a_blink"],
         ["2:05", "BREAK  (10 minutes)"],
         ["2:15", "Talking back: the Serial Monitor.  Upload l1b_serial_monitor"],
         ["2:40", "Your first light.  Upload l1c_first_pixel"],
         ["2:55", "Safety, batteries, and what to expect next lesson"]],
        col_widths=[1, 9])

    deck.bullets_image(
        "Mechatronics",
        [("Much of the design and building of robots belongs to a field called "
          "MECHATRONICS - mechanical systems, electrical systems, and the "
          "computers that control them, treated as one thing.", 0),
         ("An unmanned ground vehicle is one small corner of it.", 0),
         ("What you learn here applies to the rest: factory automation, "
          "appliances, printers, cars, aircraft, and undersea vehicles.", 0),
         ("The reason we teach it with a rover is that you can watch a rover, "
          "reach it when it gets stuck, and a mistake means a bumped wall rather "
          "than a flooded electronics bay.", 0)],
        img("engineering-to-mechatronics.png"),
        caption="Engineering, narrowing down to what you are about to build",
        image_ratio=0.60)

    deck.two_columns(
        "STEM, and where it shows up in this course",
        "Maths and science you will actually use",
        [("Circumference and pi, to work out how far one wheel turn takes you", 0),
         ("Distance = speed x time, for every pre-programmed maneuver", 0),
         ("Ratios and percentages, for duty cycle", 0),
         ("Linear mapping, to turn a stick reading into a motor speed", 0),
         ("Ohm's law and power, for the LED current budget", 0),
         ("Wavelength and colour perception, for the lighting", 0),
         ("Trigonometry, when you plot where a maneuver ends up", 0)],
        "Engineering practice",
        [("Reading somebody else's program before changing it", 0),
         ("Changing ONE thing at a time and observing the result", 0),
         ("Measuring rather than guessing", 0),
         ("Writing down what you measured", 0),
         ("Working out whether a fault is mechanical, electrical or software", 0),
         ("Version control, so a working program is never lost", 0)],
        note="STEM is not four subjects. It is using maths and science to build "
             "something that has to actually work.")

    deck.image_pair(
        "The Pathfinder",
        img("vehicle-front-blue-leds.jpg"),
        "Turn signal running on one side...",
        img("vehicle-side-blue-leds.jpg"),
        "...and on the other. Same 32 LEDs, different program.",
        lead="A fast, rugged, four-wheel-drive vehicle with an ESP32 computer, "
             "four independently driven motors, 32 programmable LEDs, four "
             "servo outputs and a rechargeable lithium battery.",
        speaker=[
            "Pass a vehicle round while you talk over this slide. Let them pick "
            "it up - it is built to be handled.",
            "Point out that BOTH pictures are the same vehicle running the same "
            "hardware. The only difference is which LEDs the program lit.",
            "That is the through-line of the whole course: the hardware is "
            "fixed, and everything interesting comes from the program.",
        ])

    deck.bullets(
        "What is on the vehicle",
        [("ESP32 computer - a small, fast microcontroller with Bluetooth and "
          "Wi-Fi built in. Your program runs here.", 0),
         ("Four brushed DC motors, each with its own DRV8871 H-bridge driver.", 0),
         ("32 WS2812B addressable LEDs, in two bars of 16.", 0),
         ("Four servo outputs on the top plate.", 0),
         ("A 4S 3300 mAh lithium polymer battery, about 16 volts charged.", 0),
         ("A top plate with a breadboard area, for the sensors you add later.", 0)],
        note="No steering rack. It turns by driving one side faster than the "
             "other, like a tank. That matters from Lesson 2 onwards.",
        speaker=[
            "Name each part while holding a vehicle, then put the labelled "
            "picture up on the next slide and let them match the two.",
            "The tank-steering point is the one to land. Everything in Lesson 2 "
            "follows from it.",
        ])

    deck.image_slide(
        "Every part, called out",
        img("vehicle-parts-labelled.jpg"),
        caption="Keep this page open for the rest of the lesson",
        speaker=[
            "Give them a minute to find each part on a real vehicle.",
            "Worth pointing out the two LED bars are 16 each, front and rear - "
            "that becomes important in Lesson 4.",
        ])

    deck.two_columns(
        "Gen 2 and Gen 3  -  which vehicle are you holding?",
        "Gen 2",
        [("The vehicle most of the fleet is built from.", 0),
         ("ESP32, four DRV8871 H-bridges, 32 LEDs, four servo outputs.", 0),
         ("No current sensor.", 0),
         ("", 0),
         ("Everything in this course works on it. Nothing in these five "
          "lessons needs anything a Gen 2 does not have.", 0)],
        "Gen 3",
        [("A UNIBODY frame. The structure is one piece, and the board, battery "
          "and wiring sit inside it rather than on top, so nothing catches on "
          "anything when the vehicle rolls.", 0),
         ("The wheels are mounted at the vehicle's VERTICAL CENTRE, so it "
          "drives just as well upside down. Flip it over and keep going.", 0),
         ("A clear lexan TOP PLATE: somewhere to mount sensors, and you can "
          "see the electronics working underneath it.", 0),
         ("An INA219 power monitor, so the vehicle can measure its own battery "
          "voltage and current draw - which is what makes a powered self-test "
          "possible.", 0)],
        note="The programs detect which one they are running on at boot, by "
             "looking for the sensor. One program, either vehicle - you do not "
             "have to know which you have before you upload.")

    diagrams.system_block(deck, controller=T["pad_short"])

    deck.two_columns(
        "What a microcontroller actually is",
        "The ESP32",
        [("A whole computer on one chip: processor, memory, storage, and the "
          "hardware to drive pins.", 0),
         ("240 MHz, dual core, with Wi-Fi and Bluetooth built in.", 0),
         ("It runs ONE program. There is no desktop, no windows, no files.", 0),
         ("It starts running the instant it has power, and stops when it "
          "does not.", 0),
         ("We compile programs in C++ on a laptop and send them down the USB "
          "cable into its flash memory.", 0)],
        "How that differs from your laptop",
        [("Your laptop runs hundreds of programs at once and shares its "
          "processor between them.", 0),
         ("The ESP32 gives your program the whole machine. Nothing interrupts "
          "you, and nothing rescues you either.", 0),
         ("If your program stops, the vehicle stops. If your program waits, "
          "the vehicle waits. You will meet that head-on in Lesson 5.", 0),
         ("There is no error message on a screen unless you print one. That "
          "is why the Serial Monitor matters so much.", 0)])

    deck.section("Setting up the Arduino IDE", minutes=35)

    deck.bullets(
        "Step 1  -  install the Arduino IDE",
        [("Go to arduino.cc/en/software and download the Arduino IDE, version "
          "2.3.4 or newer.", 0),
         ("Windows: install it and sign in to your Microsoft account if it asks. "
          "You can ignore the donation and newsletter prompts.", 1),
         ("Mac: open the downloaded .zip and drag Arduino.app into your "
          "Applications folder.", 1),
         ("It needs about 500 MB of disk space.", 1),
         ("", 0),
         ("The IDE is where you will write code, compile it, upload it to the "
          "vehicle, and read what the vehicle prints back.", 0),
         ("\"Compile\" means turn your C++ into the machine language the ESP32 "
          "runs. \"Upload\" means send that down the USB cable.", 0)],
        note="If the install will not go through, pair up with a neighbour and "
             "carry on - you lose nothing today. Note down the error and we "
             "will sort the machine out afterwards.")

    _board_setup_slide(deck, T)

    deck.bullets(
        "Step 3  -  select the board and the port",
        [("BOARD:  {}".format(T["board_menu"]), 0),
         ("Then set Upload Speed to 921600 and Flash Frequency to 80 MHz.", 1),
         ("", 0),
         ("PORT:  plug the vehicle into your computer with a micro USB cable "
          "FIRST, then choose Tools > Port.", 0),
         ("Windows: the next available COM port, such as COM4.", 1),
         ("Mac: something like /dev/cu.usbserial-14130.", 1),
         ("", 0),
         ("If no port appears at all, the usual causes are: the vehicle is not "
          "switched on, or the cable is a charge-only cable with no data wires "
          "in it. Try a different cable before you try anything else.", 0)],
        note="A charge-only USB cable looks identical to a real one and is the "
             "single most common reason a board \"will not connect\".",
        note_kind="warn")

    deck.bullets(
        "Step 4  -  install the libraries",
        [("A LIBRARY is code somebody else already wrote and tested, which your "
          "program can call instead of you writing it again.", 0),
         ("Tools > Manage Libraries, then search and click Install.", 0),
         ("", 0),
         ("\"Adafruit NeoPixel\" by Adafruit  -  drives the 32 LEDs.", 0),
         (T["extra_lib"] if T["extra_lib"].startswith("none")
          else "{}  -  talks to the controller.".format(T["extra_lib"]), 0),
         ("", 0),
         (T["extra_lib_note"], 0)],
        note="Without the NeoPixel library, nothing in Lesson 4 will compile.")

    deck.table(
        "Every program in this course, and what it needs",
        ["Lesson", "Sketch", "Libraries beyond the board package"],
        [["1", "l1a_blink", "none"],
         ["1", "l1b_serial_monitor", "none"],
         ["1", "l1c_first_pixel", "Adafruit NeoPixel"],
         ["2", "l2a_one_motor", "none"],
         ["2", "l2b_speed_ramp", "none"],
         ["2", "l2c_maneuver_square", "none"],
         ["3", "l3a_controller_check", T["lib_short"]],
         ["3", "l3b_deadzone_and_map", T["lib_short"]],
         ["3", "l3c_tank_drive", T["lib_short"]],
         ["4", "l4a / l4b / l4c", "Adafruit NeoPixel"],
         ["5", "l5a / l5b / l5c", "Adafruit NeoPixel" + T["lib_and"]],
         ["5", T["full_program"], "Adafruit NeoPixel" + T["lib_and"]]],
        lead="Install both libraries now and you are set for all five lessons.",
        col_widths=[1, 3.6, 5.4],
        size=14,
        note="If a sketch will not compile, check this table before you change "
             "any code. A missing library is by far the most common cause.",
        speaker=[
            "This is the reference page. Tell students to photograph it.",
            "Worth saying out loud: the board package is not a library. The "
            "board package teaches the IDE what an ESP32 is; a library is code "
            "your program calls.",
        ])

    deck.bullets(
        "Getting the course files onto your computer",
        [("The sketches arrive on a thumb drive, or as a zip in an email. They "
          "have to end up in the right folder or the IDE will not find them.", 0),
         ("", 0),
         ("WINDOWS", 0),
         ("Copy the sketch folders into  Documents\\\\Arduino\\\\", 1),
         ("Copy the library folders into  Documents\\\\Arduino\\\\libraries\\\\", 1),
         ("", 0),
         ("MAC", 0),
         ("Copy the sketch folders into  Documents/Arduino/", 1),
         ("Copy the library folders into  Documents/Arduino/libraries/", 1),
         ("", 0),
         ("If they came as .zip files, UNZIP them first - except a library you "
          "are adding through Sketch > Include Library > Add .ZIP Library, "
          "which wants the zip as it is.", 0),
         ("", 0),
         ("Then restart the Arduino IDE. It only looks for sketches and "
          "libraries when it starts.", 0)],
        lead="Thumb drive or email, into your Arduino folder",
        note="One folder per sketch, and the folder name must match the .ino "
             "inside it. l1a_blink/l1a_blink.ino. The IDE will refuse to open a "
             "sketch whose folder is named differently.",
        note_kind="warn",
        speaker=[
            "Do this together. It is the single most common place a beginner "
            "gets stuck, and it costs ten minutes now against forty later.",
            "Walk the room. Check File > Sketchbook actually lists the lessons "
            "before you move on.",
        ])

    deck.section("Your first program", minutes=25)

    diagrams.program_structure(deck)

    deck.bullets(
        "The ESP32 has no LED we can use, so you are going to give it one",
        [("Some development boards have a little LED soldered on. The module on "
          "this vehicle does not have one free for us, so your first program "
          "will blink an LED you wire up yourself.", 0),
         ("", 0),
         ("That is a better place to start anyway. Before you can make "
          "something blink, you have to make a circuit that works - and a "
          "circuit is the thing underneath everything else on this vehicle.", 0),
         ("", 0),
         ("You need three things from the kit:", 0),
         ("a breadboard", 1),
         ("an LED", 1),
         ("a 220 ohm resistor", 1),
         ("two jumper wires", 1)],
        lead="Your first circuit",
        speaker=[
            "Hold up the three parts as you name them.",
            "If anyone asks why the resistor: tell them to wait ninety seconds, "
            "it is the next slide but one.",
        ])

    deck.image_pair(
        "What a breadboard is",
        img("breadboard-parts-named.jpg"),
        "Terminal strips down the middle, power rails along the edges",
        img("breadboard-parts.jpg"),
        "A 220 ohm resistor and an LED. Note the LED has one leg longer.",
        lead="A breadboard lets you build a circuit without soldering, and take "
             "it apart again.",
        note="The five holes in a row are joined to each other under the "
             "plastic. That is the whole trick: push two legs into the same "
             "row and they are connected.",
        speaker=[
            "Get them to look down into the holes. The metal clips are visible.",
            "The row-of-five rule is the single fact that makes breadboards "
            "make sense. Say it twice.",
        ])

    diagrams.led_circuit(deck)

    deck.image_pair(
        "Building it",
        img("breadboard-step1.jpg"),
        "Resistor and LED in the same row, so they are connected",
        img("breadboard-complete.jpg"),
        "Powered up. Kevin's photos use a 9 V battery; yours runs from GPIO 2.",
        lead="Bend the resistor legs, push everything into the board, and "
             "connect GPIO 2 and GND with the two jumper wires.",
        note="Long leg of the LED towards the resistor and GPIO 2. Short leg "
             "towards ground. Backwards means no light, no damage - just turn "
             "it round.",
        note_kind="warn",
        speaker=[
            "Circulate. The two failures you will see are the LED in "
            "backwards and legs in different rows so nothing is connected.",
            "Nobody uploads anything until their circuit is built and checked.",
        ])

    deck.code(
        "l1a_blink  -  the whole program",
        ["const int LED_PIN = 2;   // the pin your LED is wired to",
         "const int BLINK_MS = 1000;",
         "",
         "void setup() {",
         "  pinMode(LED_PIN, OUTPUT);",
         "}",
         "",
         "void loop() {",
         "  digitalWrite(LED_PIN, HIGH);",
         "  delay(BLINK_MS);",
         "",
         "  digitalWrite(LED_PIN, LOW);",
         "  delay(BLINK_MS);",
         "}"],
        filename="l1a_blink.ino",
        notes=[("const int  says this is a whole number that never changes. "
                "Naming it means you change it in one place - move your LED "
                "to another pin and only this line changes.", 0),
               ("pinMode(..., OUTPUT)  tells the ESP32 we intend to WRITE to "
                "that pin. Without it, digitalWrite does nothing.", 0),
               ("HIGH puts 3.3 volts on the pin.  LOW puts 0 volts on it. "
                "That is the entire idea of digital output.", 0),
               ("delay(1000)  waits one second. Remember this line - in Lesson 5 "
                "we will throw it away.", 0)],
        size=14)

    deck.progress(STAGES["lesson1"], 1)

    deck.activity(
        "Do it now  -  blink",
        "l1a_blink",
        [("1.  File > Open, and find l1a_blink.ino.", 0),
         ("2.  Click the tick to VERIFY. It should compile with no errors.", 0),
         ("3.  Click the arrow to UPLOAD. Do not unplug during upload.", 0),
         ("4.  Wait for \"Done uploading\" and look at your LED.", 0),
         ("", 0),
         ("Then change things:", 0),
         ("5.  Set BLINK_MS to 100. Upload. Then 2000. Upload.", 0),
         ("6.  Delete ONE of the two delay lines and upload.", 0)],
        expect=[("YOUR LED, on the breadboard, one second on and one "
                 "second off.", 0),
                ("Nothing at all means the LED is in backwards, or a leg is "
                 "in the wrong row.", 0)],
        questions=[("Which makes it blink faster, a bigger number or a "
                    "smaller one? Why?", 0),
                   ("With one delay deleted, the LED is still blinking. How "
                    "fast, and why can you barely see it?", 0),
                   ("What resistor would a RED LED need? It drops about "
                    "1.8 V instead of 2.0 V.", 0)],
        minutes=15)

    deck.section("Making the vehicle talk back", minutes=25)

    deck.bullets(
        "The Serial Monitor",
        [("The ESP32 has no screen. If you want to know what it is thinking, it "
          "has to tell you - down the same USB cable you upload through.", 0),
         ("Serial.begin(115200);   opens the connection. Put it in setup().", 1),
         ("Serial.print(\"text\");    prints and stays on the same line.", 1),
         ("Serial.println(x);       prints and moves to a new line.", 1),
         ("", 0),
         ("Open it with Tools > Serial Monitor, or the magnifying glass at the "
          "top right.", 0),
         ("The baud rate box at the bottom MUST say 115200, because that is the "
          "number in Serial.begin(). Rows of nonsense characters mean the two "
          "numbers disagree.", 0),
         ("", 0),
         ("When something does not work, print the numbers your program is "
          "working from. That will find the fault faster than staring at the "
          "code will.", 0)],
        note="Every program from here on prints something. Get in the habit of "
             "having the Serial Monitor open.")

    deck.table(
        "Variables: named boxes that hold a value",
        ["Type", "Holds", "Example", "Used for"],
        [["int", "A whole number", "int loopCount = 0;", "Speeds, pin numbers, counts"],
         ["float", "A number with a decimal point", "float volts = 16.8;", "Voltages, measurements"],
         ["bool", "Only true or false", "bool lightsOn = true;", "Switches and flags"],
         ["const int", "A whole number that never changes", "const int LED_PIN = 2;", "Pins and settings"],
         ["unsigned long", "A big whole number, never negative", "unsigned long now;", "millis(), in Lesson 5"]],
        col_widths=[1.8, 3, 3, 3],
        note="const means the compiler will stop you changing it by accident. "
             "Use it for anything that is a setting rather than a working value.")

    deck.code(
        "One trap worth meeting on day one",
        ["Serial.print(\"7 / 2 = \");",
         "Serial.println(7 / 2);          // prints 3, not 3.5",
         "",
         "Serial.print(\"7.0 / 2 = \");",
         "Serial.println(7.0 / 2);        // prints 3.50",
         "",
         "Serial.print(\"7 % 2 = \");",
         "Serial.println(7 % 2);          // prints 1"],
        filename="from l1b_serial_monitor.ino",
        notes=[("Both 7 and 2 are whole numbers, so the ESP32 does whole-number "
                "division and throws the remainder away.", 0),
               ("It does NOT round. 7 / 2 is 3, and 9 / 10 is 0.", 0),
               ("Writing one number with a decimal point asks for decimal "
                "division instead.", 0),
               ("The % operator gives you the remainder that the first "
                "division discarded.", 0),
               ("This bites everybody eventually, usually when a speed "
                "calculation quietly comes out as zero.", 0)],
        size=14, highlight={1, 4})

    deck.progress(STAGES["lesson1"], 2)

    deck.activity(
        "Do it now  -  make it talk",
        "l1b_serial_monitor",
        [("1.  Open and upload l1b_serial_monitor.ino.", 0),
         ("2.  Open the Serial Monitor and set the baud rate to 115200.", 0),
         ("3.  Press the EN (reset) button on the ESP32. Watch it start again.", 0),
         ("4.  Change the greeting to your group's name and upload.", 0),
         ("5.  Change the two numbers in the maths section. PREDICT the answers "
          "before you upload, then check.", 0),
         ("6.  Add a line of your own that prints every time round the loop.", 0)],
        expect=[("A greeting, some arithmetic, then one line per second "
                 "counting up.", 0)],
        questions=[("What is 9 / 10 in this program? What did you expect?", 0),
                   ("What does millis() / 1000 give you, and why?", 0)],
        minutes=15)

    deck.section("Your first light", minutes=20)

    deck.bullets(
        "An addressable LED is a small computer of its own",
        [("An ordinary LED has two wires and is either on or off.", 0),
         ("A NeoPixel has FOUR connections - power, ground, data in, data out - "
          "and a tiny controller chip inside it.", 0),
         ("Send a colour message into the first one and it keeps the first "
          "message, then passes everything after it down the chain to the next.", 0),
         ("That is why all 32 LEDs on the vehicle need only ONE signal wire, "
          "on GPIO 5.", 0),
         ("It is also why they are NUMBERED, 0 to 31, in the order the data "
          "flows. You will map that out properly in Lesson 4.", 0),
         ("", 0),
         ("Each one holds three LEDs: red, green and blue. Every colour you can "
          "make is a mixture of those three, each from 0 to 255.", 0)],
        note="setPixelColor() only changes the ESP32's memory. Nothing happens "
             "on the vehicle until you call show(). This is the most common "
             "NeoPixel mistake there is.",
        note_kind="warn")

    deck.code(
        "l1c_first_pixel  -  the important lines",
        ["#include <Adafruit_NeoPixel.h>",
         "",
         "const int LED_PIN   = 5;      // all 32 LEDs are on GPIO 5",
         "const int LED_COUNT = 32;",
         "const int WHICH_LED = 0;      // the one we light",
         "const int BRIGHTNESS = 60;    // master brightness, 0 to 255",
         "",
         "Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);",
         "",
         "void setup() {",
         "  strip.begin();",
         "  strip.setBrightness(BRIGHTNESS);",
         "  strip.clear();",
         "  strip.show();",
         "}",
         "",
         "void loop() {",
         "  strip.setPixelColor(WHICH_LED, strip.Color(255, 0, 0));",
         "  strip.show();",
         "  delay(1000);",
         "  ...",
         "}"],
        filename="l1c_first_pixel.ino",
        notes=[("#include pulls in the library.", 0),
               ("NEO_GRB says these LEDs want their colour data green first. "
                "NEO_KHZ800 is the speed it is clocked out at. Both are correct "
                "for the WS2812B parts we use.", 0),
               ("strip.begin() wakes the strip up. Always needed.", 0),
               ("Color(red, green, blue), each 0 to 255.", 0),
               ("show() is what actually sends it.", 0)],
        size=12, highlight={18})

    deck.progress(STAGES["lesson1"], 3)

    deck.activity(
        "Do it now  -  light one pixel",
        "l1c_first_pixel",
        [("1.  Upload it and find which physical LED comes on.", 0),
         ("2.  Change WHICH_LED to 15, then 16, then 31, uploading each time.", 0),
         ("3.  Draw the vehicle on paper and mark where LEDs 0, 8, 15, 16, 24 "
          "and 31 are. Keep that drawing - you need it in Lesson 4.", 0),
         ("4.  Invent a colour with strip.Color(r, g, b). What do you get from "
          "(255, 255, 0)? From (128, 0, 128)?", 0),
         ("5.  Delete the strip.show() line and upload. What happens?", 0)],
        expect=[("One LED cycling red, green, blue, white, off.", 0)],
        questions=[("Where is LED 0 on the vehicle? Where is LED 31?", 0),
                   ("With show() deleted, why does nothing happen?", 0)],
        minutes=15)

    deck.bullets(
        "Safety, and looking after the vehicle",
        [("LITHIUM BATTERIES. 4S, 3300 mAh, about 16 volts and a lot of stored "
          "energy. Never puncture, crush or short one. If a pack is swollen, hot, "
          "or has been dropped hard, stop and tell an instructor.", 0),
         ("Charge only on the bench, only supervised, and never leave a charging "
          "pack unattended.", 0),
         ("WHEELS OFF THE GROUND when you upload a change for the first time. A "
          "vehicle on a block cannot drive off the desk.", 0),
         ("Keep fingers, hair, sleeves and USB cables away from the wheels.", 0),
         ("Know where your power switch is BEFORE you start a program that "
          "drives.", 0),
         ("Do not stare into the LEDs at full brightness from close up.", 0),
         ("Carry the vehicle by the base plate, not by the top plate or the "
          "wires.", 0)],
        note="Everything in Lesson 2 onwards moves. This page stops being "
             "theoretical next lesson.",
        note_kind="safety")

    deck.quiz(
        "Check yourself",
        [("1.  Name the three parts of an Arduino program, and say how many "
          "times each one runs.", 0),
         ("2.  What does pinMode(LED_PIN, OUTPUT) do, and what happens if you "
          "leave it out?", 0),
         ("3.  Your Serial Monitor shows rows of strange characters. What is "
          "almost certainly wrong?", 0),
         ("4.  What does 7 / 2 evaluate to in C++? What about 7.0 / 2?", 0),
         ("5.  All 32 LEDs share one data wire. How does LED number 20 know "
          "that a message is for it?", 0),
         ("6.  You call setPixelColor() and nothing lights up. What did you "
          "forget?", 0)],
        lead="Next lesson: motors. Bring the drawing you made of the LED numbers.")

    return deck


def _board_setup_slide(deck, T):
    if T["boards_url"]:
        deck.bullets(
            "Step 2  -  install the board package",
            [("The Arduino IDE does not know about the ESP32 until you add it.", 0),
             ("", 0),
             ("1.  File > Preferences > \"Additional boards manager URLs\".", 0),
             ("2.  Paste this in (one line, no spaces):", 0),
             (T["boards_url"], 1),
             ("3.  Click OK.", 0),
             ("4.  Tools > Board > Boards Manager, search \"bluepad32\", install "
              "it. Choose version 4.1.0.", 0),
             ("5.  Tools > Board > esp32_bluepad32 > \"ESP32 Dev Module\".", 0),
             ("", 0),
             ("Package:  {}".format(T["board_pkg"]), 0)],
            note="You now have TWO entries called \"ESP32 Dev Module\" - one under "
                 "\"esp32\" and one under \"esp32_bluepad32\". Everything on this "
                 "track needs the esp32_bluepad32 one. A pile of errors that make "
                 "no sense usually means the wrong one is selected.",
            note_kind="warn")
    else:
        deck.bullets(
            "Step 2  -  install the board package",
            [("The Arduino IDE does not know about the ESP32 until you add it.", 0),
             ("", 0),
             ("1.  Tools > Board > Boards Manager.", 0),
             ("2.  Search for \"esp32\".", 0),
             ("3.  Find \"esp32 by Espressif Systems\".", 0),
             ("4.  In the version box choose 3.0.7 - NOT the latest.", 0),
             ("5.  Click Install and wait. It is a large download.", 0),
             ("6.  Tools > Board > ESP32 Arduino > \"ESP32 Dev Module\".", 0),
             ("", 0),
             ("Package:  {}".format(T["board_pkg"]), 0)],
            note="Version 3.0.7 specifically. Newer versions changed the way PWM "
                 "is set up, and every motor program on this track uses the 3.0.7 "
                 "spelling.",
            note_kind="warn")


# ===================================================================
# LESSON 2
# ===================================================================

def lesson2(deck, T):
    deck.title_slide(
        "H-bridges, pulse width modulation, tank steering, and driving a "
        "square you worked out on paper.",
        BYLINE + ["", "Three hours.  Wheels off the ground until told otherwise."],
        hero_image=img("vehicle-top-plate-esp32.jpg"), logo=LOGO)

    deck.objectives([
        "Explain how an H-bridge reverses a motor, in terms of current",
        "Say what a duty cycle is, and work out the duty value for a "
         "given percentage of full power",
        "Use Ohm's law to find voltage, current or resistance given the "
         "other two",
        "Tell a series circuit from a parallel one, and say what is "
         "shared in each",
        "Measure your own vehicle's speed in feet per second",
        "Predict where a pre-programmed maneuver will finish, then tune "
         "it",
        "Explain why dead reckoning drifts",
    ])

    deck.bullets(
        "Where we got to last lesson",
        [("You installed the Arduino IDE, the board package and the libraries.", 0),
         ("You know the three parts of a program: the top of the file, setup(), "
          "and loop().", 0),
         ("You can upload a program and read what it prints.", 0),
         ("You lit one of the 32 LEDs and found out where number 0 is.", 0),
         ("", 0),
         ("Today the vehicle moves.", 0)],
        note="Wheels off the ground for everything until the square maneuver at the end. Then we clear the floor and do it properly.",
        note_kind="safety")

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. Two kinds of motor, and how an H-bridge works"],
         ["0:25", "Pulse width modulation and duty cycle"],
         ["0:50", "Upload l2a_one_motor. Find your vehicle's minimum speed"],
         ["1:15", "BREAK  (10 minutes)"],
         ["1:25", "Upload l2b_speed_ramp. Duty against actual speed"],
         ["1:45", "STEM break: Ohm's law, and a circuit on a breadboard"],
         ["2:10", "BREAK  (10 minutes)"],
         ["2:20", "Tank drive, mixing, and the maths that predicts the square"],
         ["2:40", "Clear the floor. Upload l2c_maneuver_square and tune it"],
         ["2:57", "Sources of error, and what dead reckoning cannot do"]],
        col_widths=[1, 9])

    deck.two_columns(
        "Two kinds of motor, and why we use the one we do",
        "Brushed DC, with an H-bridge  (the Pathfinder)",
        [("Simple, cheap, rugged, and forgiving of students.", 0),
         ("Direction comes from which way current flows through it.", 0),
         ("Speed comes from how much power you give it.", 0),
         ("Needs an H-bridge driver to reverse the current. Ours are TI "
          "DRV8871 chips, one per motor.", 0),
         ("Two signal wires per motor from the ESP32.", 0)],
        "Brushless, with an ESC",
        [("More power for the weight, and far longer life - no brushes to "
          "wear out.", 0),
         ("Needs an Electronic Speed Controller to energise three windings in "
          "the right order.", 0),
         ("The ESC takes a throttle signal, not a direction and a speed.", 0),
         ("Used on drones, and on our larger vehicles.", 0),
         ("Worth knowing about. Not what is under your hands today.", 0)],
        note="A motor goes one way when current flows one way through it, and the "
             "other way when it flows the other way. Everything else is detail.")

    diagrams.h_bridge(deck)

    deck.two_columns(
        "Why driving A rather than B reverses the motor",
        "What is physically happening",
        [("A motor is a coil of wire sitting in a magnetic field. Push current "
          "through the coil and the field around the coil pushes against the "
          "magnets, and the shaft turns.", 0),
         ("", 0),
         ("Reverse the direction of the current and the coil's field reverses "
          "with it. Same magnets, same coil, opposite push - so the shaft "
          "turns the other way.", 0),
         ("", 0),
         ("That is the whole trick. DIRECTION OF ROTATION IS DIRECTION OF "
          "CURRENT. Everything the H-bridge does is in service of that one "
          "fact.", 0)],
        "What the two pins do about it",
        [("The motor has two terminals. Current flows from the one at the "
          "higher voltage to the one at the lower.", 0),
         ("", 0),
         ("A at 3.3 V, B at 0 V  ->  current flows A to B", 1),
         ("A at 0 V, B at 3.3 V  ->  current flows B to A", 1),
         ("", 0),
         ("So the pins do not mean \"forwards\" and \"backwards\". They mean "
          "\"this end is positive\". Which way the wheel then turns depends on "
          "which way round the motor was wired - which is why swapping the two "
          "pin numbers in the program fixes a wheel that runs backwards.", 0),
         ("", 0),
         ("How MUCH current flows sets how hard it pushes, and that is what "
          "the duty cycle controls.", 0)],
        note="Voltage is the push, current is the flow, and the motor turns "
             "because of the flow. Hold that and the next slide - Ohm's law - "
             "is the same idea with a resistor instead of a motor.",
        speaker=[
            "If a student asks about back-EMF, that is a good question and it "
            "belongs in the advanced course. Park it.",
            "The practical takeaway is the last sentence on the right: a wheel "
            "running backwards is a two-number fix in the sketch, not a "
            "rewiring job.",
        ])

    deck.table(
        "Your four motors and their eight pins",
        ["Motor", "Pin A", "Pin B", "Driving A gives you"],
        [[label, a, b, "forward"] for label, a, b in T["motor_pins"]],
        lead="Two pins per motor. Which one you drive is the direction; how hard "
             "you drive it is the speed.",
        col_widths=[3, 1.5, 1.5, 4],
        note="If one wheel on your vehicle runs backwards, swap that motor's two "
             "pin numbers in the program. Do not rewire anything.")

    deck.bullets(
        "The problem: a pin is only ever on or off",
        [("digitalWrite(pin, HIGH) puts 3.3 volts on the pin. LOW puts 0 volts "
          "on it. There is nothing in between.", 0),
         ("So how do you get HALF speed?", 0),
         ("", 0),
         ("You switch the pin on and off very fast, and vary how much of each "
          "cycle it spends switched on. The motor cannot respond quickly enough "
          "to follow the switching, so it feels the AVERAGE.", 0),
         ("That is PULSE WIDTH MODULATION. The fraction of each cycle that is "
          "on is the DUTY CYCLE.", 0),
         ("", 0),
         ("The ESP32 has dedicated hardware for this, so your program does not "
          "have to sit there flipping pins. You set it up once and then just "
          "write the number you want.", 0)],
        lead="Pulse width modulation")

    diagrams.pwm_duty(deck)

    deck.two_columns(
        "Setting PWM up on this track",
        "The two lines you need",
        [(T["attach"].split("\n")[0], 0)] +
        ([(T["attach"].split("\n")[1], 0)] if "\n" in T["attach"] else []) +
        [("", 0),
         (T["write"], 0),
         ("", 0),
         (T["pwm_story"], 0)],
        "The numbers we use, and why",
        [("Frequency {} Hz. Human hearing stops around 20 kHz, so the "
          "switching is silent. Set it to 300 and you will hear the motors "
          "sing - try it.".format(T["pwm_freq"]), 0),
         ("Resolution {0} bits. That gives duty values 0 to {1}, because 2 to "
          "the power {0} is {2}.".format(T["pwm_bits"], T["motor_max"],
                                         T["motor_max"] + 1), 0),
         ("0 is off, {} is about half power, {} is flat out.".format(
             (T["motor_max"] + 1) // 2, T["motor_max"]), 0),
         ("", 0),
         ("The advanced program uses 10 bits, 0 to 1023, for finer control at "
          "low speed.", 0)],
        size=15)

    deck.code(
        "l2a_one_motor  -  the whole idea in six lines",
        ["ledcWrite(MOTOR_PIN_A, SPEED);   // drive A",
         "ledcWrite(MOTOR_PIN_B, 0);       // leave B off",
         "//   -> the motor turns one way",
         "",
         "ledcWrite(MOTOR_PIN_A, 0);",
         "ledcWrite(MOTOR_PIN_B, SPEED);   // now the other way round",
         "//   -> the motor turns the other way",
         "",
         "ledcWrite(MOTOR_PIN_A, 0);",
         "ledcWrite(MOTOR_PIN_B, 0);",
         "//   -> both off: the motor coasts to a stop"],
        filename="l2a_one_motor.ino",
        notes=[("Only the FRONT LEFT motor is driven, so the vehicle cannot run "
                "away while you are still finding out what the numbers do.", 0),
               ("Direction is which pin you drive.", 0),
               ("Speed is how hard you drive it.", 0),
               ("Both off is a coast. Both ON would be a hard brake - the "
                "advanced course uses that.", 0)],
        size=14)

    deck.progress(STAGES["lesson2"], 1)

    deck.activity(
        "Do it now  -  one motor",
        "l2a_one_motor",
        [("1.  Put the vehicle on a block. Check all four wheels spin free.", 0),
         ("2.  Upload it. Watch the front left wheel and the Serial Monitor.", 0),
         ("3.  Change SPEED from 200 to 60. Upload. Does the wheel still turn?", 0),
         ("4.  Try 40, then 30, then 20. Find the SMALLEST number that still "
          "gets the wheel moving from a standstill. Write it down.", 0),
         ("5.  Swap MOTOR_PIN_A and MOTOR_PIN_B. Upload. What changed?", 0),
         ("6.  Change the pins to 22 and 23. Which wheel moves now?", 0)],
        expect=[("Front left wheel: forward, stop, reverse, stop, repeating.", 0)],
        questions=[("What was your minimum turning speed?", 0),
                   ("Compare with another group. Are they the same? Why not?", 0)],
        safety="Wheels off the ground. Nothing on this slide should touch the floor.",
        minutes=25)

    deck.bullets(
        "That minimum number matters",
        [("Below a certain duty, a motor does not turn at all. It has to "
          "overcome friction in the gearbox and the bearings before anything "
          "moves.", 0),
         ("Your number will not be the same as the next group's. It depends on "
          "your motors, your wheels, your bearings and how charged your battery "
          "is.", 0),
         ("", 0),
         ("In the full vehicle program there is a setting called MOTOR_MIN for "
          "exactly this. It is currently 0, which means the speed climbs "
          "smoothly from nothing.", 0),
         ("If the first part of your stick travel ever feels dead, raising "
          "MOTOR_MIN is the fix - it lifts the slowest speed the vehicle is ever "
          "given, so the wheels start moving the moment you leave the deadzone.", 0),
         ("", 0),
         ("You will meet the deadzone next lesson.", 0)],
        note="Measure it, do not guess it. The number in the program was once a "
             "guess, and it was wrong.")

    deck.progress(STAGES["lesson2"], 2)

    deck.activity(
        "Do it now  -  duty against speed",
        "l2b_speed_ramp",
        [("1.  Wheels still off the ground. Upload it.", 0),
         ("2.  Watch the wheels and read the Serial Monitor at the same time.", 0),
         ("3.  Note the duty value at which the wheels START to turn.", 0),
         ("4.  Note the value at which the sound stops changing.", 0),
         ("5.  Change PWM_FREQ from 20000 to 300 and upload. LISTEN.", 0),
         ("6.  Put it back to 20000.", 0)],
        expect=[("All four wheels winding up from nothing to full speed and back, "
                 "with the duty and the percentage printing as it goes.", 0)],
        questions=[("Is the speed proportional to the duty all the way down? "
                    "Where does it stop being a straight line?", 0),
                   ("What is the 300 Hz whine, and where does it go at 20 kHz?", 0)],
        safety="Wheels off the ground. This one reaches full speed.",
        minutes=20)

    deck.section("STEM break: Ohm's law and circuits", minutes=25)

    deck.bullets(
        "Three quantities, one relationship",
        [("You met these in Lesson 1 when you sized the resistor for your LED. "
          "Here they are properly.", 0),
         ("", 0),
         ("VOLTAGE (V), in volts. The push. How hard the supply is trying to "
          "move charge round the circuit.", 0),
         ("CURRENT (I), in amps. The flow. How much charge is actually "
          "moving past a point each second.", 0),
         ("RESISTANCE (R), in ohms. The opposition. How much the circuit "
          "fights the flow.", 0),
         ("", 0),
         ("Ohm's law ties all three together. Know any two and you can work "
          "out the third - which is exactly what you did to pick 220 ohms.", 0)],
        lead="Voltage, current, resistance",
        speaker=[
            "Ask them to draw the triangle in their engineering notebook "
            "before you show the next slide.",
            "Kevin's Ohm's Law deck has more practice problems if this group "
            "wants them.",
        ])

    diagrams.ohms_and_power_law(
        deck, title="Ohm's law, and the power law that goes with it")

    deck.bullets(
        "Practise it",
        [("Work these in your engineering notebook. Cover the unknown on the "
          "triangle with your thumb and the triangle tells you the sum.", 0),
         ("", 0),
         ("1.  V = 12 volts, I = 15 mA.  R = ?", 0),
         ("     (careful: convert milliamps to amps first)", 1),
         ("", 0),
         ("2.  V = 12 volts, R = 220 ohms.  I = ?", 0),
         ("", 0),
         ("3.  I = 2 mA, R = 1.5k ohms.  V = ?", 0),
         ("     (1.5k means 1500 ohms)", 1),
         ("", 0),
         ("4.  Your vehicle's battery is 16 V. A motor stalls and draws "
          "3 amps. What resistance is it presenting?", 0)],
        lead="Four minutes, in your notebook",
        note="Answers: 800 ohms, 0.055 A (55 mA), 3 volts, and about "
             "5.3 ohms. The last one is why a stalled motor gets hot.",
        speaker=[
            "Give them four minutes, then take answers from the room rather "
            "than reading them out.",
            "Question 4 is the one worth dwelling on - it connects the maths "
            "back to the vehicle they are about to drive.",
        ])

    diagrams.series_parallel(deck)

    deck.bullets(
        "Series and parallel on the vehicle",
        [("SERIES. Your LED circuit from Lesson 1: supply, resistor, LED, "
          "back to ground. One loop, one current.", 0),
         ("Add resistance and the current everywhere drops.", 1),
         ("", 0),
         ("PARALLEL. The four motors. Each one hangs across the same supply, "
          "so each gets the full battery voltage, and the currents add up.", 0),
         ("Four motors at 1.5 A each is 6 A out of the battery.", 1),
         ("That is why the battery goes flat four times faster with all four "
          "driving than with one.", 1),
         ("", 0),
         ("The 32 LEDs are in parallel too. Each pixel draws its own current "
          "from the same 5 V rail, which is exactly why the power budget in "
          "Lesson 4 adds up the way it does.", 0)],
        lead="You have already built both",
        note="Series: current is shared, voltage divides. Parallel: voltage is "
             "shared, current divides. Almost every wiring question comes down "
             "to knowing which one you are looking at.",
        speaker=[
            "This is the payoff slide. Tie it to hardware they have handled.",
            "If time is short, this is the slide to keep and the practice "
            "problems are the ones to drop.",
        ])

    deck.section("Making it go where you want", minutes=60)

    deck.bullets(
        "Tank drive",
        [("The Pathfinder has no steering rack. Nothing on it points the wheels.", 0),
         ("The two wheels on the LEFT are driven together. The two on the RIGHT "
          "are driven together.", 0),
         ("Both sides the same  ->  a straight line.", 0),
         ("Left faster than right  ->  it curves to the right.", 0),
         ("Left forward and right backward  ->  it spins on the spot.", 0),
         ("", 0),
         ("This is how a tank steers, how an excavator steers, and how most "
          "small robots steer. It is called differential or skid steering.", 0),
         ("", 0),
         ("It costs you something: the wheels have to scrub sideways across the "
          "floor to turn, so turning is much more affected by the surface than "
          "driving straight is. That will show up in your square today.", 0)],
        lead="Two sides, driven separately")

    diagrams.tank_mixing(deck)

    deck.bullets(
        "The maths you need to predict where it ends up",
        [("Wheel diameter is 3.0 inches.", 0),
         ("Circumference = pi x diameter = 3.1416 x 3.0 = 9.42 inches", 1),
         ("Distance per wheel turn = 9.42 / 12 = 0.79 feet", 1),
         ("", 0),
         ("Speed = revolutions per second x distance per revolution", 0),
         ("Distance = speed x time", 0),
         ("Degrees turned = turn rate x time", 0),
         ("", 0),
         ("You know the time - you wrote it in the delay(). You have to MEASURE "
          "the speed and the turn rate, because they depend on your motors, "
          "your floor and your battery.", 0)],
        note="Worked example: at 3.34 feet per second, 100 feet takes 29.94 "
             "seconds. At a turn rate of 30 degrees per second, a 90 degree "
             "corner takes 3 seconds.")

    diagrams.square_path(deck)

    deck.table(
        "The three kinds of loop in C",
        ["Loop", "When the test happens", "Use it when"],
        [["for", "Before each pass", "You know how many times. Stepping through the 32 LEDs, or four sides of a square."],
         ["while", "Before each pass", "You do not know how many times. It depends on a condition, and it might not run at all."],
         ["do-while", "After each pass", "The body must run at least once, whatever the condition says."]],
        lead="You have been using for loops all lesson. There are two more, and "
             "between them they turn up in almost every program you will ever "
             "read.",
        col_widths=[1.4, 2.6, 7],
        size=14,
        note="Study these three properly. If you genuinely understand when to "
             "reach for each one, you are already a better programmer than the "
             "syntax alone would make you.")

    deck.code(
        "The same job, written three ways",
        ["// for  -  a known number of passes",
         "for (int i = 0; i < 32; i++) {",
         "  strip.setPixelColor(i, colour);",
         "}",
         "",
         "// while  -  keep going until something changes",
         "while (!Ps3.isConnected()) {",
         "  showWaitingLights();        // may never run at all",
         "}",
         "",
         "// do-while  -  always at least one pass",
         "do {",
         "  reading = analogRead(PIN);  // take a reading FIRST,",
         "} while (reading > LIMIT);    // then decide whether to repeat"],
        filename="the three loop forms",
        notes=[("The for loop is three things in one line: where to start, "
                "when to keep going, and what to change each pass.", 0),
               ("A while loop can run zero times. That is usually what you "
                "want when you are waiting for something.", 0),
               ("A do-while always runs once before it checks. Useful when "
                "the test needs a value you have not got yet.", 0),
               ("", 0),
               ("loop() itself is really a while(true) that the Arduino "
                "framework writes for you.", 0)],
        size=13, highlight={1, 6, 11})

    deck.code(
        "l2c_maneuver_square  -  the loop",
        ["void loop() {",
         "  // Four sides and four corners makes a square.",
         "  for (int corner = 0; corner < 4; corner++) {",
         "    goForward();",
         "    delay(FORWARD_MS);",
         "",
         "    spinRight();",
         "    delay(TURN_MS);",
         "  }",
         "",
         "  stopMoving();",
         "  delay(20000);   // pick your vehicle up",
         "}"],
        filename="l2c_maneuver_square.ino",
        notes=[("A for loop repeats a block a fixed number of times. Four sides, "
                "four corners.", 0),
               ("The whole shape is four numbers: DRIVE_SPEED, TURN_SPEED, "
                "FORWARD_MS and TURN_MS.", 0),
               ("TURN_MS is the one you will spend most of your time on. It sets "
                "how far round each corner goes.", 0),
               ("Note the twenty second stop at the end. That is so the program "
                "does not immediately start a second square while you are "
                "walking towards it.", 0)],
        size=14, highlight={4, 7})

    deck.progress(STAGES["lesson2"], 3)

    deck.activity(
        "Do it now  -  drive a square",
        "l2c_maneuver_square",
        [("1.  Clear a space about three metres square. Bags and feet out.", 0),
         ("2.  Mark the starting point with a piece of tape.", 0),
         ("3.  Upload. It waits five seconds, then goes.", 0),
         ("4.  MEASURE one side of the square it actually drove.", 0),
         ("5.  Work out the speed:  side length / (FORWARD_MS / 1000)", 0),
         ("6.  The corners will not be 90 degrees. Adjust TURN_MS by 25 ms at a "
          "time until the vehicle comes back to its tape.", 0),
         ("7.  Now change FORWARD_MS and PREDICT the new square before running "
          "it. How close were you?", 0)],
        expect=[("Roughly a square. Roughly.", 0),
                ("It will not close perfectly on the first attempt. That is the "
                 "point of the exercise, not a failure.", 0)],
        questions=[("What is your vehicle's speed in feet per second?", 0),
                   ("What is its turn rate in degrees per second?", 0)],
        safety="This one drives on the floor. Know where your power switch is "
               "before you upload.",
        minutes=30)

    deck.bullets(
        "Dead reckoning, and what it cannot do",
        [("DEAD RECKONING is working out where you are from your speed, your "
          "heading and the time - with no outside reference at all.", 0),
         ("Ships and aircraft navigated this way for centuries. Submersibles "
          "still fall back on it, because GPS does not work underwater.", 0),
         ("", 0),
         ("It DRIFTS. Every small error accumulates, and nothing ever corrects "
          "it. Errors in your square come from:", 0),
         ("Battery charge - a fresher pack drives further in the same time", 1),
         ("Floor surface - carpet, tile and concrete all differ", 1),
         ("Wheel slip during the turns, which skid steering guarantees", 1),
         ("Motors that are not perfectly matched to each other", 1),
         ("", 0),
         ("The fix is a SENSOR that tells you something true about the outside "
          "world. That is where the course goes after this one.", 0)],
        note="Recharge the battery and run the same numbers again. The square "
             "gets bigger. Nothing in the program changed.",
        note_kind="warn")

    deck.bullets_image(
        "The same ideas, underwater",
        [("The Explorer Gen 2 ROV is built from the pieces you just met:", 0),
         ("", 0),
         ("An ESP32, like the one on your desk", 0),
         ("Brushless motors on ESCs, not brushed on H-bridges - more "
          "thrust for the weight", 0),
         ("A pressure sensor and an IMU on an I2C bus", 0),
         ("A CAN bus tying it together", 0),
         ("", 0),
         ("Different vehicle, same engineering.", 0)],
        img("rov-can-bus.jpg"),
        caption="Explorer Gen 2 ROV: ESP32, ESC, sensors, CAN bus",
        image_ratio=0.50,
        note="If this looks interesting, there is a whole ROV course. The "
             "ground vehicle is the easiest place to learn the ideas; the "
             "submersible is where they get hard.",
        speaker=[
            "Worth thirty seconds of enthusiasm. Several students each year "
            "come for the rover and stay for the ROV.",
            "The honest pitch: a mistake on a rover bumps a wall. A mistake on "
            "a submersible floods an electronics bay.",
        ])

    deck.bullets(
        "Drag race  -  next lesson and the one after",
        [("Mark a start line and a finish line with tape. Measure the distance.", 0),
         ("Line up two or three vehicles. Hold the reset button until the start "
          "signal, then let go.", 0),
         ("Time from start to crossing the line. Speed = distance / time.", 0),
         ("First across the line INSIDE the lane wins. Leaving the lane does "
          "not.", 0),
         ("", 0),
         ("Calibrate: you now know what duty value 255 gives you in feet per "
          "second. Try 200 and 150 as well. Is it a straight line?", 0),
         ("If your vehicle pulls to one side, you can add a short turn "
          "correction into the program to straighten it.", 0),
         ("You can only brake by driving in reverse - try half speed for a "
          "short time and see how far it slides.", 0)],
        lead="Calibrate first, then race")

    deck.two_columns(
        "If you want to go further before the next lesson",
        "Watch",
        [("How PWM works, controlling a motor   (10:10)", 0),
         ("youtube.com/watch?v=5nwNKPs2gco", 1),
         ("", 0),
         ("Arduino DC motor control   (11:44)", 0),
         ("youtube.com/watch?v=HtbCL2NruUY", 1),
         ("", 0),
         ("Motor speed tutorial with PWM motors   (17:33)", 0),
         ("youtube.com/watch?v=UPTU6nYSaMo", 1),
         ("", 0),
         ("Loops in C   (video)", 0),
         ("youtube.com/watch?v=b4DPj0XAfSg", 1)],
        "Read",
        [("Loops in C, worked through properly", 0),
         ("geeksforgeeks.org/c/c-loops/", 1),
         ("", 0),
         ("Control structures - loops and conditionals", 0),
         ("cplusplus.com/doc/tutorial/control/", 1),
         ("", 0),
         ("Functions", 0),
         ("cplusplus.com/doc/tutorial/functions/", 1),
         ("", 0),
         ("Everything above is optional and none of it is examined.", 0)],
        size=15,
        note="Watch the PWM video before Lesson 3 if today felt fast. It covers "
             "the same ground more slowly, and with an oscilloscope.")

    deck.quiz(
        "Check yourself",
        [("1.  Why does each motor need two pins instead of one?", 0),
         ("2.  A pin can only be on or off. Explain in one sentence how the "
          "vehicle gets a half speed.", 0),
         ("3.  What duty value is 25% of full power at 8-bit resolution?", 0),
         ("4.  Why do we switch at 20 kHz rather than 300 Hz?", 0),
         ("5.  The left wheels get 200 and the right wheels get -200. What does "
          "the vehicle do?", 0),
         ("6.  Your wheel is 3 inches across. How far does the vehicle travel "
          "in one wheel turn?", 0),
         ("7.  Your square comes out as a rectangle. Which number is wrong, and "
          "which way should you change it?", 0),
         ("8.  Give two reasons the same program gives a different square today "
          "than it did yesterday.", 0)],
        lead="Next lesson: you get to drive it yourself.")

    return deck


# ===================================================================
# LESSON 3
# ===================================================================

def lesson3(deck, T):
    deck.title_slide(
        "Bluetooth, thumbsticks, deadzones, and turning a number from your "
        "thumb into a speed for four wheels.",
        BYLINE + ["", "Three hours.  Bring the {} paired to your vehicle.".format(
            T["pad_short"])],
        hero_image=T["hero"], logo=LOGO)

    deck.objectives([
        "Explain how a controller reaches one vehicle and not another",
        "Say what a deadzone is for, and what goes wrong without one",
        "Use map() to turn a stick reading into a motor speed",
        "Mix a forward value and a turn value into two wheel speeds",
        "Drive the vehicle under your own control",
        "Write the failsafe that stops the motors when the controller "
         "drops",
    ])

    deck.bullets(
        "Where we got to last lesson",
        [("You know how an H-bridge reverses a motor, and how PWM sets its speed.", 0),
         ("You measured your own vehicle's minimum turning speed.", 0),
         ("You drove a square you calculated, and found out why it did not "
          "close.", 0),
         ("", 0),
         ("Today you take the wheel.", 0)],
        note="Everything today until the last hour is print-only. Nothing moves "
             "until you have seen the numbers first.")

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. What Bluetooth is, and how a pad finds a vehicle"],
         ["0:25", "One vehicle, one controller"],
         ["0:50", "Upload l3a_controller_check. Find every button and axis"],
         ["1:15", "BREAK  (10 minutes)"],
         ["1:25", "The deadzone problem, and map()"],
         ["1:50", "Upload l3b_deadzone_and_map. Watch the arithmetic"],
         ["2:10", "BREAK  (10 minutes)"],
         ["2:20", "Mixing forward and turn"],
         ["2:35", "Upload l3c_tank_drive. Drive it"],
         ["2:55", "Failsafe, and what happens when the link drops"]],
        col_widths=[1, 9])

    deck.bullets(
        "What Bluetooth is doing",
        [("A short-range radio link in the 2.4 GHz band - the same band as "
          "Wi-Fi, microwave ovens and almost everything else.", 0),
         ("It hops between 79 narrow channels many times a second, so a busy "
          "band does not stop it. That is why twenty vehicles in one room "
          "actually works.", 0),
         ("Range is tens of metres, and it goes down sharply if somebody stands "
          "between the pad and the vehicle.", 0),
         ("", 0),
         ("Every Bluetooth device has an ADDRESS - six bytes, written as six "
          "pairs of hexadecimal digits, like 02:02:03:04:05:08. It is the "
          "equivalent of a phone number.", 0),
         ("", 0),
         ("The ESP32 has Bluetooth built into the chip. That is a large part of "
          "why we use it.", 0)],
        lead="A radio link, and a phone number for each device",
        note="A Bluetooth sketch fills about 85% of the ESP32's program storage. "
             "That is the radio stack, not your code - but it does mean there is "
             "not much room left, so keep additions lean.")

    _pairing_slide(deck, T)

    _pad_map_slide(deck, T)

    deck.two_columns(
        "Two kinds of control, and the axes they move",
        "Digital: buttons",
        [("A button is either pressed or it is not. true or false.", 0),
         ("Face buttons, D-pad, shoulders, triggers, and clicking the sticks "
          "in.", 0),
         ("Good for: on/off, mode changes, one-shot actions.", 0),
         ("", 0),
         ("On this track:", 0),
         (T["read_button"], 1)],
        "Analog: thumbsticks",
        [("A stick reports HOW FAR it is pushed, on two axes.", 0),
         ("On this track each axis reads {}.".format(T["stick_range"]), 0),
         ("Good for: speed, steering, aiming a servo.", 0),
         ("", 0),
         ("On this track:", 0),
         (T["read_x"] + "     // left/right", 1),
         (T["read_y"] + "     // up/down", 1)],
        note="Push a stick UP and the number goes NEGATIVE. That catches "
             "everybody. It is why the driving code has a minus sign in front of "
             "the Y reading.",
        note_kind="warn")

    deck.bullets(
        "Six degrees of freedom",
        [("Any rigid object in space can move in six independent ways. Three "
          "are translations - sliding along an axis - and three are rotations.", 0),
         ("", 0),
         ("TRANSLATION", 0),
         ("Forward and back  -  surge", 1),
         ("Side to side  -  sway", 1),
         ("Up and down  -  heave", 1),
         ("", 0),
         ("ROTATION", 0),
         ("Roll  -  tipping left or right about the fore-aft axis", 1),
         ("Pitch  -  nose up or nose down", 1),
         ("Yaw  -  turning left or right, which is your steering", 1),
         ("", 0),
         ("A ground vehicle on a flat floor really only controls surge and yaw. "
          "A submersible or an aircraft controls all six, which is why they need "
          "far more from the operator.", 0)],
        note="The PS3 controller was originally called the Sixaxis because it "
             "could sense all six. Spelled backwards, Sixaxis is still Sixaxis.")

    deck.progress(STAGES["lesson3"], 1)

    deck.activity(
        "Do it now  -  what does the controller actually send?",
        "l3a_controller_check",
        [("1.  Upload it and open the Serial Monitor.", 0),
         (("2.  Press the PS button on the controller." if T["key"].endswith("ps3")
           else "2.  Hold the SYNC button until the controller lights run."), 0),
         ("3.  Press every button in turn and match it to the printed name.", 0),
         ("4.  Push the left stick fully UP. Positive or negative?", 0),
         ("5.  Let go of the sticks and DO NOT touch the controller. Does it "
          "print exactly 0, or does it drift?", 0),
         ("6.  Push a stick diagonally. Watch x and y move together.", 0)] +
        ([("7.  Copy the MY_CONTROLLER line it prints. You need it all day.", 0)]
         if not T["key"].endswith("ps3") else
         [("7.  Squeeze L2 slowly. It is not just on or off - watch the number.", 0)]),
        expect=[("A line for every press, every release, and every stick "
                 "movement.", 0)],
        questions=[("Which way is negative on the Y axis?", 0),
                   ("How far does your stick drift when nobody is touching it?", 0)],
        minutes=25)

    deck.bullets(
        "The problem with believing the stick",
        [("A thumbstick that has been thumbed by a hundred students does not "
          "come back to exactly zero.", 0),
         ("You just measured it. Yours probably reads a few counts either side "
          "of centre when nobody is touching it.", 0),
         ("", 0),
         ("If we fed that straight to the motors, the vehicle would creep across "
          "the room on its own while the controller sat on the desk. In a "
          "classroom with twenty vehicles, that is not a minor annoyance.", 0),
         ("", 0),
         ("The answer is a DEADZONE: any reading smaller than a threshold is "
          "treated as a firm, deliberate zero.", 0),
         ("On this track the deadzone is {}, which is {} of the stick "
          "travel.".format(T["deadzone"], T["deadzone_pct"]), 0),
         ("", 0),
         ("Too small and the vehicle creeps. Too big and you waste the stick.", 0)],
        lead="Deadzone")

    diagrams.deadzone_map(deck, stick_max=T["stick_max"], deadzone=T["deadzone"])

    deck.code(
        "map()  -  the most useful function in the course",
        ["map(value, fromLow, fromHigh, toLow, toHigh)",
         "",
         "// \"value sits somewhere between fromLow and fromHigh.",
         "//  Where does it sit between toLow and toHigh?\"",
         "",
         "map(0,   0, {0}, 0, 255)   ->    0".format(T["stick_max"]),
         "map({0}, 0, {0}, 0, 255)   ->  127".format(T["stick_max"] // 2),
         "map({0}, 0, {0}, 0, 255)   ->  255".format(T["stick_max"]),
         "",
         "// In stickToSpeed the low end is the DEADZONE, not zero,",
         "// so the leftover travel is stretched across the whole",
         "// speed range. Without that you could never reach 255.",
         "",
         "size = map(abs(v), {}, {}, MOTOR_MIN, maxSpeed);".format(
             T["deadzone"], T["stick_max"])],
        filename="the idea, and how the program uses it",
        notes=[("map() is just proportion. Nothing more.", 0),
               ("It does NOT clamp. Feed it something outside the input range "
                "and you get something outside the output range, which is why "
                "the program calls constrain() straight afterwards.", 0),
               ("abs() throws away the sign, so we map the SIZE of the push and "
                "put the direction back at the end.", 0)],
        size=13, highlight={13})

    deck.progress(STAGES["lesson3"], 2)

    deck.activity(
        "Do it now  -  watch the arithmetic",
        "l3b_deadzone_and_map",
        [("1.  Upload it. Nothing moves - this is print only.", 0),
         ("2.  Push the stick VERY slowly from centre to full forward. Watch "
          "both columns. Where does the speed stop being 0?", 0),
         ("3.  Set STICK_DEADZONE to 0. Upload. Put the controller down and do "
          "not touch it. Does the speed stay at 0?", 0),
         ("4.  Set it to {}. How much of the stick is now wasted?".format(
             T["stick_max"] * 4 // 5), 0),
         ("5.  Put it back. Now set MOTOR_MIN to 60 and watch what happens the "
          "instant you leave the deadzone.", 0)],
        expect=[("Two columns: the raw number from the controller, and the motor "
                 "speed it turns into.", 0)],
        questions=[("With the deadzone at 0, what does the vehicle do when "
                    "nobody is holding the controller?", 0),
                   ("What does MOTOR_MIN = 60 change, and when would you want "
                    "it?", 0)],
        minutes=20)

    deck.bullets(
        "Mixing, and why steering has its own limit",
        [("left = forward + turn        right = forward - turn", 0),
         ("", 0),
         ("Driving gets the full MOTOR_MAX of 255.", 0),
         ("Steering gets turnMax, which is HALF of that by default.", 0),
         ("", 0),
         ("Why? Because at full power a small nudge sideways spins the vehicle "
          "faster than anybody can correct for. Half-power steering makes it "
          "controllable without making it slow.", 0),
         ("", 0),
         ("The full program lets you switch between the two with {}. Sharp mode "
          "is for spinning on the spot deliberately; normal mode is for actually "
          "getting somewhere.".format(T["btn_stickclick"]), 0),
         ("", 0),
         ("constrain() catches the overflow. forward 200 plus turn 60 is 260, "
          "which is more than a motor can take.", 0)],
        lead="Full speed forwards, half speed steering")

    deck.code(
        "l3c_tank_drive  -  the whole of loop()",
        ["void loop() {",
         "  if ({}) {{".format(T["guard"]),
         "    drive(0, 0);        // never drive without a controller",
         "    return;",
         "  }",
         "",
         "  int leftStickX = {};".format(T["read_x"]),
         "  int leftStickY = {};".format(T["read_y"]),
         "",
         "  int forward = stickToSpeed(-leftStickY, MOTOR_MAX);",
         "  int turn    = stickToSpeed(leftStickX,  turnMax);",
         "",
         "  int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);",
         "  int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);",
         "",
         "  drive(leftSpeed, rightSpeed);",
         "}"],
        filename="l3c_tank_drive.ino",
        notes=[("Read the first four lines again. If there is no controller, "
                "the motors stop and we go round again. That is the FAILSAFE, "
                "and it is the most important part of the program.", 0),
               ("Note the minus sign on leftStickY. Up is negative.", 0),
               ("Driving gets MOTOR_MAX; steering gets turnMax.", 0),
               ("Everything else you have already seen.", 0)],
        size=13, highlight={1, 2, 3, 9, 10})

    deck.progress(STAGES["lesson3"], 3)

    deck.activity(
        "Do it now  -  drive it",
        "l3c_tank_drive",
        [("1.  Wheels off the ground. Upload. Check that up is forward and "
          "right is right BEFORE you put it down.", 0),
         ("2.  Now put it on the floor in a clear space and drive.", 0),
         ("3.  While driving, switch the controller off. What happens?", 0),
         ("4.  Set turnMax to MOTOR_MAX and drive again. Which is easier?", 0),
         ("5.  Swap the + and the - in the mixing lines and drive. What is "
          "wrong now?", 0),
         ("6.  Try  right = forward  with no turn term at all. Why is that a "
          "worse way to steer?", 0)],
        expect=[("A vehicle that goes where you point it.", 0)],
        questions=[("What happened when the controller switched off, and which "
                    "four lines made that happen?", 0)],
        safety="Check the controls with the wheels off the ground first. Every "
               "time you upload a change.",
        minutes=30)

    deck.bullets(
        "Failsafe: what a robot does when it stops hearing you",
        [("A radio link is not reliable. Batteries go flat, people walk between "
          "the pad and the vehicle, controllers get switched off.", 0),
         ("", 0),
         ("The only safe behaviour is to STOP. Not carry on with the last "
          "command, and not do something clever.", 0),
         ("", 0),
         ("Three lines at the top of loop() do it:", 0),
         ("if (!connected) { drive(0, 0); return; }", 1),
         ("", 0),
         ("This is not a Pathfinder detail. Every unmanned vehicle has a "
          "failsafe, and what it should do when the link drops is a real "
          "engineering decision. A drone hovers, or returns home. A submersible "
          "surfaces. A ground vehicle stops.", 0),
         ("", 0),
         ("Ask yourself, on any system you build: what happens when the "
          "operator disappears?", 0)],
        note="If you take one habit away from this course, take this one.",
        note_kind="safety")

    deck.table(
        "When it will not connect",
        ["Symptom", "Most likely cause", "What to do"],
        [["Nothing connects at all",
          "Address mismatch",
          "Check the address in the sketch matches the pad"],
         ["Connects, then drops",
          "Flat controller battery",
          "Charge it over USB for a few minutes"],
         ["Somebody else's vehicle moves",
          "Two vehicles on one address",
          "Give every vehicle a different address"],
         ["Wheels spin the wrong way",
          "Motor pins swapped",
          "Swap that motor's two pin numbers in the sketch"],
         ["It creeps with the stick centred",
          "Deadzone too small",
          "Raise STICK_DEADZONE"],
         ["Nothing at all, no serial output",
          "Wrong board selected",
          "Check the board menu - see Lesson 1"]],
        col_widths=[3.5, 3, 4],
        size=13)

    deck.quiz(
        "Check yourself",
        [("1.  What is a Bluetooth address, and why must every vehicle in the "
          "room have a different one?", 0),
         ("2.  Push the left stick fully forward. Is the number positive or "
          "negative? Why does the program put a minus sign in front of it?", 0),
         ("3.  What is a deadzone for? What goes wrong if it is 0? What goes "
          "wrong if it is far too big?", 0),
         ("4.  In your own words, what does map() do?", 0),
         ("5.  forward is 180 and turn is 100. What speed does each side get, "
          "before and after constrain()?", 0),
         ("6.  Why is steering limited to half power by default?", 0),
         ("7.  Write the three lines that stop the vehicle when the controller "
          "disconnects.", 0)],
        lead="Next lesson: the 32 LEDs, and what colour actually is.")

    return deck


def _pairing_slide(deck, T):
    if T["key"].endswith("ps3"):
        deck.bullets(
            "One vehicle, one controller  -  the PS3 way",
            [("A PS3 controller has no discovery mode. It only ever talks to ONE "
              "address, and that address has to be written into the controller "
              "over a USB cable.", 0),
             ("", 0),
             ("So on this track, {}.".format(T["lock_story"]), 0),
             ("", 0),
             ("The tool is {}.".format(T["lock_tool"]), 0),
             ("1.  Plug the controller into a Windows PC with a USB cable.", 1),
             ("2.  Open SixaxisPairTool. It shows the current master address.", 1),
             ("3.  Type in the address you want and click Update.", 1),
             ("4.  Put that same address into PS3_MAC_ADDRESS in the sketch.", 1),
             ("", 0),
             ("Give every vehicle in the room a different one. If two vehicles "
              "share an address, one controller drives both of them.", 0),
             ("", 0),
             ("Suggested scheme:  02:02:03:04:05:NN, where NN is the vehicle "
              "number. Write it on a sticker on the vehicle AND the controller.", 0)],
            note="The first pair of digits must be an EVEN number. An odd one "
                 "silently fails to connect, which is a very confusing way to "
                 "spend an afternoon.",
            note_kind="warn")
    else:
        deck.bullets(
            "One vehicle, one controller  -  the allowlist",
            [("A Switch pad in pairing mode will happily connect to whichever "
              "vehicle answers first. In a room with twenty vehicles that means "
              "two students end up driving the same one.", 0),
             ("", 0),
             ("So on this track, {}.".format(T["lock_story"]), 0),
             ("", 0),
             ("Bluepad32 keeps an ALLOWLIST - a guest list checked BEFORE a "
              "connection is accepted. A pad that is not on the list is turned "
              "away before it is ever let in.", 0),
             ("", 0),
             ("uni_bt_allowlist_remove_all();", 1),
             ("uni_bt_allowlist_add_addr(myControllerAddress);", 1),
             ("uni_bt_allowlist_set_enabled(true);", 1),
             ("", 0),
             ("You find your pad's address with {}, then paste the line it "
              "prints into the top of every sketch for the rest of the "
              "day.".format(T["lock_tool"]), 0)],
            note="The list is rebuilt from scratch at every boot, so the sketch "
                 "is always the single source of truth about who may drive. Write "
                 "the address on a sticker on the vehicle AND the controller.")

        deck.bullets(
            "How YOUR vehicle gets paired",
            [("In pathfinder_nintendoswitch - the program this whole course is "
              "about - the address is a CONSTANT IN THE PROGRAM, called "
              "MY_CONTROLLER.", 0),
             ("", 0),
             ("1.  Upload l3a_controller_check and connect your pad.", 0),
             ("2.  It prints a ready-made MY_CONTROLLER line.", 0),
             ("3.  Paste that line into the top of the sketch.", 0),
             ("4.  Upload. From now on the vehicle answers only to that pad.", 0),
             ("", 0),
             ("To change controllers, you edit the sketch and upload again.", 0),
             ("", 0),
             ("Nothing is hidden. The whole mechanism is visible in the file in "
              "front of you, which is exactly what you want while you are still "
              "learning what the pieces are.", 0)],
            lead="pathfinder_nintendoswitch: the address lives in the sketch",
            note="Side note: the ADVANCED program, Pathfinder_Op_Program12, does "
                 "this differently - it stores the address in EEPROM and you "
                 "pair by typing 'pair' in the Serial Monitor, or pressing the "
                 "BOOT button, then A and HOME on the pad. If somebody hands you "
                 "a vehicle and tells you to type 'pair', it is running that "
                 "program, not this one.")


def _pad_map_slide(deck, T):
    if T["key"].endswith("ps3"):
        deck.image_slide(
            "The PS3 controller",
            T["pad_image"],
            caption="Two analog sticks, a D-pad, four face buttons, four "
                    "shoulder buttons and triggers, and three in the middle",
            items=[("Every one of those has a name in the program. Today you "
                    "will press all of them and see what each is called.", 0)])
    else:
        deck.image_slide(
            "The Switch controller",
            T["pad_image"],
            caption="Two analog sticks, a D-pad, four face buttons, shoulders "
                    "and triggers, and the small ones in the middle",
            items=[("Every one of those has a name in the program - but not "
                    "always the name printed on it. The next slide is the "
                    "table you will actually need.", 0)])

        deck.table(
            "The Switch controller, as Bluepad32 sees it",
            ["Printed on the pad", "In the program", "Where it is"],
            [["B", "myController->a()", "bottom face button"],
             ["A", "myController->b()", "right face button"],
             ["Y", "myController->x()", "left face button"],
             ["X", "myController->y()", "top face button"],
             ["L / R", "l1() / r1()", "shoulders"],
             ["ZL / ZR", "l2() / r2()", "triggers"],
             ["stick click", "thumbL() / thumbR()", "press a stick in"],
             ["D-pad", "dpad() & DPAD_UP  etc.", "one number, one bit per direction"],
             ["left stick", "axisX() / axisY()", "-512 to +511"],
             ["right stick", "axisRX() / axisRY()", "-512 to +511"]],
            lead="Bluepad32 names the face buttons by POSITION, Xbox style - not "
                 "by the letter printed on your pad.",
            col_widths=[2.5, 4, 4],
            size=13,
            note="So y() is the button MARKED X, because it is the top one. "
                 "Third-party pads do not always agree. l3a_controller_check "
                 "prints what you actually pressed, which settles it.",
            note_kind="warn")


# ===================================================================
# LESSON 4
# ===================================================================

def lesson4(deck, T):
    deck.title_slide(
        "Addressable LEDs, colour, power budgets, and making 32 lights do "
        "what you tell them.",
        BYLINE + ["", "Three hours.  Nothing moves today."],
        hero_image=img("vehicle-green-leds.jpg"), logo=LOGO)

    deck.objectives([
        "Explain how 32 LEDs are controlled independently over one wire",
        "Mix any colour from red, green and blue values",
        "Work out the current a lighting pattern will draw, and say "
         "whether the battery can afford it",
        "Find any LED on the vehicle from its number, and the LED "
         "opposite it",
        "Write a for loop that lights a chosen run of LEDs",
        "Write a lighting pattern of your own as a function",
    ])

    deck.bullets(
        "Where we got to last lesson",
        [("You know how a controller reaches the vehicle, and why every vehicle "
          "needs its own address.", 0),
         ("You know what a deadzone is and why it exists.", 0),
         ("You can turn a stick reading into a motor speed, and mix forward "
          "with turn.", 0),
         ("You can drive.", 0),
         ("", 0),
         ("Today: the 32 lights. Nothing on the vehicle moves, so the wheels can "
          "stay on the desk.", 0)],
        note="Next lesson everything comes together and we race.")

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. What an addressable LED is"],
         ["0:25", "Colour: RGB, wavelength, and how your eye works"],
         ["0:50", "The power budget. Upload l4a_all_one_colour"],
         ["1:15", "BREAK  (10 minutes)"],
         ["1:25", "Mapping the loop. Upload l4b_led_map"],
         ["1:55", "for loops, and four patterns. Upload l4c_patterns"],
         ["2:20", "BREAK  (10 minutes)"],
         ["2:30", "Design a pattern of your own"],
         ["2:55", "Recap"]],
        col_widths=[1, 9])

    deck.bullets(
        "What makes a NeoPixel different",
        [("An ordinary LED has two legs. It is on or off, and if you want ten of "
          "them you need ten pins.", 0),
         ("", 0),
         ("A NeoPixel - the part is a WS2812B - has four connections: 5 volts, "
          "ground, DATA IN and DATA OUT. Inside it there is a tiny controller "
          "chip and three LEDs: one red, one green, one blue.", 0),
         ("", 0),
         ("Send a stream of colour messages into the first pixel. It keeps the "
          "FIRST message for itself and passes the rest out of its data-out pin "
          "to the next one, which does the same.", 0),
         ("", 0),
         ("So one wire drives the whole chain. Our vehicle has 32 of them on "
          "GPIO 5 - and they are numbered in the order the data flows, which is "
          "why the numbering does what it does.", 0),
         ("", 0),
         ("In theory a chain can be any length. In practice the whole chain has "
          "to be refreshed every frame, so about 350 pixels is the limit before "
          "the refresh rate drops below 40 Hz.", 0)],
        lead="One wire, 32 addressable devices")

    deck.table(
        "The whole NeoPixel API you need",
        ["Call", "What it does"],
        [["strip.begin()", "Wakes the strip up. Once, in setup(). Always required."],
         ["strip.setBrightness(n)", "Master scale, 0 to 255, applied to every pixel"],
         ["strip.Color(r, g, b)", "Packs three 0-255 numbers into one colour value"],
         ["strip.setPixelColor(i, c)", "Sets pixel i - IN MEMORY ONLY"],
         ["strip.fill(c)", "Sets every pixel to the same colour, in memory"],
         ["strip.clear()", "Sets every pixel to off, in memory"],
         ["strip.show()", "Sends the memory to the hardware. Nothing happens without it."],
         ["strip.numPixels()", "How many there are - 32 here"],
         ["strip.gamma32(c)", "Corrects a colour so fades look even to your eye"]],
        lead="Nine calls. Everything in Lesson 4 and everything in the vehicle "
             "program is built from these.",
        col_widths=[3.4, 7.6],
        size=13,
        note="Six of those nine only touch memory. Exactly one of them, show(), "
             "changes what you can see.")

    diagrams.rgb_mixing(deck)

    deck.bullets(
        "setBrightness is not the same as a smaller colour value",
        [("There are two ways to make an LED dimmer, and they are not "
          "interchangeable.", 0),
         ("", 0),
         ("strip.Color(60, 0, 0) sets the RED channel to a quarter. The green "
          "and blue channels are untouched. This changes the COLOUR you asked "
          "for.", 0),
         ("", 0),
         ("strip.setBrightness(60) scales EVERY channel of EVERY pixel on the "
          "way out. It changes how bright the whole strip is without changing "
          "any of the colours you set.", 0),
         ("", 0),
         ("Two things worth knowing about setBrightness:", 0),
         ("It is applied when show() is called, so changing it does nothing "
          "until the next show().", 1),
         ("It is LOSSY. Set a pixel to 4, scale by 60/255, and it rounds to 0. "
          "Very dim colours disappear rather than getting dimmer, which is why "
          "a fade can end abruptly.", 1),
         ("", 0),
         ("Rule of thumb: use setBrightness once, in setup(), to fix the "
          "overall level for the room and the power budget. Use the colour "
          "values for everything else.", 0)],
        lead="Two different dimmers")

    deck.two_columns(
        "How your eye turns wavelengths into colour",
        "The physics",
        [("Visible light runs from about 380 nanometres, which you see as "
          "violet, to about 750, which you see as deep red.", 0),
         ("Red      625 - 740 nm", 1),
         ("Yellow   565 - 590 nm", 1),
         ("Green    500 - 565 nm", 1),
         ("Blue     440 - 485 nm", 1),
         ("Violet   380 - 440 nm", 1),
         ("", 0),
         ("A single LED emits one narrow band of wavelengths.", 0)],
        "The biology",
        [("Your retina has three kinds of cone cell, each most sensitive to a "
          "different wavelength. They are called long, medium and short - or, "
          "more usefully, red, green and blue.", 0),
         ("Your brain compares them: red minus green, and blue minus red plus "
          "green. That comparison is called OPPONENCY.", 0),
         ("", 0),
         ("This is why three LEDs can fake every colour you can see. They are "
          "not producing yellow light - they are producing red and green light "
          "in the ratio that makes your cones report yellow.", 0),
         ("Dogs have two cone types. Some birds have four.", 0)],
        note="Magenta and pink are not on the spectrum at all. There is no single "
             "wavelength for them - they exist only as mixtures, which means they "
             "are made by your brain rather than by the light.")

    deck.bullets(
        "The power budget  -  read this before turning the brightness up",
        [("Each of the three LEDs inside a pixel draws about 20 mA at full "
          "brightness.", 0),
         ("One pixel showing full white:  3 x 20 = 60 mA", 1),
         ("32 pixels of full white:  32 x 60 = 1920 mA, nearly 2 amps", 1),
         ("At 5 volts:  1920 mA x 5 V = 9600 mW = 9.6 watts", 1),
         ("", 0),
         ("That is a lot of power through a small board, and it makes real "
          "heat. Two things keep it sane:", 0),
         ("setBrightness() scales EVERY pixel before it is sent. At 60 out of "
          "255 you draw roughly a quarter of the worst case.", 1),
         ("Coloured light is cheaper than white. Pure red only lights one of "
          "the three, so it costs about a third of what white costs.", 1),
         ("", 0),
         ("That is why the vehicle programs run at about 120 rather than 255, "
          "and why full white across all 32 is used for a moment and not held.", 0)],
        lead="Ohm's law, applied to something you can see",
        note="Power = voltage x current. Energy from the battery becomes light "
             "and heat, and the ratio is not as favourable as you would hope.",
        note_kind="warn")

    diagrams.ohms_and_power_law(deck)

    deck.progress(STAGES["lesson4"], 1)

    deck.activity(
        "Do it now  -  all 32, one colour at a time",
        "l4a_all_one_colour",
        [("1.  Upload it. Watch both the vehicle and the Serial Monitor.", 0),
         ("2.  Note the estimated current for each colour. Which is cheapest? "
          "Which is most expensive? Why?", 0),
         ("3.  Change BRIGHTNESS to 255 and upload. After a minute of white, "
          "put your hand near the LED bars - carefully.", 0),
         ("4.  Put it back to 60.", 0),
         ("5.  Change the for loop to  i = i + 2. What happens, and why?", 0),
         ("6.  Look up strip.fill(). Can you replace the loop with one line?", 0)],
        expect=[("All 32 LEDs cycling through seven colours and off, with the "
                 "estimated current printed for each.", 0)],
        questions=[("Yellow costs about twice what red costs. Why?", 0),
                   ("At brightness 60, what is the worst-case current?", 0)],
        minutes=20)

    diagrams.led_map(deck)

    deck.progress(STAGES["lesson4"], 2)

    deck.activity(
        "Do it now  -  learn the map",
        "l4b_led_map",
        [("1.  Sit the vehicle in front of you where you can see all four bars.", 0),
         ("2.  Upload it. Follow the walking dot with your finger.", 0),
         ("3.  Write the numbers onto a sketch of the vehicle. Keep it.", 0),
         ("4.  During the four-sides section, check that LEFT really is 0-7 and "
          "24-31.", 0),
         ("5.  During the mirror section, check that the two lit LEDs really are "
          "opposite each other.", 0),
         ("6.  Change showSide so that FRONT lights only the middle four LEDs. "
          "Which numbers are those?", 0),
         ("7.  Add a section that lights the four CORNERS.", 0)],
        expect=[("A dot walking all the way round, then each side in turn, then "
                 "front and rear LEDs lighting in matched pairs.", 0)],
        questions=[("Which LED is directly behind number 3?", 0),
                   ("You want to light the whole right side. Which two ranges?", 0)],
        minutes=25)

    deck.code(
        "for loops: doing something 32 times without writing it 32 times",
        ["// The long way. Nobody writes this.",
         "strip.setPixelColor(0, white);",
         "strip.setPixelColor(1, white);",
         "strip.setPixelColor(2, white);      // ...and 29 more",
         "",
         "// The for loop. Same thing.",
         "for (int i = 0; i < 32; i++) {",
         "  strip.setPixelColor(i, white);",
         "}",
         "",
         "// Just the front:",
         "for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {",
         "  strip.setPixelColor(i, headlight);",
         "}"],
        filename="the pattern you will use all day",
        notes=[("int i = 0     start here", 0),
               ("i < 32        keep going while this is true", 0),
               ("i++           add one each time round", 0),
               ("", 0),
               ("Watch the difference between  <  and  <=. With FRONT_LAST = 15, "
                "you want <= or you will miss LED 15.", 0),
               ("Off-by-one errors are the most common bug in the whole of "
                "programming, and this is where you meet them.", 0)],
        size=14, highlight={6, 7, 8})

    deck.two_columns(
        "Four patterns, four ideas",
        "colorWipe  and  theaterChase",
        [("colorWipe fills the strip one LED at a time, in order, so the colour "
          "appears to sweep along it.", 0),
         ("One loop, one delay per LED.", 1),
         ("", 0),
         ("theaterChase lights every third LED and shuffles the pattern along by "
          "one each frame, so the lights appear to chase each other.", 0),
         ("Three loops nested inside each other: how many times to repeat, "
          "which of the three positions is lit, and stepping along the strip "
          "in threes.", 1)],
        "rainbow  and  scanner",
        [("rainbow uses HUE instead of red, green and blue - one number that "
          "goes all the way round the colour wheel, 0 to 65535.", 0),
         ("Each LED gets a hue slightly further round, which spreads a "
          "rainbow along the strip. Then the start point creeps forward so "
          "it appears to flow.", 1),
         ("", 0),
         ("scanner is the KITT effect. One bright dot with a fading tail, and "
          "the matching rear LED at 31 - p so front and rear stay lined up.", 0),
         ("The tail uses  >> tail  to halve the brightness each step: 255, "
          "127, 63, 31.", 1)],
        size=14,
        note="gamma32() corrects for the fact that your eye does not see "
             "brightness in a straight line. Without it, the middle of a fade "
             "looks too bright.")

    deck.progress(STAGES["lesson4"], 3)

    deck.activity(
        "Do it now  -  four patterns",
        "l4c_patterns",
        [("1.  Upload it and watch all four run.", 0),
         ("2.  In colorWipe, change the wait from 30 to 5, then to 150.", 0),
         ("3.  In theaterChase, change BOTH 3s to 4s. What changes?", 0),
         ("4.  Change SCANNER_TAIL from 3 to 0, then to 8.", 0),
         ("5.  Change the scanner colour to blue. You now have a Cylon instead "
          "of KITT.", 0),
         ("6.  Copy colorWipe, rename it, and make it run BACKWARDS. Call it "
          "from loop().", 0)],
        expect=[("Colour wipe, theatre chase, a flowing rainbow, then the "
                 "scanner sweeping front and rear together.", 0)],
        questions=[("In theaterChase, why does the inner loop step in threes?", 0),
                   ("What does >> 1 do to a number? Why does that dim an LED?", 0)],
        minutes=25)

    deck.bullets(
        "Design a pattern of your own",
        [("Twenty minutes. Working in your group, invent a lighting pattern for "
          "your vehicle and write it as a function.", 0),
         ("", 0),
         ("Some starting points:", 0),
         ("Police lights - left side red, right side blue, alternating", 1),
         ("A countdown - all 32 red, then fewer and fewer, then green", 1),
         ("A battery gauge - green through amber to red across the front bar", 1),
         ("Breathing - all 32 fading smoothly up and down", 1),
         ("A collision warning that flashes faster as something gets closer", 1),
         ("", 0),
         ("Rules: it must be a function you call from loop(), and it must use "
          "at least one for loop.", 0),
         ("", 0),
         ("Then show the class. Best one gets used in the race next lesson.", 0)],
        lead="Twenty minutes, in your groups")

    deck.table(
        "When the lights will not do what you want",
        ["Symptom", "Most likely cause", "What to do"],
        [["Nothing lights at all",
          "No show() call",
          "Add strip.show() after the setPixelColor calls"],
         ["Only the first few light",
          "LED_COUNT too small",
          "It must be 32, not 12 or 16"],
         ["Colours are wrong - red shows green",
          "Wrong colour order",
          "Check NEO_GRB in the constructor"],
         ["The last few flicker",
          "Not enough power, or a long data run",
          "Lower the brightness and try again"],
         ["A pattern leaves a trail behind it",
          "The strip was not cleared",
          "strip.clear() at the start of each frame"],
         ["Lights work, vehicle stutters",
          "delay() inside an animation",
          "Lesson 5 - use millis() instead"],
         ["One LED stays dark forever",
          "That pixel or its solder joint has failed",
          "Everything after it still works - it passes data through"]],
        lead="Work down this list before you change any code.",
        col_widths=[3.4, 3.2, 4.4],
        size=12)

    deck.quiz(
        "Check yourself",
        [("1.  How can 32 LEDs be controlled independently over one wire?", 0),
         ("2.  What are the four connections on a NeoPixel?", 0),
         ("3.  You call setPixelColor for LEDs 0 to 15 but nothing lights up. "
          "What did you forget?", 0),
         ("4.  What colour is strip.Color(255, 255, 0)? Why?", 0),
         ("5.  32 pixels of full white draw about how much current?", 0),
         ("6.  Which LEDs are on the LEFT side of the vehicle?", 0),
         ("7.  Which LED is directly opposite front LED 6?", 0),
         ("8.  Write a for loop that lights only the rear bar in red.", 0)],
        lead="Next lesson: everything at once, and a race.")

    return deck


# ===================================================================
# LESSON 5
# ===================================================================

def lesson5(deck, T):
    deck.title_slide(
        "Non-blocking timing, edge detection, and the full {} "
        "program.".format(T["full_program"]),
        BYLINE + ["", "Three hours.  Bring a charged battery."],
        hero_image=img("vehicle-side-blue-leds.jpg"), logo=LOGO)

    deck.objectives([
        "Explain why delay() breaks a program that has to do two things "
         "at once",
        "Write the millis() pattern from memory",
        "Detect the edge of a button press, and say what happens "
         "without it",
        "Explain what a dirty flag saves",
        "Drive the vehicle with its full lighting behaviour running",
        "Calibrate your vehicle's speed and race it",
    ])

    deck.bullets(
        "Everything so far",
        [("Lesson 1  -  the toolchain, the three parts of a program, digital "
          "output, the Serial Monitor.", 0),
         ("Lesson 2  -  H-bridges, PWM and duty cycle, tank drive, dead "
          "reckoning, and the maths that predicts where the vehicle ends up.", 0),
         ("Lesson 3  -  Bluetooth, one vehicle per controller, deadzones, map(), "
          "mixing, and the failsafe.", 0),
         ("Lesson 4  -  addressable LEDs, colour and perception, the power "
          "budget, the LED map, and four patterns.", 0),
         ("", 0),
         ("Today: two more ideas, and then all of it at once.", 0)],
        note="By the end of today you will be driving the same program that "
             "leaves the factory on a finished vehicle.")

    deck.table(
        "Today, in order",
        ["Time", "What we do"],
        [["0:00", "Recap. Why delay() has to go"],
         ["0:25", "Upload l5a_millis_not_delay"],
         ["0:45", "Edge detection. Upload l5b_button_toggle"],
         ["1:10", "BREAK  (10 minutes)"],
         ["1:20", "Driving lights from driving state. Upload l5c_drive_with_lights"],
         ["1:50", "A tour of the full program"],
         ["2:10", "BREAK  (10 minutes)"],
         ["2:20", "Servos, the scanner, and sharp steering"],
         ["2:35", "Calibrate, then race"],
         ["2:57", "Where this course goes next"]],
        col_widths=[1, 9])

    deck.bullets(
        "The problem with delay()",
        [("delay(1000) does not mean \"come back in a second\". It means \"stand "
          "here and do absolutely nothing for a second\".", 0),
         ("", 0),
         ("While the ESP32 is inside a delay() it is not reading the controller, "
          "not checking anything, and not updating any other light. Everything "
          "stops.", 0),
         ("", 0),
         ("In Lesson 2 that was fine, because blinking was all we wanted. But "
          "now we need a turn signal every 300 ms, a scanner every 40 ms, and "
          "the motors updated as fast as possible - and you cannot write those "
          "three numbers as delays without them fighting.", 0),
         ("", 0),
         ("The old System Test program did exactly this. Press a button and the "
          "vehicle froze for two seconds while the light pattern played, still "
          "holding whatever motor command was last set. That is why it was "
          "replaced.", 0)],
        lead="It does not wait. It stops.",
        note="This is the single biggest difference between a program that works "
             "on the bench and one that works on a vehicle.",
        note_kind="warn")

    diagrams.millis_timeline(deck)

    deck.code(
        "The pattern, in four lines",
        ["unsigned long lastBlink = 0;",
         "",
         "void loop() {",
         "  if (millis() - lastBlink >= INTERVAL) {",
         "    lastBlink = millis();",
         "    // ...do the thing...",
         "  }",
         "",
         "  // ...and everything else keeps running...",
         "}"],
        filename="millis() instead of delay()",
        notes=[("millis() returns how many milliseconds the board has been "
                "switched on. Reading it costs nothing.", 0),
               ("Each job needs its OWN \"when did I last run\" variable.", 0),
               ("unsigned long, because millis() counts past what an int can "
                "hold in under an hour.", 0),
               ("Subtracting like this also survives the moment millis() wraps "
                "back to zero, which happens after 49 days.", 0),
               ("This exact pattern appears five times in the full vehicle "
                "program.", 0)],
        size=14, highlight={3, 4})

    deck.progress(STAGES["lesson5"], 1)

    deck.activity(
        "Do it now  -  two speeds at once",
        "l5a_millis_not_delay",
        [("1.  Upload it. Two LEDs blink at different rates, and the Serial "
          "Monitor counts how many times loop() runs per second.", 0),
         ("2.  Write down that number. It is enormous.", 0),
         ("3.  Uncomment the delay(1000) at the bottom of loop(). Upload.", 0),
         ("4.  Watch both lights break, and watch the loop counter fall to 1.", 0),
         ("5.  Comment it back out.", 0),
         ("6.  Add a THIRD light on its own timer without touching the other "
          "two.", 0)],
        expect=[("LED 0 blinking once a second, LED 15 blinking about seven "
                 "times a second, and a very large loop count.", 0)],
        questions=[("How many times per second does loop() run?", 0),
                   ("With the delay in place, what is the loop count, and what "
                    "does that tell you about how much attention the vehicle has "
                    "left over?", 0)],
        minutes=20)

    deck.bullets(
        "Edge detection: one press, one action",
        [("loop() runs tens of thousands of times a second. A finger holds a "
          "button for about 100 milliseconds - thousands of trips round the "
          "loop.", 0),
         ("", 0),
         ("So this is wrong:", 0),
         ("if ({}) lightsOn = !lightsOn;".format(T["read_button"]), 1),
         ("", 0),
         ("It flips the lights thousands of times during ONE press. Whether "
          "they end up on or off is pure luck.", 0),
         ("", 0),
         ("What we want is the MOMENT the button goes down - the edge, not the "
          "level. To find an edge you have to remember what it was doing last "
          "time round:", 0),
         ("bool isNewPress = isDown && !wasDown;", 1),
         ("wasDown = isDown;", 1),
         ("", 0),
         ("That is justPressed(), and the full program uses it for every "
          "single button.", 0)],
        lead="The level is not the event")

    deck.progress(STAGES["lesson5"], 2)

    deck.activity(
        "Do it now  -  see the bug, then the fix",
        "l5b_button_toggle",
        [("1.  Upload it.", 0),
         ("2.  Press {} a few times. One press, one change, every "
          "time.".format(T["btn_lights"]), 0),
         ("3.  Now press {} ONCE and read the Serial "
          "Monitor.".format(T["btn_scanner"]), 0),
         ("4.  How many times did one press register?", 0),
         ("5.  Hold the good button down for five seconds. Does anything extra "
          "happen? Should it?", 0),
         ("6.  Change justPressed so it fires on RELEASE instead. Which feels "
          "better to use?", 0)],
        expect=[("The good button toggles the headlights cleanly. The broken one "
                 "reports flipping the value hundreds or thousands of times.", 0)],
        questions=[("Why does the broken version sometimes appear to work?", 0),
                   ("What does the & mean in  bool &wasDown?", 0)],
        minutes=25)

    deck.bullets(
        "State: what the vehicle remembers",
        [("A variable that survives from one pass of loop() to the next is "
          "STATE. It is what makes a program feel like it has a mind rather "
          "than just reflexes.", 0),
         ("", 0),
         ("The vehicle remembers:", 0),
         ("lightsOn        -  are the running lights on at all", 1),
         ("headlightLevel  -  dim or bright", 1),
         ("lightPattern    -  stopped, forward, reverse, left or right", 1),
         ("lightsChanged   -  does the strip need redrawing", 1),
         ("turnMax         -  normal or sharp steering", 1),
         ("scannerOn       -  is the KITT scanner running", 1),
         ("", 0),
         ("That last flag is worth a slide of its own.", 0)],
        lead="Variables that live between passes")

    deck.bullets(
        "Draw only when something changed",
        [("Pushing 32 LEDs out to the strip takes about a millisecond. loop() "
          "runs tens of thousands of times a second.", 0),
         ("Redrawing every pass would waste most of the vehicle's attention and "
          "make the lights flicker.", 0),
         ("", 0),
         ("So the program keeps a flag: lightsChanged.", 0),
         ("Anything that would alter the picture sets it to true.", 1),
         ("The drawing code at the bottom of loop() runs only when it is set, "
          "and then clears it.", 1),
         ("", 0),
         ("if (lightsChanged) { lightsChanged = false; showDrivingLights(); }", 1),
         ("", 0),
         ("This is called a DIRTY FLAG, and you will find the same idea in "
          "graphics engines, spreadsheets and web browsers. Anywhere redrawing "
          "is expensive, something is keeping track of what actually changed.", 0)],
        lead="The dirty flag")

    deck.code(
        "Choosing the lighting picture from what the vehicle is doing",
        ["LightPattern newPattern = LIGHTS_STOPPED;",
         "if      (forward > 0) newPattern = LIGHTS_FORWARD;",
         "else if (forward < 0) newPattern = LIGHTS_REVERSE;",
         "else if (turn > 0)    newPattern = LIGHTS_RIGHT;",
         "else if (turn < 0)    newPattern = LIGHTS_LEFT;",
         "",
         "if (newPattern != lightPattern) {",
         "  lightPattern = newPattern;",
         "  lightsChanged = true;",
         "}"],
        filename="from l5c_drive_with_lights.ino",
        notes=[("The lights are not controlled by a button. They follow what "
                "the vehicle is ALREADY doing, which is how a real car works.", 0),
               ("An enum is a named list of the states something can be in. "
                "Better than 1, 2, 3, 4, 5, because the compiler can check it "
                "and you can read it.", 0),
               ("The else-if chain means exactly one pattern is chosen. Driving "
                "forward wins over turning.", 0),
               ("Only if it CHANGED do we set the dirty flag.", 0)],
        size=14, highlight={6, 7, 8})

    deck.progress(STAGES["lesson5"], 3)

    deck.activity(
        "Do it now  -  drive with lights",
        "l5c_drive_with_lights",
        [("1.  Wheels off the ground. Upload. Check the controls.", 0),
         ("2.  Put it down and drive. Watch the lights follow the stick: "
          "headlights, tail lights, brake lights when you stop, white in "
          "reverse, amber on the side you are turning towards.", 0),
         ("3.  Press {} to toggle the lights.".format(T["btn_lights"]), 0),
         ("4.  D-pad up and down for bright and dim headlights.", 0),
         ("5.  Make the turn signals BLINK rather than stay on. Use millis(), "
          "not delay(), or the vehicle will stutter.", 0)],
        expect=[("A vehicle that drives and lights itself correctly at the same "
                 "time, with no stuttering.", 0)],
        questions=[("Which lines make the brake lights come on?", 0),
                   ("Why does the amber only appear on one side?", 0)],
        safety="Check the controls with the wheels off the ground first.",
        minutes=30)

    deck.section("The full program", minutes=45)

    deck.table(
        "{}  -  what it adds".format(T["full_program"]),
        ["Feature", "Where you have seen it"],
        [["Tank drive with mixing", "Lesson 3 - l3c_tank_drive"],
         ["Deadzone and map", "Lesson 3 - l3b_deadzone_and_map"],
         ["Failsafe on disconnect", "Lesson 3 - the top of loop()"],
         ["Headlights, tail, brake, reverse, signals", "Lesson 5 - l5c"],
         ["The dirty flag", "Lesson 5 - l5c"],
         ["Edge-detected buttons", "Lesson 5 - l5b"],
         ["Two servos on the right stick", "New today"],
         ["The KITT scanner, non-blocking", "Lesson 4 - l4c, made non-blocking"],
         ["Sharp steering toggle", "New today"],
         ["A startup light show", "New today - it proves every LED works"]],
        lead="You have already written most of it. Here is what is left.",
        col_widths=[5.5, 5.5],
        size=14)

    diagrams.servo_pulse(deck)

    deck.bullets(
        "The last two additions",
        [("THE KITT SCANNER, done properly.", 0),
         ("The version in Lesson 4 used delay(), so the vehicle froze while it "
          "swept. The full program moves the dot one step every 40 ms off "
          "millis(), so you can drive with the scanner running.", 1),
         ("Toggled with {}.".format(T["btn_scanner"]), 1),
         ("", 0),
         ("SHARP STEERING.", 0),
         ("turnMax is normally MOTOR_MAX / 2, which makes the vehicle easy to "
          "aim. {} flips it to full power, for spinning on the "
          "spot.".format(T["btn_stickclick"]), 1),
         ("The bottom of the range stays the same in both modes, so a gentle "
          "turn still breaks the wheels loose.", 1),
         ("", 0),
         ("THE STARTUP LIGHT SHOW.", 0),
         ("One scanner sweep at power-up. delay() is fine THERE, because "
          "nothing else needs to happen yet - and it proves every LED works "
          "before you drive off.", 1)],
        note="Read the top of the file. Every setting you might want to change "
             "is in the SETTINGS block, with a comment saying what it does.")

    deck.progress(STAGES["lesson5"], 4)

    deck.activity(
        "Do it now  -  the real thing",
        T["full_program"],
        [("1.  Open {}.ino.".format(T["full_program"]), 0),
         ("2.  READ IT FIRST. You will recognise almost all of it.", 0),
         (("3.  Set PS3_MAC_ADDRESS to your vehicle's address."
           if T["key"].endswith("ps3") else
           "3.  Paste your MY_CONTROLLER line into the top."), 0),
         ("4.  Wheels off the ground. Upload. Check every control.", 0),
         ("5.  Put it down and drive properly.", 0),
         ("6.  Find three things in the file you have NOT seen before, and work "
          "out what they do.", 0)],
        expect=[("A startup sweep, then a green blink while it waits, then full "
                 "driving with lights, servos and the scanner.", 0)],
        questions=[("What does the green blink mean?", 0),
                   ("What are the three things you found?", 0)],
        safety="Wheels off the ground until every control is checked.",
        minutes=30)

    deck.bullets(
        "Calibrate, then race",
        [("CALIBRATE FIRST.", 0),
         ("Mark a start and a finish line with tape. Measure the distance.", 1),
         ("Drive it flat out and time it. Speed = distance / time.", 1),
         ("Compare with the number you calculated in Lesson 2. Were you right?", 1),
         ("", 0),
         ("DRAG RACE.", 0),
         ("Two or three vehicles side by side. First across the line INSIDE "
          "the lane wins - leaving the lane does not count.", 1),
         ("Straight-line speed is not the whole problem. Keeping it straight "
          "is, because only one command runs at a time.", 1),
         ("You can only brake by reversing. Half power for a short time is "
          "usually right.", 1),
         ("", 0),
         ("COURSE RACE.", 0),
         ("Tape out a course with corners. Now sharp steering earns its keep.", 1),
         ("Fastest clean lap. Touching a cone costs you two seconds.", 1)],
        lead="Measure first. Then argue about who is fastest.")

    deck.bullets(
        "Take it further  -  projects that fit on this vehicle",
        [("Each of these is a real addition to the program you now understand. "
          "Pick one and build it.", 0),
         ("", 0),
         ("LOW BATTERY WARNING.  Flash the LEDs yellow below 12 volts and red "
          "below 10. A 4S pack should never be run flat, and the vehicle is in "
          "a better position to notice than you are.", 0),
         ("", 0),
         ("COLLISION WARNING.  An ultrasonic range finder on the top plate. "
          "Closer than 20 inches: stop the motors, flash red, wait three "
          "seconds, then carry on.", 0),
         ("", 0),
         ("AUTOMATIC LIGHTS.  You already switch the turn signals from the "
          "stick. Add reversing beeps, hazard lights, or headlights that come "
          "on only when the vehicle is moving.", 0),
         ("", 0),
         ("WAYPOINT NAVIGATION.  Lesson 2 drove one square. Give the program a "
          "LIST of moves - distance and heading - and drive a course you typed "
          "in rather than one that was hard-coded.", 0),
         ("", 0),
         ("SERVO PAN AND TILT.  Two servos and a bracket, aimed with the right "
          "stick. The mount points are already on the top plate.", 0)],
        note="All five are within reach of what you learned in five lessons. "
             "The range finder and the servos are the two the kit already has "
             "parts for.")

    deck.two_columns(
        "Where this goes next",
        "Straight on from here",
        [("SENSORS - a range finder that stops the vehicle before it hits "
          "something, line followers, light and temperature.", 0),
         ("Once the vehicle can sense the world, dead reckoning stops being "
          "the only way it knows where it is.", 0),
         ("", 0),
         ("VEHICLE TO VEHICLE - ESP-NOW lets ESP32s talk directly to each "
          "other with no router in between.", 0),
         ("", 0),
         ("CAMERAS AND AI - object recognition, and a 6-DOF arm that picks "
          "things up.", 0)],
        "The advanced program",
        [("Pathfinder_Op_Program12 is the same vehicle with:", 0),
         ("the program split across eight files by subsystem", 1),
         ("10-bit PWM and hybrid drive for finer low-speed control", 1),
         ("speed ramping, so it does not lurch", 1),
         ("a serial console for tuning without recompiling", 1),
         ("settings saved to EEPROM", 1),
         ("a current sensor and a powered self-test", 1),
         ("", 0),
         ("It is a five-lesson course of its own.", 0)],
        note="Everything you learned this term transfers directly. The advanced "
             "course starts by taking this program apart.")

    deck.bullets(
        "Why this is worth your time",
        [("At Oceanology International Americas in San Diego, we asked a panel "
          "what the transition opportunities are:", 0),
         ("", 0),
         ("Dr Rick Spinrad, NOAA Administrator, said a knowledge of robotics "
          "and STEM is a competitive advantage for entry-level positions.", 0),
         ("Rear Admiral Ron Piret, USN, said they are looking for students with "
          "robotics and STEM education for operational careers.", 0),
         ("Kendra MacDonald, CEO of Canada's Ocean Supercluster, said that "
          "whether you become a doctor, a lawyer or a scientist, STEM will help "
          "you.", 0),
         ("", 0),
         ("What you have actually practised this term: reading somebody else's "
          "code, changing one thing at a time, measuring instead of guessing, "
          "and working out whether a fault is mechanical, electrical or "
          "software.", 0),
         ("", 0),
         ("Those four habits are the job. The vehicle was just the excuse.", 0)],
        lead="Robotics and STEM, and where they lead")

    deck.quiz(
        "Check yourself",
        [("1.  Why can a program not use delay() if it has to do two things at "
          "different rates?", 0),
         ("2.  Write the four-line millis() pattern from memory.", 0),
         ("3.  Why does unsigned long matter for millis()?", 0),
         ("4.  What is edge detection, and what goes wrong without it?", 0),
         ("5.  What is a dirty flag, and what does it save?", 0),
         ("6.  A servo is centred by a pulse of what length? How often is it "
          "sent?", 0),
         ("7.  Name three things the full program does that l5c does not.", 0),
         ("8.  Your vehicle drifts left on the drag strip. Give one mechanical "
          "cause and one software fix.", 0)],
        lead="Thank you. Go and break something interesting.")

    return deck


# ===================================================================
# BUILD
# ===================================================================

def enrich(track):
    """
    Fills in the numbers the slides quote by reading them out of the sketches,
    so a retune of the vehicle cannot leave a stale figure on a slide. If a
    constant has been renamed, srcfacts raises and the build stops.
    """
    drive = track["drive_sketch"]
    led = track["led_sketch"]

    track["stick_max"] = srcfacts.number(drive, "STICK_MAX")
    track["deadzone"] = srcfacts.number(drive, "STICK_DEADZONE")
    track["motor_max"] = srcfacts.number(drive, "MOTOR_MAX")
    track["motor_min"] = srcfacts.number(drive, "MOTOR_MIN")
    track["pwm_freq"] = srcfacts.number(drive, "MOTOR_PWM_FREQ")
    track["pwm_bits"] = srcfacts.number(drive, "MOTOR_PWM_BITS")
    track["led_pin"] = srcfacts.number(led, "LED_PIN")
    track["led_count"] = srcfacts.number(led, "LED_COUNT")
    track["motor_pins"] = srcfacts.motor_pins(drive, track["pin_style"])

    percent = 100.0 * track["deadzone"] / track["stick_max"]
    track["deadzone_pct"] = "about %d%%" % round(percent)
    return track


LESSONS = [
    ("l1_introduction.pptx", "Lesson 1", "Introduction", lesson1),
    ("l2_motor_control.pptx", "Lesson 2", "Motor Control", lesson2),
    ("l3_controller_programming.pptx", "Lesson 3", "Controller Programming", lesson3),
    ("l4_neopixel_leds.pptx", "Lesson 4", "NeoPixel LEDs", lesson4),
    ("l5_maneuvers_with_lights.pptx", "Lesson 5", "Maneuvers with Lights", lesson5),
]


def build(track, out_dir):
    made = []
    enrich(track)
    for filename, label, title, builder in LESSONS:
        deck = Deck(filename, title, label, track["track_label"])
        builder(deck, track)
        made.append(deck.save(out_dir))
    return made
