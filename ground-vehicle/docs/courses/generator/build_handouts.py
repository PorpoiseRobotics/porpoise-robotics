"""
build_handouts.py - the printed handouts for Pathfinder Explorers.

    python build_handouts.py                       all three handouts
    python build_handouts.py --names names.txt     certificates for a class:
                                                   one page per name, one name
                                                   per line in names.txt

Writes into docs/courses/explorers/handouts/:

    mission_log.docx                 the student workbook, a page or two a lesson
    letter_to_families.docx          a letter home, with [BRACKETED] blanks
    certificate.pptx                 the certificate of completion
    questionnaire_1_starting_line.docx   start of Lesson 1
    questionnaire_2_halfway.docx         start of Lesson 7
    questionnaire_3_finish_line.docx     Lesson 12, after the mission
    questionnaire_tracker.xlsx       where the answers go, and the growth they show

Like the decks, these files are the deliverable. Once anybody has edited one
in Word, PowerPoint or Excel, edit it there - re-running this overwrites it.
The questionnaire wording lives in questionnaires.py, so the three forms and
the tracker always agree.

The Mission Log's glossary and badge names are read out of the BUILT DECKS
(the "New words" tables and the "Stamp:" lines), so build the decks first.
That way the workbook cannot teach a word the slides never used, or award a
badge the slides call something else.
"""

import os
import sys

from pptx import Presentation
from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_SHAPE
from pptx.enum.text import MSO_ANCHOR, PP_ALIGN
from pptx.util import Inches, Pt

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import content_explorers as course          # noqa: E402
import questionnaires as q                  # noqa: E402
from docxlite import (AMBER, GREY, NAVY, TEAL, Document, Span,  # noqa: E402
                      run)

COURSE_DIR = os.path.normpath(os.path.join(HERE, "..", "explorers"))
OUT = os.path.join(COURSE_DIR, "handouts")
LOGO = os.path.normpath(os.path.join(HERE, "..", "images", "porpoise-logo.png"))

BOX = "☐"          # an empty check box


# ===================================================================
# What the decks say
# ===================================================================

def read_decks():
    """
    The New words rows and the badge for every lesson, from the built decks.
    Returns {lesson number: {"title":..., "badge":..., "words": [...]}}.
    """
    found = {}
    for number, (filename, _label, deck_title, _fn) in enumerate(
            course.LESSONS, 1):
        path = os.path.join(COURSE_DIR, filename)
        if not os.path.exists(path):
            raise SystemExit("Build the decks first: %s is missing.\n"
                             "    python build_decks.py explorers" % filename)
        entry = {"title": deck_title, "badge": None, "words": []}
        for slide in Presentation(path).slides:
            title = slide.shapes.title.text if slide.shapes.title else ""
            for shape in slide.shapes:
                if shape.has_table and title == "New words":
                    rows = list(shape.table.rows)[1:]
                    entry["words"] += [(r.cells[0].text, r.cells[1].text)
                                       for r in rows]
                if shape.has_text_frame:
                    for para in shape.text_frame.paragraphs:
                        if para.text.startswith("Stamp: "):
                            entry["badge"] = para.text[len("Stamp: "):]
        if not entry["badge"]:
            raise SystemExit("%s has no 'Stamp:' line on its wrap-up slide."
                             % filename)
        found[number] = entry
    return found


# ===================================================================
# The Mission Log
# ===================================================================

def lesson_heading(doc, number, decks, mission):
    """
    The top of every lesson: the title and today's mission on the left, and
    the badge with room for the teacher's stamp on the right. The badge lives
    up here rather than at the end so that a page that runs a little long
    never pushes a lonely badge box onto a page of its own.
    """
    doc.body.append('<w:p><w:pPr><w:pageBreakBefore/><w:spacing w:before="0" '
                    'w:after="0"/></w:pPr></w:p>')
    doc.table([[[[run("Lesson %d" % number, bold=True, color=TEAL, size=14)],
                 [run(decks[number]["title"], bold=True, color=NAVY,
                      size=24)],
                 [run("Today's mission: ", bold=True, color=TEAL, size=12),
                  run(mission, size=12)]],
                [[run("BADGE", bold=True, color=TEAL, size=10)],
                 [run(decks[number]["badge"], bold=True, color=NAVY,
                      size=13)],
                 [run("teacher's stamp", italic=True, color=GREY, size=9)]]]],
              [5.2, 1.9], borders="none", heights=1.05,
              fills={(0, 1): "EEF3F8"}, align=["left", "center"])


def short_answer(doc, question, count=2):
    doc.para([run(question, bold=True)], after=0, keep_next=True)
    doc.table([[""] for _ in range(count)], [doc.text_width()],
              borders="lines", heights=0.4)


