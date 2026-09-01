"""
diagrams.py - the drawn figures used in the Pathfinder course decks.

Everything here is built from native PowerPoint autoshapes and text boxes, so
each figure stays editable, scales without going fuzzy, and prints sharp in
black and white. Nothing is a bitmap.

Each function takes a Deck, adds one slide, and returns it.
"""

from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_CONNECTOR, MSO_SHAPE
from pptx.enum.text import MSO_ANCHOR, PP_ALIGN
from pptx.util import Emu, Inches, Pt

from slidelib import (AMBER, BODY_FONT, CODE_BG, CODE_FONT, CONTENT_W,
                      EMU_PER_PT, GREY, INK, MARGIN_L, MIN_SMALL_PT, NAVY,
                      MIN_CODE_PT, PANEL, RED, RULE, TEAL, WHITE, BODY_TOP,
                      measure_pt,
                      _set_fitted, _set_text)

LIGHT_TEAL = RGBColor(0xD6, 0xEC, 0xEE)
LIGHT_AMBER = RGBColor(0xFA, 0xE6, 0xC8)
LIGHT_RED = RGBColor(0xF6, 0xD8, 0xD8)
LIGHT_GREY = RGBColor(0xE8, 0xEC, 0xF0)


# ===================================================================
# small drawing helpers
# ===================================================================

def _box(slide, left, top, width, height, text, *, fill=WHITE, edge=NAVY,
         size=13, bold=False, color=INK, shape=MSO_SHAPE.ROUNDED_RECTANGLE,
         font=BODY_FONT, edge_w=1.25, align=PP_ALIGN.CENTER,
         anchor=MSO_ANCHOR.MIDDLE):
    box = slide.shapes.add_shape(shape, left, top, width, height)
    if shape == MSO_SHAPE.ROUNDED_RECTANGLE:
        box.adjustments[0] = 0.12
    box.fill.solid()
    box.fill.fore_color.rgb = fill
    box.line.color.rgb = edge
    box.line.width = Pt(edge_w)
    box.shadow.inherit = False

    frame = box.text_frame
    frame.word_wrap = True
    frame.margin_left = Inches(0.05)
    frame.margin_right = Inches(0.05)
    frame.margin_top = Inches(0.02)
    frame.margin_bottom = Inches(0.02)
    frame.vertical_anchor = anchor
    # A box holding CODE follows the code floor, not the prose floor - the
    # same rule the full-slide listings use.
    floor = MIN_CODE_PT if font == CODE_FONT else MIN_SMALL_PT
    _set_fitted(frame, text if isinstance(text, list) else [text],
                width=width, height=height, size=size, floor=floor,
                bold=bold, color=color, align=align, space_after=0,
                font=font)
    return box


def _label(slide, left, top, width, text, *, size=12, bold=False, color=INK,
           align=PP_ALIGN.LEFT, font=BODY_FONT, height=None):
    # A label with no explicit height gets one that suits how many lines it
    # holds, so a multi-line note is never squeezed into a single-line box.
    # Size it for the font it will ACTUALLY render at: the house minimum wins
    # over whatever the caller asked for, so a 12pt request still gets a box
    # tall enough for 18pt text.
    effective = max(size, MIN_SMALL_PT)
    if height is None:
        # Measure the text properly rather than counting list items: a short
        # label in a narrow box still wraps onto a second line at 18pt, and
        # counting items would size the box for one.
        items = text if isinstance(text, list) else [text]
        wanted = measure_pt(items, width / EMU_PER_PT, effective,
                            font=font, space_after=1)
        height = Inches(max(0.34, (wanted + 6) / 72.0))
    box = slide.shapes.add_textbox(left, top, width, height)
    frame = box.text_frame
    frame.word_wrap = True
    frame.margin_left = 0
    frame.margin_top = 0
    frame.margin_right = 0
    frame.margin_bottom = 0
    _set_fitted(frame, text if isinstance(text, list) else [text],
                width=width, height=height, size=size, floor=MIN_SMALL_PT,
                bold=bold, color=color, align=align, space_after=1, font=font)
    return box


def _arrow(slide, x1, y1, x2, y2, *, color=NAVY, width=1.75, dashed=False):
    line = slide.shapes.add_connector(MSO_CONNECTOR.STRAIGHT, x1, y1, x2, y2)
    line.line.color.rgb = color
    line.line.width = Pt(width)
    if dashed:
        from pptx.enum.dml import MSO_LINE_DASH_STYLE
        line.line.dash_style = MSO_LINE_DASH_STYLE.DASH
    # python-pptx has no arrowhead API, so set it on the XML directly.
    tail = line.line._get_or_add_ln()
    from pptx.oxml.ns import qn
    head = tail.makeelement(qn("a:headEnd"), {})
    end = tail.makeelement(qn("a:tailEnd"), {"type": "triangle", "w": "med",
                                             "len": "med"})
    tail.append(head)
    tail.append(end)
    return line


def _plain_line(slide, x1, y1, x2, y2, *, color=GREY, width=1.0):
    line = slide.shapes.add_connector(MSO_CONNECTOR.STRAIGHT, x1, y1, x2, y2)
    line.line.color.rgb = color
    line.line.width = Pt(width)
    return line


# ===================================================================
# 1. the three parts of a program
# ===================================================================

def program_structure(deck, title="Every program has the same three parts"):
    slide = deck.blank(title)

    # Named the way the course names them: INITIALIZATION, SETUP, LOOP. The
    # first has no keyword of its own in C++ - it is simply the top of the
    # file - but giving it a name makes the three-part shape easy to hold on to.
    parts = [
        ("1.  INITIALIZATION", "Libraries, constants, variables.\nThe top of the file.",
         "#include <Adafruit_NeoPixel.h>\nconst int LED_PIN = 5;", LIGHT_GREY),
        ("2.  SETUP", "setup() runs ONCE, at power-up.\nGet the hardware ready.",
         "void setup() {\n  pinMode(LED_PIN, OUTPUT);\n}", LIGHT_TEAL),
        ("3.  LOOP", "loop() runs OVER AND OVER,\nuntil the power goes off.",
         "void loop() {\n  digitalWrite(LED_PIN, HIGH);\n  delay(1000);\n}", LIGHT_AMBER),
    ]

    col_w = Inches(3.85)
    gap = Inches(0.32)
    top = BODY_TOP + Inches(0.15)

    for index, (heading, blurb, code, fill) in enumerate(parts):
        left = MARGIN_L + (col_w + gap) * index

        _box(slide, left, top, col_w, Inches(0.5), heading, fill=fill,
             edge=NAVY, size=14, bold=True, color=NAVY)

        _label(slide, left + Inches(0.1), top + Inches(0.68), col_w - Inches(0.2),
               blurb.split("\n"), size=13, color=INK)

        _box(slide, left, top + Inches(1.55), col_w, Inches(1.5),
             code.split("\n"), fill=CODE_BG, edge=RULE, size=11,
             font=CODE_FONT, shape=MSO_SHAPE.RECTANGLE, edge_w=0.75,
             align=PP_ALIGN.LEFT, anchor=MSO_ANCHOR.TOP)

        if index < 2:
            _arrow(slide, left + col_w + Inches(0.05), top + Inches(1.6),
                   left + col_w + gap - Inches(0.05), top + Inches(1.6))

    # The loop arrow, curling back on itself.
    loop_left = MARGIN_L + (col_w + gap) * 2
    _label(slide, loop_left, top + Inches(3.2), col_w,
           "and round again, thousands of times a second", size=12,
           color=AMBER, align=PP_ALIGN.CENTER, bold=True)

    deck._note(slide,
               "Ingredients, then preparation, then cooking. A 500-line vehicle "
               "program has exactly this shape - there is just more in each part.",
               "info")
    return slide


