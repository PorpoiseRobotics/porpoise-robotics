"""
questionnaires.py - the Explorers questionnaires, as data.

Eddie asked for a questionnaire at the beginning, the middle and the end of
every cohort, "so we can see how students are growing, hear what's
resonating, and keep refining the modules together".

Growth only shows if the SAME questions are asked every time, worded the same
way, scored the same way. So the questions live here, once, and everything
that uses them reads them from here: the three printed forms and the results
tracker (build_handouts.py) and the slides that hand the forms out
(content_explorers.py). Reword an item here and every one of those follows.
Reword it after a cohort has started, and that cohort's growth numbers for the
item stop meaning anything - so don't, mid-cohort.

Every form has three parts:

  Part 1  About me        - the same eight sentences every time, each rated
                            on the same four-point scale. Confidence,
                            persistence, teamwork and interest.
  Part 2  What do you know - the same eight questions every time, one from
                            each big idea of the course, each with an
                            "I don't know yet" option so nobody guesses.
  Part 3                   - different at each checkpoint: background at the
                            start, lesson ratings and open feedback halfway
                            and at the finish.

These are our own items, written for this course and for nine-to-thirteen
year olds. They are not a validated research instrument, and with a class of
twelve the numbers are conversation starters, not statistics.
"""

# -------------------------------------------------------------------
# When
# -------------------------------------------------------------------

# (key, title, number, lesson, when it is given)
CHECKPOINTS = [
    ("Start", "Starting Line", 1, "lesson1",
     "at the very start of Lesson 1, before anything is taught"),
    ("Halfway", "Halfway Check-in", 2, "lesson7",
     "at the start of Lesson 7, after six lessons"),
    ("Finish", "Finish Line", 3, "lesson12",
     "in Lesson 12, straight after the mission"),
]

# -------------------------------------------------------------------
# Part 1 - About me. Same sentences, same scale, every time.
# -------------------------------------------------------------------

SCALE = ["Not true for me", "A little true", "Mostly true", "Very true"]

# (code, what it measures, the sentence)
ABOUT_ME = [
    ("A1", "Confidence", "I am good at figuring out how things work."),
    ("A2", "Confidence", "I could write a program that makes a robot do "
                         "something."),
    ("A3", "Confidence", "When my code doesn't work, I can find the bug and "
                         "fix it."),
    ("A4", "Never give up", "I keep trying when a problem is hard."),
    ("A5", "Teamwork", "I can explain my ideas to my team."),
    ("A6", "Teamwork", "I like working with a team."),
    ("A7", "Interest", "Robots and coding are fun."),
    ("A8", "Interest", "I could be an engineer or a programmer one day."),
]

GROUPS = ["Confidence", "Never give up", "Teamwork", "Interest"]

# -------------------------------------------------------------------
# Part 2 - What do you know? Same questions every time.
# -------------------------------------------------------------------

DONT_KNOW = "I don't know yet"

# (code, lesson it comes from, question, [three options], index of the
#  right one). The right answer moves around on purpose.
KNOW = [
    ("K1", 1, "A robot is a machine that can...",
     ["only move", "only light up", "sense, think and act"], 2),
    ("K2", 2, "In a program, loop() runs...",
     ["over and over, forever", "only once", "only when you press a button"],
     0),
    ("K3", 3, "Red light and green light mixed together make...",
     ["brown", "yellow", "purple"], 1),
    ("K4", 5, "A Pathfinder turns right by...",
     ["turning its front wheels", "making its right wheels go faster",
      "making its left wheels go faster"], 2),
    ("K5", 6, "1000 milliseconds is...",
     ["10 seconds", "1 second", "1 minute"], 1),
    ("K6", 7, "A function is...",
     ["a move you invent and give a name", "a kind of battery",
      "a light on the controller"], 0),
    ("K7", 8, "In  if (buttonA()) { ... } else { ... }  the else part "
              "happens when...",
     ["A is held down", "A is NOT held down", "always"], 1),
    ("K8", 10, "While a program is waiting in delay(), it...",
     ["can still read the stick", "drives faster", "can NOT read the stick"],
     2),
]

# -------------------------------------------------------------------
# Part 3 - closed questions the tracker counts
# -------------------------------------------------------------------

LIKED = ["Not much", "OK", "Loved it"]           # scored 1, 2, 3
HOW_HARD = ["Too easy", "Just right", "Too hard"]  # scored 1, 2, 3
PACE = ["Too slow", "Just right", "Too fast"]      # scored 1, 2, 3
YES_SOMETIMES_NO = ["Yes", "Sometimes", "No"]
YES_MAYBE_NO = ["Yes", "Maybe", "No"]

# Lessons rated at each checkpoint.
RATED_LESSONS = {"Halfway": range(1, 7), "Finish": range(7, 13)}

# (code, question, options, checkpoints that ask it)
CLOSED = [
    ("PACE", "The lessons go...", PACE, ("Halfway", "Finish")),
    ("FAIR", "My team shares the jobs fairly.", YES_SOMETIMES_NO,
     ("Halfway", "Finish")),
    ("SAFE", "I feel safe in this class.", YES_SOMETIMES_NO,
     ("Halfway", "Finish")),
    ("FRIEND", "Would you tell a friend to take this course?", YES_MAYBE_NO,
     ("Finish",)),
    ("MORE", "Do you want to keep learning robotics or coding?",
     YES_MAYBE_NO, ("Finish",)),
]

# -------------------------------------------------------------------
# Part 3 - the rest, which are read rather than counted
# -------------------------------------------------------------------

BACKGROUND = [
    "built with LEGO or a robot kit",
    "coded with blocks, like Scratch",
    "typed code, in any language",
    "used an Arduino",
    "driven a remote-control car",
    "none of these yet",
]

LEARN_BEST = [
    "I can try it with my hands",
    "somebody shows me first",
    "I can read the steps",
    "I work with a partner",
]

OPEN = {
    "Start": [
        ("What are you most excited to do in this course?", 1),
        ("Is there anything that worries you about this course?", 2),
    ],
    "Halfway": [
        ("What has been your favorite part so far?", 2),
        ("What has been confusing?", 2),
        ("I wish we did MORE of...", 1),
        ("I wish we did LESS of...", 1),
    ],
    "Finish": [
        ("My favorite lesson was... because...", 2),
        ("The hardest thing I learned to do was...", 1),
        ("If I could change one thing about this course, I would...", 2),
        ("Something I'm proud of:", 1),
    ],
}


def checkpoint(key):
    for entry in CHECKPOINTS:
        if entry[0] == key:
            return entry
    raise KeyError(key)