def mission_log(decks):
    learner = course.FACTS["learner"]
    doc = Document(base_size=13, footer="Pathfinder Explorers  -  Mission Log")

    # ---- Cover --------------------------------------------------------
    doc.spacer(30)
    if os.path.exists(LOGO):
        doc.picture(LOGO, 1.8)
    doc.para([run("PATHFINDER EXPLORERS", bold=True, color=TEAL, size=18)],
             align="center", after=0)
    doc.para([run("Mission Log", bold=True, color=NAVY, size=44)],
             align="center", after=24)
    doc.table([[[run("This log belongs to:", bold=True)], ""],
               [[run("Team name:", bold=True)], ""],
               [[[run("Explorer code:", bold=True)],
                 [run("your teacher gives you this", italic=True, color=GREY,
                      size=9)]], ""],
               [[run("Vehicle number:", bold=True)], ""],
               [[run("Teacher:", bold=True)], ""]],
              [2.4, 4.7], borders="lines", heights=0.6, valign="bottom")
    doc.spacer(16)
    doc.table([[[[run("Bring this log to every lesson. Write in it, draw in "
                      "it, and collect a stamp at the end of every "
                      "mission.")]]]],
              [doc.text_width()], heights=0.9, valign="center",
              fills={(0, 0): "EEF3F8"})
    doc.spacer(20)
    doc.para([run("Porpoise Robotics  -  porpoiserobotics.org", color=GREY,
                  size=11)], align="center")

    # ---- Badges -------------------------------------------------------
    doc.heading("My badges", level=1, page_break=True)
    doc.para("Finish a lesson's mission and your teacher stamps its badge.",
             after=10)
    cells = []
    for number in range(1, 13):
        cells.append([[run("Lesson %d" % number, color=GREY, size=11)],
                      [run(decks[number]["badge"], bold=True, color=NAVY,
                           size=14)]])
    rows = [cells[i:i + 3] for i in range(0, 12, 3)]
    doc.table(rows, [2.36, 2.36, 2.38], heights=1.95, align="center")

    # ---- Driver's license ----------------------------------------------
    doc.heading("My driver's license", level=1, page_break=True)
    doc.para([run("SPEED_LIMIT is a percent. At {}, a full push on the stick "
                  "gives half power. At {}, it gives everything the motors "
                  "have.".format(learner, course.EXPERT))], after=10)
    for name, limit, how in (
            ("LEARNER PERMIT", learner,
             "Know the Explorer Code, and finish the driving course."),
            ("DRIVER'S LICENSE", course.LICENSED,
             "Pass the driving test: weave, park, back out, home - no cups "
             "knocked over."),
            ("EXPERT LICENSE", course.EXPERT,
             "Drive the test course at {}, signaling every turn.".format(
                 course.LICENSED))):
        doc.table([[[[run(name, bold=True, color=NAVY, size=17)],
                     [run("SPEED_LIMIT = %d" % limit, bold=True, color=TEAL,
                          font="Consolas")],
                     [run(how)]],
                    [[run("Date passed:", bold=True)], [run("")],
                     [run("Teacher's stamp:", bold=True)]]]],
                  [4.4, 2.7], heights=1.75)
        doc.spacer(8)

    # ---- Team jobs ----------------------------------------------------
    doc.heading("Our team jobs", level=1, page_break=True)
    doc.bullets([
        [run("PILOT  ", bold=True, color=TEAL),
         run("holds the controller and drives. Runs the tests.")],
        [run("CODER  ", bold=True, color=TEAL),
         run("sits at the laptop. Types the changes and uploads.")],
        [run("CREW CHIEF  ", bold=True, color=TEAL),
         run("is in charge of the vehicle: the stand, the power switch, the "
             "cable. Reads the steps out loud. Calls STOP if anything looks "
             "unsafe.")],
    ])
    doc.para("Switch jobs every lesson. Write who does which job:", after=6)
    rows = [["Lesson", "Pilot", "Coder", "Crew Chief"]]
    rows += [[str(n), "", "", ""] for n in range(1, 13)]
    doc.table(rows, [1.0, 2.03, 2.03, 2.04], header=True, heights=0.42)

    # ---- The Explorer Code --------------------------------------------
    doc.heading("The Explorer Code", level=1, page_break=True)
    rules = [
        "WHEELS UP whenever the cable is plugged in. The vehicle sits on its "
        "stand.",
        "Hands, hair and sleeves away from the wheels while the power is on.",
        "Power OFF before you pick it up. Carry it by the bottom plate, never "
        "by the wires.",
        "Drive on the floor, inside the arena. Never on a table.",
        "Batteries and chargers are for grown-ups only.",
        "A puffy, hot or damaged battery - or a burning smell? Power OFF, step "
        "back, tell a grown-up.",
        "Don't stare into the lights.",
    ]
    for index, rule in enumerate(rules, 1):
        doc.para([run("%d.  " % index, bold=True, color=TEAL), run(rule)],
                 after=8, size=14)
    doc.para([run("The Crew Chief can call STOP at any time, and everybody "
                  "stops. No arguing.", bold=True, color=AMBER)],
             before=6, after=18)
    doc.para([run("I promise to follow the Explorer Code.", bold=True)],
             after=4)
    doc.table([[[run("Signed:", bold=True)], "", [run("Date:", bold=True)],
                ""]], [1.0, 3.4, 0.8, 1.9], borders="lines", heights=0.6,
              valign="bottom")

    # ---- Lesson 1 -----------------------------------------------------
    lesson_heading(doc, 1, decks,
                   "meet Pathfinder, learn the Explorer Code, and drive.")
    doc.heading("Robot or not?", level=2)
    doc.table([["Thing", "Robot?", "Why?"]] +
              [[thing, "YES  /  NO", ""] for thing in
               ("A robot vacuum", "A TV remote", "An automatic door",
                "A Mars rover", "A remote-control car", "A calculator")],
              [2.4, 1.4, 3.3], header=True, heights=0.4)
    doc.heading("Every robot does three things", level=2)
    doc.table([[[run("S", bold=True, size=16, color=TEAL)],
                [run("T", bold=True, size=16, color=TEAL)],
                [run("A", bold=True, size=16, color=TEAL)]]],
              [2.36, 2.36, 2.38], heights=0.6)
    doc.box("Draw your team's vehicle, seen from above", 2.3,
            note="Label: the computer, the battery, the 4 motors, the front "
                 "lights, the back lights, and the power switch.")

    # ---- Lesson 2 -----------------------------------------------------
    lesson_heading(doc, 2, decks,
                   "write instructions, upload a program, and hunt bugs.")
    doc.heading("Our Human Robot program", level=2)
    doc.para([run("Use only: ", bold=True),
              run("STEP FORWARD (number)   TURN LEFT   TURN RIGHT   PICK UP",
                  font="Consolas", size=12)], after=4)
    doc.table([[str(n), ""] for n in range(1, 5)], [0.5, 6.6],
              borders="lines", heights=0.38)
    short_answer(doc, "What went wrong the first time?", 1)
    doc.heading("Two parts of every program", level=2)
    doc.table([[[run("setup()", font="Consolas", bold=True)],
                [run("runs ", color=GREY)]],
               [[run("loop()", font="Consolas", bold=True)],
                [run("runs ", color=GREY)]]],
              [1.6, 5.5], borders="lines", heights=0.42, valign="bottom")
    doc.heading("Bug hunt log", level=2)
    doc.table([["Bug", "Line number", "What was wrong?", "How we fixed it"],
               ["1", "", "", ""], ["2", "", "", ""], ["3", "", "", ""]],
              [0.6, 1.3, 2.6, 2.6], header=True, heights=0.45)

    # ---- Lesson 3 -----------------------------------------------------
    lesson_heading(doc, 3, decks,
                   "mix colors with light, and map all 32 lights.")
    doc.heading("Predict, then test", level=2)
    doc.para("Write your guess BEFORE you upload e03a_color_lab.", after=4)
    doc.table([["Light", "Red", "Green", "Blue", "My guess", "What we saw"],
               ["3", "255", "255", "0", "", ""],
               ["4", "0", "255", "255", "", ""],
               ["5", "255", "255", "255", "", ""],
               ["6", "255", "0", "255", "", ""]],
              [0.8, 0.8, 0.9, 0.8, 1.9, 1.9], header=True, heights=0.38)
    doc.heading("Our team color", level=2)
    doc.para([run("mixColor(  ______  ,  ______  ,  ______  )",
                  font="Consolas", size=15)], after=2)
    doc.para([run("red          green          blue", color=GREY,
                  size=11, font="Consolas")], after=8, indent=1.15)
    doc.heading("The light map", level=2)
    doc.para("Run e03b_light_map. Write each light's number in its box. "
             "Light 0 is done for you.", after=4)
    doc.table([[[run("LEFT side", bold=True, color=AMBER)],
                [run("FRONT of the vehicle", bold=True, color=NAVY)],
                [run("RIGHT side", bold=True, color=TEAL)]]],
              [1.8, 3.5, 1.8], borders="none",
              align=["left", "center", "right"])
    front = [["0"] + [""] * 15]
    doc.table(front, [7.1 / 16] * 16, heights=0.45, align="center", size=11)
    doc.table([[""] * 16], [7.1 / 16] * 16, heights=0.45, align="center",
              size=11)
    doc.para([run("BACK of the vehicle", bold=True, color=NAVY)],
             align="center", after=6)
    doc.table([[[run("The back light right behind front light 5 is light "
                     "number:", bold=True)], ""]], [5.6, 1.5],
              borders="lines", heights=0.5, valign="bottom")

    # ---- Lesson 4 -----------------------------------------------------
    lesson_heading(doc, 4, decks,
                   "let a loop do the counting, and design a light show.")
    doc.heading("A for loop, piece by piece", level=2)
    doc.para([run("for (int number = 0; number < 32; number++) {",
                  font="Consolas", size=13, bold=True)], after=6)
    doc.table([[[run("Where does it START?", bold=True)], ""],
               [[run("When does it STOP?", bold=True)], ""],
               [[run("How does it STEP?", bold=True)], ""]],
              [2.6, 4.5], borders="lines", heights=0.5, valign="bottom")
    short_answer(doc, "What happened when we deleted lightsOff()? Why?", 1)
    doc.heading("Design your pattern", level=2)
    doc.para("Color in the front lights for each step. Then build it!",
             after=4)
    grid = [["Step"] + [str(n) for n in range(16)]]
    grid += [[str(step)] + [""] * 15 + [""] for step in range(1, 6)]
    doc.table(grid, [0.62] + [6.48 / 16] * 16, header=True, heights=0.38,
              align="center", size=10)
    doc.table([[[run("Our pattern's name:", bold=True)], ""]], [2.2, 4.9],
              borders="lines", heights=0.5, valign="bottom")

    # ---- Lesson 5 -----------------------------------------------------
    lesson_heading(doc, 5, decks,
                   "make the wheels move - safely, wheels up.")
    doc.heading("Predict each step", level=2)
    doc.table([["Step", "Which wheels turn? Which way?", "Right?"]] +
              [[[run(code, font="Consolas", size=12)], "",
                "YES  /  NO"] for code in
               ("drive(50, 0)", "drive(0, 50)", "drive(50, 50)",
                "drive(-50, -50)", "drive(50, -50)")],
              [2.0, 3.6, 1.5], header=True, heights=0.42)
    doc.table([[[run("Our wake-up speed:", bold=True)], ""],
               [[run("To turn RIGHT, this side goes faster:", bold=True)],
                [run("LEFT   /   RIGHT")]]],
              [4.4, 2.7], borders="lines", heights=0.55, valign="bottom")
    doc.heading("Our wheel dance", level=2)
    doc.table([["Step", "drive(left, right)", "delay( )", "What it looks like"]]
              + [[str(n), "", "", ""] for n in range(1, 5)],
              [0.7, 2.4, 1.4, 2.6], header=True, heights=0.4)

    # ---- Lesson 6 -----------------------------------------------------
    lesson_heading(doc, 6, decks,
                   "drive a square on autopilot, and tune it until it comes "
                   "home.")
    doc.heading("Milliseconds", level=2)
    doc.table([[[run("1 second = ", bold=True)], "", [run("3 seconds = ",
                                                          bold=True)], "",
                [run("half a second = ", bold=True)], ""]],
              [1.2, 1.0, 1.25, 1.0, 1.65, 1.0], borders="lines",
              heights=0.5, valign="bottom", size=12)
    doc.heading("Tuning table", level=2)
    doc.para("Change TURN_TIME by 50 at a time. Write down every test.",
             after=4)
    doc.table([["Try", "TURN_TIME", "Where did it end up?",
                "Too much or too little?"]] +
              [[str(n), "", "", ""] for n in range(1, 7)],
              [0.7, 1.5, 2.9, 2.0], header=True, heights=0.42)
    doc.table([[[run("Our best TURN_TIME:", bold=True, size=16,
                     color=NAVY)], ""]], [3.4, 3.7], heights=0.6,
              valign="center")
    short_answer(doc, "Why might another team's best TURN_TIME be different?")

    # ---- Lesson 7 -----------------------------------------------------
    lesson_heading(doc, 7, decks,
                   "plan a route to Mars, invent your own moves, and collect "
                   "a sample.")
    doc.table([[[run("One square takes this many milliseconds:", bold=True)],
                ""]], [4.6, 2.5], borders="lines", heights=0.5,
              valign="bottom")
    doc.heading("Our route", level=2)
    doc.para("Draw START, the crater, the SAMPLE zone, and your route. "
             "One box = one square.", after=4)
    doc.table([[""] * 9 for _ in range(6)], [7.1 / 9] * 9, heights=0.79)
    doc.heading("Our mission plan", level=2, page_break=True)
    plan = [["#", "Move", "#", "Move"]]
    plan += [[str(n), "", str(n + 6), ""] for n in range(1, 7)]
    doc.table(plan, [0.45, 3.1, 0.45, 3.1], header=True, heights=0.45)
    doc.heading("Mission debrief", level=2, page_break=False)
    short_answer(doc, "What went right?", 1)
    short_answer(doc, "What went wrong, and how did you fix it?", 2)
    short_answer(doc, "What would you plan differently next time?", 1)

    # ---- Lesson 8 -----------------------------------------------------
    lesson_heading(doc, 8, decks,
                   "decide what every button on the controller does.")
    doc.heading("Our button plan", level=2)
    doc.table([["Button", "Held or tapped?", "What it does"]] +
              [[b, "", ""] for b in ("A", "B", "X", "Y", "D-pad",
                                     "L and R", "ZL and ZR")],
              [1.5, 1.9, 3.7], header=True, heights=0.42)
    short_answer(doc, "We changed tappedY() to buttonY() and held Y. What was "
                      "different? Why?")
    doc.heading("Our secret combo", level=2)
    doc.para([run("if (  ____________  &&  ____________  ) {",
                  font="Consolas", size=14)], after=4)
    short_answer(doc, "What happens:", 1)

    # ---- Lesson 9 -----------------------------------------------------
    lesson_heading(doc, 9, decks,
                   "turn stick numbers into driving, and take your driving "
                   "test.")
    doc.heading("What the stick sends", level=2)
    doc.table([["Push the stick...", "forward", "turn"]] +
              [[w, "", ""] for w in ("all the way UP", "all the way DOWN",
                                     "all the way RIGHT", "all the way LEFT",
                                     "LET GO")],
              [3.1, 2.0, 2.0], header=True, heights=0.45)
    doc.heading("Wobble zone experiments", level=2)
    doc.table([["WOBBLE_ZONE", "What happened?"], ["0", ""], ["10", ""],
               ["60", ""]], [1.8, 5.3], header=True, heights=0.5)
    doc.heading("Be the computer", level=2, page_break=True)
    doc.table([["forward", "turn", "left = forward + turn",
                "right = forward - turn"],
               ["50", "20", "", ""], ["0", "50", "", ""],
               ["-50", "0", "", ""], ["100", "40", "", ""]],
              [1.2, 1.1, 2.4, 2.4], header=True, heights=0.45)
    doc.heading("Driver's License test", level=2)
    doc.para([run("{0} Weave through the cups     {0} Park in the garage     "
                  "{0} Back out and drive home     {0} No cups knocked over"
                  .format(BOX))], after=6)

    # ---- Lesson 10 ----------------------------------------------------
    lesson_heading(doc, 10, decks,
                   "make lights that signal, like a real car.")
    doc.heading("Our light rules", level=2)
    doc.table([["When the vehicle is...", "Front lights", "Back lights",
                "Turn signals"]] +
              [[w, "", "", ""] for w in ("driving forward", "stopped",
                                         "reversing", "turning right",
                                         "turning left")],
              [2.2, 1.6, 1.6, 1.7], header=True, heights=0.55)
    short_answer(doc, "Why can't we blink the lights with delay() while "
                      "driving?")
    short_answer(doc, "What does blinkIsOn() do?", 1)
    doc.heading("Expert License test", level=2)
    doc.para([run("{0} Course at SPEED_LIMIT {1}     {0} Signaled EVERY turn"
                  "     {0} No cups knocked over".format(BOX,
                                                         course.LICENSED))],
             after=6)

    # ---- Lesson 11 ----------------------------------------------------
    lesson_heading(doc, 11, decks,
                   "design an upgrade, and build it the way engineers do.")
    doc.heading("ASK: what should our upgrade do?", level=2)
    doc.lines(1)
    doc.heading("IMAGINE: our ideas", level=2)
    doc.lines(2)
    doc.heading("PLAN", level=2)
    doc.table([[[run("Which button?", bold=True)], "",
                [run("Held or tapped?", bold=True)], ""]],
              [1.6, 1.9, 1.7, 1.9], borders="lines", heights=0.5,
              valign="bottom")
    doc.para([run("Which zones will we change?   ", bold=True),
              run("{0} Zone 1: switches     {0} Zone 2: buttons     "
                  "{0} Zone 3: lights".format(BOX))], before=8, after=6)
    doc.box("Draw what it will do", 1.5)
    short_answer(doc, "How will we TEST that it works?", 1)
    doc.para([run("{} Teacher checked our plan".format(BOX), bold=True,
                  color=TEAL)], before=6)
    doc.heading("CREATE and TEST: our test log", level=1, page_break=True)
    doc.table([["Test", "What we changed", "What happened", "Next step"]] +
              [[str(n), "", "", ""] for n in range(1, 9)],
              [0.7, 2.2, 2.2, 2.0], header=True, heights=0.75)
    short_answer(doc, "IMPROVE: one thing we'd make better next", 2)

    # ---- Lesson 12 ----------------------------------------------------
    lesson_heading(doc, 12, decks,
                   "show everything you've learned on Mission Day.")
    doc.heading("Our mission", level=2)
    doc.table([["Station", "Pilot", "Done?", "Notes"],
               ["1  Slalom", "", BOX, ""],
               ["2  No-signal zone", "", BOX, ""],
               ["3  Signal check", "", BOX, ""],
               ["4  Precision parking", "", BOX, ""]],
              [2.4, 1.5, 0.9, 2.3], header=True, heights=0.45)
    doc.para([run("We used:   ", bold=True),
              run("{0} e12a_mission_day     {0} our own program".format(BOX))],
             after=8)
    short_answer(doc, "The line of code I explained at the showcase:", 1)
    short_answer(doc, "My proudest moment in this course:", 2)
    short_answer(doc, "Something I'd like to build next:", 2)

    # ---- Glossary -----------------------------------------------------
    doc.heading("Glossary", level=1, page_break=True)
    rows = [["Word", "What it means", "Lesson"]]
    for number in range(1, 13):
        for word, meaning in decks[number]["words"]:
            rows.append([word, meaning, str(number)])
    doc.table(rows, [1.9, 4.4, 0.8], header=True, size=12,
              align=["left", "left", "center"])

    return doc.save(os.path.join(OUT, "mission_log.docx"))