# ===================================================================
# 2. system block diagram
# ===================================================================

def system_block(deck, title="How a command gets from your thumb to a wheel",
                 controller="PS3 controller", link="Bluetooth"):
    slide = deck.blank(title)

    top = BODY_TOP + Inches(0.5)
    box_h = Inches(0.95)

    stages = [
        (controller, "you move a stick", LIGHT_TEAL, Inches(2.1)),
        (link, "radio, 2.4 GHz", WHITE, Inches(1.6)),
        ("ESP32", "your program runs here", LIGHT_TEAL, Inches(1.9)),
    ]

    left = MARGIN_L
    centres = []
    for name, blurb, fill, width in stages:
        _box(slide, left, top, width, box_h, name, fill=fill, size=15, bold=True,
             color=NAVY)
        _label(slide, left, top + box_h + Inches(0.06), width, blurb, size=11,
               color=GREY, align=PP_ALIGN.CENTER)
        centres.append((left, width))
        left += width + Inches(0.45)

    for index in range(len(stages) - 1):
        box_left, box_w = centres[index]
        _arrow(slide, box_left + box_w, top + Emu(int(box_h / 2)),
               box_left + box_w + Inches(0.45), top + Emu(int(box_h / 2)))

    # The ESP32 fans out to three kinds of output.
    esp_left, esp_w = centres[2]
    fan_left = esp_left + esp_w + Inches(0.45)
    outputs = [
        ("DRV8871 drivers", "4 motors", "12 13 16 17 18 19 22 23", LIGHT_AMBER),
        ("WS2812B strip", "32 LEDs", "GPIO 5", LIGHT_AMBER),
        ("Servo headers", "up to 4 servos", "25 26 27 14", LIGHT_AMBER),
    ]

    out_w = Inches(3.3)
    out_h = Inches(0.82)
    out_top = BODY_TOP + Inches(0.1)

    for index, (name, what, pins) in enumerate([(a, b, c) for a, b, c, _ in outputs]):
        y = out_top + (out_h + Inches(0.42)) * index
        _box(slide, fan_left + Inches(0.45), y, out_w, out_h,
             [name, what], fill=LIGHT_AMBER, edge=AMBER, size=13, bold=True,
             color=NAVY)
        _label(slide, fan_left + Inches(0.45), y + out_h + Inches(0.03), out_w,
               pins, size=10, color=GREY, align=PP_ALIGN.CENTER, font=CODE_FONT)
        _arrow(slide, fan_left, top + Emu(int(box_h / 2)),
               fan_left + Inches(0.45), y + Emu(int(out_h / 2)), color=AMBER)

    deck._note(slide,
               "Everything the vehicle can do happens on one of those three "
               "outputs. The rest of the program is deciding what to send.",
               "info")
    return slide


# ===================================================================
# 3. PWM duty cycle
# ===================================================================

def pwm_duty(deck, title="Pulse width modulation: how a pin makes a half speed"):
    slide = deck.blank(title)

    left = MARGIN_L + Inches(1.55)
    width = Inches(8.2)
    row_h = Inches(1.05)
    top = BODY_TOP + Inches(0.15)
    cycles = 4

    rows = [
        ("25% duty", 0.25, "quarter power", "ledcWrite(pin, 64)"),
        ("50% duty", 0.50, "half power", "ledcWrite(pin, 128)"),
        ("100% duty", 1.00, "full power", "ledcWrite(pin, 255)"),
    ]

    for index, (name, fraction, meaning, call) in enumerate(rows):
        y = top + (row_h + Inches(0.32)) * index
        high = y
        low = y + Inches(0.62)

        _label(slide, MARGIN_L, y + Inches(0.12), Inches(1.45), name, size=14,
               bold=True, color=NAVY)

        # Baseline
        _plain_line(slide, left, low, left + width, low, color=RULE, width=1.0)

        cycle_w = Emu(int(width / cycles))
        on_w = Emu(int(cycle_w * fraction))

        for c in range(cycles):
            x0 = left + cycle_w * c
            if on_w > 0:
                _plain_line(slide, x0, high, x0 + on_w, high, color=TEAL, width=2.5)
                _plain_line(slide, x0, low, x0, high, color=TEAL, width=2.5)
                if fraction < 1.0:
                    _plain_line(slide, x0 + on_w, high, x0 + on_w, low,
                                color=TEAL, width=2.5)
            if fraction < 1.0:
                _plain_line(slide, x0 + on_w, low, x0 + cycle_w, low,
                            color=TEAL, width=2.5)

        _label(slide, left + width + Inches(0.12), y + Inches(0.02),
               Inches(2.6), [meaning, call], size=11, color=GREY)

    _label(slide, left, top + Inches(4.35), width,
           "one cycle = 1/20000 second, so this whole picture happens 5000 times a second",
           size=11, color=GREY, align=PP_ALIGN.CENTER)

    deck._note(slide,
               "The pin is only ever fully on or fully off. Speed comes from "
               "the FRACTION of each cycle it spends on - the duty cycle.",
               "info")
    return slide


# ===================================================================
# 4. H-bridge
# ===================================================================

