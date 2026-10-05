"""
content_explorers.py - Pathfinder Explorers, twelve one-hour lessons for
students aged nine to thirteen.

This is the beginner course rebuilt for younger students, on the Nintendo
Switch track only. Same vehicle, same board package, same slide engine - but
one hour at a time instead of three, a short helper file (explorer.h) so a
student's own program fits on one screen, unplugged games before every new
idea, three levels on every mission so a nine-year-old and a
thirteen-year-old can share a vehicle, and a driver's license that turns
speed into something students earn.

Each lesson is a function that fills a Deck. The numbers the slides quote -
the learner speed limit, the PWM frequency, line numbers in the sketches -
are read out of the sources by srcfacts, the same way the other courses do
it, so a retune cannot leave a stale figure on a slide.
"""

import os

import diagrams_explorers as dx
import srcfacts
import questionnaires
import sync_explorer_h
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
TRACK_LABEL = "Pathfinder Explorers"

SRC = "lessons/explorers"


def sketch(name):
    """The path srcfacts wants for one Explorers sketch."""
    return "%s/%s/%s.ino" % (SRC, name, name)


ENGINE = SRC + "/explorer.h"

# Read from the sources by enrich(), before any deck is built.
FACTS = {}

# The license levels. These are course policy rather than code - every
# sketch that moves carries them in a comment on its SPEED_LIMIT line, and
# the learner level itself is read from the code by enrich().
LICENSED = 75
EXPERT = 100


# ===================================================================
# Pictures we have not taken yet
# ===================================================================
#
# Every one of these draws a dashed, labeled box on the slide until somebody
# supplies the picture. The course README lists them all in one place.

SOJOURNER = Placeholder(
    "IMAGE: NASA's Sojourner rover on Mars, 1997",
    "NASA images are generally free to use in teaching. Search 'Sojourner' "
    "at photojournal.jpl.nasa.gov and save one as sojourner-on-mars.jpg")

STICKERS = Placeholder(
    "PHOTO: a vehicle and its controller with matching number stickers",
    "Both stickers in the same shot, close enough to read the number")

WHEELS_UP = Placeholder(
    "PHOTO: a vehicle on its wheels-up stand",
    "A block or small box under the frame, all four wheels clear of the "
    "table. Shoot it from the side so the gap under the wheels shows")

POWER_SWITCH = Placeholder(
    "PHOTO: the vehicle's power switch, close up",
    "Show which way is ON. Add an arrow if the switch is not marked")

ARENA = Placeholder(
    "PHOTO: the driving arena on the classroom floor",
    "Masking-tape rectangle, four cups, the START box and the GARAGE, "
    "taken from a chair so the whole course is in the frame")

IDE_BUTTONS = Placeholder(
    "SCREENSHOT: the Arduino IDE with e02a_hello_rover open",
    "Circle Verify (check mark), Upload (arrow), Serial Monitor (magnifying "
    "glass), and the explorer.h tab")

SERIAL_HELLO = Placeholder(
    "SCREENSHOT: the Serial Monitor showing the hello message",
    "115200 baud selected, three or four 'Hello! I am a Pathfinder rover.' "
    "lines visible")

BUG_SCREEN = Placeholder(
    "SCREENSHOT: the IDE after clicking Verify on e02b_bug_hunt",
    "The first error message showing at the bottom of the window, with its "
    "line number")

COLOR_LIGHTS = Placeholder(
    "PHOTO: lights 0 to 6 running e03a_color_lab",
    "Taken in a dim room from the front, so the seven colors are easy to "
    "tell apart")

TAPE_SQUARE = Placeholder(
    "PHOTO: the tape square on the floor, START marked",
    "About 4 feet on a side. An X or an arrow on the START corner showing "
    "which way the vehicle faces")

MARS_COURSE = Placeholder(
    "PHOTO: the Mission to Mars course",
    "START base, the crater (a hula hoop or a ring of paper plates), and "
    "the SAMPLE zone, with the one-foot grid taped or chalked on the floor")

MISSION_DAY = Placeholder(
    "PHOTO: the Mission Day course, from above",
    "Take it on the day if there is no earlier chance - it makes a good "
    "slide for next year's class")


# ===================================================================
# THE LESSONS, THE STAGES, AND THE CLOCK
# ===================================================================

STAGES = {
    "lesson1": ["What is a robot?", "Meet Pathfinder", "The Explorer Code",
                "First drive"],
    "lesson2": ["Instructions", "Your first upload", "Bug hunt"],
    "lesson3": ["Mixing light", "The color lab", "The light map"],
    "lesson4": ["Repeat after me", "The light show", "Your own show"],
    "lesson5": ["How motors work", "Speed", "Tank steering", "Motor lab"],
    "lesson6": ["Autopilot", "Drive a square", "Tune it"],
    "lesson7": ["Too far to steer", "Plan the route", "Run the mission"],
    "lesson8": ["Radio links", "Choices", "Button lab"],
    "lesson9": ["Stick numbers", "The wobble zone", "Tank mixing",
                "License test"],
    "lesson10": ["Lights that talk", "Lights from driving", "Blinking"],
    "lesson11": ["Ask and imagine", "Plan", "Create and test"],
    "lesson12": ["Final checks", "The mission", "Look back", "Celebrate"],
}

# One agenda per lesson, and the only place timings live. The "Today, in
# order" table, every divider's "About N minutes", and the minutes on every
# activity are all worked out from these rows. Each row is
# (start, what we do, stage), with None for the opening and the wrap-up.
AGENDA = {
    "lesson1": [
        ("0:00", "Welcome, and the Starting Line questionnaire", None),
        ("0:06", "The game 'Robot or not?'", "What is a robot?"),
        ("0:14", "Meet Pathfinder: every part, and what it does",
         "Meet Pathfinder"),
        ("0:21", "Team jobs and the Explorer Code", "The Explorer Code"),
        ("0:30", "First drive in the arena", "First drive"),
        ("0:50", "Mission Log, Learner Permits, and pack up", None),
    ],
    "lesson2": [
        ("0:00", "Warm-up: what did the rover do last time?", None),
        ("0:05", "Program a Human Robot", "Instructions"),
        ("0:17", "The Arduino IDE, setup() and loop()", "Your first upload"),
        ("0:25", "Upload e02a_hello_rover and change it", "Your first upload"),
        ("0:40", "Bug hunt: e02b_bug_hunt", "Bug hunt"),
        ("0:52", "Mission Log and pack up", None),
    ],
    "lesson3": [
        ("0:00", "Warm-up: what IS red, to a computer?", None),
        ("0:05", "Mixing light is not like mixing paint", "Mixing light"),
        ("0:15", "Predict, then upload e03a_color_lab", "The color lab"),
        ("0:35", "Find every light with e03b_light_map", "The light map"),
        ("0:50", "Check yourself, Mission Log, and pack up", None),
    ],
    "lesson4": [
        ("0:00", "Warm-up: how would you light all 32?", None),
        ("0:05", "The Human Light Strip", "Repeat after me"),
        ("0:15", "for loops, piece by piece", "Repeat after me"),
        ("0:22", "Upload e04a_light_show and change it", "The light show"),
        ("0:37", "Design and build your own pattern", "Your own show"),
        ("0:52", "Mission Log and pack up", None),
    ],
    "lesson5": [
        ("0:00", "Warm-up, and the wheels-up rule", None),
        ("0:05", "What is inside a motor", "How motors work"),
        ("0:12", "Pedal and coast: how a motor goes half speed", "Speed"),
        ("0:20", "The Human Tank", "Tank steering"),
        ("0:30", "Upload e05a_motor_lab - wheels up!", "Motor lab"),
        ("0:52", "Mission Log and pack up", None),
    ],
    "lesson6": [
        ("0:00", "Warm-up: what does drive(50, -50) do?", None),
        ("0:05", "Autopilot moves, and milliseconds", "Autopilot"),
        ("0:13", "Upload e06a_drive_a_square and run it on the floor",
         "Drive a square"),
        ("0:28", "Tune TURN_TIME until it comes home", "Tune it"),
        ("0:52", "Mission Log and pack up", None),
    ],
    "lesson7": [
        ("0:00", "The Halfway Check-in questionnaire", None),
        ("0:07", "Warm-up, and the Mars Delay Game", "Too far to steer"),
        ("0:15", "Functions: invent your own moves", "Plan the route"),
        ("0:20", "Plan the route on the grid", "Plan the route"),
        ("0:28", "Program it, run it, fix it", "Run the mission"),
        ("0:50", "Mission debrief, Mission Log, and pack up", None),
    ],
    "lesson8": [
        ("0:00", "Warm-up: what was hard about autopilot?", None),
        ("0:05", "Radio, and one controller per vehicle", "Radio links"),
        ("0:12", "Simon Says, if/else edition", "Choices"),
        ("0:20", "if and else, held and tapped", "Choices"),
        ("0:28", "Upload e08a_button_lab and make the buttons yours",
         "Button lab"),
        ("0:52", "Mission Log and pack up", None),
    ],
    "lesson9": [
        ("0:00", "Warm-up: held or tapped?", None),
        ("0:05", "What the stick really sends", "Stick numbers"),
        ("0:10", "Upload e09a_joystick_drive and watch the numbers",
         "The wobble zone"),
        ("0:22", "Mixing forward and turn", "Tank mixing"),
        ("0:30", "The driving test", "License test"),
        ("0:52", "Mission Log and pack up", None),
    ],
    "lesson10": [
        ("0:00", "Warm-up: car-light spotting", None),
        ("0:05", "What car lights mean", "Lights that talk"),
        ("0:12", "Upload e10a_smart_lights, then add turn signals",
         "Lights from driving"),
        ("0:30", "The nap problem, and blinkIsOn()", "Blinking"),
        ("0:37", "Make them blink, add a headlight switch", "Blinking"),
        ("0:52", "Mission Log and pack up", None),
    ],
    "lesson11": [
        ("0:00", "Warm-up: upgrades real engineers make", None),
        ("0:05", "The design process, and picking an upgrade",
         "Ask and imagine"),
        ("0:13", "Plan it on paper", "Plan"),
        ("0:20", "Build it, test it, improve it", "Create and test"),
        ("0:48", "Show and tell, Mission Log, and pack up", None),
    ],
    "lesson12": [
        ("0:00", "Mission briefing and the course", None),
        ("0:05", "Pre-mission checks and autopilot tuning", "Final checks"),
        ("0:15", "The mission: four stations, every team", "The mission"),
        ("0:40", "Look back: the Finish Line questionnaire", "Look back"),
        ("0:47", "Upgrade showcase and certificates", "Celebrate"),
        ("0:56", "What's next, and the last pack up", None),
    ],
}

LESSON_ENDS = "1:00"


def _minutes(clock):
    hours, minutes = clock.split(":")
    return int(hours) * 60 + int(minutes)


def agenda_rows(lesson):
    return [[start, what] for start, what, _stage in AGENDA[lesson]]


def stage_minutes(lesson, stage):
    """Minutes the agenda gives a stage, rounded to five. Unknown = stop."""
    rows = AGENDA[lesson]
    if stage not in {row[2] for row in rows}:
        raise SystemExit("stage_minutes: %s has no agenda row for stage %r"
                         % (lesson, stage))
    total = 0
    for index, (start, _what, tag) in enumerate(rows):
        if tag != stage:
            continue
        nxt = rows[index + 1][0] if index + 1 < len(rows) else LESSON_ENDS
        total += _minutes(nxt) - _minutes(start)
    return max(5, int(round(total / 5.0)) * 5)


def block_minutes(lesson, start):
    """Minutes in the one agenda row that starts at `start`."""
    rows = AGENDA[lesson]
    for index, (when, _what, _stage) in enumerate(rows):
        if when == start:
            nxt = rows[index + 1][0] if index + 1 < len(rows) else LESSON_ENDS
            return _minutes(nxt) - _minutes(when)
    raise SystemExit("block_minutes: %s has no agenda row at %s"
                     % (lesson, start))


def line(name, needle):
    """The line number of `needle` in a sketch, for 'go to line N'."""
    return srcfacts.line_of(sketch(name), needle)


# ===================================================================
# SLIDES EVERY LESSON USES
# ===================================================================

def title(deck, subtitle, hero, speaker):
    deck.title_slide(subtitle,
                     BYLINE + ["", "One hour.  Teams of three, one vehicle "
                               "per team."],
                     hero_image=hero, logo=LOGO, speaker=speaker)


def todays_mission(deck, items, speaker):
    deck.objectives(items, title="Today's mission",
                    lead="By the end of today, you can:", speaker=speaker)


def agenda(deck, lesson, speaker=None):
    deck.table(
        "Today, in order", ["Time", "What we do"], agenda_rows(lesson),
        col_widths=[1, 9],
        speaker=speaker or [
            "Put the agenda on the board as well, with the driving or "
            "uploading block circled. Students settle when they can see "
            "when the hands-on part starts.",
            "The timings are a guide for a one-hour session. If the "
            "hands-on block runs long, take the time out of the check "
            "yourself questions, never out of the pack-up - a rushed "
            "pack-up is how cables and controllers go missing.",
        ])


def divider(deck, lesson, stage, subtitle=None, speaker=None):
    deck.section(stage, subtitle=subtitle,
                 minutes=stage_minutes(lesson, stage),
                 stages=STAGES[lesson], stage=stage,
                 speaker=speaker or [
                     "A breath between parts of the lesson. Read the "
                     "title out and point at where it sits on the strip.",
                 ])


def mission(deck, lesson, start, title_text, sketch_name, steps, expect=None,
            questions=None, safety=None, speaker=None):
    deck.activity(title_text, sketch_name, steps, expect=expect,
                  questions=questions, minutes=block_minutes(lesson, start),
                  safety=safety, speaker=speaker, label="MISSION:   ")


def unplugged(deck, lesson, start, title_text, game, steps, expect=None,
              questions=None, safety=None, speaker=None):
    deck.activity(title_text, game, steps, expect=expect, questions=questions,
                  minutes=block_minutes(lesson, start), safety=safety,
                  speaker=speaker, label="UNPLUGGED:   ",
                  sketch_is_code=False)


CHECKINS = {
    "Start": {
        "steps": [
            ("Your teacher gives you a questionnaire, and your EXPLORER "
             "CODE. Write the code at the top - not your name.", 0),
            ("This is NOT a test. Nobody gets a grade.", 0),
            ("Your teacher reads each question out loud. Answer on your "
             "own - no peeking!", 0),
            ("Not sure? Tick \"I don't know yet\". Today that's the RIGHT "
             "answer for most of Part 2!", 0),
            ("Done? Turn it over and wait quietly.", 0)],
        "expect": [("You won't know most of Part 2 yet. That's the point: "
                    "in twelve lessons, you will.", 0)],
        "speaker": [
            "Hand this out before ANYTHING is taught, so the answers show "
            "where students start rather than what the first ten minutes "
            "taught them.",
            "Give every student an Explorer code: team number plus a letter "
            "(3A, 3B, 3C). Write it on a sticky note on their desk; they "
            "copy it onto the questionnaire and the Mission Log cover. Keep "
            "the list of codes and names yourself. The questionnaires never "
            "carry names, which is what lets students be honest.",
            "Read every question aloud, at a steady pace, for the whole "
            "room. Nine-year-olds read at very different speeds, and you "
            "are measuring what they know, not how fast they read.",
            "Don't explain or hint at Part 2. If somebody asks, the answer "
            "is always: if you're not sure, tick I don't know yet.",
            "Collect them straight away, and don't read them in front of "
            "the class. The README says how to type them into "
            "questionnaire_tracker.xlsx. A student who misses today does "
            "it at the start of Lesson 2, with the real date on it.",
        ],
    },
    "Halfway": {
        "steps": [
            ("Write your Explorer code at the top. It's on your Mission Log "
             "cover.", 0),
            ("Parts 1 and 2 are the same as last time. Answer for how you "
             "feel, and what you know, TODAY.", 0),
            ("Part 3: rate Lessons 1 to 6. Flip through your Mission Log if "
             "you need a reminder.", 0),
            ("Part 4: tell us what's working and what isn't. We really do "
             "change the course because of what you write.", 0)],
        "expect": [("Honest answers. \"Too hard\" and \"Not much\" help "
                    "us just as much as \"Loved it\".", 0)],
        "speaker": [
            "Same routine as the Starting Line: codes not names, read every "
            "question aloud, no hints on Part 2, collect straight away.",
            "This is the most useful questionnaire for THIS cohort, because "
            "it comes early enough to change the next six lessons. Skim "
            "Parts 3 and 4 before Lesson 8, pick one thing you can change, "
            "and tell the class you changed it: 'lots of you said the "
            "lessons felt rushed, so today...'. Students who see their "
            "answers matter give better answers at the finish.",
            "Any 'No' to I feel safe in this class deserves a quiet "
            "follow-up within the week - you can't tell who from the code "
            "alone, so use your list.",
        ],
    },
    "Finish": {
        "steps": [
            ("Write your Explorer code at the top.", 0),
            ("Parts 1 and 2 are the same as the first two times. Answer for "
             "TODAY.", 0),
            ("Part 3: rate Lessons 7 to 12.", 0),
            ("Part 4: your favorite lesson, the hardest thing, and what "
             "you'd change.", 0),
            ("Hand it in. Then: the showcase!", 0)],
        "expect": [("Answer on your own. Your answers shape the course for "
                    "the next Explorers.", 0)],
        "speaker": [
            "Do this straight after the mission and before the showcase, "
            "while the whole course is fresh. If families are here, invite "
            "them to look through their child's Mission Log while the "
            "class writes.",
            "If the day is running late, keep this and shorten the "
            "showcase. The Finish Line is the questionnaire that shows how "
            "far the cohort has come.",
            "Same routine as before: codes not names, read every question "
            "aloud, no hints on Part 2. Collect every form before the "
            "certificates come out.",
        ],
    },
}


def checkin(deck, key):
    """The slide that hands out one of the three questionnaires."""
    _key, name, number, lesson, _when = questionnaires.checkpoint(key)
    start = next(row[0] for row in AGENDA[lesson]
                 if "questionnaire" in row[1].lower())
    text = CHECKINS[key]
    deck.activity("%s  -  questionnaire %d of 3" % (name, number),
                  "%s questionnaire" % name, text["steps"],
                  expect=text["expect"],
                  minutes=block_minutes(lesson, start),
                  speaker=text["speaker"], label="CHECK-IN:   ",
                  sketch_is_code=False)


def new_words(deck, rows, speaker=None):
    deck.table(
        "New words", ["Word", "What it means"], rows, col_widths=[2.6, 8],
        speaker=speaker or [
            "Read each word out and have the room say it back. Saying a "
            "word out loud is half of owning it.",
            "These are also in the glossary at the back of the Mission Log.",
        ])