# ===================================================================
# The letter to families
# ===================================================================

def blank(text):
    """A fill-in-the-blank the teacher has to replace, highlighted."""
    return run(text, bold=True, highlight="yellow")


def letter():
    doc = Document(base_size=11.5, margins=0.75)
    if os.path.exists(LOGO):
        doc.picture(LOGO, 0.9, align="left")
    doc.para([run("Pathfinder Explorers", bold=True, color=NAVY, size=20)],
             after=0)
    doc.para([run("A robotics course for ages 9 to 13, from Porpoise "
                  "Robotics", color=TEAL, bold=True, size=13)], after=12)
    doc.para([run("Dear families,")], after=8)
    doc.para([run("Your child is joining "), run("Pathfinder Explorers", bold=True),
              run(", a twelve-lesson course in robotics and programming. In "
                  "teams of three, students share a small four-wheel robot "
                  "vehicle called a Pathfinder. Over twelve one-hour lessons "
                  "they program its 32 lights, its four motors, and its "
                  "wireless controller - and by the last lesson they have "
                  "written the program that drives it.")])
    doc.para("No experience is needed. Everything is provided: the "
             "vehicles, controllers, laptops and batteries.")

    doc.heading("What your child will learn", level=3)
    doc.bullets([
        "How robots sense, think and act - and how to give a computer "
        "exact instructions",
        "Real programming in the Arduino language: loops, choices, "
        "variables and functions",
        "Light and color, how motors work, and why Mars rovers have to "
        "drive themselves",
        "How engineers work: plan, build, test, and fix one thing at a "
        "time - as a team",
    ])

    doc.heading("Course details", level=3)
    doc.table([[[run("Dates", bold=True)], [blank("[DATES]")]],
               [[run("Time", bold=True)], [blank("[START AND END TIME]")]],
               [[run("Location", bold=True)], [blank("[ROOM AND ADDRESS]")]],
               [[run("Instructor", bold=True)], [blank("[NAME]")]],
               [[run("Contact", bold=True)],
                [blank("[EMAIL]"), run("   "), blank("[PHONE]")]],
               [[run("Mission Day", bold=True)],
                [run("Families are invited to Lesson 12, on "),
                 blank("[DATE AND TIME]"),
                 run(", to watch the teams drive the final course.")]]],
              [1.5, 5.5], heights=0.32, size=11.5)

    doc.heading("Safety", level=3)
    doc.para("Every student learns and signs the Explorer Code in the first "
             "lesson. Vehicles sit on a stand whenever they are connected to "
             "a laptop, and drive only on the floor inside a taped arena. "
             "Students earn their speed: every vehicle starts at a reduced "
             "learner speed, raised only after a driving test. The vehicles "
             "run on rechargeable lithium batteries, which only adults "
             "charge or handle.")

    doc.heading("What to wear", level=3)
    doc.para("Closed-toe shoes, and long hair tied back. Please avoid loose "
             "scarves or dangling cords - the wheels turn during some "
             "activities.")

    doc.heading("At home", level=3)
    doc.para("Each student keeps a Mission Log: a workbook with a page for "
             "every lesson and a badge to earn each time. Ask your child to "
             "show it to you, and to explain one thing they programmed. "
             "Explaining is one of the best ways to learn.")

    doc.heading("Three short questionnaires", level=3)
    doc.para("At the start of the course, halfway through, and at the end, "
             "students spend a few minutes on a questionnaire about what they "
             "know, how confident they feel, and which lessons they enjoyed. "
             "It is not a test and is never graded. Students write a code "
             "instead of their name, and the answers are used only to see how "
             "the class is growing and to improve the course.")

    doc.heading("Photos", level=3)
    doc.para([run("[Delete this section if your organization uses its own "
                  "photo release.] ", italic=True, color=GREY),
              run("We would like to photograph the vehicles and the course "
                  "for future lessons. We will only photograph your child "
                  "with your permission. Please return the slip below.")])

    doc.para([run("We look forward to working with your Explorer!")],
             before=4, after=2)
    doc.para([blank("[INSTRUCTOR NAME]"),
              run(", for the Porpoise Robotics team")], after=10)

    doc.para([run("- - - - - - - - - - - - - - - - - - - -  cut here  - - - - "
                  "- - - - - - - - - - - - - - - -", color=GREY, size=9)],
             align="center", after=6)
    doc.table([[[run("Student's name:", bold=True)], "",
                [run("Grade:", bold=True)], ""]],
              [1.5, 3.3, 0.8, 1.4], borders="lines", heights=0.4,
              valign="bottom")
    doc.para([run("{0}  Yes, photos that include my child may be used for "
                  "Porpoise Robotics lessons and materials.".format(BOX))],
             before=6, after=2)
    doc.para([run("{0}  No photos of my child, please.".format(BOX))],
             after=6)
    doc.table([[[run("Parent or guardian:", bold=True)], "",
                [run("Date:", bold=True)], ""]],
              [1.8, 3.2, 0.7, 1.3], borders="lines", heights=0.4,
              valign="bottom")
    return doc.save(os.path.join(OUT, "letter_to_families.docx"))