def h_bridge(deck, title="The H-bridge: four switches, four things a motor can do"):
    slide = deck.blank(title)

    states = [
        ("A high, B low", "FORWARD", "driven one way", LIGHT_TEAL, TEAL),
        ("A low, B high", "REVERSE", "driven the other way", LIGHT_TEAL, TEAL),
        ("both low", "COAST", "terminals floating,\nmotor freewheels", LIGHT_GREY, GREY),
        ("both high", "BRAKE", "terminals shorted,\nmotor stops hard", LIGHT_AMBER, AMBER),
    ]

    col_w = Inches(2.85)
    gap = Inches(0.28)
    top = BODY_TOP + Inches(0.35)

    for index, (pins, name, meaning, fill, edge) in enumerate(states):
        left = MARGIN_L + (col_w + gap) * index

        _box(slide, left, top, col_w, Inches(0.48), pins, fill=WHITE, edge=RULE,
             size=13, font=CODE_FONT, shape=MSO_SHAPE.RECTANGLE, edge_w=0.75)

        _box(slide, left, top + Inches(0.55), col_w, Inches(0.72), name,
             fill=fill, edge=edge, size=18, bold=True, color=NAVY)

        _label(slide, left, top + Inches(1.4), col_w, meaning.split("\n"),
               size=13, color=INK, align=PP_ALIGN.CENTER)

    # The bridge itself, drawn once underneath.
    bx = MARGIN_L + Inches(3.4)
    by = top + Inches(2.35)
    bw = Inches(6.4)
    bh = Inches(1.9)

    _plain_line(slide, bx, by, bx + bw, by, color=NAVY, width=2.0)
    _plain_line(slide, bx, by + bh, bx + bw, by + bh, color=NAVY, width=2.0)
    _plain_line(slide, bx, by, bx, by + bh, color=NAVY, width=2.0)
    _plain_line(slide, bx + bw, by, bx + bw, by + bh, color=NAVY, width=2.0)

    mid_y = by + Emu(int(bh / 2))
    _plain_line(slide, bx, mid_y, bx + Inches(2.4), mid_y, color=NAVY, width=2.0)
    _plain_line(slide, bx + bw - Inches(2.4), mid_y, bx + bw, mid_y, color=NAVY,
                width=2.0)

    _box(slide, bx + Inches(2.4), mid_y - Inches(0.35), Inches(1.6), Inches(0.7),
         "MOTOR", fill=WHITE, edge=NAVY, size=13, bold=True,
         shape=MSO_SHAPE.OVAL)

    _label(slide, bx - Inches(1.05), mid_y - Inches(0.5), Inches(1.0),
           "IN1\n(pin A)", size=12, bold=True, color=TEAL, align=PP_ALIGN.RIGHT)
    _label(slide, bx + bw + Inches(0.1), mid_y - Inches(0.5), Inches(1.1),
           "IN2\n(pin B)", size=12, bold=True, color=TEAL)
    _label(slide, bx + Inches(2.6), by - Inches(0.36), Inches(1.4), "+ V",
           size=12, bold=True, color=GREY, align=PP_ALIGN.CENTER)
    _label(slide, bx + Inches(2.6), by + bh + Inches(0.06), Inches(1.4), "GND",
           size=12, bold=True, color=GREY, align=PP_ALIGN.CENTER)

    return slide


# ===================================================================
# 5. the 32-LED loop
# ===================================================================

def led_map(deck, title="Where every LED number is on the vehicle"):
    slide = deck.blank(title)

    left = MARGIN_L + Inches(0.9)
    width = Inches(10.6)
    led_w = Emu(int(width / 16))
    led_h = Inches(0.42)

    front_y = BODY_TOP + Inches(0.75)
    rear_y = BODY_TOP + Inches(2.85)

    _label(slide, left, front_y - Inches(0.4), width, "FRONT   -   data goes in at LED 0",
           size=13, bold=True, color=NAVY, align=PP_ALIGN.CENTER)
    _label(slide, left, rear_y + led_h + Inches(0.1), width,
           "REAR   -   numbering runs the other way, right to left",
           size=13, bold=True, color=NAVY, align=PP_ALIGN.CENTER)

    for i in range(16):
        fill = LIGHT_AMBER if i < 8 else LIGHT_TEAL
        _box(slide, left + led_w * i, front_y, led_w, led_h, str(i), fill=fill,
             edge=NAVY, size=11, shape=MSO_SHAPE.RECTANGLE, edge_w=0.75)

    for slot in range(16):
        number = 16 + slot                       # 16 at the right, 31 at the left
        fill = LIGHT_TEAL if number <= 23 else LIGHT_AMBER
        x = left + width - led_w * (slot + 1)    # so 16 sits under 15
        _box(slide, x, rear_y, led_w, led_h, str(number), fill=fill, edge=NAVY,
             size=11, shape=MSO_SHAPE.RECTANGLE, edge_w=0.75)

    # Side labels
    _label(slide, MARGIN_L - Inches(0.05), front_y + Inches(0.85), Inches(0.9),
           ["LEFT", "side"], size=13, bold=True, color=AMBER, align=PP_ALIGN.CENTER)
    _label(slide, left + width + Inches(0.06), front_y + Inches(0.85), Inches(1.0),
           ["RIGHT", "side"], size=13, bold=True, color=TEAL, align=PP_ALIGN.CENTER)

    # The mirror pairs
    for p in (0, 5, 10, 15):
        x = left + led_w * p + Emu(int(led_w / 2))
        _plain_line(slide, x, front_y + led_h, x, rear_y, color=RULE, width=1.0)

    _label(slide, left, front_y + Inches(1.05), width,
           "front LED p  and  rear LED 31 - p  are directly opposite each other",
           size=13, bold=True, color=TEAL, align=PP_ALIGN.CENTER)
    _label(slide, left, front_y + Inches(1.38), width,
           "0 pairs with 31      5 pairs with 26      10 pairs with 21      15 pairs with 16",
           size=12, color=GREY, align=PP_ALIGN.CENTER, font=CODE_FONT)

    deck._note(slide,
               "LEFT is 0-7 and 24-31.  RIGHT is 8-15 and 16-23.  The strip is one "
               "loop, so each side is in two pieces.", "info")
    return slide


# ===================================================================
# 6. deadzone and map
# ===================================================================