PACK_UP = [
    ("Crew Chief: vehicle power OFF.", 0),
    ("Unplug the cable and coil it up.", 0),
    ("Vehicle and controller back in their spot, stickers matching.", 0),
    ("Close the laptop. Push your chairs in.", 0),
]


def wrap_up(deck, log_items, badge, speaker=None, pack=None):
    deck.two_columns(
        "Mission Log, then pack up",
        "In your Mission Log",
        [(text, 0) for text in log_items] +
        [("Stamp: " + badge, 0)],
        "Pack up",
        pack or PACK_UP,
        note="Last job: every team member says one thing they learned today.",
        speaker=speaker or [
            "Give the Mission Log a real five minutes. Writing down what "
            "they did is what turns an activity into something they "
            "remember next week.",
            "Stamp or sign the badge box as each team shows you their "
            "page. It costs you a minute and it matters to them.",
            "Do not let anyone leave until their Crew Chief has shown you "
            "a switched-off vehicle.",
        ])


def license_table(deck, speaker=None):
    deck.table(
        "Your driver's license",
        ["License", "SPEED_LIMIT", "How you earn it"],
        [["Learner Permit", str(FACTS["learner"]),
          "Lesson 1: know the Explorer Code and finish the driving course"],
         ["Driver's License", str(LICENSED),
          "Lesson 9: pass the driving test - no cups knocked over"],
         ["Expert License", str(EXPERT),
          "Lesson 10 or later: the test course at {}, signaling every "
          "turn".format(LICENSED)]],
        col_widths=[2.6, 2.0, 6.5],
        note="Speed is something you EARN, and your teacher decides. "
             "{} means full power - it is FAST.".format(EXPERT),
        speaker=speaker or [
            "This is the reward system for the whole course, so make it "
            "feel like a real license. Stamp it. Make a little ceremony "
            "of the upgrade.",
            "SPEED_LIMIT is a percent of full power. At the Learner level "
            "a full stick gives half power; at Expert it gives everything "
            "the motors have.",
            "It is entirely your call when a team goes up a level. A team "
            "that drives carelessly at {} can be asked to go back to "
            "{} - say so now, before anyone has earned anything.".format(
                LICENSED, FACTS["learner"]),
        ])


# ===================================================================
# LESSON 1 - MEET PATHFINDER
# ===================================================================

def lesson1(deck):
    L = "lesson1"
    title(deck,
          "What a robot is, the vehicle your team will program for twelve "
          "lessons, how we stay safe - and your very first drive.",
          img("vehicle-turn-signal-left.jpg"),
          speaker=[
              "BEFORE CLASS: every vehicle charged, on its stand, switched "
              "off, with e01a_learner_drive already uploaded and its "
              "controller claimed with e00_claim_controller. The arena is "
              "taped on the floor. The README has the full checklist.",
              "Introduce yourself, then go round the room for names. Ask "
              "who has built or programmed anything before - a LEGO kit, "
              "Scratch, Minecraft redstone all count. It tells you who "
              "can help their team, and makes beginners feel normal.",
              "Promise them one thing right now: everybody drives a robot "
              "before this hour is up.",
          ])

    todays_mission(deck, [
        "Say the three things every robot does",
        "Find the computer, motors, lights and battery on a Pathfinder",
        "Explain the Explorer Code, and why each rule is there",
        "Drive a Pathfinder through the course and park it in the garage",
    ], speaker=[
        "Read these out. Come back to this slide in the last five minutes "
        "and ask for a thumbs-up, sideways or down on each one.",
        "The fourth one is the one they care about. Say so, and say that "
        "the first three are how they earn it.",
    ])

    deck.bullets(
        "Welcome, Explorers!",
        [("For the next twelve lessons, your team gets its own robot "
          "vehicle.", 0),
         ("You will make it light up, move, drive itself, and listen to "
          "your controller.", 0),
         ("By the last lesson, YOU will have written the program that "
          "drives it.", 0),
         ("Mistakes are part of the job. Every engineer breaks things. The "
          "good ones find out why.", 0)],
        note="Questions are always welcome here. \"I don't know - let's "
             "find out\" is a great answer.",
        speaker=[
            "Set the tone in the next two minutes: this is a room where "
            "being wrong is normal and asking is good.",
            "The last bullet matters most. A nine-year-old whose vehicle "
            "does something strange needs to hear that this is the job, "
            "not a sign they're bad at it. Say it slowly.",
            "If you have a few older students, ask them now to be the "
            "people who help, not the people who grab the keyboard. Most "
            "rise to it.",
        ])

    deck.table(
        "Where we're going",
        ["Lesson", "What you'll do"],
        [["1", "Meet Pathfinder, and drive it"],
         ["2", "Write your first program, and hunt bugs"],
         ["3", "Mix colors with light, and map every light"],
         ["4", "Loops, and a light show you design"],
         ["5", "Make the wheels move"],
         ["6", "Autopilot: drive a square by code"],
         ["7", "Mission to Mars: plan a route"],
         ["8", "Make the controller's buttons do your bidding"],
         ["9", "Joystick driving, and your driving test"],
         ["10", "Smart lights that signal, like a car"],
         ["11", "Design your own upgrade"],
         ["12", "Mission Day!"]],
        col_widths=[1.2, 9],
        speaker=[
            "One line each - this is a map, not a lesson.",
            "Point at Lesson 12 and tell them families are invited to "
            "watch, if they are. Having somebody to show is a strong "
            "motivator all term.",
        ])

    agenda(deck, L)

    checkin(deck, "Start")

    divider(deck, L, "What is a robot?")

    unplugged(
        deck, L, "0:06", "Robot or not?", "Thumbs up, thumbs down",
        [("Your teacher names something. You decide: is it a ROBOT?", 0),
         ("Thumbs UP for robot. Thumbs DOWN for not a robot.", 0),
         ("A robot vacuum.  A TV remote.  An automatic door.", 1),
         ("A Mars rover.  A remote-control car.  A calculator.", 1),
         ("Pick one and explain your vote to the person next to you.", 0)],
        expect=[("Lots of arguing. That's the point!", 0)],
        questions=[("What does a machine need to do to count as a "
                    "robot?", 0)],
        speaker=[
            "Do these one at a time and make them commit before anyone "
            "explains. Then ask a thumbs-up and a thumbs-down to defend "
            "their vote.",
            "There are no perfect answers, and that is the fun. Where most "
            "rooms land: robot vacuum YES (it senses bumps and decides "
            "where to go). TV remote NO (it does what your thumb says). "
            "Automatic door - arguable, it senses and acts but hardly "
            "thinks. Mars rover YES. Remote-control car NO - a person does "
            "the thinking. Calculator - it thinks, but doesn't sense the "
            "world or move anything.",
            "Steer the discussion toward the three verbs on the next "
            "slide: does it SENSE, THINK and ACT by itself?",
        ])

    dx.sense_think_act(deck)

    deck.bullets_image(
        "Robots go where people can't",
        [("In 1997, NASA landed a mission called Mars Pathfinder.", 0),
         ("It carried Sojourner - the first rover ever to drive on "
          "Mars. It was about the size of a microwave oven.", 0),
         ("Its top speed was about 1 centimeter per second. Your vehicle "
          "is MUCH faster.", 0),
         ("Here in San Diego, scientists send robots deep into the ocean "
          "too.", 0)],
        SOJOURNER,
        caption="Sojourner, on Mars",
        image_ratio=0.42,
        note="Robots explore where it is too far, too deep, or too "
             "dangerous for people to go.",
        speaker=[
            "Your vehicle shares its name with that mission. It's a good "
            "story to tell: a robot the size of a microwave, driving very "
            "slowly across another planet, run by a team in Pasadena, "
            "California, a couple of hours north of here.",
            "Sojourner landed on July 4, 1997. One centimeter a second is "
            "about the width of a fingernail every second.",
            "The ocean line is the local hook. Porpoise Robotics builds "
            "robots for the sea, and Scripps Institution of Oceanography "
            "in La Jolla sends robot floats and gliders all over the "
            "world's oceans. Ask who has been to the Birch Aquarium or "
            "the tide pools.",
        ])

    divider(deck, L, "Meet Pathfinder")

    deck.image_slide(
        "Meet Pathfinder",
        img("vehicle-parts-labeled.jpg"),
        caption="Every part, labeled. Find each one on your team's vehicle.",
        speaker=[
            "Give each team two minutes to find every labeled part on "
            "their own vehicle. Hands on is fine; power stays OFF.",
            "Make sure every team finds the POWER SWITCH. Ask every Crew "
            "Chief to put a finger on it. You will ask for it again in "
            "the safety section.",
            "Don't explain the battery voltage or the motor drivers - "
            "that comes later. Names and jobs only.",
        ])

    deck.table(
        "A robot is a bit like a body",
        ["Part", "What it does", "Like your..."],
        [["ESP32 computer", "Runs your program. Makes every decision.",
          "brain"],
         ["4 motors", "Turn the wheels", "muscles"],
         ["32 lights", "Show what the robot is doing", "face"],
         ["Controller and radio", "Bring your commands in", "ears"],
         ["Battery", "Stores energy for everything", "food"],
         ["Frame and wheels", "Hold it together, and roll", "skeleton and feet"]],
        col_widths=[3, 5.5, 2.6],
        speaker=[
            "Ask the room to fill in the last column before you show it - "
            "cover it with your hand, or read the middle column and let "
            "them shout.",
            "The comparison is loose and that's fine. If a student argues "
            "that the lights are more like a voice than a face, they have "
            "understood it perfectly.",
        ])

    deck.image_pair(
        "Built for exploring",
        img("vehicle-gen3-upside-down.jpg"),
        "Flipped over: the wheels stick out past the frame on BOTH sides.",
        img("switch-controller-pair.jpg"),
        "Your controller. Its radio talks to ONE vehicle only.",
        speaker=[
            "Left: a Pathfinder on its back. The wheels are bigger than "
            "the frame on the top and the bottom, so it still touches the "
            "ground with its wheels. Ask why an explorer robot might need "
            "that. (Bumps, rocks, flips.)",
            "Right: the controllers. Each one is locked to one vehicle, "
            "which is the next slide.",
        ])

    deck.bullets_image(
        "One vehicle, one controller",
        [("Every vehicle has a number sticker.", 0),
         ("Its controller has the SAME number.", 0),
         ("The vehicle ignores every other controller in the room.", 0),
         ("Lights blinking GREEN: it's looking for its controller. Press a "
          "button on it.", 0),
         ("Lights blinking RED: tell your teacher.", 0)],
        STICKERS,
        caption="Matching numbers",
        image_ratio=0.40,
        speaker=[
            "Without this, twelve controllers in one room would grab "
            "whichever vehicle answered first. The stickers are how "
            "students keep the pairs together.",
            "Blinking RED means the vehicle has never been told which "
            "controller is its own. That is an instructor job: run "
            "e00_claim_controller on it (the README walks you through "
            "it). It takes two minutes.",
            "If a controller seems dead: check it's charged, and that it "
            "isn't the neighbor's. Swapped controllers are the most common "
            "'broken vehicle' of the whole course.",
        ])

    divider(deck, L, "The Explorer Code")

    deck.table(
        "Your team, and your jobs",
        ["Job", "What you do"],
        [["Pilot", "Holds the controller and drives. Runs the tests."],
         ["Coder", "Sits at the laptop. Types the changes and uploads."],
         ["Crew Chief", "In charge of the vehicle: the stand, the power "
          "switch, the cable. Reads the mission steps out loud. Calls "
          "STOP if anything looks unsafe."]],
        col_widths=[2.2, 9],
        note="Switch jobs EVERY lesson. Everybody does every job.",
        speaker=[
            "Assign today's jobs now, out loud, team by team. Write them "
            "on the board so you can rotate them fairly next lesson.",
            "Teams of two: one person is Crew Chief AND Pilot. Teams of "
            "four: add a Navigator who reads the steps and keeps the "
            "Mission Log.",
            "Crew Chief is the most important job, not the least. Give it "
            "to your most careful student first, so the others see how "
            "it's done.",
        ])

    deck.bullets(
        "The Explorer Code",
        [("1.  WHEELS UP whenever the cable is plugged in. The vehicle sits "
          "on its stand.", 0),
         ("2.  Hands, hair and sleeves away from the wheels while the power "
          "is on.", 0),
         ("3.  Power OFF before you pick it up. Carry it by the bottom "
          "plate, never by the wires.", 0),
         ("4.  Drive on the floor, inside the arena. Never on a table.", 0),
         ("5.  Batteries and chargers are for grown-ups only.", 0),
         ("6.  A puffy, hot or damaged battery - or a burning smell? Power "
          "OFF, step back, tell a grown-up.", 0),
         ("7.  Don't stare into the lights.", 0)],
        note="The Crew Chief can call STOP at any time, and everybody stops. "
             "No arguing.",
        note_kind="safety",
        speaker=[
            "Slow down. This is the slide that keeps everybody safe for "
            "twelve weeks.",
            "Ask WHY for at least three rules and let students answer. "
            "Rules they can explain are rules they keep. (1: a vehicle on "
            "a table can drive itself off it. 3: the wires are the "
            "weakest part. 5: the battery is a 16-volt lithium pack with "
            "a lot of energy in it.)",
            "Rule 5 is absolute. Students never charge, unplug, or swap a "
            "battery. If you have a puffy pack, show it - kids remember "
            "what they've seen far better than what they've heard.",
            "Have every student sign the Explorer Code page in the Mission "
            "Log before anyone drives.",
        ])

    deck.image_pair(
        "Wheels up, and the power switch",
        WHEELS_UP, "Wheels up = all four wheels in the air",
        POWER_SWITCH, "Crew Chiefs: know where this is",
        speaker=[
            "Show a real vehicle on a real stand. Anything works: a wooden "
            "block, a small sturdy box, a stack of books. All four wheels "
            "must be clear.",
            "Crew Chiefs point at their power switch. Every one of them, "
            "before you move on.",
        ])

    divider(deck, L, "First drive")

    dx.license_course(deck, title="Today's driving course")

    deck.image_slide(
        "The arena",
        ARENA,
        caption="Drive inside the tape. Wait your turn behind the line.",
        speaker=[
            "If your room is small, run the arena in shifts: two or three "
            "vehicles at a time, and the waiting teams label the parts "
            "page in their Mission Log. Everybody gets a turn.",
            "Tell them where to stand while they wait - behind the tape, "
            "not in the arena. Feet in the arena are the main hazard.",
        ])

    mission(
        deck, L, "0:30", "First drive", "e01a_learner_drive",
        [("Crew Chief: vehicle on the floor at START. Power ON. The lights "
          "blink GREEN.", 0),
         ("Pilot: press any button on YOUR controller. The blinking stops - "
          "you're connected!", 0),
         ("Weave through the cups. Park in the garage. Back out. Drive "
          "home.", 0),
         ("Switch jobs. Everybody drives at least twice.", 0),
         ("LEVEL 2: hold A. Tap Y. Tap X. What happens?", 0)],
        expect=[("White headlights at the front", 0),
                ("Red brake lights when you stop", 0),
                ("Orange blinking on the side you turn toward", 0)],
        questions=[("Which way do you push to turn right?", 0),
                   ("What do the back lights do when you back up?", 0)],
        safety="If anything goes wrong, LET GO of the stick. The vehicle "
               "stops.",
        speaker=[
            "Your teacher has already uploaded e01a_learner_drive to every "
            "vehicle, so there's no laptop in this activity.",
            "Watch the first lap of every team closely. Kids push the "
            "stick all the way at first. The learner speed limit keeps it "
            "manageable; praise smooth driving loudly.",
            "Answers: the stick goes right to turn right, and the vehicle "
            "turns on the spot if you aren't also pushing forward. The "
            "back lights turn white when reversing - like a car's backup "
            "lights. A flashes the headlights, Y switches the lights off "
            "and on, X is party mode.",
            "If a vehicle won't connect: is it the right controller? Is "
            "the controller charged? Is the vehicle blinking green (good) "
            "or red (needs e00)?",
        ])

    deck.code(
        "The one line to look at today",
        ["// ===== SETTINGS =====",
         "const int SPEED_LIMIT = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100",
         "const int WOBBLE_ZONE = 10;   // Stick numbers smaller than this count as zero"],
        filename="e01a_learner_drive.ino, line {}".format(
            line("e01a_learner_drive", "const int SPEED_LIMIT")),
        speaker=[
            "Say it plainly: this number is why your vehicle drives gently "
            "today. It's a percent - at {} a full push on the stick gives "
            "half power. Pass the driving test, and you get to raise "
            "it.".format(FACTS["learner"]),
            "Open e01a_learner_drive on your own laptop and project it. "
            "Don't let anybody change it yet - today is look, don't touch.",
            "This is the hook for the whole course: there's a number in "
            "the program, and changing it changes the robot. By Lesson 9 "
            "they'll change it themselves.",
        ])

    license_table(deck)

    deck.quiz(
        "Check yourself",
        [("1.  What are the three things every robot does?", 0),
         ("2.  Which part of Pathfinder is its brain?", 0),
         ("3.  When must the vehicle be WHEELS UP?", 0),
         ("4.  Who is allowed to touch the batteries?", 0),
         ("5.  Something smells hot. What do you do?", 0)],
        lead="Talk it over with your team, then we'll share.",
        speaker=[
            "Answers: 1 sense, think, act. 2 the ESP32 computer. 3 whenever "
            "the cable is plugged in. 4 grown-ups only. 5 power OFF, step "
            "back, tell a grown-up.",
            "Questions 3 to 5 are the ones that matter. Anybody who "
            "can't answer them doesn't get a Learner Permit stamp yet - "
            "say that kindly and go over it again.",
        ])

    wrap_up(deck,
            ["Write your team name and draw your vehicle",
             "Label the parts",
             "Sign the Explorer Code"],
            "Learner Permit",
            speaker=[
                "Stamp the Learner Permit for every student who answered "
                "the safety questions. It's the first page they'll show "
                "their families.",
                "Next lesson they upload their first program, so laptops "
                "need the board package and the course sketches by then. "
                "See the README.",
                "Do not let anyone leave until their Crew Chief has shown "
                "you a switched-off vehicle.",
            ])


# ===================================================================
# LESSON 2 - TALKING TO ROBOTS
# ===================================================================