# ===================================================================
# The questionnaires
# ===================================================================

QUESTIONNAIRE_FILES = {
    "Start": "questionnaire_1_starting_line.docx",
    "Halfway": "questionnaire_2_halfway.docx",
    "Finish": "questionnaire_3_finish_line.docx",
}


def _tick_grid(items, columns=2, size=11):
    """Check-box options laid out in a borderless grid, reading across."""
    cells = ["{}  {}".format(BOX, item) for item in items]
    while len(cells) % columns:
        cells.append("")
    rows = [cells[i:i + columns] for i in range(0, len(cells), columns)]
    return rows, [7.1 / columns] * columns


def _options_line(options):
    return "      ".join("{}  {}".format(BOX, option) for option in options)


def questionnaire(key):
    _key, title, number, _lesson, _when = q.checkpoint(key)
    doc = Document(base_size=12, margins=0.7,
                   footer="Pathfinder Explorers  -  %s questionnaire" % title)

    # ---- Heading and code --------------------------------------------
    doc.para([run("PATHFINDER EXPLORERS   -   questionnaire %d of 3"
                  % number, bold=True, color=TEAL, size=11)], after=0)
    doc.para([run(title, bold=True, color=NAVY, size=24)], after=2)
    doc.table([[[run("Explorer code:", bold=True)], "",
                [run("Date:", bold=True)], ""]],
              [1.5, 2.6, 0.7, 2.3], borders="lines", heights=0.4,
              valign="bottom")
    doc.table([[[[run("This is NOT a test, and nobody gets a grade. ",
                      bold=True, size=11),
                  run("There are no wrong answers - just tell us what you "
                      "really think. Your teacher will read each question "
                      "out loud.", size=11)]]]],
              [7.1], heights=0.45, valign="center", fills={(0, 0): "EEF3F8"})

    # ---- Part 1: About me ---------------------------------------------
    doc.heading("Part 1:  About me", level=2)
    doc.para("Tick ONE box for each sentence.", after=2, size=11)
    rows = [[""] + [[run(label, size=9.5)] for label in q.SCALE]]
    for _code, _group, sentence in q.ABOUT_ME:
        rows.append([[run(sentence, size=11)]] + [BOX] * len(q.SCALE))
    doc.table(rows, [3.3] + [0.95] * 4, header=True, heights=0.33,
              align=["left"] + ["center"] * 4, valign="center")

    # ---- Part 2: What do you know? ------------------------------------
    doc.heading("Part 2:  What do you know?", level=2)
    doc.para([run("Tick ONE box for each question. ", size=11),
              run("\"%s\" is a great answer!" % q.DONT_KNOW, bold=True,
                  color=TEAL, size=11)], after=2)
    cells = []
    for index, (_code, _lesson, question, options, _right) in enumerate(
            q.KNOW, 1):
        paras = [[run("%d.  %s" % (index, question), bold=True,
                      size=10.5)]]
        paras += [[run("{}  {}".format(BOX, option), size=10.5)]
                  for option in options + [q.DONT_KNOW]]
        cells.append(paras)
    rows = [cells[i:i + 2] for i in range(0, len(cells), 2)]
    doc.table(rows, [3.55, 3.55], size=10.5)

    # ---- Part 3 -------------------------------------------------------
    if key == "Start":
        doc.heading("Part 3:  About you", level=2)
        doc.para([run("Have you ever...  ", bold=True),
                  run("tick ALL that are true")], after=2)
        rows, widths = _tick_grid(q.BACKGROUND)
        doc.table(rows, widths, borders="none", size=11)
        doc.para([run("I learn best when...  ", bold=True),
                  run("tick ALL that are true")], after=2)
        rows, widths = _tick_grid(q.LEARN_BEST)
        doc.table(rows, widths, borders="none", size=11)
    else:
        doc.heading("Part 3:  The lessons", level=2)
        doc.para("For each lesson, tick ONE box under LIKED IT and ONE box "
                 "under HOW HARD.", after=2, size=11)
        lessons = q.RATED_LESSONS[key]
        rows = [["", Span(3, "Did you like it?"), Span(3, "How hard was it?")],
                ["Lesson"] + [[run(x, size=10)] for x in q.LIKED + q.HOW_HARD]]
        for number in lessons:
            title_text = course.LESSONS[number - 1][2]
            rows.append([[run("%d  %s" % (number, title_text), size=11)]] +
                        [BOX] * 6)
        doc.table(rows, [2.3] + [0.8] * 6, header_rows=2, heights=0.36,
                  align=["left"] + ["center"] * 6, valign="center")

        closed = [item for item in q.CLOSED if key in item[3]]
        rows = [[[run(question, bold=True, size=11)],
                 [run(_options_line(options), size=11)]]
                for _code, question, options, _when in closed]
        doc.table(rows, [3.4, 3.7], borders="lines", heights=0.38,
                  valign="center")

    # ---- Open questions -----------------------------------------------
    doc.heading("Part 4:  Tell us more" if key != "Start"
                else "And two more questions", level=2)
    for question, count in q.OPEN[key]:
        doc.para([run(question, bold=True)], after=0, keep_next=True)
        doc.table([[""] for _ in range(count)], [7.1], borders="lines",
                  heights=0.36)

    doc.para([run("Thank you, Explorer!", bold=True, color=TEAL, size=14)],
             align="center", before=4)
    return doc.save(os.path.join(OUT, QUESTIONNAIRE_FILES[key]))