def deadzone_map(deck, title="From thumbstick to motor speed", stick_max=127,
                 deadzone=20):
    slide = deck.blank(title)

    ox = MARGIN_L + Inches(1.3)
    oy = BODY_TOP + Inches(4.05)
    w = Inches(6.4)
    h = Inches(3.6)

    # Axes
    _plain_line(slide, ox, oy, ox + w, oy, color=NAVY, width=1.75)
    _plain_line(slide, ox, oy, ox, oy - h, color=NAVY, width=1.75)

    _label(slide, ox, oy + Inches(0.12), w,
           "stick reading   0  to  {}".format(stick_max), size=12, color=GREY,
           align=PP_ALIGN.CENTER)
    _label(slide, ox - Inches(1.25), oy - h - Inches(0.05), Inches(1.15),
           ["motor", "speed", "0 to 255"], size=12, color=GREY,
           align=PP_ALIGN.RIGHT)

    dz_frac = deadzone / float(stick_max)
    dz_x = ox + Emu(int(w * dz_frac))

    # Deadzone band
    band = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, ox, oy - h,
                                  Emu(int(w * dz_frac)), h)
    band.fill.solid()
    band.fill.fore_color.rgb = LIGHT_RED
    band.line.fill.background()
    band.shadow.inherit = False
    band.text_frame.text = ""

    # Flat part, then the ramp
    _plain_line(slide, ox, oy, dz_x, oy, color=RED, width=3.0)
    _plain_line(slide, dz_x, oy, ox + w, oy - h, color=TEAL, width=3.0)

    _label(slide, ox, oy - h - Inches(0.42), Emu(int(w * dz_frac)) + Inches(0.6),
           "DEADZONE\nanswer is 0", size=11, bold=True, color=RED,
           align=PP_ALIGN.CENTER)

    _label(slide, dz_x + Inches(0.3), oy - h + Inches(0.1), Inches(4.0),
           "map() stretches what is left\nacross the whole speed range",
           size=13, bold=True, color=TEAL)

    _label(slide, dz_x - Inches(0.5), oy + Inches(0.36), Inches(1.2),
           str(deadzone), size=12, color=RED, align=PP_ALIGN.CENTER,
           font=CODE_FONT)

    # The code, alongside
    code_left = ox + w + Inches(1.15)
    code_w = Inches(3.6)
    _label(slide, code_left, BODY_TOP + Inches(0.15), code_w,
           "stickToSpeed()", size=14, bold=True, color=TEAL, font=CODE_FONT)

    lines = [
        "if (abs(v) < DEADZONE)",
        "    return 0;",
        "",
        "size = map(abs(v),",
        "           DEADZONE, {},".format(stick_max),
        "           MOTOR_MIN, maxSpeed);",
        "",
        "return (v > 0) ? size",
        "                : -size;",
    ]
    _box(slide, code_left, BODY_TOP + Inches(0.55), code_w, Inches(2.5), lines,
         fill=CODE_BG, edge=RULE, size=11, font=CODE_FONT,
         shape=MSO_SHAPE.RECTANGLE, edge_w=0.75,
         align=PP_ALIGN.LEFT, anchor=MSO_ANCHOR.TOP)

    _label(slide, code_left, BODY_TOP + Inches(3.25), code_w,
           ["Why a deadzone at all?",
            "A worn stick does not return",
            "to exactly zero. Without this",
            "the vehicle creeps away on",
            "its own."],
           size=12, color=INK)

    return slide


# ===================================================================
# 7. tank drive mixing
# ===================================================================

def tank_mixing(deck, title="Mixing: one stick, two sides"):
    slide = deck.blank(title)

    _label(slide, MARGIN_L, BODY_TOP, CONTENT_W,
           "left = forward + turn            right = forward - turn",
           size=22, bold=True, color=TEAL, align=PP_ALIGN.CENTER)

    cases = [
        ("stick straight up", "forward 200\nturn 0", "left 200\nright 200",
         "straight ahead", LIGHT_TEAL),
        ("up and right", "forward 200\nturn 60", "left 260 -> 255\nright 140",
         "curves right\nwhile moving", LIGHT_TEAL),
        ("hard right only", "forward 0\nturn 127", "left 127\nright -127",
         "spins on the spot", LIGHT_AMBER),
        ("stick centred", "forward 0\nturn 0", "left 0\nright 0",
         "both sides coast", LIGHT_GREY),
    ]

    col_w = Inches(2.85)
    gap = Inches(0.28)
    top = BODY_TOP + Inches(0.75)

    for index, (what, inputs, outputs, meaning, fill) in enumerate(cases):
        left = MARGIN_L + (col_w + gap) * index

        _box(slide, left, top, col_w, Inches(0.45), what, fill=fill, edge=NAVY,
             size=13, bold=True, color=NAVY)

        _box(slide, left, top + Inches(0.6), col_w, Inches(0.85),
             inputs.split("\n"), fill=WHITE, edge=RULE, size=13, font=CODE_FONT,
             shape=MSO_SHAPE.RECTANGLE, edge_w=0.75)

        _arrow(slide, left + Emu(int(col_w / 2)), top + Inches(1.5),
               left + Emu(int(col_w / 2)), top + Inches(1.85))

        _box(slide, left, top + Inches(1.9), col_w, Inches(0.85),
             outputs.split("\n"), fill=CODE_BG, edge=TEAL, size=13,
             font=CODE_FONT, shape=MSO_SHAPE.RECTANGLE, edge_w=1.0)

        _label(slide, left, top + Inches(2.9), col_w, meaning.split("\n"),
               size=13, color=INK, align=PP_ALIGN.CENTER)

    deck._note(slide,
               "260 is over the maximum, which is exactly what constrain() is "
               "there to catch. Steering also has its own lower cap, turnMax, "
               "so the vehicle stays easy to aim.", "info")
    return slide


# ===================================================================
# 8. delay vs millis
# ===================================================================

def millis_timeline(deck, title="Why the vehicle programs never call delay()"):
    slide = deck.blank(title)

    left = MARGIN_L + Inches(1.9)
    width = Inches(9.3)

    # ---- with delay ----
    y = BODY_TOP + Inches(0.45)
    _label(slide, MARGIN_L, y - Inches(0.02), Inches(1.8),
           ["with delay()", "one job only"], size=13, bold=True, color=RED)

    segments = [("blink", 0.10, LIGHT_TEAL), ("frozen - delay(1000)", 0.40, LIGHT_RED),
                ("blink", 0.10, LIGHT_TEAL), ("frozen - delay(1000)", 0.40, LIGHT_RED)]
    x = left
    for name, frac, fill in segments:
        seg_w = Emu(int(width * frac))
        _box(slide, x, y, seg_w, Inches(0.55), name, fill=fill, edge=RULE,
             size=11, shape=MSO_SHAPE.RECTANGLE, edge_w=0.75)
        x += seg_w

    _label(slide, left, y + Inches(0.62), width,
           "while it is frozen it is not reading the controller, not checking the "
           "battery, and not updating any other light",
           size=12, color=RED, align=PP_ALIGN.CENTER)

    # ---- with millis ----
    y = BODY_TOP + Inches(2.15)
    _label(slide, MARGIN_L, y - Inches(0.02), Inches(1.8),
           ["with millis()", "many jobs"], size=13, bold=True, color=TEAL)

    jobs = [
        ("drive motors", 0.02, TEAL),
        ("scanner frame", 0.08, AMBER),
        ("turn signal", 0.16, NAVY),
        ("battery check", 0.5, GREY),
    ]
    row_h = Inches(0.42)
    for row, (name, spacing, colour) in enumerate(jobs):
        ry = y + row_h * row
        _label(slide, left - Inches(1.85), ry + Inches(0.04), Inches(1.75), name,
               size=11, color=colour, align=PP_ALIGN.RIGHT)
        _plain_line(slide, left, ry + Inches(0.18), left + width, ry + Inches(0.18),
                    color=RULE, width=0.75)
        step = max(spacing, 0.02)
        count = int(1.0 / step)
        for tick in range(count + 1):
            tx = left + Emu(int(width * step * tick))
            if tx > left + width:
                break
            _plain_line(slide, tx, ry + Inches(0.06), tx, ry + Inches(0.30),
                        color=colour, width=2.0)

    _label(slide, left, y + row_h * 4 + Inches(0.12), width,
           "each job fires on its own schedule, and loop() never stops running",
           size=12, color=TEAL, align=PP_ALIGN.CENTER)

    _box(slide, MARGIN_L, BODY_TOP + Inches(4.55), CONTENT_W, Inches(0.72),
         "if (millis() - lastTime >= INTERVAL) {  lastTime = millis();  ...do the thing...  }",
         fill=CODE_BG, edge=TEAL, size=15, font=CODE_FONT,
         shape=MSO_SHAPE.RECTANGLE, edge_w=1.0)

    return slide