def lesson2(deck):
    L = "lesson2"
    title(deck,
          "A program is a list of instructions. Today you write your first "
          "one, send it to your vehicle, and hunt down three bugs.",
          img("vehicle-and-laptop.jpg"),
          speaker=[
              "BEFORE CLASS: every laptop has the Arduino IDE, the "
              "esp32_bluepad32 board package, Adafruit NeoPixel, and the "
              "explorers sketch folder in its sketchbook. The board is "
              "already set to ESP32 Dev Module. Students should not be "
              "installing anything today.",
              "Have one good USB data cable per team, plus two spares in "
              "your pocket. A charge-only cable looks identical and is the "
              "number one reason an upload 'doesn't work'.",
          ])

    todays_mission(deck, [
        "Give instructions so exact that a robot can follow them",
        "Name the two parts of every program, and say when each one runs",
        "Upload a program to your vehicle, and change it",
        "Read an error message and fix the bug it points to",
    ], speaker=[
        "Read them out. The third one is the big moment - their first "
        "upload. Build it up.",
    ])

    deck.bullets(
        "Last time",
        [("You met Pathfinder and drove it.", 0),
         ("A robot SENSES, THINKS and ACTS.", 0),
         ("Today: the THINK part. Who tells the robot what to think?", 0),
         ("You do.", 1),
         ("Switch jobs! Last lesson's Pilot is today's Coder.", 0)],
        speaker=[
            "Ask for the three robot verbs before you show the slide.",
            "Rotate the jobs now, before anybody sits down at a laptop. "
            "Whoever was Pilot last time is the Coder today.",
        ])

    agenda(deck, L)

    divider(deck, L, "Instructions")

    unplugged(
        deck, L, "0:05", "Program a Human Robot", "Instructions on a card",
        [("One person is the ROBOT. Everybody else on the team is a "
          "PROGRAMMER.", 0),
         ("Mission: walk the robot from the door to the teacher's desk, and "
          "PICK UP the marker.", 0),
         ("Write the program on a card using ONLY these words:", 0),
         ("STEP FORWARD (number)   TURN LEFT   TURN RIGHT   PICK UP", 1),
         ("The robot does EXACTLY what the card says. Even if it's "
          "wrong!", 0),
         ("Didn't work? Fix the card and run it again.", 0)],
        expect=[("The robot walks into a chair, turns the wrong way, or "
                 "stops short", 0)],
        questions=[("What went wrong the first time?", 0),
                   ("Why can't the robot just figure it out?", 0)],
        safety="Robots WALK, slowly. Programmers: no instructions that walk "
               "your robot into anything.",
        speaker=[
            "Play the robot yourself for the first team, and be "
            "ruthlessly literal. 'Step forward' with no number? Stand "
            "still and say 'how many?' 'Turn' without left or right? "
            "Freeze. Kids love catching you out, and it lands the point.",
            "Then let teams run it with a student robot. Two runs each is "
            "plenty.",
            "Answers: computers can't guess what you meant - they only "
            "have what you wrote. That's both the frustrating part and "
            "the reliable part.",
        ])

    deck.bullets(
        "What the human robot taught us",
        [("A PROGRAM is a list of instructions.", 0),
         ("The computer follows them in order, from top to bottom.", 0),
         ("It does EXACTLY what you said - not what you meant.", 0),
         ("A mistake in a program is called a BUG. Finding and fixing it is "
          "called DEBUGGING.", 0)],
        note="In 1947, engineers working with Grace Hopper found a real moth "
             "stuck inside their computer - and taped it into their "
             "notebook.",
        speaker=[
            "Pull the lessons out of the game in their own words before you "
            "show the bullets.",
            "The moth story is true: September 9, 1947, on a computer "
            "called the Harvard Mark II. The notebook page is in the "
            "Smithsonian. People had called machine faults 'bugs' before "
            "that, but the moth made the word famous. Grace Hopper went "
            "on to become a rear admiral in the Navy and a computing "
            "pioneer.",
        ])

    divider(deck, L, "Your first upload")

    deck.image_slide(
        "The Arduino IDE",
        IDE_BUTTONS,
        caption="Where you write programs, and send them to the vehicle",
        items=[("See the second tab called explorer.h? That's the ENGINE "
                "ROOM. It does the hard work. You don't need to change it.",
                0)],
        speaker=[
            "Project your own screen and point at each button as you name "
            "it on the next slide.",
            "IDE stands for Integrated Development Environment - a long "
            "name for 'the program you write programs in'. Only tell the "
            "older ones; nobody needs it.",
            "The explorer.h tab is in every Explorers sketch. It holds "
            "the code that talks to the motors, lights and controller, so "
            "the students' own programs stay short. Curious students are "
            "welcome to read it.",
        ])

    deck.table(
        "The buttons you'll use",
        ["Button", "What it does"],
        [["Check mark: Verify",
          "Checks your program for mistakes. Sends nothing."],
         ["Arrow: Upload",
          "Checks it, then sends it down the cable."],
         ["Magnifying glass: Serial Monitor",
          "A window where the vehicle types messages to you."],
         ["The explorer.h tab",
          "The engine room. Read it. Don't change it."]],
        col_widths=[4.2, 7],
        speaker=[
            "If your IDE is set up with the toolbar at the top-left (the "
            "default in Arduino IDE 2), Verify and Upload are the first "
            "two round buttons, and the Serial Monitor is the magnifying "
            "glass at the top right.",
        ])

    dx.setup_and_loop(deck)

    deck.code(
        "Your first program",
        ["#include \"explorer.h\"",
         "",
         "void setup() {",
         "  // This happens ONCE",
         "  startRover();",
         "  setLight(0, RED);",
         "}",
         "",
         "void loop() {",
         "  // This happens OVER AND OVER",
         "  Serial.println(\"Hello! I am a Pathfinder rover.\");",
         "  delay(2000);     // Wait 2 seconds",
         "}"],
        filename="e02a_hello_rover.ino",
        notes=[("#include brings in the engine room.", 0),
               ("setLight(0, RED) turns light 0 red.", 0),
               ("Serial.println() types a message to your screen.", 0),
               ("delay(2000) waits 2 seconds.", 0),
               ("Every instruction ends with a semicolon  ;", 0)],
        speaker=[
            "Read it top to bottom like a story, the way the robot does.",
            "Lines starting with // are COMMENTS - notes for people. The "
            "computer skips them. Ask somebody to find one.",
            "Point out the semicolons now. They'll meet a missing one in "
            "twenty minutes.",
        ])

    deck.bullets(
        "How to upload",
        [("1.  Crew Chief: vehicle on its stand, WHEELS UP.", 0),
         ("2.  Plug the cable into the vehicle and the laptop.", 0),
         ("3.  Crew Chief: vehicle power ON.", 0),
         ("4.  Coder: File > Sketchbook > explorers > e02a_hello_rover.", 0),
         ("5.  Click the arrow (Upload). Wait for \"Done uploading\".", 0)],
        note="Stuck on \"Connecting...\"? Ask your teacher. It's usually "
             "the cable.",
        note_kind="warn",
        speaker=[
            "The board and port should already be chosen on every laptop. "
            "If a laptop shows no port, check in this order: is the "
            "vehicle switched on? Is it a data cable, not a charge-only "
            "one? Then try another USB socket.",
            "If an upload sits on 'Connecting.....' and times out, hold the "
            "ESP32's BOOT button until the percentage starts climbing. Some "
            "boards need it every time; most never do.",
            "A first upload takes the better part of a minute because it "
            "compiles the Bluetooth library. Warn them, or they'll click "
            "Upload three more times.",
        ])

    mission(
        deck, L, "0:25", "Your first upload", "e02a_hello_rover",
        [("Upload it. Light 0 turns RED.", 0),
         ("Open the Serial Monitor. Choose 115200 baud. Read the "
          "message.", 0),
         ("LEVEL 1: change 0 to 5 in setLight(0, RED) (line {}). Upload. "
          "Which light is it now?".format(
              line("e02a_hello_rover", "setLight(0, RED)")), 0),
         ("LEVEL 2: change RED to BLUE, GREEN, PURPLE or PINK. Make the "
          "message say your team's name.", 0),
         ("LEVEL 3: make the light BLINK.", 0)],
        expect=[("A red light at the front left", 0),
                ("\"Hello! I am a Pathfinder rover.\" every 2 seconds", 0)],
        questions=[("Why does the message keep coming, but the light only "
                    "turns on once?", 0)],
        safety="Wheels up for every upload - even when you're only changing "
               "a light.",
        speaker=[
            "Walk the room during the first upload. Expect at least one "
            "port problem and one cable problem.",
            "Answer: setLight is in setup(), which runs once. "
            "Serial.println is in loop(), which runs forever.",
            "Level 3 answer, for you: move setLight into loop() and add "
            "the off half - setLight(0, RED); delay(500); setLight(0, "
            "OFF); delay(500); - inside loop(). Let them struggle a "
            "little first. A good hint is: 'what four things happen in a "
            "blink?'",
            "Typos are fine. They're about to learn how to fix them.",
        ])

    deck.image_slide(
        "What the Serial Monitor shows",
        SERIAL_HELLO,
        caption="Set the speed in the corner to 115200 baud, or you'll see "
                "nonsense",
        speaker=[
            "If students see gibberish, the baud rate is wrong - it has to "
            "match the 115200 in the engine room.",
            "If they see nothing at all, they may have opened the monitor "
            "on the wrong port, or the program isn't running - press the "
            "reset (EN) button or switch the vehicle off and on.",
        ])

    divider(deck, L, "Bug hunt")

    deck.bullets_image(
        "Bugs are clues",
        [("Everybody's programs have bugs. Even NASA's.", 0),
         ("Click Verify and the computer checks your program.", 0),
         ("Something wrong? A message appears at the bottom of the "
          "window.", 0),
         ("It tells you the LINE NUMBER. Go there first.", 0),
         ("It often guesses what you meant!", 0)],
        BUG_SCREEN,
        caption="An error message, and its line number",
        image_ratio=0.46,
        note="Fix the FIRST message first. Then click Verify again.",
        speaker=[
            "The real skill here is not panicking at a wall of orange "
            "text. Model it: read the first line out loud, slowly, find "
            "the line number, go there.",
            "An error often points at the line AFTER the real mistake - "
            "especially a missing semicolon, which the computer only "
            "notices when the next line starts. Tell them to look at the "
            "line it names AND the line above.",
        ])

    deck.code(
        "Three bugs. Can you spot them?",
        ["void setup() {",
         "  startRover();",
         "  setLight(0, GREEN)",
         "  setlight(1, YELLOW);",
         "  setLight(2, BLEU);",
         "}"],
        filename="e02b_bug_hunt.ino",
        notes=[("This program will not upload until all three are "
                "fixed.", 0),
               ("Find them by eye first.", 0),
               ("Then let Verify find them for you.", 0)],
        speaker=[
            "Give them thirty seconds of silent spotting, then ask for "
            "hands. Don't confirm or deny yet - the IDE is about to.",
            "The bugs: a missing semicolon after setLight(0, GREEN); "
            "setlight with a small L; BLEU spelled wrong.",
        ])

    deck.table(
        "Decoding the messages",
        ["The message says...", "It means..."],
        [["expected ';' before 'setlight'",
          "A semicolon is missing just BEFORE this word - at the end of the "
          "line above"],
         ["'setlight' was not declared in this scope",
          "The computer doesn't know this word. The next line guesses: "
          "suggested alternative: 'setLight'"],
         ["'BLEU' was not declared in this scope",
          "It doesn't know this word either. It suggests 'BLUE'."]],
        col_widths=[4.6, 6.4],
        note="One bug can hide another. Fix the semicolon, and the setlight "
             "bug appears.",
        speaker=[
            "These are the real messages from the compiler, word for word. "
            "On the first Verify only two appear - the missing semicolon "
            "confuses the computer so much that it doesn't notice "
            "setlight until the semicolon is fixed.",
            "'Not declared in this scope' sounds scary. Translate it every "
            "time: 'I don't know that word.'",
        ])

    mission(
        deck, L, "0:40", "Bug hunt", "e02b_bug_hunt",
        [("Open e02b_bug_hunt.", 0),
         ("Click Verify (the check mark). Read the FIRST message.", 0),
         ("Go to the line it names. Fix that ONE bug.", 0),
         ("Verify again. Repeat until it says \"Done compiling\".", 0),
         ("Upload it. Lights 0, 1 and 2 come on.", 0)],
        expect=[("Green, yellow and blue lights at the front left", 0)],
        questions=[("Which bug was hardest to find?", 0),
                   ("Why did one bug hide until you fixed another?", 0)],
        speaker=[
            "Resist fixing it for them. Ask: 'what line does it say? What's "
            "on that line? What's on the line above?'",
            "Fast teams: have them break e02a on purpose - delete a "
            "semicolon, misspell a color - and swap laptops with another "
            "team to fix each other's bugs. Kids love this.",
        ])

    new_words(deck, [
        ["Program", "A list of instructions for a computer"],
        ["Upload", "Send your program down the cable to the vehicle"],
        ["setup()", "The part of a program that runs ONCE"],
        ["loop()", "The part that runs OVER AND OVER"],
        ["Bug", "A mistake in a program"],
        ["Debugging", "Finding and fixing bugs"],
        ["Comment", "A note for people, after //. The computer skips it."],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  Your instructions have a mistake. What does the robot "
          "do?", 0),
         ("2.  How many times does setup() run? And loop()?", 0),
         ("3.  What does delay(1000) do?", 0),
         ("4.  What's a bug? What's debugging?", 0),
         ("5.  The message says expected ';'. What do you check?", 0)],
        speaker=[
            "Answers: 1 exactly what you wrote, mistake and all. 2 once; "
            "forever. 3 waits one second. 4 a mistake; finding and fixing "
            "it. 5 the end of the line it names, and the line above.",
        ])

    wrap_up(deck,
            ["Write the three bugs you found, and how you fixed each one",
             "Fill in the two parts of every program"],
            "Bug Hunter",
            speaker=[
                "Have each team read one bug and its fix out loud. It "
                "rehearses the vocabulary.",
                "Remind Coders that the program on the vehicle stays there "
                "when it's switched off. Next lesson's program replaces "
                "it.",
            ])


# ===================================================================
# LESSON 3 - COLOR LAB
# ===================================================================