# ===================================================================
# The questionnaire tracker
# ===================================================================

def tracker():
    """
    One workbook for a cohort's answers. Teachers type each form into the
    Answers sheet as one row; Summary and Growth work everything out with
    formulas, so nothing has to be calculated by hand.
    """
    from openpyxl import Workbook
    from openpyxl.chart import BarChart, Reference
    from openpyxl.comments import Comment
    from openpyxl.styles import Alignment, Border, Font, PatternFill, Side
    from openpyxl.utils import get_column_letter
    from openpyxl.worksheet.datavalidation import DataValidation

    FONT = "Arial"
    HEAD_FILL = PatternFill("solid", fgColor="1F3A5F")
    BAND_FILL = PatternFill("solid", fgColor="D6ECEE")
    INPUT_FILL = PatternFill("solid", fgColor="FFF2CC")
    EXAMPLE_FILL = PatternFill("solid", fgColor="EEEEEE")
    thin = Side(style="thin", color="C8D2DE")
    BOX_BORDER = Border(left=thin, right=thin, top=thin, bottom=thin)
    FIRST, LAST = 3, 400            # Answers rows that hold data
    EXAMPLE = "EXAMPLE"

    def font(**kw):
        return Font(name=FONT, **kw)

    def header(cell, text):
        cell.value = text
        cell.font = font(bold=True, color="FFFFFF")
        cell.fill = HEAD_FILL
        cell.alignment = Alignment(horizontal="center", vertical="center",
                                   wrap_text=True)

    wb = Workbook()

    # ---- Answers: the columns ------------------------------------------
    columns = [("CODE", "Explorer code", None), ("CHECKPOINT", "Checkpoint",
                                                 None),
               ("DATE", "Date", None)]
    columns += [(code, sentence, "about") for code, _g, sentence in q.ABOUT_ME]
    columns += [(code, question, "know") for code, _l, question, _o, _r
                in q.KNOW]
    columns += [("LIKE%d" % n, "Did you like Lesson %d?" % n, "like")
                for n in range(1, 13)]
    columns += [("HARD%d" % n, "How hard was Lesson %d?" % n, "hard")
                for n in range(1, 13)]
    columns += [(code, question, "closed") for code, question, _o, _w
                in q.CLOSED]
    columns += [("NOTES", "Anything worth keeping from Part 4", None)]
    letter_of = {code: get_column_letter(i + 1)
                 for i, (code, _t, _k) in enumerate(columns)}

    def rng(code):
        col = letter_of[code]
        return "Answers!$%s$%d:$%s$%d" % (col, FIRST, col, LAST)

    def block(first_code, last_code):
        return "Answers!$%s$%d:$%s$%d" % (letter_of[first_code], FIRST,
                                          letter_of[last_code], LAST)

    CODES = rng("CODE")
    CPS = rng("CHECKPOINT")

    # ---- Sheet 1: Start here -------------------------------------------
    guide = wb.active
    guide.title = "Start here"
    guide.column_dimensions["A"].width = 18
    guide.column_dimensions["B"].width = 62
    guide.column_dimensions["C"].width = 26
    lines = [
        ("Pathfinder Explorers - questionnaire tracker", "title"),
        ("One copy of this workbook per cohort.", None),
        ("", None),
        ("How to use it", "head"),
        ("1. After each questionnaire, type every student's form into the "
         "Answers sheet: one row per form.", None),
        ("2. Yellow cells are the ones you type in. Everything else is "
         "worked out for you.", None),
        ("3. Summary shows the whole class at Start, Halfway and Finish. "
         "Growth shows each student.", None),
        ("4. Type each Explorer code once in column A of the Growth sheet.",
         None),
        ("5. The grey rows coded EXAMPLE show the format. They are left out "
         "of the Summary. Delete them whenever you like.", None),
        ("", None),
        ("How to type each answer", "head"),
    ]
    coding = [
        ("Checkpoint", "Start, Halfway or Finish (pick from the list)"),
        ("A1 to A8", "Part 1. Not true for me = 1, A little true = 2, Mostly "
                     "true = 3, Very true = 4"),
        ("K1 to K8", "Part 2. Right = 1. Wrong, or \"I don't know yet\" = 0. "
                     "Left blank = leave the cell empty."),
        ("LIKE1 to LIKE12", "Part 3. Not much = 1, OK = 2, Loved it = 3"),
        ("HARD1 to HARD12", "Part 3. Too easy = 1, Just right = 2, Too hard "
                            "= 3"),
        ("PACE", "Too slow = 1, Just right = 2, Too fast = 3"),
        ("FAIR, SAFE", "Yes, Sometimes or No (pick from the list)"),
        ("FRIEND, MORE", "Yes, Maybe or No (pick from the list)"),
        ("NOTES", "Copy any Part 4 answer worth discussing, word for word"),
    ]
    row = 1
    for text, kind in lines:
        cell = guide.cell(row=row, column=1, value=text)
        cell.font = font(bold=kind in ("title", "head"),
                         size=16 if kind == "title" else 11,
                         color="1F3A5F" if kind else "222222")
        row += 1
    for code, meaning in coding:
        guide.cell(row=row, column=1, value=code).font = font(bold=True)
        guide.cell(row=row, column=2, value=meaning).font = font()
        row += 1

    row += 1
    guide.cell(row=row, column=1, value="Part 1: the sentences").font = font(
        bold=True, color="1F3A5F")
    row += 1
    for code, group, sentence in q.ABOUT_ME:
        guide.cell(row=row, column=1, value=code).font = font(bold=True)
        guide.cell(row=row, column=2, value=sentence).font = font()
        guide.cell(row=row, column=3, value=group).font = font(color="666666")
        row += 1

    row += 1
    guide.cell(row=row, column=1, value="Part 2: answer key").font = font(
        bold=True, color="1F3A5F")
    row += 1
    for code, lesson, question, options, right in q.KNOW:
        guide.cell(row=row, column=1, value=code).font = font(bold=True)
        guide.cell(row=row, column=2, value=question).font = font()
        guide.cell(row=row, column=3,
                   value="%s  (Lesson %d)" % (options[right], lesson)
                   ).font = font(color="0E7C86", bold=True)
        row += 1

    row += 1
    guide.cell(row=row, column=1, value="Reading the results").font = font(
        bold=True, color="1F3A5F")
    row += 1
    for text in (
            "With a class of twelve, every number is a conversation starter, "
            "not a statistic. Look for big changes and for anything a lot of "
            "students agree on.",
            "Part 2 should climb from Start to Finish. A question most students "
            "still miss at Finish points at a lesson worth strengthening.",
            "Any SAFE = No is worth a quiet conversation the same week, not a "
            "spreadsheet.",
            "These are our own questions, written for this course. They are "
            "not a validated research instrument."):
        cell = guide.cell(row=row, column=1, value=text)
        cell.font = font()
        guide.merge_cells(start_row=row, start_column=1, end_row=row,
                          end_column=3)
        cell.alignment = Alignment(wrap_text=True, vertical="top")
        guide.row_dimensions[row].height = 30
        row += 1

    # ---- Sheet 2: Answers ----------------------------------------------
    answers = wb.create_sheet("Answers")
    bands = [("CODE", "DATE", "Who and when"),
             (q.ABOUT_ME[0][0], q.ABOUT_ME[-1][0], "Part 1: About me (1 to 4)"),
             (q.KNOW[0][0], q.KNOW[-1][0], "Part 2: right = 1, wrong = 0"),
             ("LIKE1", "LIKE12", "Liked it? (1 to 3)"),
             ("HARD1", "HARD12", "How hard? (1 to 3)"),
             (q.CLOSED[0][0], q.CLOSED[-1][0], "Part 3 questions"),
             ("NOTES", "NOTES", "Notes")]
    for first, last, label in bands:
        a, b = letter_of[first], letter_of[last]
        cell = answers["%s1" % a]
        cell.value = label
        cell.font = font(bold=True, color="1F3A5F")
        cell.fill = BAND_FILL
        cell.alignment = Alignment(horizontal="center")
        if a != b:
            answers.merge_cells("%s1:%s1" % (a, b))
    for i, (code, text, _kind) in enumerate(columns, 1):
        cell = answers.cell(row=2, column=i)
        header(cell, code if code not in ("CODE", "CHECKPOINT", "DATE",
                                          "NOTES") else text)
        cell.comment = Comment(text, "Porpoise Robotics")
        answers.column_dimensions[get_column_letter(i)].width = (
            14 if code in ("CODE", "CHECKPOINT", "DATE") else
            40 if code == "NOTES" else 7.5)
    answers.row_dimensions[2].height = 30
    answers.freeze_panes = "D3"

    # Two example rows, so the Growth sheet has something to show.
    example = {
        "Start": {"about": [2, 1, 1, 3, 2, 3, 4, 2],
                  "know": [1, 1, 0, 0, 1, 0, 0, 0]},
        "Finish": {"about": [3, 3, 3, 3, 3, 4, 4, 3],
                   "know": [1, 1, 1, 1, 1, 1, 1, 0],
                   "like": {7: 3, 8: 2, 9: 3, 10: 3, 11: 2, 12: 3},
                   "hard": {7: 2, 8: 2, 9: 3, 10: 2, 11: 2, 12: 2},
                   "closed": {"PACE": 2, "FAIR": "Yes", "SAFE": "Yes",
                              "FRIEND": "Yes", "MORE": "Maybe"},
                   "notes": "Favorite was Mission to Mars because we planned "
                            "it ourselves"},
    }
    for offset, (checkpoint_key, values) in enumerate(example.items()):
        r = FIRST + offset
        answers["%s%d" % (letter_of["CODE"], r)] = EXAMPLE
        answers["%s%d" % (letter_of["CHECKPOINT"], r)] = checkpoint_key
        answers["%s%d" % (letter_of["DATE"], r)] = (
            "2026-09-08" if checkpoint_key == "Start" else "2026-11-24")
        for (code, _g, _s), v in zip(q.ABOUT_ME, values["about"]):
            answers["%s%d" % (letter_of[code], r)] = v
        for (code, _l, _q, _o, _r), v in zip(q.KNOW, values["know"]):
            answers["%s%d" % (letter_of[code], r)] = v
        for n, v in values.get("like", {}).items():
            answers["%s%d" % (letter_of["LIKE%d" % n], r)] = v
        for n, v in values.get("hard", {}).items():
            answers["%s%d" % (letter_of["HARD%d" % n], r)] = v
        for code, v in values.get("closed", {}).items():
            answers["%s%d" % (letter_of[code], r)] = v
        if "notes" in values:
            answers["%s%d" % (letter_of["NOTES"], r)] = values["notes"]
        for c in range(1, len(columns) + 1):
            answers.cell(row=r, column=c).fill = EXAMPLE_FILL
            answers.cell(row=r, column=c).font = font(italic=True,
                                                      color="666666")

    for r in range(FIRST + len(example), LAST + 1):
        for c in range(1, len(columns) + 1):
            cell = answers.cell(row=r, column=c)
            cell.fill = INPUT_FILL
            cell.font = font()
            cell.border = BOX_BORDER

    def validate(rule, codes):
        answers.add_data_validation(rule)
        for code in codes:
            rule.add("%s%d:%s%d" % (letter_of[code], FIRST, letter_of[code],
                                    LAST))

    validate(DataValidation(type="list", formula1='"Start,Halfway,Finish"',
                            allow_blank=True), ["CHECKPOINT"])
    validate(DataValidation(type="whole", operator="between", formula1="1",
                            formula2="4", allow_blank=True),
             [code for code, _g, _s in q.ABOUT_ME])
    validate(DataValidation(type="whole", operator="between", formula1="0",
                            formula2="1", allow_blank=True),
             [k[0] for k in q.KNOW])
    validate(DataValidation(type="whole", operator="between", formula1="1",
                            formula2="3", allow_blank=True),
             ["LIKE%d" % n for n in range(1, 13)] +
             ["HARD%d" % n for n in range(1, 13)] + ["PACE"])
    validate(DataValidation(type="list", formula1='"Yes,Sometimes,No"',
                            allow_blank=True), ["FAIR", "SAFE"])
    validate(DataValidation(type="list", formula1='"Yes,Maybe,No"',
                            allow_blank=True), ["FRIEND", "MORE"])

    # ---- Sheet 3: Summary ----------------------------------------------
    summary = wb.create_sheet("Summary")
    summary.column_dimensions["A"].width = 66
    for col in "BCDE":
        summary.column_dimensions[col].width = 13
    summary.column_dimensions["F"].width = 16

    def put(cell, value, *, bold=False, fmt=None, color=None):
        summary[cell] = value
        summary[cell].font = font(bold=bold, color=color)
        if fmt:
            summary[cell].number_format = fmt

    put("A1", "Whole class, at each checkpoint", bold=True, color="1F3A5F")
    summary["A1"].font = font(bold=True, size=14, color="1F3A5F")
    put("A2", "Rows coded EXAMPLE are left out. Blank = no answers yet.",
        color="666666")
    cps = [c[0] for c in q.CHECKPOINTS]

    def table_head(r, first, labels):
        header(summary["A%d" % r], first)
        summary["A%d" % r].alignment = Alignment(horizontal="left")
        for i, label in enumerate(labels):
            header(summary.cell(row=r, column=2 + i), label)

    not_example = '%s,"<>%s"' % (CODES, EXAMPLE)

    # Responses
    r = 4
    table_head(r, "Forms entered", cps)
    r += 1
    put("A%d" % r, "Number of questionnaires")
    for i, cp in enumerate(cps):
        put("%s%d" % ("BCD"[i], r),
            '=COUNTIFS(%s,"%s",%s,"<>%s",%s,"<>")' % (
                CPS, cp, CODES, EXAMPLE, CODES))
    count_row = r

    # Part 1
    r += 2
    table_head(r, "Part 1: About me (average, 1 to 4)",
               cps + ["Change", "Start to Finish"])
    summary.merge_cells("E%d:F%d" % (r, r))
    part1_first = r + 1

    def avg_block(first_code, last_code, cp):
        cells = block(first_code, last_code)
        cond = '(%s="%s")*(%s<>"%s")' % (CPS, cp, CODES, EXAMPLE)
        return ('=IFERROR(SUMPRODUCT(%s*%s)/SUMPRODUCT(%s*(%s<>"")),"")'
                % (cond, cells, cond, cells))

    group_rows = {}
    for group in q.GROUPS:
        codes = [code for code, g, _s in q.ABOUT_ME if g == group]
        r += 1
        put("A%d" % r, group, bold=True)
        for i, cp in enumerate(cps):
            put("%s%d" % ("BCD"[i], r), avg_block(codes[0], codes[-1], cp),
                bold=True, fmt="0.0")
        put("E%d" % r, '=IF(OR(B%d="",D%d=""),"",D%d-B%d)' % (r, r, r, r),
            bold=True, fmt="+0.0;-0.0;0.0")
        group_rows[group] = r
        for code, g, sentence in q.ABOUT_ME:
            if g != group:
                continue
            r += 1
            put("A%d" % r, "   %s  %s" % (code, sentence))
            for i, cp in enumerate(cps):
                put("%s%d" % ("BCD"[i], r),
                    '=IFERROR(AVERAGEIFS(%s,%s,"%s",%s),"")'
                    % (rng(code), CPS, cp, not_example), fmt="0.0")
            put("E%d" % r, '=IF(OR(B%d="",D%d=""),"",D%d-B%d)'
                % (r, r, r, r), fmt="+0.0;-0.0;0.0")

    # Part 2
    r += 2
    table_head(r, "Part 2: What do you know? (share answered right)",
               cps + ["Change", "Start to Finish"])
    summary.merge_cells("E%d:F%d" % (r, r))
    r += 1
    put("A%d" % r, "All eight questions", bold=True)
    know_total_row = r
    first_k, last_k = q.KNOW[0][0], q.KNOW[-1][0]
    for i, cp in enumerate(cps):
        put("%s%d" % ("BCD"[i], r), avg_block(first_k, last_k, cp),
            bold=True, fmt="0%")
    put("E%d" % r, '=IF(OR(B%d="",D%d=""),"",D%d-B%d)' % (r, r, r, r),
        bold=True, fmt="+0%;-0%;0%")
    for code, lesson, question, _o, _right in q.KNOW:
        r += 1
        put("A%d" % r, "   %s  (Lesson %d)  %s" % (code, lesson, question))
        for i, cp in enumerate(cps):
            put("%s%d" % ("BCD"[i], r),
                '=IFERROR(AVERAGEIFS(%s,%s,"%s",%s),"")'
                % (rng(code), CPS, cp, not_example), fmt="0%")
        put("E%d" % r, '=IF(OR(B%d="",D%d=""),"",D%d-B%d)' % (r, r, r, r),
            fmt="+0%;-0%;0%")

    # Lessons
    r += 2
    table_head(r, "Lessons (average)", ["Liked it", "How hard", "Answers",
                                        "Worth a look?"])
    summary.merge_cells("E%d:F%d" % (r, r))
    put("A%d" % (r + 1), "Liked: 1 not much, 3 loved it.  Hard: 1 too easy, "
        "2 just right, 3 too hard.", color="666666")
    r += 1
    for number in range(1, 13):
        r += 1
        put("A%d" % r, "%d  %s" % (number, course.LESSONS[number - 1][2]))
        put("B%d" % r, '=IFERROR(AVERAGEIFS(%s,%s),"")'
            % (rng("LIKE%d" % number), not_example), fmt="0.0")
        put("C%d" % r, '=IFERROR(AVERAGEIFS(%s,%s),"")'
            % (rng("HARD%d" % number), not_example), fmt="0.0")
        put("D%d" % r, '=COUNTIFS(%s,"<>",%s)'
            % (rng("LIKE%d" % number), not_example))
        # Both flags can be true at once - a lesson that is too hard is
        # often the one that isn't landing - so show them side by side.
        put("E%d" % r,
            '=IF(B%d="","",TRIM(IF(C%d>=2.5,"Too hard? ",IF(C%d<=1.5,'
            '"Too easy? ",""))&IF(B%d<2,"Not landing?","")))' % (r, r, r, r),
            color="B46A0F", bold=True)

    # Part 3 closed questions
    r += 2
    table_head(r, "Part 3 questions", ["Halfway", "Finish"])
    for code, question, options, when in q.CLOSED:
        if code == "PACE":
            r += 1
            put("A%d" % r, "%s  (1 too slow, 2 just right, 3 too fast)"
                % question)
            for i, cp in enumerate(("Halfway", "Finish")):
                put("%s%d" % ("BC"[i], r),
                    '=IFERROR(AVERAGEIFS(%s,%s,"%s",%s),"")'
                    % (rng(code), CPS, cp, not_example), fmt="0.0")
            continue
        for answer in (options[0], options[-1]):
            r += 1
            put("A%d" % r, '%s  share answering "%s"' % (question, answer))
            for i, cp in enumerate(("Halfway", "Finish")):
                if cp not in when:
                    continue
                put("%s%d" % ("BC"[i], r),
                    '=IFERROR(COUNTIFS(%s,"%s",%s,"%s",%s)/COUNTIFS(%s,"<>",'
                    '%s,"%s",%s),"")' % (rng(code), answer, CPS, cp,
                                         not_example, rng(code), CPS, cp,
                                         not_example), fmt="0%")

    # Charts
    chart = BarChart()
    chart.type = "col"
    chart.title = "About me, by checkpoint"
    chart.y_axis.title = "Average (1 to 4)"
    chart.y_axis.scaling.min = 1
    chart.y_axis.scaling.max = 4
    data_rows = [group_rows[g] for g in q.GROUPS]
    # One series per checkpoint, one category per group. The group rows are
    # not adjacent (the sentences sit between them), so build each series
    # from a contiguous helper block further down the sheet.
    helper = r + 3
    put("A%d" % helper, "Chart data (worked out from the tables above)",
        color="666666")
    for i, cp in enumerate(cps):
        summary.cell(row=helper + 1, column=2 + i, value=cp).font = font()
    for j, group in enumerate(q.GROUPS):
        summary.cell(row=helper + 2 + j, column=1, value=group).font = font()
        for i in range(len(cps)):
            col = "BCD"[i]
            summary["%s%d" % (col, helper + 2 + j)] = "=%s%d" % (
                col, data_rows[j])
            summary["%s%d" % (col, helper + 2 + j)].number_format = "0.0"
            summary["%s%d" % (col, helper + 2 + j)].font = font()
    data = Reference(summary, min_col=2, max_col=4, min_row=helper + 1,
                     max_row=helper + 1 + len(q.GROUPS))
    cats = Reference(summary, min_col=1, min_row=helper + 2,
                     max_row=helper + 1 + len(q.GROUPS))
    chart.add_data(data, titles_from_data=True)
    chart.set_categories(cats)
    chart.height, chart.width = 8, 16
    summary.add_chart(chart, "H4")

    know_chart = BarChart()
    know_chart.type = "col"
    know_chart.title = "Part 2 answered right, by checkpoint"
    know_chart.y_axis.scaling.min = 0
    know_chart.y_axis.scaling.max = 1
    know_chart.y_axis.number_format = "0%"
    know_chart.legend = None
    data = Reference(summary, min_col=2, max_col=4, min_row=know_total_row)
    know_chart.add_data(data, from_rows=True, titles_from_data=False)
    know_chart.set_categories(Reference(summary, min_col=2, max_col=4,
                                        min_row=count_row - 1))
    know_chart.height, know_chart.width = 8, 16
    summary.add_chart(know_chart, "H22")

    # ---- Sheet 4: Growth by student ------------------------------------
    growth = wb.create_sheet("Growth")
    growth.column_dimensions["A"].width = 16
    growth["A1"] = "Each student, start to finish"
    growth["A1"].font = font(bold=True, size=14, color="1F3A5F")
    growth["A2"] = ("Type each Explorer code once in column A (yellow). The "
                    "rest fills itself in from the Answers sheet.")
    growth["A2"].font = font(color="666666")
    growth.merge_cells("A3:A4")
    header(growth["A3"], "Explorer code")
    for first_col, label in ((2, "About me, average of A1 to A8 (1 to 4)"),
                             (6, "Part 2, questions right (out of %d)"
                              % len(q.KNOW))):
        growth.merge_cells(start_row=3, start_column=first_col, end_row=3,
                           end_column=first_col + 3)
        header(growth.cell(row=3, column=first_col), label)
        for i, sub in enumerate(cps + ["Change"]):
            header(growth.cell(row=4, column=first_col + i), sub)
            growth.column_dimensions[get_column_letter(first_col + i)].width = 11
    growth.row_dimensions[3].height = 30
    growth.freeze_panes = "B5"

    about_block = block(q.ABOUT_ME[0][0], q.ABOUT_ME[-1][0])
    know_block = block(first_k, last_k)
    for r in range(5, 65):
        code_cell = growth["A%d" % r]
        code_cell.fill = INPUT_FILL
        code_cell.font = font()
        code_cell.border = BOX_BORDER
        for i, cp in enumerate(cps):
            cond = '(%s=$A%d)*(%s="%s")' % (CODES, r, CPS, cp)
            col = get_column_letter(2 + i)
            growth["%s%d" % (col, r)] = (
                '=IF($A%d="","",IFERROR(SUMPRODUCT(%s*%s)/SUMPRODUCT(%s*(%s<>"'
                '")),""))' % (r, cond, about_block, cond, about_block))
            growth["%s%d" % (col, r)].number_format = "0.0"
            col = get_column_letter(6 + i)
            growth["%s%d" % (col, r)] = (
                '=IF($A%d="","",IF(COUNTIFS(%s,$A%d,%s,"%s")=0,"",'
                'SUMPRODUCT(%s*%s)))' % (r, CODES, r, CPS, cp, cond,
                                          know_block))
        growth["E%d" % r] = '=IF(OR(B%d="",D%d=""),"",D%d-B%d)' % (r, r, r, r)
        growth["E%d" % r].number_format = "+0.0;-0.0;0.0"
        growth["I%d" % r] = '=IF(OR(F%d="",H%d=""),"",H%d-F%d)' % (r, r, r, r)
        growth["I%d" % r].number_format = "+0;-0;0"
        for c in range(2, 10):
            growth.cell(row=r, column=c).font = font()
    growth["A5"] = EXAMPLE
    growth["A5"].fill = EXAMPLE_FILL
    growth["A5"].font = font(italic=True, color="666666")

    for sheet in wb.worksheets:
        sheet.sheet_view.showGridLines = sheet.title in ("Answers", "Growth")
    # openpyxl writes formulas without results; this asks Excel (and Google
    # Sheets) to work every one out the moment the file is opened.
    from openpyxl.workbook.properties import CalcProperties
    wb.calculation = CalcProperties(fullCalcOnLoad=True)
    path = os.path.join(OUT, "questionnaire_tracker.xlsx")
    wb.save(path)
    return path


