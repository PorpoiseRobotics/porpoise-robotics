"""
srcfacts.py - read constants straight out of the Arduino sources.

The decks quote a lot of numbers: the deadzone, the stick range, the PWM
frequency, which pins drive which motor. Every one of those is a chance for a
slide to go stale the day somebody retunes the vehicle.

So the decks do not contain those numbers. They ask for them here, and this
reads them out of the .ino files at build time. If a constant is renamed or
deleted the build FAILS LOUDLY rather than quietly printing last month's value
onto a slide.

    from srcfacts import const, pins
    const("lessons/beginner_ps3/l3c_tank_drive/l3c_tank_drive.ino", "STICK_MAX")
"""

import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.normpath(os.path.join(HERE, "..", "..", "..", "src"))

_cache = {}


def _read(rel):
    if rel not in _cache:
        path = os.path.join(SRC, rel.replace("/", os.sep))
        if not os.path.exists(path):
            raise SystemExit("srcfacts: no such source file: %s" % rel)
        with open(path, encoding="utf-8", errors="replace") as handle:
            _cache[rel] = handle.read()
    return _cache[rel]


def const(rel, name):
    """
    The value of `const <type> NAME = <value>;` in a source file, as a string
    with the trailing 'f' of a float literal removed.

    Raises if it is not there, which is the whole point: a renamed constant
    should break the build, not silently leave a stale number on a slide.
    """
    text = _read(rel)
    escaped = re.escape(name)

    match = re.search(
        r"^\s*(?:static\s+)?const\s+\w+\s+%s\s*=\s*([^;,]+)\s*[;,]" % escaped,
        text, re.MULTILINE)

    # The sketches also declare pins two to a line:
    #     const int FRONT_LEFT_A = 12, FRONT_LEFT_B = 13;
    # so the second name has no `const int` in front of it.
    if not match:
        match = re.search(r"\b%s\s*=\s*([^;,]+)\s*[;,]" % escaped, text)

    if not match:
        raise SystemExit("srcfacts: %s does not define %s" % (rel, name))
    value = match.group(1).strip()
    return value[:-1] if re.fullmatch(r"-?[\d.]+f", value) else value


def number(rel, name):
    """Same as const(), but evaluated so it can be used in arithmetic."""
    value = const(rel, name)
    try:
        return int(value, 0)
    except ValueError:
        pass
    try:
        return float(value)
    except ValueError:
        raise SystemExit("srcfacts: %s in %s is not a number: %r"
                         % (name, rel, value))


def pins(rel, *names):
    """
    Several pin constants from one file, as strings, in the order asked for.
    Handles the `const int A = 12, B = 13;` form the sketches use.
    """
    return [const(rel, n) for n in names]


def motor_pins(rel, style="pin"):
    """
    The four motors and their two pins each, as (label, a, b) tuples.

    The PS3 sketches name them FRONT_LEFT_A / FRONT_LEFT_B; the Bluepad32 ones
    add a channel and use FRONT_LEFT_PIN_A / FRONT_LEFT_PIN_B. Ask for whichever
    the file actually uses.
    """
    suffix = {"pin": ("_A", "_B"), "channel": ("_PIN_A", "_PIN_B")}[style]
    out = []
    for label, prefix in (("Front left", "FRONT_LEFT"),
                          ("Rear left", "REAR_LEFT"),
                          ("Front right", "FRONT_RIGHT"),
                          ("Rear right", "REAR_RIGHT")):
        out.append((label,
                    const(rel, prefix + suffix[0]),
                    const(rel, prefix + suffix[1])))
    return out
