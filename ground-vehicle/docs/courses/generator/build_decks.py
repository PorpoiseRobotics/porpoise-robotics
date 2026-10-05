"""
build_decks.py - regenerates the course decks.

    python build_decks.py                 every course
    python build_decks.py explorers       just the courses named

READ THIS FIRST. The .pptx files in the track folders are the DELIVERABLE and,
once anybody has edited them in PowerPoint, they are the source of truth.
Re-running this script OVERWRITES them and any hand edits go with them.

This script exists so the first version of the decks is reproducible and so a
change that affects every deck - a new house color, a corrected pin number -
can be made in one place. If you have edited a deck by hand, edit it by hand
from then on.
"""

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import content_advanced          # noqa: E402
import content_beginner          # noqa: E402
import content_explorers         # noqa: E402

COURSES = os.path.normpath(os.path.join(HERE, ".."))


def out_dir(name):
    path = os.path.join(COURSES, name)
    os.makedirs(path, exist_ok=True)
    return path


# One entry per course folder. Name folders on the command line to build only
# those - worth doing whenever a deck in another folder has been edited by hand,
# because building a course overwrites every deck in its folder.
COURSE_BUILDERS = {
    "beginner-ps3": lambda path: content_beginner.build(content_beginner.PS3, path),
    "beginner-switch": lambda path: content_beginner.build(content_beginner.SWITCH, path),
    "advanced": content_advanced.build,
    "explorers": content_explorers.build,
}


def main():
    wanted = sys.argv[1:] or list(COURSE_BUILDERS)
    unknown = [name for name in wanted if name not in COURSE_BUILDERS]
    if unknown:
        sys.exit("Unknown course %s. Choose from: %s"
                 % (", ".join(unknown), ", ".join(COURSE_BUILDERS)))

    built = []
    for name in wanted:
        built += [(name,) + r for r in COURSE_BUILDERS[name](out_dir(name))]

    print()
    print("{:<18} {:<42} {:>7} {:>9}".format("track", "deck", "slides", "size"))
    print("-" * 80)
    total_slides = 0
    for track, path, slides in built:
        kb = os.path.getsize(path) // 1024
        print("{:<18} {:<42} {:>7} {:>7} KB".format(
            track, os.path.basename(path), slides, kb))
        total_slides += slides

    print("-" * 80)
    print("{} decks, {} slides".format(len(built), total_slides))


if __name__ == "__main__":
    main()