# ===================================================================
# 9. dead reckoning square
# ===================================================================

def square_path(deck, title="Dead reckoning: the vehicle has no idea where it is"):
    """
    The square the vehicle drives, with the four legs numbered and the two
    equations that predict it. Redrawn so the corner labels sit outside the
    path instead of on top of it.
    """
    slide = deck.blank(title)

    side = Inches(2.9)
    ox = MARGIN_L + Inches(1.5)
    oy = BODY_TOP + Inches(4.05)

    corners = [(ox, oy), (ox, oy - side), (ox + side, oy - side), (ox + side, oy)]

    # The four legs, drawn anticlockwise from the start.
    for index in range(4):
        x1, y1 = corners[index]
        x2, y2 = corners[(index + 1) % 4]
        _arrow(slide, x1, y1, x2, y2, color=TEAL, width=3.0)

    # Number each leg on the OUTSIDE of the path.
    leg_labels = [
        (ox - Inches(1.75), oy - Emu(int(side / 2)) - Inches(0.2), "1  forward"),
        (ox + Emu(int(side / 2)) - Inches(0.8), oy - side - Inches(0.75), "2  forward"),
        (ox + side + Inches(0.3), oy - Emu(int(side / 2)) - Inches(0.2), "3  forward"),
        (ox + Emu(int(side / 2)) - Inches(0.8), oy + Inches(0.35), "4  forward"),
    ]
    for lx, ly, text in leg_labels:
        _label(slide, lx, ly, Inches(1.6), text, size=14, bold=True,
               color=NAVY, align=PP_ALIGN.CENTER)

    # Corner markers, small, just outside each turn.
    for index, (x, y) in enumerate(corners):
        dot = slide.shapes.add_shape(MSO_SHAPE.OVAL, x - Inches(0.13),
                                     y - Inches(0.13), Inches(0.26), Inches(0.26))
        dot.fill.solid()
        dot.fill.fore_color.rgb = AMBER
        dot.line.fill.background()
        dot.shadow.inherit = False
        _set_text(dot.text_frame, [""], size=8)

    _label(slide, ox - Inches(1.9), oy + Inches(0.30), Inches(1.7),
           "START", size=15, bold=True, color=TEAL, align=PP_ALIGN.RIGHT)
    _label(slide, ox - Inches(1.9), oy + Inches(0.72), Inches(1.7),
           "each dot is a", size=13, color=AMBER, align=PP_ALIGN.RIGHT)
    _label(slide, ox - Inches(1.9), oy + Inches(1.10), Inches(1.7),
           "90 degree turn", size=13, color=AMBER, align=PP_ALIGN.RIGHT)

    # The maths, well clear of the figure.
    right = MARGIN_L + Inches(6.6)
    col = Inches(5.6)

    _label(slide, right, BODY_TOP, col, "The two equations", size=19,
           bold=True, color=TEAL)

    _box(slide, right, BODY_TOP + Inches(0.5), col, Inches(1.15),
         ["distance = speed x time",
          "degrees turned = turn rate x time"],
         fill=CODE_BG, edge=TEAL, size=15, font=CODE_FONT,
         shape=MSO_SHAPE.RECTANGLE, edge_w=1.0, align=PP_ALIGN.LEFT)

    _label(slide, right, BODY_TOP + Inches(1.85), col,
           ["You know the TIME - you wrote it in the delay().",
            "You have to MEASURE the speed and the turn rate.",
            "",
            "wheel circumference = pi x 3.0 in = 9.42 in",
            "one wheel turn = 9.42 / 12 = 0.79 ft",
            "",
            "Drive it, measure one side, divide by the time, and",
            "you have feet per second for YOUR vehicle on THIS",
            "floor with THIS battery charge."],
           size=14, color=INK)

    deck._note(slide,
               "Recharge the battery and the same numbers give a bigger square. "
               "That drift is why dead reckoning is a starting point, not an "
               "answer.", "warn")
    return slide


# ===================================================================
# 10. RGB additive mixing
# ===================================================================

def rgb_mixing(deck, title="One pixel is three LEDs, and colour is a mixture"):
    slide = deck.blank(title)

    cx = MARGIN_L + Inches(3.0)
    cy = BODY_TOP + Inches(2.35)
    r = Inches(1.5)

    circles = [
        ("R", RGBColor(0xE8, 0x2C, 0x2C), cx, cy - Inches(0.85)),
        ("G", RGBColor(0x2C, 0xB4, 0x4A), cx - Inches(0.85), cy + Inches(0.6)),
        ("B", RGBColor(0x2C, 0x5A, 0xE8), cx + Inches(0.85), cy + Inches(0.6)),
    ]

    for name, colour, x, y in circles:
        circle = slide.shapes.add_shape(MSO_SHAPE.OVAL, x - r, y - r, r * 2, r * 2)
        circle.fill.solid()
        circle.fill.fore_color.rgb = colour
        circle.fill.transparency = 0.45
        circle.line.color.rgb = colour
        circle.line.width = Pt(1.5)
        circle.shadow.inherit = False
        _set_text(circle.text_frame, [""], size=8)

    _label(slide, MARGIN_L, cy + Inches(2.5), Inches(6.0),
           "where all three overlap you get white", size=13, color=GREY,
           align=PP_ALIGN.CENTER)

    right = MARGIN_L + Inches(6.6)
    col_w = Inches(6.1)

    _label(slide, right, BODY_TOP + Inches(0.05), col_w,
           "strip.Color(red, green, blue)", size=17, bold=True, color=TEAL,
           font=CODE_FONT)
    _label(slide, right, BODY_TOP + Inches(0.45), col_w,
           "Each number is 0 to 255 - how hard that one LED is driven.",
           size=14, color=INK)

    swatches = [
        ("(255,   0,   0)", "red", RGBColor(0xE8, 0x2C, 0x2C)),
        ("(  0, 255,   0)", "green", RGBColor(0x2C, 0xB4, 0x4A)),
        ("(  0,   0, 255)", "blue", RGBColor(0x2C, 0x5A, 0xE8)),
        ("(255, 255,   0)", "yellow", RGBColor(0xE8, 0xC8, 0x2C)),
        ("(255, 100,   0)", "amber - the turn signals", RGBColor(0xE8, 0x8A, 0x1A)),
        ("(255, 255, 255)", "white - and three times the current",
         RGBColor(0xDD, 0xDD, 0xDD)),
    ]

    row_h = Inches(0.52)
    for index, (code, name, colour) in enumerate(swatches):
        y = BODY_TOP + Inches(1.05) + row_h * index
        chip = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, right, y,
                                      Inches(0.55), Inches(0.34))
        chip.fill.solid()
        chip.fill.fore_color.rgb = colour
        chip.line.color.rgb = GREY
        chip.line.width = Pt(0.75)
        chip.shadow.inherit = False
        _set_text(chip.text_frame, [""], size=8)

        _label(slide, right + Inches(0.75), y + Inches(0.02), Inches(1.9), code,
               size=13, font=CODE_FONT, color=INK)
        _label(slide, right + Inches(2.8), y + Inches(0.02), Inches(3.3), name,
               size=13, color=GREY)

    deck._note(slide,
               "Each of the three LEDs draws about 20 mA. White costs three times "
               "what red costs, and 32 pixels of white is nearly 2 amps.", "warn")
    return slide