# ===================================================================
# The certificate
# ===================================================================

NAVY_RGB = RGBColor(0x1F, 0x3A, 0x5F)
TEAL_RGB = RGBColor(0x0E, 0x7C, 0x86)
GREY_RGB = RGBColor(0x66, 0x66, 0x66)
INK_RGB = RGBColor(0x22, 0x22, 0x22)


def _text(slide, left, top, width, height, lines, *, size, color=INK_RGB,
          bold=False, align=PP_ALIGN.CENTER, italic=False, font="Calibri"):
    box = slide.shapes.add_textbox(Inches(left), Inches(top), Inches(width),
                                   Inches(height))
    frame = box.text_frame
    frame.word_wrap = True
    frame.vertical_anchor = MSO_ANCHOR.MIDDLE
    for index, line in enumerate(lines if isinstance(lines, list) else [lines]):
        para = frame.paragraphs[0] if index == 0 else frame.add_paragraph()
        para.alignment = align
        r = para.add_run()
        r.text = line
        r.font.size = Pt(size)
        r.font.bold = bold
        r.font.italic = italic
        r.font.color.rgb = color
        r.font.name = font
    return box


def _frame(slide, inset, color, weight):
    w, h = 11.0, 8.5
    shape = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(inset),
                                   Inches(inset), Inches(w - 2 * inset),
                                   Inches(h - 2 * inset))
    shape.fill.background()
    shape.line.color.rgb = color
    shape.line.width = Pt(weight)
    shape.shadow.inherit = False
    return shape