def lesson3(deck):
    L = "lesson3"
    title(deck,
          "Every color on a screen is made from just three: red, green and "
          "blue. Today you mix your own, and map every light on the "
          "vehicle.",
          img("vehicle-green-leds.jpg"),
          speaker=[
              "A dim room helps today. If you can, turn off half the lights "
              "for the color lab - colors on the LEDs are much easier to "
              "judge.",
              "Have the color prediction page of the Mission Log ready to "
              "go. Predicting BEFORE uploading is the whole point of the "
              "first activity.",
          ])

    todays_mission(deck, [
        "Predict the color you get when red, green and blue light mix",
        "Mix any color you like with three numbers from 0 to 255",
        "Paint your team color on your vehicle",
        "Find any light on the vehicle by its number",
    ], speaker=["Read them out. Ask who already knows what red and green "
                "light make together. Don't tell them yet."])

    deck.bullets(
        "Last time",
        [("You uploaded your first program, and fixed three bugs.", 0),
         ("setLight(0, RED) turned light 0 red.", 0),
         ("But what IS red, to a computer? It's a number. Three numbers, "
          "actually.", 0),
         ("Switch jobs!", 0)],
        speaker=[
            "Ask a Coder from last lesson to explain what setup() and "
            "loop() do. Then rotate the jobs.",
        ])

    agenda(deck, L)

    divider(deck, L, "Mixing light")

    deck.bullets_image(
        "Each light is really THREE lights",
        [("Look closely at a light on your vehicle while it's OFF.", 0),
         ("Inside are three tiny lights: one red, one green, one blue.", 0),
         ("Turn them up and down, and your eye blends them into one "
          "color.", 0),
         ("Your TV, tablet and phone screens work the same way, with "
          "millions of tiny red, green and blue dots.", 0)],
        img("ws2812b-stick-8.jpg"),
        caption="Eight of these lights on a strip. Yours has 32.",
        image_ratio=0.40,
        speaker=[
            "If you have a magnifying glass, pass it round with a vehicle - "
            "switched off - so they can see the three tiny chips inside one "
            "light.",
            "A magnifying glass on a phone or tablet screen showing white "
            "is an even better demo: they'll see the red, green and blue "
            "dots. Kids are amazed by this every time.",
        ])

    deck.bullets_image(
        "Light doesn't mix like paint",
        [("Mix red and green PAINT: you get a muddy brown.", 0),
         ("Mix red and green LIGHT: you get YELLOW!", 0),
         ("All three lights together make WHITE.", 0),
         ("Light ADDS together. That's why it gets brighter as you "
          "mix.", 0)],
        img("rgb-additive-mixing.png"),
        caption="Red, green and blue light, overlapping",
        image_ratio=0.40,
        speaker=[
            "Red plus green making yellow is the moment of surprise. Let "
            "it land before you explain.",
            "Why it happens, for the curious: your eye has three kinds of "
            "color sensor, roughly tuned to red, green and blue. Light "
            "that tickles the red and green sensors together looks yellow "
            "to your brain, even with no yellow light in it at all.",
            "Paint works the opposite way: each paint SOAKS UP some colors, "
            "so mixing paints takes more colors away and gets darker.",
        ])

    deck.table(
        "mixColor(red, green, blue)  -  predict!",
        ["Red", "Green", "Blue", "You get..."],
        [["255", "0", "0", "red"],
         ["0", "0", "255", "blue"],
         ["255", "255", "0", "?"],
         ["0", "255", "255", "?"],
         ["255", "0", "255", "?"],
         ["255", "255", "255", "?"]],
        col_widths=[2, 2, 2, 4],
        lead="Each number goes from 0 (none of that color) to 255 (all of "
             "it). Write your guesses in your Mission Log.",
        speaker=[
            "Don't reveal the question marks. They test them on the "
            "vehicle in a few minutes.",
            "Answers, for you: yellow, cyan (a light blue-green), magenta "
            "(a hot pink-purple), white.",
            "Why 255? Each number is stored in eight bits, which can count "
            "from 0 to 255 - that's 256 steps. Only worth saying to older "
            "students who ask.",
        ])

    deck.bullets(
        "The numbers are volume knobs",
        [("Each number turns ONE color up or down.", 0),
         ("0 means that color is off. 255 means all the way up.", 0),
         ("mixColor(255, 100, 0) = lots of red, a little green, no blue = "
          "ORANGE.", 0),
         ("Three knobs with 256 steps each makes over 16 million "
          "colors!", 0)],
        note="RED, ORANGE, PURPLE and the other names are just mixes the "
             "engine room made for you.",
        speaker=[
            "16 million: 256 x 256 x 256 = 16,777,216. Older students may "
            "enjoy checking it on a calculator.",
            "The next slide shows the actual lines in explorer.h where the "
            "named colors are mixed.",
        ])

    deck.code(
        "The named colors are just mixes",
        ["const Color RED    = mixColor(255,   0,   0);",
         "const Color ORANGE = mixColor(255,  90,   0);",
         "const Color YELLOW = mixColor(255, 190,   0);",
         "const Color GREEN  = mixColor(  0, 255,   0);",
         "const Color CYAN   = mixColor(  0, 255, 255);",
         "const Color BLUE   = mixColor(  0,   0, 255);",
         "const Color PURPLE = mixColor(140,   0, 255);",
         "const Color PINK   = mixColor(255,  30, 120);",
         "const Color WHITE  = mixColor(255, 255, 255);",
         "const Color OFF    = mixColor(  0,   0,   0);"],
        filename="explorer.h  -  the engine room",
        notes=[("These lines are in the explorer.h tab.", 0),
               ("Every name is three numbers.", 0),
               ("YELLOW isn't (255, 255, 0). On these lights, a bit less "
                "green looks MORE yellow.", 0)],
        speaker=[
            "Have them click the explorer.h tab and find these lines. It "
            "demystifies the engine room a little.",
            "The yellow point is real engineering: the green chip in "
            "these lights is very bright, so full green swamps the red "
            "and looks lime. Somebody tuned that number by eye.",
            "Remind them: look, but don't change explorer.h.",
        ])

    divider(deck, L, "The color lab")

    deck.code(
        "The color lab",
        ["Color myColor   = mixColor(255, 0, 255);   // Red + blue = ?",
         "Color teamColor = mixColor(0, 0, 0);       // LEVEL 2",
         "",
         "void setup() {",
         "  startRover();",
         "",
         "  setLight(0, mixColor(255, 0, 0));       // Red only",
         "  setLight(1, mixColor(0, 255, 0));       // Green only",
         "  setLight(2, mixColor(0, 0, 255));       // Blue only",
         "  setLight(3, mixColor(255, 255, 0));     // Red + green = ?",
         "  setLight(4, mixColor(0, 255, 255));     // Green + blue = ?",
         "  setLight(5, mixColor(255, 255, 255));   // All three = ?",
         "  setLight(6, myColor);",
         "",
         "  // setBackLights(teamColor);",
         "}"],
        filename="e03a_color_lab.ino",
        speaker=[
            "Point out that myColor and teamColor are VARIABLES: names that "
            "hold a value. Make a color once, give it a name, use the name "
            "anywhere.",
            "The // in front of setBackLights turns that line into a "
            "comment, so the computer skips it. Removing the // brings it "
            "to life. That's how they'll switch things on all course.",
        ])

    mission(
        deck, L, "0:15", "The color lab", "e03a_color_lab",
        [("BEFORE you upload: write your guesses for lights 3, 4, 5 and 6 in "
          "your Mission Log.", 0),
         ("Upload. Check your guesses. Were you right?", 0),
         ("LEVEL 2: mix your TEAM COLOR in teamColor (line {}). Remove the "
          "// in front of setBackLights. Upload.".format(
              line("e03a_color_lab", "Color teamColor")), 0),
         ("LEVEL 3: paint a flag or a rainbow across the front, lights 0 to "
          "15. Try setLights(0, 4, RED);", 0)],
        expect=[("Seven colors, on lights 0 to 6", 0)],
        questions=[("What did red + green make?", 0),
                   ("What three numbers make your team color?", 0)],
        safety="Look at the lights from the side, not straight into them.",
        speaker=[
            "Hold the line on predictions. A team that uploads first and "
            "'predicts' afterwards has skipped the science.",
            "Common snag: a team color that looks too dark. Light can't "
            "make dark colors - (100, 50, 0) just looks like dim orange, "
            "not brown. That's a great discovery, not a failure.",
            "Level 3: setLights(first, last, color) paints a run of "
            "lights. Three stripes: setLights(0, 4, RED); setLights(5, 10, "
            "WHITE); setLights(11, 15, BLUE);",
        ])

    deck.image_slide(
        "What it looks like",
        COLOR_LIGHTS,
        caption="Lights 0 to 6, running e03a_color_lab",
        speaker=[
            "Use this to settle arguments about what a color 'really' is - "
            "or skip it and let them judge their own vehicles.",
        ])

    deck.bullets(
        "Mixing tips",
        [("Orange: lots of red, a little green.  (255, 90, 0)", 0),
         ("Purple: red and blue, more blue.  (140, 0, 255)", 0),
         ("Pastels: add some of all three.  Light pink is (255, 120, "
          "150)", 0),
         ("Brown and black are impossible! Lights can only ADD. Dark just "
          "looks dim.", 0),
         ("Too bright? setBrightness(30) dims every light at once.", 0)],
        speaker=[
            "Put this up while teams work, as a help sheet.",
            "setBrightness takes a percent, 0 to 100. The engine room caps "
            "100% well below what the LEDs can do, to protect eyes and the "
            "battery.",
        ])

    divider(deck, L, "The light map")

    dx.light_map(deck)

    mission(
        deck, L, "0:35", "Map every light", "e03b_light_map",
        [("Upload e03b_light_map. Open the Serial Monitor.", 0),
         ("Watch the vehicle AND the numbers on the screen.", 0),
         ("Fill in the light map in your Mission Log.", 0),
         ("Where are 0, 15, 16 and 31? Point to each on the vehicle.", 0),
         ("Too fast? Make WAIT bigger (line {}).".format(
             line("e03b_light_map", "const int WAIT")), 0)],
        expect=[("One white light at a time, going all the way round", 0)],
        questions=[("Which two lights are at the LEFT end?", 0),
                   ("Which back light is right behind front light 5?", 0)],
        speaker=[
            "This program uses a for loop to count from 0 to 31. They "
            "don't need to understand it yet - it's next lesson's star. "
            "Tell them to notice it, and come back to it.",
            "Answers: 0 and 31 are both at the left end. Behind front "
            "light 5 is back light 26 (31 minus 5).",
            "The map they draw here gets used for the rest of the course. "
            "Check every team's before they pack up.",
        ])

    new_words(deck, [
        ["RGB", "Red, green and blue: the three colors of light"],
        ["mixColor()", "Makes a color from three numbers, 0 to 255"],
        ["Variable", "A name that holds a value, like teamColor"],
        ["Comment out", "Put // in front of a line so the computer skips "
         "it"],
        ["Light map", "Where each light number is on the vehicle"],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  What three colors of light make every color on a "
          "screen?", 0),
         ("2.  Red light + green light = ?", 0),
         ("3.  What does mixColor(0, 0, 255) make?", 0),
         ("4.  What number turns a color all the way up?", 0),
         ("5.  Where is light 16 on your vehicle?", 0)],
        speaker=[
            "Answers: 1 red, green, blue. 2 yellow. 3 blue. 4 255. 5 the "
            "back, at the right-hand end - right behind light 15.",
        ])

    wrap_up(deck,
            ["Check your color predictions - right or wrong?",
             "Write your team color recipe: (red, green, blue)",
             "Finish the light map"],
            "Color Scientist",
            speaker=[
                "The light map has to be finished before they leave - "
                "Lesson 4 depends on it.",
                "Teams who were wrong about yellow should write that down "
                "proudly. Being surprised by an experiment is what science "
                "is for.",
            ])


# ===================================================================
# LESSON 4 - LOOPS AND LIGHT SHOWS
# ===================================================================

def lesson4(deck):
    L = "lesson4"
    title(deck,
          "Why write 32 lines when one loop can do it? Today the computer "
          "does the counting, and you design a light show.",
          img("vehicle-turn-signal-right.jpg"),
          speaker=[
              "For the Human Light Strip you need eight sheets of colored "
              "paper (or eight sheets with a big number on each). Clear a "
              "strip at the front of the room.",
              "The design grid page in the Mission Log is used in the "
              "last part of the lesson.",
          ])

    todays_mission(deck, [
        "Say what a loop does, and why programmers use them",
        "Read a for loop: where it starts, when it stops, and how it "
        "counts",
        "Change a light show's colors, speed and direction",
        "Design a light pattern on paper, then build it",
    ], speaker=["Read them out. Then ask: who has watched a light show? "
                "Holiday lights, a concert, a theme park ride?"])

    deck.bullets(
        "Last time",
        [("You mixed colors, and mapped all 32 lights.", 0),
         ("Here's a puzzle: how would you turn ALL 32 lights blue, one at a "
          "time?", 0),
         ("32 lines of setLight? There's a better way.", 0),
         ("Switch jobs!", 0)],
        speaker=[
            "Let somebody suggest 32 lines of setLight. Agree that it would "
            "work - and ask how long it would take to change the color to "
            "green afterwards. (32 changes.)",
        ])

    agenda(deck, L)

    divider(deck, L, "Repeat after me")

    unplugged(
        deck, L, "0:05", "The Human Light Strip", "Eight volunteers",
        [("Eight volunteers stand in a row, facing the class. Number "
          "yourselves 0 to 7.", 0),
         ("Hold your colored paper DOWN. You're OFF.", 0),
         ("The teacher is the computer. When your number is called, hold it "
          "UP. You're ON.", 0),
         ("ROUND 1 - CHASE: everybody down, one number up, clap. Next "
          "number!", 0),
         ("ROUND 2 - WIPE: the same, but nobody goes back down.", 0),
         ("ROUND 3 - BACKWARD: start at 7 and count down.", 0)],
        expect=[("A light that runs along the row", 0),
                ("...then a row that fills up", 0)],
        questions=[("What changed between round 1 and round 2?", 0),
                   ("What did the 'computer' do over and over?", 0)],
        speaker=[
            "Narrate it like code, out loud, every time: 'number is 0. "
            "Everybody down. Number 0 up. Clap. Add one: number is 1. "
            "Everybody down...' Speed up as they get it.",
            "Round 2 drops the 'everybody down' step. That's exactly the "
            "experiment they'll do on the vehicle later - removing "
            "lightsOff() from the chase.",
            "Answers: in round 2 we skipped 'everybody down'. The computer "
            "repeated the same four steps with a different number each "
            "time - that's a loop.",
        ])

    deck.bullets(
        "What's a loop?",
        [("A LOOP repeats instructions without you writing them again.", 0),
         ("You already know one: loop() repeats forever.", 0),
         ("A FOR loop repeats a set number of times - and counts as it "
          "goes.", 0),
         ("Computers are amazing at counting. Let them do the boring "
          "part.", 0)],
        note="If you're copying the same line over and over, you probably "
             "want a loop.",
        speaker=[
            "Programmers have a saying: be lazy in a smart way. A loop is "
            "the smart kind of lazy.",
        ])

    dx.for_loop_parts(deck)

    deck.code(
        "The chase, as a loop",
        ["// One light runs all the way around the vehicle.",
         "void chase(Color color) {",
         "  for (int number = 0; number < 32; number++) {",
         "    lightsOff();",
         "    setLight(number, color);",
         "    delay(SPEED);",
         "  }",
         "}"],
        filename="e04a_light_show.ino",
        highlight={2},
        notes=[("The loop counts 0, 1, 2 ... 31.", 0),
               ("Each time round: all off, ONE light on, wait.", 0),
               ("Fast enough, and the light seems to MOVE.", 0),
               ("That's how cartoons work: still pictures, shown fast.", 0)],
        speaker=[
            "Map it back to the Human Light Strip: lightsOff() is 'everybody "
            "down', setLight is 'your number up', delay is the clap.",
            "The braces { } hold everything that repeats. Ask what's "
            "inside the loop's braces. (Three lines.)",
            "Ignore 'void chase(Color color)' for now - it means 'here's a "
            "pattern called chase, and you tell it which color'. Functions "
            "get their own lesson in Lesson 7.",
        ])

    divider(deck, L, "The light show")

    deck.code(
        "The whole show",
        ["void loop() {",
         "  chase(BLUE);",
         "  chase(GREEN);",
         "  policeLights(6);",
         "  scanner(RED, 3);",
         "}"],
        filename="e04a_light_show.ino",
        notes=[("Each line plays one pattern.", 0),
               ("The words in the ( ) tell the pattern which color, or how "
                "many times.", 0),
               ("Change the order, or add lines, to make it your show.", 0)],
        speaker=[
            "This is the easiest place to start changing things, so it's "
            "Level 1.",
            "policeLights(6) flashes six times; scanner(RED, 3) sweeps "
            "three times in red.",
        ])

    mission(
        deck, L, "0:22", "The light show", "e04a_light_show",
        [("Upload e04a_light_show. Watch the whole show once.", 0),
         ("LEVEL 1: in loop(), change the colors. Change the order. Play "
          "one pattern twice.", 0),
         ("Change SPEED (line {}). Try 20. Try 200.".format(
             line("e04a_light_show", "const int SPEED")), 0),
         ("LEVEL 2: in chase(), delete  lightsOff();  and upload. What "
          "happens? Why?", 0),
         ("Then make the chase run BACKWARD, from 31 down to 0.", 0)],
        expect=[("A blue chase, a green chase, police lights, then a red "
                 "scanner", 0)],
        questions=[("Why does deleting lightsOff() change the "
                    "pattern?", 0),
                   ("Does a bigger SPEED make it faster or slower?", 0)],
        speaker=[
            "SPEED is the delay between steps, so BIGGER is SLOWER. That "
            "catches almost everybody - let them find it.",
            "Without lightsOff(), nothing ever turns off, so the chase "
            "becomes a wipe that fills the vehicle. Exactly Round 2 of the "
            "Human Light Strip.",
            "Counting backward is on the next slide. Show it only to teams "
            "who have tried for a few minutes.",
        ])

    deck.code(
        "Hint: counting backward",
        ["void scanner(Color color, int sweeps) {",
         "  for (int sweep = 0; sweep < sweeps; sweep++) {",
         "    for (int number = 0; number < 16; number++) {",
         "      lightsOff();",
         "      setLight(number, color);",
         "      setLight(behind(number), color);",
         "      delay(SPEED);",
         "    }",
         "    for (int number = 15; number >= 0; number--) {",
         "      lightsOff();",
         "      setLight(number, color);",
         "      setLight(behind(number), color);",
         "      delay(SPEED);",
         "    }",
         "  }",
         "}"],
        filename="e04a_light_show.ino",
        highlight={8},
        notes=[("Start at the TOP number.", 0),
               ("Keep going while it's 0 or more:  >= 0", 0),
               ("Take 1 away each time:  --", 0),
               ("behind(number) is the back light right behind a front "
                "one.", 0)],
        speaker=[
            "The highlighted line is the backward count. For the chase "
            "they need: for (int number = 31; number >= 0; number--)",
            "The scanner has a loop INSIDE a loop. The outer one counts "
            "sweeps, the inner ones count lights. Only point that out to "
            "students who are flying.",
        ])

    divider(deck, L, "Your own show")

    deck.bullets(
        "Design your own pattern",
        [("Draw it first! Color in the grid in your Mission Log - one row "
          "per step.", 0),
         ("Ideas:", 0),
         ("Two lights chasing each other", 1),
         ("Front and back going opposite ways", 1),
         ("A rainbow that fills up:  rainbowColor(number * 3)", 1),
         ("Lights that bounce out from the middle", 1),
         ("Then copy chase(), give it a new name, and change it.", 0)],
        note="Test after every small change. If it breaks, you'll know which "
             "change did it.",
        speaker=[
            "Insist on the drawing. Teams that draw first build the right "
            "thing; teams that type first build SOMETHING and call it "
            "done.",
            "rainbowColor takes a percent around the color wheel, 0 to "
            "100, so number * 3 spreads 32 lights across most of a "
            "rainbow.",
            "Copying chase() and renaming it (say, to myPattern) is "
            "exactly how real programmers start new code. Then add "
            "myPattern(); to loop().",
        ])

    mission(
        deck, L, "0:37", "Your own light show", "e04a_light_show",
        [("Draw your pattern in the Mission Log grid: at least four "
          "steps.", 0),
         ("Copy chase() at the bottom. Rename the copy, for example "
          "myPattern.", 0),
         ("Change the copy to match your drawing.", 0),
         ("Add  myPattern();  to loop(). Upload.", 0),
         ("Does it match your drawing? Show another team!", 0)],
        expect=[("Your drawing, in lights", 0)],
        questions=[("What did you have to change to make it match your "
                    "drawing?", 0)],
        speaker=[
            "If a copied function won't compile, it's almost always a "
            "brace: the copy is missing its last }. Count them together.",
            "Two names the same is the other classic error: two functions "
            "both called chase. The compiler says 'redefinition'.",
            "Finish with a two-minute gallery: everyone switches lights on "
            "and walks round looking at the other vehicles.",
        ])

    new_words(deck, [
        ["Loop", "Code that repeats"],
        ["for loop", "A loop that counts: start, keep going while..., "
         "step"],
        ["++  and  --", "Add 1, and take away 1"],
        ["Animation", "Pictures changing fast enough to look like "
         "movement"],
        ["{ }  braces", "Hold the lines that belong together"],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  What does a loop do?", 0),
         ("2.  for (int number = 0; number < 32; number++): what's the first "
          "number? The last?", 0),
         ("3.  What does ++ mean?", 0),
         ("4.  How do you make the light show go faster?", 0),
         ("5.  Why does the chase need lightsOff()?", 0)],
        speaker=[
            "Answers: 1 repeats instructions. 2 first 0, last 31. 3 add "
            "one. 4 make SPEED smaller. 5 to turn the last light off, or "
            "they all stay on.",
            "Question 2 is the real test - 'last is 32' is the common "
            "wrong answer.",
        ])

    wrap_up(deck,
            ["Your pattern design, colored in",
             "Fill in the for loop: start, keep going, step"],
            "Light Show Designer",
            speaker=[
                "Next lesson the wheels move for the first time. Remind "
                "everybody: hair tied back next time.",
            ])


# ===================================================================
# LESSON 5 - MAKE IT MOVE
# ===================================================================