# ===================================================================
# 11. servo pulse widths
# ===================================================================

def servo_pulse(deck, title="A servo listens to the LENGTH of a pulse"):
    slide = deck.blank(title)

    left = MARGIN_L + Inches(1.7)
    width = Inches(8.0)
    top = BODY_TOP + Inches(0.35)

    rows = [
        ("1.0 ms", 0.05, "one end of the travel", TEAL),
        ("1.5 ms", 0.075, "centred", NAVY),
        ("2.0 ms", 0.10, "the other end", AMBER),
    ]

    for index, (name, frac, meaning, colour) in enumerate(rows):
        y = top + Inches(1.25) * index
        base = y + Inches(0.68)

        _label(slide, MARGIN_L, y + Inches(0.2), Inches(1.6), name, size=15,
               bold=True, color=colour, font=CODE_FONT)

        _plain_line(slide, left, base, left + width, base, color=RULE, width=1.0)

        pulse_w = Emu(int(width * frac))
        _plain_line(slide, left, base, left, y + Inches(0.1), color=colour, width=2.5)
        _plain_line(slide, left, y + Inches(0.1), left + pulse_w, y + Inches(0.1),
                    color=colour, width=2.5)
        _plain_line(slide, left + pulse_w, y + Inches(0.1), left + pulse_w, base,
                    color=colour, width=2.5)

        _label(slide, left + pulse_w + Inches(0.15), y + Inches(0.16), Inches(4.5),
               meaning, size=13, color=INK)

    _label(slide, left, top + Inches(3.85), width,
           "the whole cycle is 20 ms, repeated 50 times a second, whatever the pulse length",
           size=12, color=GREY, align=PP_ALIGN.CENTER)

    _box(slide, MARGIN_L, BODY_TOP + Inches(4.4), CONTENT_W, Inches(0.85),
         ["The ESP32 counts duty, not microseconds. At 16-bit resolution one 20 ms cycle is 65536 counts:",
          "duty = microseconds * 65536 / 20000"],
         fill=CODE_BG, edge=TEAL, size=14, font=CODE_FONT,
         shape=MSO_SHAPE.RECTANGLE, edge_w=1.0, align=PP_ALIGN.LEFT)

    return slide


# ===================================================================
# 12. lighting state machine
# ===================================================================

def state_machine(deck, title="The lighting state machine"):
    slide = deck.blank(title)

    top = BODY_TOP + Inches(0.35)

    nodes = {
        "DISCONNECTED": (MARGIN_L + Inches(0.2), top, LIGHT_GREY, "green pulse\nno controller"),
        "PAIRING": (MARGIN_L + Inches(0.2), top + Inches(2.1), LIGHT_TEAL, "blue breathing\nwaiting to pair"),
        "STANDBY": (MARGIN_L + Inches(4.6), top + Inches(1.05), WHITE, "headlights + tail lights\nredrawn only when dirty"),
        "TURN_LEFT": (MARGIN_L + Inches(9.0), top - Inches(0.25), LIGHT_AMBER, "amber bar, 3 cycles"),
        "TURN_RIGHT": (MARGIN_L + Inches(9.0), top + Inches(1.05), LIGHT_AMBER, "amber bar, 3 cycles"),
        "KITT_SCANNER": (MARGIN_L + Inches(9.0), top + Inches(2.35), LIGHT_TEAL, "sweeping dot, 3 cycles"),
    }

    node_w = Inches(3.1)
    node_h = Inches(0.62)
    placed = {}

    for name, (x, y, fill, blurb) in nodes.items():
        _box(slide, x, y, node_w, node_h, name, fill=fill, edge=NAVY, size=13,
             bold=True, color=NAVY, font=CODE_FONT)
        _label(slide, x, y + node_h + Inches(0.03), node_w, blurb.split("\n"),
               size=11, color=GREY, align=PP_ALIGN.CENTER)
        placed[name] = (x, y)

    def centre_right(name):
        x, y = placed[name]
        return x + node_w, y + Emu(int(node_h / 2))

    def centre_left(name):
        x, y = placed[name]
        return x, y + Emu(int(node_h / 2))

    for source in ("DISCONNECTED", "PAIRING"):
        x1, y1 = centre_right(source)
        x2, y2 = centre_left("STANDBY")
        _arrow(slide, x1, y1, x2, y2, color=TEAL)

    for target in ("TURN_LEFT", "TURN_RIGHT", "KITT_SCANNER"):
        x1, y1 = centre_right("STANDBY")
        x2, y2 = centre_left(target)
        _arrow(slide, x1, y1, x2, y2, color=AMBER)

    _label(slide, MARGIN_L + Inches(3.5), top + Inches(0.55), Inches(1.4),
           "controller\nconnects", size=10, color=TEAL, align=PP_ALIGN.CENTER)
    _label(slide, MARGIN_L + Inches(7.75), top + Inches(0.35), Inches(1.3),
           "D-pad\nor button", size=10, color=AMBER, align=PP_ALIGN.CENTER)
    _label(slide, MARGIN_L + Inches(7.6), top + Inches(2.75), Inches(1.5),
           "each animation counts\nits own cycles and\nhands itself back",
           size=10, color=GREY, align=PP_ALIGN.CENTER)

    deck._note(slide,
               "One variable holds the mode, so two modes can never be half-on at "
               "once. Adding a mode is one enum value and one branch - not a new "
               "combination of flags to test.", "info")
    return slide


# ===================================================================
# 13. current signature
# ===================================================================