def _rule(slide, left, top, width):
    line = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(left),
                                  Inches(top), Inches(width), Pt(1.25))
    line.fill.solid()
    line.fill.fore_color.rgb = INK_RGB
    line.line.fill.background()
    line.shadow.inherit = False


def certificate_slide(prs, name):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    _frame(slide, 0.3, NAVY_RGB, 6)
    _frame(slide, 0.48, TEAL_RGB, 1.5)

    if os.path.exists(LOGO):
        pic = slide.shapes.add_picture(LOGO, 0, Inches(0.75),
                                       height=Inches(1.05))
        pic.left = int((Inches(11) - pic.width) / 2)

    _text(slide, 1, 1.9, 9, 0.45, "CERTIFICATE OF COMPLETION", size=16,
          color=TEAL_RGB, bold=True)
    _text(slide, 1, 2.35, 9, 0.85, "Pathfinder Explorer", size=46,
          color=NAVY_RGB, bold=True)
    _text(slide, 1, 3.25, 9, 0.4, "This certifies that", size=16,
          color=GREY_RGB, italic=True)
    _text(slide, 1.5, 3.65, 8, 0.75, name, size=34, color=INK_RGB, bold=True)
    _rule(slide, 2.0, 4.42, 7.0)
    _text(slide, 1.2, 4.55, 8.6, 1.05,
          ["completed all twelve missions of Pathfinder Explorers - lights, "
           "loops, motors, autopilot, wireless control and a design of their "
           "own - and programmed a robot vehicle to drive the Mission Day "
           "course."], size=15, color=INK_RGB)

    _text(slide, 3.0, 5.6, 5.0, 0.4, "Highest license earned:  "
          "____________________", size=14, color=NAVY_RGB)

    for left, label in ((1.4, "Date"), (6.1, "Instructor")):
        _rule(slide, left, 6.75, 3.5)
        _text(slide, left, 6.8, 3.5, 0.35, label, size=12, color=GREY_RGB)

    _text(slide, 1, 7.45, 9, 0.35, "Porpoise Robotics  -  "
          "porpoiserobotics.org", size=11, color=GREY_RGB)
    return slide


def certificates(names):
    prs = Presentation()
    prs.slide_width = Inches(11)
    prs.slide_height = Inches(8.5)
    for name in names:
        certificate_slide(prs, name)
    filename = "certificate.pptx" if names == ["[Student name]"] \
        else "certificates_for_class.pptx"
    path = os.path.join(OUT, filename)
    prs.save(path)
    return path


# ===================================================================

def main():
    os.makedirs(OUT, exist_ok=True)
    course.enrich()

    if "--names" in sys.argv:
        source = sys.argv[sys.argv.index("--names") + 1]
        with open(source, encoding="utf-8") as handle:
            names = [line.strip() for line in handle if line.strip()]
        print("wrote", certificates(names), "-", len(names), "certificates")
        return

    decks = read_decks()
    made = [mission_log(decks), letter(), certificates(["[Student name]"])]
    made += [questionnaire(key) for key, *_rest in q.CHECKPOINTS]
    made.append(tracker())
    for path in made:
        print("wrote", os.path.relpath(path, os.path.join(HERE, "..")))


if __name__ == "__main__":
    main()