def lesson5(deck):
    L = "lesson5"
    title(deck,
          "Motors, magnets, and a vehicle with no steering wheel. Today the "
          "wheels turn - with the vehicle safely up on its stand.",
          img("vehicle-gen3-top.jpg"),
          speaker=[
              "Check every stand before class. Today is the first lesson "
              "where code moves the wheels, and a vehicle that walks off a "
              "block is the accident you're preventing.",
              "For the Human Tank: a pool noodle or a meter stick per "
              "demonstration pair.",
          ])

    todays_mission(deck, [
        "Say how an electric motor turns electricity into spinning",
        "Explain how a motor goes forward, backward, and half speed",
        "Explain how a vehicle with no steering wheel can turn",
        "Make the wheels move from your own code - safely, wheels up",
    ], speaker=["Read them out, then go straight to the safety slide."])

    deck.bullets(
        "Today the wheels MOVE",
        [("WHEELS UP: the vehicle stays on its stand the whole lesson.", 0),
         ("Crew Chief: keep a finger near the power switch.", 0),
         ("Hair tied back. Sleeves pushed up. Cables away from the "
          "wheels.", 0),
         ("Switch jobs! Make sure today's Crew Chief knows the power "
          "switch.", 0)],
        note="Wheels moving when you didn't expect it? Crew Chief: power "
             "OFF. Then figure it out.",
        note_kind="safety",
        speaker=[
            "Do a quick drill: 'Crew Chiefs, hands on power switches... "
            "now OFF... now hands in the air.' Twice. It's the reflex you "
            "want later.",
        ])

    agenda(deck, L)

    divider(deck, L, "How motors work")

    deck.bullets(
        "What's inside a motor",
        [("An electric motor is full of MAGNETS.", 0),
         ("Electricity flowing through coils of wire turns them into "
          "magnets too.", 0),
         ("Magnets push and pull on each other - and that push makes the "
          "motor spin.", 0),
         ("Your vehicle has FOUR motors, one for each wheel.", 0)],
        note="The same idea spins fans, drills, electric cars, and the buzzer "
             "in your controller.",
        speaker=[
            "If you have two fridge magnets, show push and pull. That's "
            "the whole idea of a motor.",
            "The 'buzzer in your controller' is the rumble: a tiny motor "
            "spinning a lopsided weight. They'll use it in Lesson 8.",
        ])

    deck.image_slide(
        "Forward and backward",
        img("vehicle-basic-layout.jpg"),
        caption="The control board from above. Each motor has its own "
                "driver.",
        items=[("Electricity one way: the motor spins forward. The OTHER way: "
                "backward.", 0),
               ("A chip called the MOTOR DRIVER is the traffic cop that picks "
                "the way. Your program tells it what to do.", 0)],
        speaker=[
            "Don't go into how the driver works inside. 'It sends the "
            "electricity one way or the other, and your program tells it "
            "which' is the right depth for this age.",
            "For any older student who wants more: the chip is called an "
            "H-bridge, because the four switches inside it are drawn in "
            "the shape of the letter H. The Beginner course covers it.",
        ])

    divider(deck, L, "Speed")

    dx.pedal_and_coast(deck, pwm_freq=FACTS["pwm_freq"])

    deck.bullets(
        "Speeds in your programs",
        [("drive(leftSpeed, rightSpeed)", 0),
         ("Speeds go from -100 to 100.", 0),
         ("100 is top speed forward.  50 is half.  0 is stop.", 1),
         ("-100 is top speed BACKWARD.", 1),
         ("Minus means backward - like a temperature below zero.", 0),
         ("SPEED_LIMIT decides how fast \"top speed\" really is.", 0)],
        speaker=[
            "Negative numbers aren't taught in school until about sixth "
            "grade, so don't assume the younger ones know them. A "
            "thermometer, or a number line on the board with 0 in the "
            "middle, is all it takes.",
            "With the Learner limit of {}, drive(100, 100) is half of the "
            "motors' full power, and drive(50, 50) is a quarter.".format(
                FACTS["learner"]),
        ])

    divider(deck, L, "Tank steering")

    unplugged(
        deck, L, "0:20", "The Human Tank", "Two volunteers and a pool noodle",
        [("Two volunteers stand side by side, holding a pool noodle between "
          "them.", 0),
         ("The LEFT person is the left wheels. The RIGHT person is the right "
          "wheels.", 0),
         ("Both step forward together: STRAIGHT.", 0),
         ("Left steps forward, right stands still: which way do you "
          "turn?", 0),
         ("Left steps forward, right steps BACKWARD: SPIN!", 0),
         ("Class: shout out drive(left, right) for the tank to act out.", 0)],
        expect=[("Turning toward the side that moves less", 0)],
        questions=[("To turn RIGHT, which side has to go faster?", 0),
                   ("How do you spin on the spot?", 0)],
        safety="Small, slow steps. The tank never runs.",
        speaker=[
            "Facing the class is confusing for left and right. Have the "
            "tank face the same way as the class, back to the room.",
            "Answers: to turn right, the LEFT side goes faster. To spin, "
            "the two sides go opposite ways.",
            "Good class shouts to try: drive(50, 50), drive(50, 0), "
            "drive(0, 50), drive(50, -50), drive(-50, -50).",
        ])

    dx.tank_turns(deck)

    divider(deck, L, "Motor lab")

    deck.code(
        "The motor lab",
        ["void loop() {",
         "  Serial.println(\"Left side forward\");",
         "  drive(50, 0);",
         "  delay(2000);",
         "",
         "  Serial.println(\"Right side forward\");",
         "  drive(0, 50);",
         "  delay(2000);",
         "",
         "  Serial.println(\"Both sides forward\");",
         "  drive(50, 50);",
         "  delay(2000);"],
        filename="e05a_motor_lab.ino   (the first three steps)",
        notes=[("Each step: say what's happening, set the wheels, wait.", 0),
               ("drive() keeps the wheels going until you change them.", 0),
               ("PREDICT before each step!", 0)],
        speaker=[
            "Important idea: drive() doesn't run the wheels FOR a time. It "
            "sets them and they keep going until the next drive(). The "
            "delay() is what decides how long each step lasts.",
        ])

    mission(
        deck, L, "0:30", "Motor lab", "e05a_motor_lab",
        [("Crew Chief: vehicle on its stand, all four wheels in the air. "
          "Check!", 0),
         ("Upload. Watch the countdown on the lights. Then the wheels "
          "move!", 0),
         ("LEVEL 1: before each step, PREDICT which wheels turn, and "
          "which way.", 0),
         ("LEVEL 2: find the WAKE-UP SPEED. Change drive(50, 0) (line {}) to "
          "30, then 20, then 10.".format(
              line("e05a_motor_lab", "drive(50, 0);")), 0),
         ("LEVEL 3: write a WHEEL DANCE: change loop() into your own "
          "routine.", 0)],
        expect=[("Left wheels, right wheels, both, backward, a spin, then a "
                 "rest", 0)],
        questions=[("Which step is a spin? Why?", 0),
                   ("What's your wake-up speed?", 0)],
        safety="WHEELS UP the whole lesson. If the vehicle creeps on its "
               "stand, Crew Chief: power OFF.",
        speaker=[
            "Walk the room and look at stands, not screens. A wheel "
            "touching the table will walk the vehicle off the edge.",
            "The wake-up speed varies by vehicle and battery charge. "
            "Collect every team's number on the board - the spread is a "
            "nice lead-in to 'no two vehicles are the same' next lesson.",
            "Level 3 dances: encourage a wiggle (drive(40, -40) then "
            "drive(-40, 40), short delays) and a 'drum roll' (both sides "
            "forward and back quickly).",
        ])

    deck.bullets(
        "Why is there a wake-up speed?",
        [("A motor needs a big enough push to get started - like you getting "
          "out of bed!", 0),
         ("Tiny speeds can't beat the friction in the gears and the "
          "wheels.", 0),
         ("On the floor it's even higher: now the motor has to move the "
          "whole vehicle.", 0)],
        note="Engineers call this STARTING FRICTION. Every motor has it.",
        speaker=[
            "Ask: once the wheel is turning, could it keep turning at an "
            "even lower speed? (Usually yes. Starting is harder than "
            "keeping going - a good thing to test if a team finishes "
            "early.)",
        ])

    new_words(deck, [
        ["Motor", "Turns electricity into spinning"],
        ["Magnet", "Pushes or pulls on other magnets"],
        ["Motor driver", "The chip that sends electricity to the motor, "
         "one way or the other"],
        ["Tank steering", "Turning by running one side faster than the "
         "other"],
        ["Negative number", "Less than zero. Here, minus means backward."],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  What's inside a motor that makes it spin?", 0),
         ("2.  How do you make a motor spin backward?", 0),
         ("3.  What does drive(50, -50) do?", 0),
         ("4.  To curve RIGHT, which side goes faster?", 0),
         ("5.  Why must the wheels be up today?", 0)],
        speaker=[
            "Answers: 1 magnets. 2 send the electricity the other way - in "
            "code, a minus speed. 3 spins right on the spot. 4 the left. "
            "5 the program moves the wheels as soon as it starts, and a "
            "vehicle on a table can drive off it.",
        ])

    wrap_up(deck,
            ["Write your wake-up speed",
             "Draw or write your wheel dance"],
            "Motor Mechanic",
            speaker=[
                "Next lesson is on the FLOOR. Check the floor space you'll "
                "use and tape the squares beforehand.",
            ])


# ===================================================================
# LESSON 6 - DRIVE BY CODE
# ===================================================================

def lesson6(deck):
    L = "lesson6"
    title(deck,
          "No controller today. You write the moves, the vehicle drives them "
          "all by itself - and you find out how good your numbers really "
          "are.",
          img("vehicle-top-plate-esp32.jpg"),
          speaker=[
              "BEFORE CLASS: tape one square per team on the floor, about 4 "
              "feet on a side, with START marked. Hard floor is better than "
              "carpet for repeatable results.",
              "Sticky notes for marking where each run ends are a big "
              "help.",
          ])

    todays_mission(deck, [
        "Write a program that drives the vehicle by itself",
        "Measure time in milliseconds",
        "Tune a number by testing, measuring, and changing one thing at a "
        "time",
        "Explain why a robot that drives by the clock makes mistakes",
    ], speaker=["Read them out. The third one is the real lesson of the "
                "day: tuning is a skill, and it has rules."])

    deck.bullets(
        "Last time",
        [("The wheels turned - safely up on the stand.", 0),
         ("drive(50, -50) made the vehicle spin.", 0),
         ("Today: the FLOOR! But no controller. The vehicle drives a plan "
          "you wrote.", 0),
         ("That's called AUTOPILOT.", 0),
         ("Switch jobs!", 0)],
        speaker=[
            "Ask what drive(50, -50) does before you show the slide, and "
            "what a team's wake-up speed was.",
        ])

    agenda(deck, L)

    divider(deck, L, "Autopilot")

    deck.table(
        "Autopilot moves",
        ["Move", "What it does"],
        [["forward(1000)", "Drive forward for 1000 milliseconds, then stop"],
         ["backward(1000)", "Drive backward for 1000 milliseconds"],
         ["spinLeft(500)", "Spin left on the spot for 500 milliseconds"],
         ["spinRight(500)", "Spin right on the spot for 500 milliseconds"],
         ["pause(1000)", "Stop and wait for 1000 milliseconds"],
         ["countdown(5)", "5... 4... 3... 2... 1... GO! on the lights"]],
        col_widths=[3, 8],
        lead="Every move takes a time, in MILLISECONDS.",
        speaker=[
            "These are all in the engine room, explorer.h. Each one runs "
            "the motors, waits, then stops.",
            "How fast they go is set by setDriveSpeed(), which is in the "
            "program's settings.",
        ])

    deck.bullets(
        "Milliseconds",
        [("A millisecond is one thousandth of a second.", 0),
         ("1000 milliseconds = 1 second.", 0),
         ("500 = half a second.  250 = a quarter of a second.", 0),
         ("Why so small? Because a robot can do a LOT in one second.", 0)],
        note="Quick! How many milliseconds in 3 seconds? In 2 and a half?",
        speaker=[
            "Answers: 3000, and 2500.",
            "A good feel for it: a blink of an eye takes very roughly 100 "
            "to 400 milliseconds. Have them blink and guess.",
        ])

    deck.code(
        "The settings you'll tune",
        ["const int SPEED_LIMIT = 50;    // Learner",
         "const int DRIVE_SPEED = 60;    // Percent",
         "const int SIDE_TIME   = 1500;  // Milliseconds",
         "const int TURN_TIME   = 500;   // TUNE ME!"],
        filename="e06a_drive_a_square.ino, line {}".format(
            line("e06a_drive_a_square", "const int SPEED_LIMIT")),
        notes=[("const means this number won't change while the program "
                "runs.", 0),
               ("The NAME is how the program uses it.", 0),
               ("Change the number here, and every move that uses the name "
                "changes too.", 0)],
        speaker=[
            "TURN_TIME starts at {} on purpose. It's a guess, and it's "
            "almost certainly wrong for your vehicle and your floor. "
            "Finding the right number is today's mission.".format(
                FACTS["turn_time"]),
        ])

    deck.code(
        "The mission lives in setup()",
        ["void setup() {",
         "  startRover();",
         "  setSpeedLimit(SPEED_LIMIT);",
         "  setDriveSpeed(DRIVE_SPEED);",
         "",
         "  countdown(5);                // 5 seconds to step back",
         "",
         "  // The mission happens ONCE, so it goes in setup().",
         "  forward(SIDE_TIME);",
         "  spinRight(TURN_TIME);",
         "  forward(SIDE_TIME);",
         "  spinRight(TURN_TIME);",
         "  forward(SIDE_TIME);",
         "  spinRight(TURN_TIME);",
         "  forward(SIDE_TIME);",
         "  spinRight(TURN_TIME);",
         "",
         "  setAllLights(GREEN);         // Mission complete!",
         "}"],
        filename="e06a_drive_a_square.ino",
        speaker=[
            "Why setup() and not loop()? Ask. (Because we want it to drive "
            "the square ONCE. In loop() it would go round and round "
            "forever.)",
            "To run it again you switch the vehicle off and on - that "
            "restarts the program, and setup() runs again.",
        ])

    dx.square_route(deck)

    divider(deck, L, "Drive a square")

    deck.image_slide(
        "Your square",
        TAPE_SQUARE,
        caption="Start on the corner, pointing along the tape",
        speaker=[
            "Show how to line up the vehicle: back wheels on the corner, "
            "nose pointing exactly along one side. A crooked start makes "
            "a crooked square, and that's a variable they should control.",
        ])

    mission(
        deck, L, "0:13", "Drive a square", "e06a_drive_a_square",
        [("Upload it with the wheels UP. The wheels will practice on the "
          "stand. That's fine!", 0),
         ("Unplug the cable. Power OFF.", 0),
         ("Put the vehicle on START, pointing along the tape.", 0),
         ("Power ON and step back behind the line. 5-second countdown!", 0),
         ("Where did it end up? Mark it with a sticky note.", 0)],
        expect=[("Countdown, four sides and four turns, then green "
                 "lights", 0)],
        questions=[("Did it come back to START?", 0),
                   ("Were the corners too much, or too little?", 0)],
        safety="Everybody behind the line before the countdown ends. Never "
               "grab a moving vehicle - wait for it to stop.",
        speaker=[
            "The first run will almost never come home. Celebrate that - "
            "it's what makes the next 20 minutes interesting.",
            "If a vehicle drives straight off the square on the very first "
            "side, check SIDE_TIME hasn't been changed, and that the "
            "vehicle was pointing along the tape.",
        ])

    divider(deck, L, "Tune it")

    deck.bullets(
        "How engineers tune",
        [("1.  PREDICT what will happen.", 0),
         ("2.  TEST it.", 0),
         ("3.  MEASURE: where did it end up? Too far, or not far "
          "enough?", 0),
         ("4.  CHANGE ONE THING - like TURN_TIME, by 50.", 0),
         ("5.  Test again!", 0)],
        note="Change ONE number at a time. Change two, and you won't know "
             "which one helped.",
        speaker=[
            "This is the scientific method in disguise: one variable at a "
            "time, everything else the same.",
            "Turns too SHORT means the vehicle under-rotates, so TURN_TIME "
            "needs to get BIGGER. Students often get the direction "
            "backward the first time; let the next run tell them.",
        ])

    mission(
        deck, L, "0:28", "Tune it", "e06a_drive_a_square",
        [("Turns too SHORT? Make TURN_TIME (line {}) bigger by 50. Too LONG? "
          "Smaller by 50.".format(
              line("e06a_drive_a_square", "const int TURN_TIME")), 0),
         ("Write every test in your Mission Log: TURN_TIME, and where it "
          "ended up.", 0),
         ("LEVEL 2: make the square bigger. Which number do you change?", 0),
         ("LEVEL 3: swap the eight moves for a for loop. Then drive a "
          "TRIANGLE.", 0)],
        expect=[("Each test ends a little closer to START", 0)],
        questions=[("What's your best TURN_TIME?", 0),
                   ("Is it the same as other teams'? Why not?", 0)],
        safety="Every upload: wheels up, then unplug, then floor.",
        speaker=[
            "Level 2: SIDE_TIME. Level 3: a for loop that runs four times "
            "with forward() and spinRight() inside it. A triangle turns "
            "120 degrees at each corner instead of 90, so TURN_TIME "
            "becomes about one-and-a-third times as long, and the loop "
            "runs three times.",
            "Put every team's best TURN_TIME on the board. They'll all be "
            "different - that's the next slide.",
        ])

    deck.bullets(
        "Why isn't every vehicle the same?",
        [("Every motor is a tiny bit different.", 0),
         ("The floor matters: carpet, tile, wood.", 0),
         ("The battery matters: a full battery pushes harder.", 0),
         ("The vehicle can't SEE where it is. It just counts time.", 0),
         ("That's called DEAD RECKONING. Sailors navigated this way for "
          "hundreds of years.", 0)],
        note="Real rovers drive by the clock AND check sensors. Sensors come "
             "in the next course!",
        speaker=[
            "Dead reckoning: if you know your speed, your direction and "
            "how long you've been going, you can work out where you are - "
            "without looking. Ships did exactly this before satellite "
            "navigation, and small errors piled up just like today.",
            "Ask: what could the vehicle use to check where it really is? "
            "(A camera, a distance sensor, a line on the floor to follow.)",
        ])

    new_words(deck, [
        ["Autopilot", "The vehicle drives a plan by itself"],
        ["Millisecond", "One thousandth of a second. 1000 ms = 1 second"],
        ["const", "A number with a name, that doesn't change while the "
         "program runs"],
        ["Tune", "Change a number a little at a time until it works"],
        ["Dead reckoning", "Working out where you are from speed and time, "
         "without looking"],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  How many milliseconds in 2 seconds?", 0),
         ("2.  What does spinRight(500) do?", 0),
         ("3.  The corners turn too little. Make TURN_TIME bigger or "
          "smaller?", 0),
         ("4.  Why change only one number at a time?", 0),
         ("5.  Why might your best TURN_TIME not work on another team's "
          "vehicle?", 0)],
        speaker=[
            "Answers: 1 2000. 2 spins right on the spot for half a second. "
            "3 bigger. 4 so you know which change made the difference. 5 "
            "different motors, battery charge, floor.",
        ])

    wrap_up(deck,
            ["Your tuning table: every TURN_TIME you tried",
             "Circle your best TURN_TIME - you need it next lesson!"],
            "Autopilot",
            speaker=[
                "Make sure every team has written down its best TURN_TIME. "
                "Lesson 7 starts from it.",
                "Batteries: today's floor work drains them. Charge every "
                "pack before next lesson.",
            ])


# ===================================================================
# LESSON 7 - MISSION TO MARS
# ===================================================================

