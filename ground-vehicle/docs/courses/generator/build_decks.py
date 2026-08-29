"""
build_decks.py - regenerates all fifteen course decks.

    python build_decks.py

READ THIS FIRST. The .pptx files in the track folders are the DELIVERABLE and,
once anybody has edited them in PowerPoint, they are the source of truth.
Re-running this script OVERWRITES them and any hand edits go with them.

This script exists so the first version of the decks is reproducible and so a
change that affects all fifteen - a new house colour, a corrected pin number -
can be made in one place. If you have edited a deck by hand, edit it by hand
from then on.
"""

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import content_advanced          # noqa: E402
import content_beginner          # noqa: E402

COURSES = os.path.normpath(os.path.join(HERE, ".."))


def out_dir(name):
    path = os.path.join(COURSES, name)
    os.makedirs(path, exist_ok=True)
    return path


def main():
    built = []

    built += [("beginner-ps3",) + r for r in
              content_beginner.build(content_beginner.PS3, out_dir("beginner-ps3"))]
    built += [("beginner-switch",) + r for r in
              content_beginner.build(content_beginner.SWITCH, out_dir("beginner-switch"))]
    built += [("advanced",) + r for r in
              content_advanced.build(out_dir("advanced"))]

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