def current_signature(deck, title="What a healthy motor looks like to a current sensor"):
    slide = deck.blank(title)

    left = MARGIN_L + Inches(0.9)
    width = Inches(3.3)
    gap = Inches(0.55)
    top = BODY_TOP + Inches(0.55)
    height = Inches(2.5)

    traces = [
        ("HEALTHY", [0.08, 0.08, 0.95, 0.62, 0.45, 0.40, 0.38, 0.38],
         "spikes as it breaks away,\nthen settles", TEAL, LIGHT_TEAL),
        ("DISCONNECTED", [0.06, 0.06, 0.08, 0.07, 0.06, 0.07, 0.06, 0.06],
         "no spike, almost no draw", GREY, LIGHT_GREY),
        ("JAMMED", [0.08, 0.08, 0.98, 0.95, 0.94, 0.96, 0.95, 0.94],
         "draws a lot and never settles", RED, LIGHT_RED),
    ]

    for index, (name, points, meaning, colour, fill) in enumerate(traces):
        x0 = left + (width + gap) * index
        base = top + height

        panel = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, x0, top, width, height)
        panel.fill.solid()
        panel.fill.fore_color.rgb = fill
        panel.line.color.rgb = RULE
        panel.line.width = Pt(0.75)
        panel.shadow.inherit = False
        _set_text(panel.text_frame, [""], size=8)

        # baseline marker
        base_y = base - Emu(int(height * 0.10))
        _plain_line(slide, x0, base_y, x0 + width, base_y, color=GREY, width=1.0)

        step = Emu(int(width / (len(points) - 1)))
        for i in range(len(points) - 1):
            y1 = base - Emu(int(height * points[i] * 0.92))
            y2 = base - Emu(int(height * points[i + 1] * 0.92))
            _plain_line(slide, x0 + step * i, y1, x0 + step * (i + 1), y2,
                        color=colour, width=2.5)

        _label(slide, x0, top - Inches(0.38), width, name, size=15, bold=True,
               color=colour, align=PP_ALIGN.CENTER)
        _label(slide, x0, base + Inches(0.1), width, meaning.split("\n"), size=12,
               color=INK, align=PP_ALIGN.CENTER)

    _label(slide, left - Inches(0.85), top + Inches(2.05), Inches(0.8), "baseline",
           size=9, color=GREY, align=PP_ALIGN.RIGHT)

    _box(slide, MARGIN_L, BODY_TOP + Inches(4.25), CONTENT_W, Inches(1.0),
         ["Both thresholds are measured ABOVE the baseline that was just taken, not as absolute amps:",
          "peak - baseline >= 1.500 A     AND     average - baseline >= 1.000 A"],
         fill=CODE_BG, edge=TEAL, size=14, font=CODE_FONT,
         shape=MSO_SHAPE.RECTANGLE, edge_w=1.0, align=PP_ALIGN.LEFT)

    deck._note(slide,
               "Op 11.2 measured a baseline, printed it, then compared against "
               "fixed absolutes anyway - so a vehicle idling at half an amp failed "
               "every motor. Op 12 fixed it.", "warn")
    return slide


# ===================================================================
# 14. Ohm's law and the power law
# ===================================================================

def ohms_and_power_law(deck,
                       title="Ohm's law, and the power law you need for the LEDs"):
    """
    The two relationships the course keeps coming back to: sizing the LED
    current budget, reading the shunt resistor, and working out battery life.
    Drawn rather than borrowed, so it stays editable and prints sharp.
    """
    slide = deck.blank(title)

    top = BODY_TOP + Inches(0.35)
    tri_w = Inches(3.0)
    tri_h = Inches(2.2)

    groups = [
        ("OHM'S LAW", MARGIN_L + Inches(0.7), "V", "I", "R", TEAL, LIGHT_TEAL,
         ["V = I x R", "I = V / R", "R = V / I"],
         ["V  volts", "I  amps", "R  ohms"]),
        ("POWER LAW", MARGIN_L + Inches(6.9), "P", "I", "V", AMBER, LIGHT_AMBER,
         ["P = I x V", "I = P / V", "V = P / I"],
         ["P  watts", "I  amps", "V  volts"]),
    ]

    for name, left, apex, bl, br, edge, fill, formulas, legend in groups:
        _label(slide, left, top - Inches(0.42), tri_w, name, size=16, bold=True,
               color=edge, align=PP_ALIGN.CENTER)

        tri = slide.shapes.add_shape(MSO_SHAPE.ISOSCELES_TRIANGLE, left, top,
                                     tri_w, tri_h)
        tri.fill.solid()
        tri.fill.fore_color.rgb = fill
        tri.line.color.rgb = edge
        tri.line.width = Pt(1.5)
        tri.shadow.inherit = False
        _set_fitted(tri.text_frame, [""], width=tri_w, height=tri_h, size=10)

        # The divider under the apex, the way the mnemonic is always drawn.
        mid_y = top + Emu(int(tri_h * 0.56))
        _plain_line(slide, left + Inches(0.62), mid_y,
                    left + tri_w - Inches(0.62), mid_y, color=edge, width=1.75)

        _label(slide, left, top + Inches(0.42), tri_w, apex, size=26, bold=True,
               color=NAVY, align=PP_ALIGN.CENTER)
        _label(slide, left + Inches(0.35), mid_y + Inches(0.30), Inches(0.9), bl,
               size=22, bold=True, color=NAVY, align=PP_ALIGN.CENTER)
        _label(slide, left + tri_w - Inches(1.25), mid_y + Inches(0.30),
               Inches(0.9), br, size=22, bold=True, color=NAVY,
               align=PP_ALIGN.CENTER)

        _box(slide, left - Inches(0.15), top + tri_h + Inches(0.3),
             tri_w + Inches(0.3), Inches(1.0), formulas, fill=CODE_BG,
             edge=RULE, size=14, font=CODE_FONT, shape=MSO_SHAPE.RECTANGLE,
             edge_w=0.75, align=PP_ALIGN.CENTER)

        _label(slide, left - Inches(0.15), top + tri_h + Inches(1.42),
               tri_w + Inches(0.3), "     ".join(legend), size=12, color=GREY,
               align=PP_ALIGN.CENTER)

    # Worked example, between the two triangles.
    mid_left = MARGIN_L + Inches(4.15)
    _box(slide, mid_left, top - Inches(0.2), Inches(2.4), Inches(4.0),
         ["ONE NEOPIXEL",
          "at full white",
          "",
          "3 LEDs x 20 mA",
          "= 60 mA",
          "",
          "P = I x V",
          "= 0.06 x 5",
          "= 0.3 W",
          "",
          "x 32 pixels",
          "= 1.92 A",
          "= 9.6 W"],
         fill=PANEL, edge=TEAL, size=13, shape=MSO_SHAPE.RECTANGLE,
         edge_w=1.0, align=PP_ALIGN.CENTER)

    deck._note(slide,
               "Cover the quantity you want with your thumb and the triangle "
               "shows you the sum. These two turn up again in the LED budget, "
               "the shunt resistor, and how long your battery lasts.", "info")
    return slide


# ===================================================================
# 15. the LED circuit students wire in Lesson 1
# ===================================================================