def lesson7(deck):
    L = "lesson7"
    title(deck,
          "When a rover is too far away to steer, it needs a plan. Today you "
          "plan a mission, invent your own moves, and send your rover to "
          "collect a sample.",
          img("vehicle-with-rangefinder.jpg"),
          speaker=[
              "BEFORE CLASS: build the Mars course - a START base, a "
              "'crater' (a hula hoop, or a ring of paper plates), and a "
              "SAMPLE zone, on a grid of one-foot squares. One course can "
              "be shared if teams take turns.",
              "Something to 'collect' at the sample zone is fun: a ball, a "
              "rock-shaped bean bag, a paper cup.",
          ])

    todays_mission(deck, [
        "Explain why Mars rovers drive themselves",
        "Plan a route on a grid, and turn it into moves",
        "Invent a function: a new move with your own name on it",
        "Test a mission, find what went wrong, and fix it",
    ], speaker=["Read them out. The theme today is planning - point at the "
                "words plan, test and fix."])

    deck.bullets(
        "Last time",
        [("Your vehicle drove a square on autopilot.", 0),
         ("You tuned TURN_TIME until it came home. Got your best number?", 0),
         ("Today: a real mission. And a planet that's too far away to "
          "steer.", 0),
         ("Switch jobs!", 0)],
        speaker=[
            "Collect best TURN_TIMEs out loud. Any team that lost theirs "
            "can use {} and re-tune quickly.".format(FACTS["turn_time"]),
        ])

    agenda(deck, L)

    checkin(deck, "Halfway")

    divider(deck, L, "Too far to steer")

    dx.mars_delay(deck)

    unplugged(
        deck, L, "0:07", "The Mars Delay Game", "Mission Control and a rover",
        [("One volunteer is the ROVER, across the room. One is MISSION "
          "CONTROL, facing away.", 0),
         ("Mission Control gives ONE command. A MESSENGER walks it over - "
          "slowly - and whispers it.", 0),
         ("The rover does it. The messenger walks back to say "
          "\"done\".", 0),
         ("Mission: get the rover to touch the door handle.", 0),
         ("ROUND 2: Mission Control writes the WHOLE plan on a card. The "
          "messenger carries it ONCE.", 0)],
        expect=[("Round 1 takes forever. Round 2 is way faster - if the plan "
                 "is right!", 0)],
        questions=[("Why was round 2 faster?", 0),
                   ("What happened when the plan had a mistake?", 0)],
        safety="Messengers WALK. No running, even in a hurry.",
        speaker=[
            "Mission Control must not watch the rover - just like the real "
            "team, they only know what the messenger tells them.",
            "Time both rounds on the board. Round 1 usually takes several "
            "minutes for a few steps.",
            "If Round 2's plan has a mistake, let it fail. Then ask what "
            "the real rover team would do: look at the pictures the rover "
            "sends back, and fix tomorrow's plan.",
        ])

    deck.bullets_image(
        "Real rovers do this",
        [("Mars rovers are driven from NASA's Jet Propulsion Laboratory, in "
          "Pasadena, California.", 0),
         ("The team plans the rover's moves, checks them, and sends them "
          "all at once.", 0),
         ("The rover drives the plan by itself, and sends back "
          "pictures.", 0),
         ("Then the team plans the next day.", 0)],
        SOJOURNER,
        caption="Sojourner, 1997",
        image_ratio=0.42,
        speaker=[
            "Pasadena is a couple of hours' drive north of San Diego - "
            "real Mars rovers are driven from right here in Southern "
            "California.",
            "Modern rovers can also steer around small rocks on their own, "
            "using their cameras. Ours can't see anything, so our plan "
            "has to be perfect.",
        ])

    divider(deck, L, "Plan the route")

    deck.bullets(
        "Invent your own moves",
        [("A FUNCTION is a new move that you invent and name.", 0),
         ("Write it once, at the bottom of your program.", 0),
         ("Then use it as many times as you like - just say its name.", 0),
         ("You've used lots already: forward(), setLight() and drive() are "
          "all functions.", 0)],
        note="turnRight() is easier to read than spinRight(QUARTER_TURN). And "
             "if you re-tune the turn, you fix it in ONE place.",
        speaker=[
            "A recipe is a good comparison: the recipe for pancakes is "
            "written once; 'make pancakes' is all you need to say after "
            "that.",
            "'void' just means the function doesn't hand anything back - "
            "it does something. Only explain it if asked.",
        ])

    deck.code(
        "Moves you invent",
        ["void turnRight() {",
         "  spinRight(QUARTER_TURN);",
         "}",
         "",
         "void turnLeft() {",
         "  spinLeft(QUARTER_TURN);",
         "}",
         "",
         "// Flash the lights purple three times: sample collected!",
         "void collectSample() {",
         "  for (int flash = 0; flash < 3; flash++) {",
         "    setAllLights(PURPLE);",
         "    delay(300);",
         "    lightsOff();",
         "    delay(300);",
         "  }",
         "}"],
        filename="e07a_mission_planner.ino",
        notes=[("void, a name, ( ), and braces { }.", 0),
               ("Everything inside the braces happens when you use the "
                "name.", 0),
               ("collectSample() uses a for loop from Lesson 4!", 0)],
        speaker=[
            "Point at the shape of each function: the name line, the open "
            "brace, the body, the close brace.",
            "Ask somebody to say what happens, in order, when the plan says "
            "collectSample(). (Purple, wait, off, wait - three times.)",
        ])

    dx.mission_map(deck)

    deck.image_slide(
        "Our Mars course",
        MARS_COURSE,
        caption="Measure it! How many squares from START to the SAMPLE?",
        speaker=[
            "If your course differs from the example grid, walk the room "
            "through it now: where START is, which way the rover faces, "
            "where the crater is.",
        ])

    divider(deck, L, "Run the mission")

    deck.code(
        "Your mission plan goes here",
        ["  countdown(5);",
         "",
         "  // ===== YOUR MISSION PLAN: one move per line =====",
         "  forward(1000);",
         "  turnRight();",
         "  forward(800);",
         "",
         "  collectSample();",
         "",
         "  // LEVEL 2: now drive back home to START",
         "",
         "  // =================================================",
         "  setAllLights(GREEN);    // Mission complete!"],
        filename="e07a_mission_planner.ino, in setup()",
        notes=[("The three example moves are just a start.", 0),
               ("Replace them with YOUR route.", 0),
               ("One move per line, top to bottom - like the Human Robot "
                "card.", 0)],
        speaker=[
            "Tie it back to Lesson 2: this is the Human Robot's card, "
            "written in a language the vehicle understands.",
        ])

    mission(
        deck, L, "0:28", "Mission to Mars", "e07a_mission_planner",
        [("Copy your best TURN_TIME into QUARTER_TURN (line {}).".format(
            line("e07a_mission_planner", "const int QUARTER_TURN")), 0),
         ("Measure: how many milliseconds to drive ONE square?", 0),
         ("Draw your route on the grid in the Mission Log. Count the "
          "squares.", 0),
         ("Write one move per line in the MISSION PLAN.", 0),
         ("Upload wheels up. Then run it on the course. Fix it. Run it "
          "again.", 0),
         ("LEVEL 1: reach the SAMPLE.  LEVEL 2: come home.  LEVEL 3: invent "
          "a move, use it twice.", 0)],
        expect=[("Countdown, your moves, purple flashes at the sample", 0)],
        questions=[("Where did your plan go wrong first?", 0),
                   ("How many tries did it take?", 0)],
        safety="One rover on the course at a time. Everybody else behind the "
               "line.",
        speaker=[
            "The measuring step is the one teams skip. Make them drive "
            "forward(1000) once and measure how many squares that covers, "
            "then work out the rest. It saves a dozen guesses.",
            "Math moment: if 1000 ms covers 2 squares, one square is 500 "
            "ms, and 5 squares is 2500. Some younger students will need "
            "to do this with a table rather than division - that's fine.",
            "Level 3 ideas: a zigzag, a victory spin (spinRight with a "
            "long time), a 'look around' (spin left a little, then right, "
            "then back).",
        ])

    deck.bullets(
        "Mission debrief",
        [("What went right?", 0),
         ("What went wrong, and how did you fix it?", 0),
         ("If you did it again, what would you plan differently?", 0)],
        note="Real mission teams meet like this after every drive.",
        speaker=[
            "Each team answers one question out loud. Keep it fast and "
            "positive; the point is to name what they learned.",
        ])

    new_words(deck, [
        ["Function", "A move you invent and name. Write it once, use it "
         "lots."],
        ["Route", "The path the rover drives"],
        ["Mission Control", "The team that plans and watches a mission"],
        ["Signal delay", "The time a radio message takes to arrive"],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  Why can't anyone on Earth steer a Mars rover live?", 0),
         ("2.  What's a function?", 0),
         ("3.  Why write turnRight() instead of spinRight(QUARTER_TURN) every "
          "time?", 0),
         ("4.  One square takes 500 milliseconds. How long for 3 "
          "squares?", 0),
         ("5.  What would you change about your plan?", 0)],
        speaker=[
            "Answers: 1 messages take minutes to get there. 2 a named move "
            "you write once and use lots. 3 easier to read, and you only "
            "fix the turn in one place. 4 1500 milliseconds. 5 anything "
            "honest!",
        ])

    wrap_up(deck,
            ["Draw the route you programmed",
             "Write: milliseconds per square",
             "Answer the debrief questions"],
            "Mission Planner",
            speaker=[
                "Next lesson the controller comes back - and they decide "
                "what every button does.",
            ])


# ===================================================================
# LESSON 8 - WIRELESS CONTROL
# ===================================================================

def lesson8(deck):
    L = "lesson8"
    title(deck,
          "Your controller talks to the vehicle by radio. Today you decide "
          "what every button does.",
          img("switch-controller-pair.jpg"),
          speaker=[
              "Charge every controller before class. A flat controller "
              "looks exactly like a broken program.",
              "Make sure each team has its OWN controller - check the "
              "stickers as they come in.",
          ])

    todays_mission(deck, [
        "Explain how the controller talks to the vehicle",
        "Use if and else to make the vehicle choose what to do",
        "Tell the difference between HOLDING a button and TAPPING it",
        "Make the controller's buttons do things you chose",
    ], speaker=["Read them out. Ask: which button on a game controller do "
                "you press the most?"])

    deck.bullets(
        "Last time",
        [("Your rover followed a plan on autopilot.", 0),
         ("It couldn't see the crater. It couldn't change its mind.", 0),
         ("Today the human is back in control - with the controller.", 0),
         ("But YOU decide what each button does.", 0),
         ("Switch jobs!", 0)],
        speaker=["Quick show of hands: which team got home from the sample "
                 "zone last lesson?"])

    agenda(deck, L)

    divider(deck, L, "Radio links")

    deck.bullets_image(
        "An invisible link",
        [("The controller and the vehicle talk by RADIO - a kind called "
          "Bluetooth.", 0),
         ("It's the same kind of radio that wireless earbuds use.", 0),
         ("Every controller has its own address, like a house number.", 0),
         ("Your vehicle only listens to ONE address: its own controller's. "
          "That's why the stickers match.", 0)],
        STICKERS,
        caption="Same number, same team",
        image_ratio=0.40,
        speaker=[
            "Radio is invisible light, more or less - it goes through "
            "walls and bodies, which is why the controller works from "
            "anywhere in the room.",
            "Your teacher told each vehicle its controller's address "
            "before Lesson 1, and the vehicle stores it in its memory. "
            "Uploading new programs doesn't erase it. (For you: that was "
            "e00_claim_controller.)",
        ])

    dx.switch_pad_map(deck)

    divider(deck, L, "Choices")

    unplugged(
        deck, L, "0:12", "Simon Says, if/else edition", "Everybody stands up",
        [("The teacher is the program. You are the robots.", 0),
         ("RULE 1: IF the teacher's hand is UP, clap. ELSE, stay still.", 0),
         ("RULE 2: the WHOLE TIME the teacher holds a hand up, keep "
          "waving.", 0),
         ("RULE 3: when the teacher TAPS their head, jump ONCE - no matter "
          "how long the hand stays there.", 0),
         ("Mix them up, and speed up!", 0)],
        expect=[("Clapping, waving and jumping at the right moments", 0)],
        questions=[("What's the difference between rule 2 and rule 3?", 0)],
        safety="Jump on the spot. Give your neighbors room.",
        speaker=[
            "Rule 1 is if/else. Rule 2 is a HELD button: true the whole "
            "time. Rule 3 is a TAPPED button: true only at the moment it "
            "starts. Name them that way after the game.",
            "The fun part of rule 3 is leaving your hand on your head for "
            "a long time. Anyone who jumps twice was reading it as 'held'.",
        ])

    dx.if_else_flow(deck)

    deck.two_columns(
        "Held, or tapped?",
        "buttonA()",
        [("TRUE the whole time you hold A", 0),
         ("Use it for:", 0),
         ("lights that stay on while you hold", 1),
         ("a horn, or a headlight flash", 1)],
        "tappedA()",
        [("TRUE once, the moment you press A", 0),
         ("Use it for:", 0),
         ("switching something ON and OFF", 1),
         ("counting presses", 1)],
        note="loop() runs thousands of times a second. Holding a button for "
             "one second is thousands of loops!",
        speaker=[
            "That note is why tapped exists. A light switch that flipped "
            "on EVERY loop while you held it would flicker on and off "
            "thousands of times.",
            "The engine room works out 'tapped' by remembering what the "
            "button was doing last time round loop(). If it's down now "
            "and was up last time, that's a tap.",
        ])

    deck.code(
        "The button lab",
        ["void loop() {",
         "  if (controllerReady() == false) {",
         "    return;     // No controller yet",
         "  }",
         "",
         "  if (buttonA()) {",
         "    setFrontLights(GREEN);   // A held",
         "  } else {",
         "    setFrontLights(OFF);     // A not held",
         "  }",
         "",
         "  if (tappedX()) {",
         "    rumble(200);             // One buzz",
         "  }"],
        filename="e08a_button_lab.ino",
        notes=[("The first if: no controller yet? Stop here and try again "
                "next loop.", 0),
               ("return means: skip the rest of loop() this time.", 0),
               ("Then A, with an else. Then X, with no else.", 0)],
        speaker=[
            "controllerReady() also stops the motors and blinks the lights "
            "green while it waits. Every driving program from now on "
            "starts with these three lines.",
            "An if doesn't NEED an else. Ask why the X part has none. "
            "(There's nothing to do when X isn't tapped.)",
        ])

    divider(deck, L, "Button lab")

    mission(
        deck, L, "0:28", "Button lab", "e08a_button_lab",
        [("Upload, wheels up. The lights blink GREEN until the controller "
          "connects.", 0),
         ("Press a button on YOUR controller to connect.", 0),
         ("LEVEL 1: try every button. Then change the colors.", 0),
         ("LEVEL 2: change tappedY() to buttonY() (line {}). HOLD Y and "
          "watch the Serial Monitor. Change it back.".format(
              line("e08a_button_lab", "if (tappedY())")), 0),
         ("LEVEL 3: make dpadLeft() and dpadRight() light up one side. Then "
          "invent a SECRET COMBO.", 0)],
        expect=[("Green front while A is held, red back while B is held, a "
                 "buzz for each X tap", 0)],
        questions=[("Why did holding Y with buttonY() print so many "
                    "lines?", 0),
                   ("What's your secret combo?", 0)],
        speaker=[
            "This program never moves the wheels, but wheels-up stays the "
            "rule - it's a habit, not a case-by-case decision.",
            "Level 2 answer: buttonY() is true on every loop while Y is "
            "held, so it prints thousands of times. tappedY() prints once.",
            "Level 3 for the D-pad: if (dpadLeft()) { setLeftLights(ORANGE); "
            "} - and the same for the right. Some third-party controllers "
            "don't rumble at all; if X does nothing, that's the "
            "controller, not the code.",
        ])

    deck.bullets(
        "Secret combos",
        [("Two ampersands  &&  mean AND.", 0),
         ("if (buttonA() && buttonB()) {   ...   }", 1),
         ("Only TRUE when A AND B are BOTH held down.", 1),
         ("Two lines  ||  mean OR.", 0),
         ("if (dpadLeft() || dpadRight()) {   ...   }", 1),
         ("TRUE when either one is pressed.", 1),
         ("Put your combo at the END of loop(). Whatever lights you set "
          "LAST are the ones that show.", 0)],
        speaker=[
            "The || key is above Enter on most keyboards, with Shift. "
            "Expect to show several people.",
            "Last-one-wins is a useful idea for the rest of the course: "
            "the lights shown are whatever the program set by the end of "
            "each loop.",
            "Combo ideas: A+B = all lights white, L+R = rainbow party "
            "(setLight(number, randomColor()) in a for loop).",
        ])

    new_words(deck, [
        ["Radio", "Invisible waves that carry messages through the air"],
        ["Bluetooth", "The kind of radio your controller uses"],
        ["if / else", "Do this if it's true... otherwise do that"],
        ["Held", "buttonA(): true the whole time it's down"],
        ["Tapped", "tappedA(): true once, the moment you press"],
        ["&&  and  ||", "AND, and OR"],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  How does the controller talk to the vehicle?", 0),
         ("2.  Why doesn't your vehicle obey another team's controller?", 0),
         ("3.  What does else mean?", 0),
         ("4.  To switch lights on and off with X, use buttonX() or "
          "tappedX()? Why?", 0),
         ("5.  What does && mean?", 0)],
        speaker=[
            "Answers: 1 radio (Bluetooth). 2 it only listens to its own "
            "controller's address. 3 otherwise. 4 tappedX(), so it flips "
            "once per press instead of thousands of times. 5 AND.",
        ])

    wrap_up(deck,
            ["Fill in the button plan: what does each button do?",
             "Write your secret combo"],
            "Radio Operator",
            speaker=["Next lesson: the stick, and the driving test. Tell "
                     "them to practice their best driving."])


# ===================================================================
# LESSON 9 - JOYSTICK DRIVING
# ===================================================================

