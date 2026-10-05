"""
sync_explorer_h.py - copies the Explorers "engine room" into every sketch.

    python sync_explorer_h.py            copy the master into every sketch
    python sync_explorer_h.py --check    only report copies that differ

Arduino compiles a sketch from its own folder and nowhere else, so every
Explorers sketch folder carries its own copy of explorer.h. That keeps each
sketch opening and compiling on its own, the way students and the IDE expect -
and it means there are fifteen copies that have to agree.

The master is ground-vehicle/src/lessons/explorers/explorer.h. Edit that one,
then run this. content_explorers.build() calls check() and stops the deck
build if any copy has drifted, so a stale copy cannot reach a classroom.
"""

import glob
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
LESSONS = os.path.normpath(os.path.join(HERE, "..", "..", "..", "src",
                                        "lessons", "explorers"))
MASTER = os.path.join(LESSONS, "explorer.h")


def sketch_folders():
    """Every folder under explorers/ that holds an .ino of its own name."""
    found = []
    for folder in sorted(glob.glob(os.path.join(LESSONS, "e*"))):
        name = os.path.basename(folder)
        if os.path.isfile(os.path.join(folder, name + ".ino")):
            found.append(folder)
    return found


def _read(path):
    with open(path, "rb") as handle:
        return handle.read().replace(b"\r\n", b"\n")


def check():
    """The sketch folders whose copy is missing or differs from the master."""
    master = _read(MASTER)
    stale = []
    for folder in sketch_folders():
        copy = os.path.join(folder, "explorer.h")
        if not os.path.exists(copy) or _read(copy) != master:
            stale.append(os.path.basename(folder))
    return stale


def sync():
    master = _read(MASTER)
    for folder in sketch_folders():
        with open(os.path.join(folder, "explorer.h"), "wb") as handle:
            handle.write(master)
    return len(sketch_folders())


def main():
    if "--check" in sys.argv:
        stale = check()
        if stale:
            print("explorer.h differs from the master in:")
            for name in stale:
                print("  " + name)
            sys.exit(1)
        print("All %d copies of explorer.h match the master."
              % len(sketch_folders()))
        return
    print("Copied explorer.h into %d sketches." % sync())


if __name__ == "__main__":
    main()