def led_circuit(deck, title="The circuit you are about to build"):
    """
    GPIO -> resistor -> LED -> ground, drawn as a loop, with the Ohm's law
    working that picks the resistor. Native shapes, so it prints sharp and a
    teacher can retype the numbers for a different LED.
    """
    slide = deck.blank(title)

    left = MARGIN_L + Inches(0.6)
    top = BODY_TOP + Inches(0.55)
    w = Inches(6.6)
    h = Inches(2.6)

    # The loop itself.
    _plain_line(slide, left, top, left + w, top, color=NAVY, width=2.5)
    _plain_line(slide, left + w, top, left + w, top + h, color=NAVY, width=2.5)
    _plain_line(slide, left, top + h, left + w, top + h, color=NAVY, width=2.5)
    _plain_line(slide, left, top, left, top + h, color=NAVY, width=2.5)

    _box(slide, left - Inches(1.0), top - Inches(0.32), Inches(2.0),
         Inches(0.64), "ESP32  GPIO 2", fill=LIGHT_TEAL, edge=TEAL, size=14,
         bold=True, color=NAVY)
    _box(slide, left - Inches(0.95), top + h - Inches(0.32), Inches(1.9),
         Inches(0.64), "ESP32  GND", fill=LIGHT_GREY, edge=GREY, size=14,
         bold=True, color=NAVY)

    _box(slide, left + Inches(1.6), top - Inches(0.34), Inches(2.1),
         Inches(0.68), "220 ohm", fill=WHITE, edge=AMBER, size=15, bold=True,
         color=NAVY)
    _box(slide, left + w - Inches(0.55), top + Inches(0.85), Inches(1.1),
         Inches(0.9), "LED", fill=LIGHT_AMBER, edge=AMBER, size=15, bold=True,
         color=NAVY)

    _label(slide, left + w - Inches(2.35), top + Inches(0.72), Inches(1.7),
           "long leg  +", size=14, color=TEAL, bold=True, align=PP_ALIGN.RIGHT)
    _label(slide, left + w - Inches(2.35), top + Inches(1.68), Inches(1.7),
           "short leg  -", size=14, color=GREY, align=PP_ALIGN.RIGHT)

    _label(slide, left, top + h + Inches(0.35), w,
           "Current goes round ONE loop. The resistor may sit on either side "
           "of the LED - what matters is that it is in the loop.",
           size=14, color=INK, align=PP_ALIGN.CENTER)

    # The sizing calculation.
    right = MARGIN_L + Inches(8.1)
    col = Inches(4.1)

    _label(slide, right, BODY_TOP + Inches(0.1), col, "Why 220 ohms?",
           size=18, bold=True, color=TEAL)

    _box(slide, right, BODY_TOP + Inches(0.6), col, Inches(1.65),
         ["R = (supply - LED drop) / current",
          "",
          "  = (3.3 - 2.0) / 0.020",
          "  = 65 ohms"],
         fill=CODE_BG, edge=RULE, size=14, font=CODE_FONT,
         shape=MSO_SHAPE.RECTANGLE, edge_w=0.75, align=PP_ALIGN.LEFT)

    _label(slide, right, BODY_TOP + Inches(2.4), col,
           ["220 ohms is in the kit, and is a good choice:",
            "",
            "(3.3 - 2.0) / 220 = about 6 mA",
            "",
            "Dimmer than flat out, but easy to see and safe."],
           size=14, color=INK)

    deck._note(slide,
               "No resistor means one bright flash and a dead LED. An ESP32 pin "
               "should not be asked for more than about 20 mA, so erring high "
               "is the right way to err.", "warn")
    return slide


# ===================================================================
# 16. series and parallel
# ===================================================================

def series_parallel(deck, title="Series and parallel: two ways to wire two loads"):
    """
    The two circuit shapes side by side with the rules that go with them.
    Redrawn rather than lifted, because Kevin's originals are screenshots of a
    PDF in a browser window.
    """
    slide = deck.blank(title)

    gap = Inches(0.7)
    col = Emu(int((CONTENT_W - gap) / 2))
    top = BODY_TOP + Inches(0.5)

    for index, kind in enumerate(("series", "parallel")):
        left = MARGIN_L + (col + gap) * index
        edge = TEAL if kind == "series" else AMBER
        fill = LIGHT_TEAL if kind == "series" else LIGHT_AMBER

        _label(slide, left, BODY_TOP, col,
               "SERIES" if kind == "series" else "PARALLEL",
               size=18, bold=True, color=edge, align=PP_ALIGN.CENTER)

        bx, by = left + Inches(0.5), top + Inches(0.25)
        bw, bh = col - Inches(1.0), Inches(1.9)

        # Battery on the left edge of both.
        _plain_line(slide, bx, by, bx, by + bh, color=NAVY, width=2.0)
        _label(slide, bx - Inches(0.95), by + Inches(0.72), Inches(0.85),
               "battery", size=13, color=GREY, align=PP_ALIGN.RIGHT)

        if kind == "series":
            # One loop, two resistors in line.
            _plain_line(slide, bx, by, bx + bw, by, color=NAVY, width=2.0)
            _plain_line(slide, bx + bw, by, bx + bw, by + bh, color=NAVY, width=2.0)
            _plain_line(slide, bx, by + bh, bx + bw, by + bh, color=NAVY, width=2.0)
            _box(slide, bx + Inches(0.7), by - Inches(0.28), Inches(1.2),
                 Inches(0.56), "R1", fill=fill, edge=edge, size=14, bold=True)
            _box(slide, bx + bw - Inches(1.9), by - Inches(0.28), Inches(1.2),
                 Inches(0.56), "R2", fill=fill, edge=edge, size=14, bold=True)
            rules = ["One path. The SAME current everywhere.",
                     "",
                     "R total = R1 + R2",
                     "I total = I1 = I2",
                     "V total = V1 + V2"]
        else:
            # Two branches between the same two rails.
            mid = bx + Emu(int(bw / 2))
            _plain_line(slide, bx, by, bx + bw, by, color=NAVY, width=2.0)
            _plain_line(slide, bx, by + bh, bx + bw, by + bh, color=NAVY, width=2.0)
            _plain_line(slide, bx + bw, by, bx + bw, by + bh, color=NAVY, width=2.0)
            _plain_line(slide, mid, by, mid, by + bh, color=NAVY, width=2.0)
            _box(slide, mid - Inches(0.6), by + Inches(0.62), Inches(1.2),
                 Inches(0.6), "R1", fill=fill, edge=edge, size=14, bold=True)
            _box(slide, bx + bw - Inches(0.6), by + Inches(0.62), Inches(1.2),
                 Inches(0.6), "R2", fill=fill, edge=edge, size=14, bold=True)
            rules = ["Two paths. The SAME voltage across each.",
                     "",
                     "V total = V1 = V2",
                     "I total = I1 + I2",
                     "1/R total = 1/R1 + 1/R2"]

        _box(slide, left, top + Inches(2.5), col, Inches(1.75), rules,
             fill=WHITE, edge=RULE, size=15, shape=MSO_SHAPE.RECTANGLE,
             edge_w=0.75, align=PP_ALIGN.CENTER)

    deck._note(slide,
               "The LED circuit you built in Lesson 1 is a SERIES circuit: one "
               "loop, so the same current flows through the resistor and the "
               "LED. That is why the resistor protects the LED wherever you "
               "put it.", "info")
    return slide