def lesson9(deck):
    L = "lesson9"
    title(deck,
          "The stick sends numbers. Today your code turns them into driving "
          "- and you take your driving test.",
          img("vehicle-gen1-and-gen2.jpg"),
          speaker=[
              "Set up the driving test course (the same one as Lesson 1) "
              "before class, and have the license stamps ready.",
              "Fully charged batteries matter today - a flat battery makes "
              "for a sluggish, unfair test.",
          ])

    todays_mission(deck, [
        "Read the numbers a joystick sends",
        "Explain the wobble zone, and fix stick wobble with code",
        "Turn forward and turn numbers into left and right wheel speeds",
        "Pass the driving test and earn your Driver's License",
    ], speaker=["Read them out, and point at the last one. Today's the "
                "day."])

    deck.bullets(
        "Last time",
        [("You made the buttons do things.", 0),
         ("buttonA() is true while held. tappedA() is true once.", 0),
         ("Today: the STICK. It's not on or off. It sends NUMBERS.", 0),
         ("Switch jobs!", 0)],
        speaker=["Ask for a held/tapped example from last lesson."])

    agenda(deck, L)

    divider(deck, L, "Stick numbers")

    dx.stick_numbers(deck)

    divider(deck, L, "The wobble zone")

    deck.code(
        "Reading the stick",
        ["  int forward = leftStickY();    // Up is +100",
         "  int turn    = leftStickX();    // Right is +100",
         "",
         "  // The wobble zone",
         "  if (abs(forward) < WOBBLE_ZONE) {",
         "    forward = 0;",
         "  }",
         "  if (abs(turn) < WOBBLE_ZONE) {",
         "    turn = 0;",
         "  }"],
        filename="e09a_joystick_drive.ino",
        notes=[("int makes a variable that holds a whole number.", 0),
               ("abs() means: how big, ignoring the minus.", 0),
               ("abs(-3) is 3.  abs(3) is 3.", 0),
               ("Smaller than WOBBLE_ZONE? Count it as 0.", 0)],
        speaker=[
            "abs is short for absolute value. Younger students only need "
            "'how far from zero, in either direction'.",
            "The program starts with WOBBLE_ZONE = 0 on purpose, so they "
            "can see what goes wrong without one.",
        ])

    mission(
        deck, L, "0:10", "Watch the numbers", "e09a_joystick_drive",
        [("Upload, wheels up. Open the Serial Monitor.", 0),
         ("Push the stick all the way up, down, left and right. Write the "
          "numbers in your Mission Log.", 0),
         ("Now LET GO. Is it exactly 0? Write what you see.", 0),
         ("LEVEL 2: set WOBBLE_ZONE (line {}) to 10. Upload. Let go "
          "again.".format(line("e09a_joystick_drive", "const int WOBBLE_ZONE")),
          0),
         ("Then try 60. Drive gently on the stand. What's wrong? Put it back "
          "to 10.", 0)],
        expect=[("100 at full up, -100 at full down", 0),
                ("Numbers NEAR zero when you let go", 0)],
        questions=[("What did your stick say when you let go?", 0),
                   ("What went wrong at 60?", 0)],
        safety="Wheels up until the numbers make sense.",
        speaker=[
            "Most sticks rest within a few counts of zero; an old or "
            "dropped controller can rest further out. Either way, the "
            "point is that it's not reliably exactly zero.",
            "At 60, more than half the stick's travel does nothing, then "
            "the wheels suddenly jump in. It feels dead and then jumpy.",
        ])

    deck.bullets(
        "Too small, too big, just right",
        [("WOBBLE_ZONE = 0: a stick resting at 3 still asks the motors for "
          "a tiny bit of power.", 0),
         ("WOBBLE_ZONE = 60: the first 60 of the stick does NOTHING. Then "
          "it jumps.", 0),
         ("WOBBLE_ZONE = 10: tiny wobbles ignored. Everything else "
          "works.", 0)],
        note="Engineers call it a DEADZONE. Nearly every game controller "
             "uses one.",
        speaker=[
            "This is a design trade-off, which is a big idea in "
            "engineering: there's no perfect number, just a good balance "
            "between two problems.",
        ])

    divider(deck, L, "Tank mixing")

    deck.code(
        "Two numbers in, two numbers out",
        ["  // Tank steering",
         "  int leftSide  = forward + turn;",
         "  int rightSide = forward - turn;",
         "  drive(leftSide, rightSide);"],
        filename="e09a_joystick_drive.ino",
        notes=[("Turn RIGHT: turn is plus.", 0),
               ("The left side gets MORE. The right side gets LESS.", 0),
               ("That's the Human Tank, in two lines of math!", 0)],
        speaker=[
            "Act it out again with two students if you can: forward 50, "
            "turn 20 - left person takes big steps, right person takes "
            "small ones.",
        ])

    deck.table(
        "Be the computer",
        ["forward", "turn", "left = forward + turn",
         "right = forward - turn", "The vehicle..."],
        [["50", "0", "50", "50", "goes straight"],
         ["50", "20", "?", "?", "?"],
         ["0", "50", "?", "?", "?"],
         ["-50", "0", "?", "?", "?"],
         ["100", "40", "?", "?", "?"]],
        col_widths=[1.4, 1.2, 3.1, 3.1, 2.6],
        lead="Fill in the table in your Mission Log.",
        speaker=[
            "Answers: (50, 20) gives 70 and 30, curving right. (0, 50) "
            "gives 50 and -50, spinning right. (-50, 0) gives -50 and -50, "
            "backing up. (100, 40) gives 140 and 60 - the engine room "
            "treats anything over 100 as 100, so it's 100 and 60, curving "
            "right at full speed.",
            "Negative results are where the younger students wobble. Use "
            "the number line: 0 minus 50 is 50 steps the other way.",
        ])

    divider(deck, L, "License test")

    dx.license_course(deck, title="The driving test")

    mission(
        deck, L, "0:30", "Driver's License test", "e09a_joystick_drive",
        [("WOBBLE_ZONE at 10. Upload. Unplug. Floor.", 0),
         ("Practice the course once.", 0),
         ("Then the TEST, with your teacher watching: weave, park, back out, "
          "home.", 0),
         ("No cups knocked over, and all four wheels in the garage = "
          "PASS!", 0),
         ("Passed? Your teacher lets you raise SPEED_LIMIT (line {}) to "
          "{}.".format(line("e09a_joystick_drive", "const int SPEED_LIMIT"),
                       LICENSED), 0),
         ("Too twitchy when you turn? Remove the // in front of  turn = turn "
          "/ 2;", 0)],
        expect=[("Smooth driving, and stopping exactly where you want", 0)],
        questions=[("Which part of the course was hardest?", 0)],
        safety="One vehicle on the test course at a time. Everybody else "
               "behind the line.",
        speaker=[
            "Every team member takes the test, one at a time, and each "
            "earns their own license stamp. The SPEED_LIMIT change is a "
            "team decision once everyone on the team has passed - or your "
            "call if one person is still learning.",
            "Retakes are fine and normal. A student who fails twice gets "
            "a practice lap with you walking beside them.",
            "Watch the first lap at {} closely. The vehicle is noticeably "
            "quicker.".format(LICENSED),
        ])

    license_table(deck, speaker=[
        "Stamp the Driver's License for everyone who passed.",
        "Point out the Expert level: it needs turn signals, which is next "
        "lesson.",
    ])

    new_words(deck, [
        ["Joystick", "A stick that sends two numbers: up/down, and "
         "left/right"],
        ["Wobble zone", "Small stick numbers that count as zero. Also "
         "called a deadzone."],
        ["abs()", "How big a number is, ignoring the minus"],
        ["int", "A variable that holds a whole number"],
        ["Mixing", "Turning forward and turn into left and right"],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  Stick all the way up: what number? All the way down?", 0),
         ("2.  Why isn't the stick exactly 0 when you let go?", 0),
         ("3.  What does WOBBLE_ZONE do?", 0),
         ("4.  forward = 50, turn = 20. Left side? Right side?", 0),
         ("5.  To turn right, which side goes faster?", 0)],
        speaker=[
            "Answers: 1 100 and -100. 2 the stick springs back NEAR the "
            "middle, not exactly to it. 3 counts small numbers as zero. 4 "
            "70 and 30. 5 the left.",
        ])

    wrap_up(deck,
            ["Your stick numbers, and the be-the-computer table",
             "Driver's License stamp, if you passed"],
            "Licensed Driver",
            speaker=["Next lesson adds the lights that make the vehicle "
                     "look like a real car."])


# ===================================================================
# LESSON 10 - SMART LIGHTS
# ===================================================================

def lesson10(deck):
    L = "lesson10"
    title(deck,
          "Lights that tell everybody what the vehicle is about to do - just "
          "like a real car.",
          img("vehicle-turn-signal-right.jpg"),
          speaker=[
              "A car-light homework idea for the lesson before: 'next time "
              "you're in a car, watch the lights of the car in front.' It "
              "makes the warm-up much livelier.",
          ])

    todays_mission(deck, [
        "Say what headlights, brake lights, backup lights and turn signals "
        "tell other people",
        "Make the lights change by themselves as you drive",
        "Add turn signals that blink on the correct side",
        "Explain why blinking with delay() would ruin your driving",
    ], speaker=["Read them out. The last one is the clever idea of the "
                "day - save some energy for it."])

    deck.bullets(
        "Car-light spotting",
        [("Picture a car on the road. When does it show...", 0),
         ("bright red at the back?", 1),
         ("white at the back?", 1),
         ("a blinking orange light?", 1),
         ("Those lights are a LANGUAGE. Every driver knows it.", 0),
         ("Today your vehicle learns to speak it. Switch jobs!", 0)],
        speaker=[
            "Take answers for each one before moving on: braking, "
            "reversing, turning.",
        ])

    agenda(deck, L)

    divider(deck, L, "Lights that talk")

    deck.table(
        "The language of car lights",
        ["Light", "Where, and what color", "It says..."],
        [["Headlights", "Front, white", "\"I'm here, and I can see the "
          "road.\""],
         ["Tail lights", "Back, dim red", "\"I'm here. This is my back.\""],
         ["Brake lights", "Back, BRIGHT red", "\"I'm stopping!\""],
         ["Backup lights", "Back, white", "\"I'm backing up!\""],
         ["Turn signal", "Blinking orange, one side",
          "\"I'm about to turn THIS way.\""]],
        col_widths=[2.6, 3.6, 4.8],
        speaker=[
            "Lights are about OTHER people: drivers behind you, people "
            "crossing the street. Ask who the backup light is for. "
            "(Anybody behind the car - especially small people who are "
            "hard to see.)",
        ])

    dx.smart_light_states(deck)

    deck.image_pair(
        "Pathfinder's turn signals",
        img("vehicle-turn-signal-left.jpg"),
        "LEFT turn signal, seen from the front",
        img("vehicle-turn-signal-right.jpg"),
        "RIGHT turn signal - same vehicle, same program",
        speaker=[
            "Both photographs show the front of the vehicle, so you can "
            "only see the front lights. The back lights on the same side "
            "are blinking too, just like a car.",
        ])

    divider(deck, L, "Lights from driving")

    deck.code(
        "Smart lights",
        ["  lightsOff();               // Fresh picture",
         "",
         "  setFrontLights(WHITE);     // Headlights",
         "",
         "  if (forward > 0) {",
         "    setBackLights(mixColor(60, 0, 0));",
         "  } else if (forward < 0) {",
         "    setBackLights(WHITE);    // Backup",
         "  } else {",
         "    setBackLights(RED);      // Brakes",
         "  }"],
        filename="e10a_smart_lights.ino",
        notes=[("Every time round loop(), draw a whole new picture.", 0),
               ("else if is a second question, asked only if the first "
                "answer was no.", 0),
               ("Forward? Backward? Neither? Then we're stopped.", 0)],
        speaker=[
            "The program reads the same stick numbers it drives with. "
            "forward bigger than zero means going forward.",
            "'else if' makes a three-way choice. Exactly ONE of the three "
            "back-light lines runs each time round.",
            "Why lightsOff() first? So that lights from last time - like a "
            "turn signal - don't stay stuck on. The engine room only sends "
            "the finished picture to the lights, so they don't flicker.",
        ])

    mission(
        deck, L, "0:12", "Smart lights", "e10a_smart_lights",
        [("Upload, wheels up. Connect your controller.", 0),
         ("Push the stick forward, let go, then pull back. Watch the BACK "
          "lights.", 0),
         ("LEVEL 2: find the turn signal lines (line {}). Remove the // at "
          "the start of each one. Upload.".format(
              line("e10a_smart_lights", "// LEVEL 2: turn signals")), 0),
         ("Steer left and right. Is the orange on the correct side?", 0),
         ("Working? Unplug, and try it on the floor.", 0)],
        expect=[("Dim red going forward, bright red stopped, white in "
                 "reverse", 0),
                ("Orange on the side you steer toward", 0)],
        questions=[("Why does the program start with lightsOff() every "
                    "time?", 0),
                   ("How does the program know you've stopped?", 0)],
        speaker=[
            "Un-commenting is satisfying and quick, but watch for teams "
            "that remove the // from the LEVEL 2 heading too - that line "
            "isn't code and won't compile. The error message will point "
            "straight at it.",
            "Answers: so old lights don't stay on; forward and turn are "
            "both 0.",
            "The turn signals are solid, not blinking, at this stage. "
            "That's the next problem.",
        ])

    divider(deck, L, "Blinking")

    deck.bullets(
        "Blinking: the obvious way is a trap",
        [("You might try: lights on, delay(400), lights off, "
          "delay(400).", 0),
         ("But while delay() waits, loop() is STUCK. It can't read the "
          "stick.", 0),
         ("Your steering would lag almost a second behind your thumb!", 0)],
        note="Never put delay() inside a driving program's loop().",
        note_kind="warn",
        speaker=[
            "If you want to prove it, add a delay(400) to loop() on your "
            "own vehicle and let a student try to drive it. It's "
            "horrible, and they'll never forget it. Take it out "
            "afterwards.",
        ])

    dx.nap_vs_clock(deck)

    deck.code(
        "blinkIsOn(): glance at the clock",
        ["// Looks at the clock instead of waiting",
         "bool blinkIsOn() {",
         "  return (millis() / 400) % 2 == 0;",
         "}"],
        filename="explorer.h  -  the engine room",
        notes=[("millis() is a clock: milliseconds since the rover woke "
                "up.", 0),
               ("Divide by 400. Is the answer even, or odd?", 0),
               ("Even: true. Odd: false. It flips every 0.4 seconds.", 0),
               ("No waiting at all.", 0)],
        speaker=[
            "% means 'the remainder after dividing'. Any number % 2 is 0 "
            "for even numbers and 1 for odd ones. That's for your older "
            "students; the younger ones need only 'it flips every 0.4 "
            "seconds, and answers instantly'.",
            "Worked example: at 1000 ms, 1000 / 400 = 2 (computers drop "
            "the leftover), and 2 is even, so the light is on. At 1300 ms, "
            "1300 / 400 = 3, odd, off.",
        ])

    mission(
        deck, L, "0:37", "Blinking signals, and a headlight switch",
        "e10a_smart_lights",
        [("LEVEL 3: change  if (turn > 0)  to  if (turn > 0 && "
          "blinkIsOn()). Do the same for the left.", 0),
         ("Upload. Do the signals blink? Can you still steer smoothly?", 0),
         ("HEADLIGHT SWITCH: add  bool headlightsOn = true;  above "
          "setup().", 0),
         ("Flip it with tappedY(), and only light the front if headlightsOn "
          "is true.", 0),
         ("Stuck? See how lightsOn works in e01a_learner_drive.", 0),
         ("Then: drive the test course, signaling every turn.", 0)],
        expect=[("Orange that blinks on the side you're turning", 0),
                ("Y switches the headlights off and on", 0)],
        questions=[("Why does && blinkIsOn() make it blink?", 0)],
        safety="Floor driving inside the arena only. Everybody behind the "
               "line.",
        speaker=[
            "The headlight switch, for you: bool headlightsOn = true; at "
            "the top. In loop(): if (tappedY()) { headlightsOn = "
            "!headlightsOn; } and wrap setFrontLights(WHITE) in if "
            "(headlightsOn) { ... }. The ! means 'the opposite of'.",
            "Answer: && means both must be true - turning right AND the "
            "clock says 'on'. Half the time the clock says off, so the "
            "light goes off.",
            "Teams that finish can attempt the Expert License: the test "
            "course at {}, with a signal on every turn.".format(LICENSED),
        ])

    deck.bullets(
        "The Expert License",
        [("Drive the test course at SPEED_LIMIT {}.".format(LICENSED), 0),
         ("Signal EVERY turn, on the correct side.", 0),
         ("No cups knocked over. All four wheels in the garage.", 0),
         ("Pass, and your teacher lets you raise SPEED_LIMIT to "
          "{}: FULL POWER.".format(EXPERT), 0)],
        note="Full power is FAST. Arena only, and the Explorer Code still "
             "applies.",
        note_kind="safety",
        speaker=[
            "This is the top of the license ladder, and it's earned, not "
            "given. A team that drives recklessly at full power goes back "
            "to {}. Say so before anyone takes the test.".format(LICENSED),
            "At full power the vehicle is genuinely quick. Make the arena "
            "bigger, or run Expert drives one vehicle at a time.",
        ])

    new_words(deck, [
        ["Signal", "A light that tells other people what you'll do next"],
        ["else if", "A second question, asked only if the first answer was "
         "no"],
        ["millis()", "A clock: milliseconds since the rover woke up"],
        ["blinkIsOn()", "Glances at the clock: on, off, on, off..."],
        ["bool", "A variable that holds true or false"],
    ])

    deck.quiz(
        "Check yourself",
        [("1.  Bright red at the back of a car. What does it mean?", 0),
         ("2.  Why does the light picture start with lightsOff()?", 0),
         ("3.  Why can't we blink with delay() while driving?", 0),
         ("4.  What does blinkIsOn() do?", 0),
         ("5.  The vehicle turns right. Which lights blink?", 0)],
        speaker=[
            "Answers: 1 stopping. 2 so old lights don't stay on. 3 loop() "
            "stops reading the stick while it waits. 4 tells you on or "
            "off, flipping every 0.4 s, without waiting. 5 the right side, "
            "front and back.",
        ])

    wrap_up(deck,
            ["Fill in the light rules table",
             "Expert License stamp, if you passed"],
            "Signal Expert",
            speaker=["Next lesson is the design challenge. Ask teams to "
                     "come with an upgrade idea in mind."])


# ===================================================================
# LESSON 11 - DESIGN CHALLENGE
# ===================================================================

def lesson11(deck):
    L = "lesson11"
    title(deck,
          "You can make it light up, move, listen and signal. Now upgrade it "
          "with your team's own idea, built the way engineers build things.",
          img("vehicle-with-arm.jpg"),
          speaker=[
              "Today is the most open lesson of the course, and the "
              "noisiest. Spend your time on the teams that are stuck, not "
              "the ones that are flying.",
              "Make sure every team saves its own copy of e11a with File > "
              "Save As, so Mission Day starts from their work, not from a "
              "fresh copy.",
          ])

    todays_mission(deck, [
        "Use the engineering design process: ask, imagine, plan, create, "
        "test, improve",
        "Plan a change to a program on paper, before typing it",
        "Build and test an upgrade one small step at a time",
        "Show your upgrade, and explain how it works",
    ], speaker=["Read them out, and say plainly that the upgrade working "
                "perfectly is NOT the goal - using the process is."])

    deck.image_pair(
        "Engineers upgrade their rovers",
        img("vehicle-with-arm.jpg"),
        "A robot arm, for picking things up",
        img("vehicle-with-pan-tilt.jpg"),
        "A mount that can point a camera around",
        lead="These Pathfinders got new hardware. Today you upgrade the "
             "SOFTWARE - no new parts needed.",
        speaker=[
            "Both upgrades use the vehicle's servo outputs, which this "
            "course hasn't touched. Students who catch the bug can go on "
            "to the Beginner course, which does.",
        ])

    agenda(deck, L)

    divider(deck, L, "Ask and imagine")

    dx.design_cycle(deck)

    deck.table(
        "Pick your upgrade",
        ["Upgrade", "What it does", "How hard?"],
        [["Team light show", "Your own pattern when the rover wakes up",
          "Easy"],
         ["Headlight flash", "Hold A: headlights flash bright white",
          "Easy"],
         ["Party mode", "Tap X: rainbow lights on and off", "Medium"],
         ["Hazard lights", "Tap B: BOTH sides blink, like a stopped car",
          "Medium"],
         ["Crawl mode", "Hold ZL: half speed, for careful parking",
          "Medium"],
         ["Victory dance", "A button runs an autopilot dance", "Hard"],
         ["Your own idea", "Check it with your teacher first", "You "
          "decide!"]],
        col_widths=[2.8, 6.2, 2.0],
        speaker=[
            "Steer younger or less confident teams toward Easy or Medium. "
            "A finished Easy upgrade beats a half-built Hard one.",
            "Your own idea: say yes to almost anything that only changes "
            "lights, buttons and moves. Say no to anything that raises "
            "the speed limit past the team's license.",
        ])

    divider(deck, L, "Plan")

    deck.bullets(
        "Plan before you type",
        [("In your Mission Log design sheet:", 0),
         ("Which button? Held, or tapped?", 1),
         ("What should happen? Draw it!", 1),
         ("Which UPGRADE ZONES will you change?", 1),
         ("How will you TEST that it works?", 1),
         ("Show your plan to your teacher before you start typing.", 0)],
        note="Teams that plan finish faster. Really.",
        speaker=[
            "Make the plan check a real gate: a team can't start typing "
            "until you've looked at their sheet for thirty seconds. It's "
            "the single best way to stop teams wandering.",
        ])

    deck.code(
        "Where to build",
        ["  // ===== BUTTONS =====",
         "  if (tappedY()) {",
         "    headlightsOn = !headlightsOn;",
         "  }",
         "",
         "  // ===== UPGRADE ZONE 2 =====",
         "  // if (tappedX()) {",
         "  //   partyMode = !partyMode;",
         "  // }"],
        filename="e11a_my_upgrade.ino",
        notes=[("Zone 1, at the top: your true/false switches.", 0),
               ("Zone 2, here: your buttons.", 0),
               ("Zone 3, at the bottom: your lights.", 0),
               ("The examples are already written. Remove the // to use "
                "them.", 0)],
        speaker=[
            "Party mode is fully sketched in the comments across the three "
            "zones, so a team that wants a quick win can un-comment it and "
            "then make it their own.",
        ])

    deck.two_columns(
        "Building blocks you already know",
        "To remember ON or OFF",
        [("Zone 1:  bool partyMode = false;", 0),
         ("Zone 2:  if (tappedX()) {", 0),
         ("partyMode = !partyMode;", 1),
         ("}", 0),
         ("Zone 3:  if (partyMode) {  ...  }", 0)],
        "Handy tools",
        [("blinkIsOn()  -  blinking", 0),
         ("rainbowColor(number * 3)  -  a rainbow", 0),
         ("randomColor()  -  surprise colors", 0),
         ("rumble(200)  -  a buzz", 0),
         ("forward(), spinLeft()  -  autopilot moves", 0)],
        speaker=[
            "This is the pattern behind most upgrades: a true/false "
            "switch, a button that flips it, and an if that does something "
            "while it's on.",
        ])

    divider(deck, L, "Create and test")

    mission(
        deck, L, "0:20", "Build your upgrade", "e11a_my_upgrade",
        [("Open e11a_my_upgrade. File > Save As, with your team name.", 0),
         ("Build ONE small piece. Upload. Test it, wheels up.", 0),
         ("Works? Build the next piece. Doesn't? Fix it before adding "
          "more.", 0),
         ("Write each test in your Mission Log.", 0),
         ("Finished? Add a second upgrade, or help another team.", 0)],
        expect=[("Your upgrade working, one piece at a time", 0)],
        questions=[("What was the first thing that didn't work?", 0),
                   ("How did you find out why?", 0)],
        safety="Dances and autopilot moves: floor only, inside the arena, "
               "everybody behind the line.",
        speaker=[
            "Hazard lights, for you: bool hazards = false; in Zone 1; if "
            "(tappedB()) { hazards = !hazards; } in Zone 2; in Zone 3: if "
            "(hazards && blinkIsOn()) { setLeftLights(ORANGE); "
            "setRightLights(ORANGE); }",
            "Crawl mode: in the DRIVING section, after the wobble zone: if "
            "(buttonZL()) { forward = forward / 2; turn = turn / 2; } - "
            "e12a_mission_day has exactly this.",
            "Victory dance: write a function at the bottom with spins and "
            "light flashes, and call it from if (tappedX()) in Zone 2. "
            "Holding B stops any autopilot move.",
        ])

    deck.bullets(
        "Stuck? Try these",
        [("Click Verify. Read the FIRST error message, and its line "
          "number.", 0),
         ("Count your braces. Every { needs a }.", 0),
         ("Is it running the program you think? Check the tab name.", 0),
         ("Add  Serial.println(\"got here\");  to see if a part of your code "
          "runs.", 0),
         ("Explain your code, line by line, to a teammate. Or to a rubber "
          "duck!", 0)],
        speaker=[
            "Rubber duck debugging is a real thing professional "
            "programmers do: explaining your code out loud, even to a "
            "toy, very often makes the bug jump out. If you have a duck, "
            "put it on the desk.",
        ])

    deck.bullets(
        "Show and tell",
        [("Every team gets one minute:", 0),
         ("What does your upgrade do? Show it!", 1),
         ("What went wrong, and how did you fix it?", 1),
         ("What would you add next?", 1)],
        speaker=[
            "Clap for every team, working upgrade or not. The second "
            "question is the one to praise most - fixing something is the "
            "skill.",
        ])

    deck.quiz(
        "Check yourself",
        [("1.  Name the six steps of the design process.", 0),
         ("2.  Why test after every small change?", 0),
         ("3.  What does the ! do in  partyMode = !partyMode; ?", 0),
         ("4.  Name one way to find a bug.", 0)],
        speaker=[
            "Answers: 1 ask, imagine, plan, create, test, improve. 2 so "
            "when something breaks you know which change did it. 3 flips "
            "it to the opposite - true becomes false, false becomes true. "
            "4 any of the 'stuck' slide.",
        ])

    wrap_up(deck,
            ["Finish your design sheet and test log",
             "Write one thing you'd improve next"],
            "Design Engineer",
            speaker=[
                "Next lesson is Mission Day. Charge every battery, and "
                "check every team's upgrade still drives safely.",
            ])


# ===================================================================
# LESSON 12 - MISSION DAY
# ===================================================================

def lesson12(deck):
    L = "lesson12"
    title(deck,
          "Four stations, one vehicle, your whole team. Today you show "
          "everything you've learned.",
          img("vehicle-turn-signal-left.jpg"),
          speaker=[
              "BEFORE CLASS: build the Mission Day course, charge every "
              "battery and controller, print the certificates with "
              "students' names filled in, and set out chairs for any "
              "families.",
              "Have a spare, known-good vehicle loaded with e12a_mission_day "
              "in case one breaks.",
          ])

    todays_mission(deck, [
        "Drive the Mission Day course with your team",
        "Cross the no-signal zone on autopilot",
        "Show your upgrade, and explain how it works",
        "Look back at how far you've come",
    ], speaker=["Read them out, and welcome any families properly. Ask the "
                "students to explain to them what a Learner Permit is."])

    deck.bullets(
        "Mission briefing",
        [("Twelve lessons ago, you had never programmed a robot.", 0),
         ("Today your team drives a course with four stations - including "
          "one where the radio 'doesn't work'.", 0),
         ("Then you show off the upgrade you designed.", 0),
         ("Everybody drives. Everybody explains something.", 0)],
        speaker=[
            "If families are here, this is your welcome. Two sentences on "
            "what the course was, then hand over to the students.",
        ])

    agenda(deck, L)

    dx.mission_day_course(deck)

    deck.table(
        "The four stations",
        ["Station", "What to do", "Done when..."],
        [["1  Slalom", "Weave through the cups", "No cups knocked over"],
         ["2  No-signal zone", "Stop at the line. Press X. Hands off!",
          "Autopilot gets you across"],
         ["3  Signal check", "Signal, then turn", "Orange on the correct "
          "side"],
         ["4  Precision parking", "Park in the garage",
          "All four wheels inside"]],
        col_widths=[3.2, 4.6, 3.6],
        note="Every station is 'done' or 'try again'. Nobody loses on "
             "Mission Day.",
        speaker=[
            "Keep it non-competitive. You can time teams if your group "
            "loves that, but a single stopwatch makes the slowest team "
            "the story of the day.",
        ])

    deck.image_slide(
        "Our course",
        MISSION_DAY,
        caption="Stations 1 to 4, then home",
        speaker=["Walk the course on foot with the class before any "
                 "vehicle goes on it."])

    divider(deck, L, "Final checks")

    deck.bullets(
        "Which program?",
        [("For the four stations: e12a_mission_day, with YOUR autopilot "
          "route in it.", 0),
         ("For the showcase: your own upgrade from Lesson 11.", 0),
         ("Check SPEED_LIMIT matches your license.", 0)],
        note="Real mission teams always carry a backup plan. "
             "e12a_mission_day is yours.",
        speaker=[
            "e12a has everything the course taught, working, plus an "
            "autopilot button. Using it for the stations means a broken "
            "upgrade can't spoil a team's mission.",
            "If a team's own program already has a reliable autopilot "
            "button, let them use it for the stations instead.",
        ])

    deck.code(
        "The autopilot route",
        ["void runAutopilot() {",
         "  Serial.println(\"AUTOPILOT ON. Hold B to stop.\");",
         "  setAllLights(CYAN);",
         "  pause(1000);",
         "",
         "  forward(1500);",
         "  spinLeft(QUARTER_TURN);",
         "  forward(1000);",
         "  spinRight(QUARTER_TURN);",
         "  forward(1000);",
         "",
         "  stopMotors();",
         "  Serial.println(\"Autopilot finished. You have control.\");",
         "}"],
        filename="e12a_mission_day.ino",
        notes=[("Press X, and these moves run.", 0),
               ("Change them to fit the no-signal zone.", 0),
               ("Hold B, and the autopilot stops. B is for Brake.", 0)],
        speaker=[
            "The engine room keeps listening during autopilot moves: "
            "holding B stops the vehicle at once and skips the rest of "
            "the route. Show this once so everyone knows it works.",
            "QUARTER_TURN should be each team's tuned number from Lesson "
            "6.",
        ])

    mission(
        deck, L, "0:05", "Pre-mission checks", "e12a_mission_day",
        [("Crew Chief: battery charged? Wheels firm? All 32 lights "
          "working?", 0),
         ("Put your QUARTER_TURN and SPEED_LIMIT into e12a_mission_day.", 0),
         ("Measure the no-signal zone. Change the moves in "
          "runAutopilot().", 0),
         ("Test the autopilot ONCE on the practice line. Hold B if it goes "
          "wrong.", 0),
         ("Pilot: one practice lap.", 0)],
        expect=[("A vehicle that's ready, and a team that knows its "
                 "route", 0)],
        questions=[("Who drives which station?", 0)],
        safety="Practice runs one vehicle at a time, everybody else behind "
               "the line.",
        speaker=[
            "Ten minutes goes fast. If a team's autopilot isn't right "
            "by the end, it can still cross the zone - a team member "
            "carries the 'radio' across. Never let a station become a "
            "dead end.",
        ])

    divider(deck, L, "The mission")

    deck.bullets(
        "Mission rules",
        [("One team on the course at a time.", 0),
         ("Everybody else: behind the line, cheering.", 0),
         ("The Crew Chief calls GO, and calls STOP if anything looks "
          "wrong.", 0),
         ("A new Pilot at every station. Everybody drives.", 0)],
        note="The Explorer Code still applies - even on Mission Day.",
        note_kind="safety",
        speaker=[
            "Narrate each team's run like a sports commentator - with "
            "names. It's the most memorable thing you'll do all course.",
        ])

    divider(deck, L, "Look back")

    deck.table(
        "Look how far you've come",
        ["Badge", "You learned to..."],
        [["Learner Permit", "Drive safely, and follow the Explorer Code"],
         ["Bug Hunter", "Upload a program, and fix bugs"],
         ["Color Scientist", "Mix any color from red, green and blue"],
         ["Light Show Designer", "Use loops to animate lights"],
         ["Motor Mechanic", "Drive motors forward, backward, and slow"],
         ["Autopilot", "Write a plan, then tune it"],
         ["Mission Planner", "Invent your own functions"],
         ["Radio Operator", "Make choices with if and else"],
         ["Licensed Driver", "Turn stick numbers into driving"],
         ["Signal Expert", "Blink without stopping the program"],
         ["Design Engineer", "Plan, build, test and improve"]],
        col_widths=[3.6, 7.5],
        speaker=[
            "Read a few out and ask students to stand when you read one "
            "they were proud of. Then hand out the Finish Line "
            "questionnaire while they're thinking about it.",
        ])

    checkin(deck, "Finish")

    divider(deck, L, "Celebrate")

    deck.bullets(
        "Upgrade showcase",
        [("Each team, one minute:", 0),
         ("Show your upgrade.", 1),
         ("Explain ONE line of your code.", 1),
         ("Tell us about a bug you fixed.", 1),
         ("Then: certificates!", 0)],
        speaker=[
            "Explaining one line is the bit families remember. Help the "
            "younger students pick a line beforehand.",
            "Hand certificates out one at a time, with each student's "
            "name and one specific thing they did well.",
        ])

    deck.bullets(
        "What's next?",
        [("The Pathfinder Beginner course: write the full driving program "
          "yourself, line by line.", 0),
         ("Add sensors: a vehicle that stops by itself before it hits a "
          "wall.", 0),
         ("Robotics clubs and teams at your school.", 0),
         ("Ocean robots! Porpoise Robotics builds robots that explore the "
          "sea.", 0)],
        speaker=[
            "If you have dates for the next course, put them on the board.",
            "Students who want more right now: the Arduino website has "
            "free project ideas, and every program in this course can be "
            "read and changed at home if they have an ESP32.",
        ])

    deck.bullets(
        "Thank you, Explorers!",
        [("You wrote real code, fixed real bugs, and drove a robot you "
          "programmed yourself.", 0),
         ("Your questionnaire answers will make this course better for the "
          "next Explorers.", 0),
         ("Keep exploring!", 0)],
        speaker=[
            "End here. Thank the families for coming, and the students for "
            "their honest answers all course. If the Halfway Check-in led "
            "you to change something, tell them what - it shows them their "
            "answers counted.",
            "The Finish Line forms go into questionnaire_tracker.xlsx, and "
            "the README describes the cohort review that turns them into "
            "changes for next time.",
        ])

    wrap_up(deck,
            ["Your Mission Day results",
             "Your last page: what you're proudest of"],
            "Pathfinder Explorer",
            pack=[("Power OFF. Last time!", 0),
                  ("Batteries go to your teacher.", 0),
                  ("Vehicles and controllers back together, stickers "
                   "matching.", 0),
                  ("Take your Mission Log and certificate home!", 0)],
            speaker=[
                "Last pack-up. Batteries come to you, every one.",
                "Let students take the Mission Log home. It's the record "
                "of everything they did.",
            ])


# ===================================================================
# BUILD
# ===================================================================

# Every sketch that drives the wheels has to start at the Learner limit. A
# sketch shipped at a higher limit would hand a nine-year-old more speed
# than they have earned, so the build checks rather than trusts.
MOVING_SKETCHES = [
    "e01a_learner_drive", "e05a_motor_lab", "e06a_drive_a_square",
    "e07a_mission_planner", "e09a_joystick_drive", "e10a_smart_lights",
    "e11a_my_upgrade", "e12a_mission_day",
]


def enrich():
    """
    Reads the numbers the slides quote out of the sketches, and stops the
    build if any sketch's copy of explorer.h has drifted from the master.
    """
    stale = sync_explorer_h.check()
    if stale:
        raise SystemExit(
            "explorer.h differs from the master in %s.\nRun "
            "sync_explorer_h.py, then build again." % ", ".join(stale))

    FACTS["learner"] = srcfacts.number(sketch("e01a_learner_drive"),
                                       "SPEED_LIMIT")
    for name in MOVING_SKETCHES:
        limit = srcfacts.number(sketch(name), "SPEED_LIMIT")
        if limit != FACTS["learner"]:
            raise SystemExit(
                "%s ships with SPEED_LIMIT = %s, but the Learner limit is "
                "%s. Students raise it themselves when they earn it."
                % (name, limit, FACTS["learner"]))

    FACTS["pwm_freq"] = srcfacts.number(ENGINE, "MOTOR_PWM_FREQ")
    FACTS["turn_time"] = srcfacts.number(sketch("e06a_drive_a_square"),
                                         "TURN_TIME")
    return FACTS


LESSONS = [
    ("l01_meet_pathfinder.pptx", "Lesson 1", "Meet Pathfinder", lesson1),
    ("l02_talking_to_robots.pptx", "Lesson 2", "Talking to Robots", lesson2),
    ("l03_color_lab.pptx", "Lesson 3", "Color Lab", lesson3),
    ("l04_loops_and_light_shows.pptx", "Lesson 4", "Loops and Light Shows",
     lesson4),
    ("l05_make_it_move.pptx", "Lesson 5", "Make It Move", lesson5),
    ("l06_drive_by_code.pptx", "Lesson 6", "Drive by Code", lesson6),
    ("l07_mission_to_mars.pptx", "Lesson 7", "Mission to Mars", lesson7),
    ("l08_wireless_control.pptx", "Lesson 8", "Wireless Control", lesson8),
    ("l09_joystick_driving.pptx", "Lesson 9", "Joystick Driving", lesson9),
    ("l10_smart_lights.pptx", "Lesson 10", "Smart Lights", lesson10),
    ("l11_design_challenge.pptx", "Lesson 11", "Design Challenge", lesson11),
    ("l12_mission_day.pptx", "Lesson 12", "Mission Day", lesson12),
]


def build(out_dir):
    made = []
    enrich()
    for filename, label, deck_title, builder in LESSONS:
        deck = Deck(filename, deck_title, label, TRACK_LABEL)
        builder(deck)
        made.append(deck.save(out_dir))
    return made
