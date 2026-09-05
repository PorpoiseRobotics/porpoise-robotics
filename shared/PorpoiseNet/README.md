# PorpoiseNet — two-way ESP-NOW for every project

One networking library, shared by the **Find the Target** game and the
**homeland security camera** system. A rover and a camera are the same kind of
node as far as the radio is concerned, so they run the same code and only
differ in a node number and a role.

- **Two-way.** Every node can talk and listen. A rover that finds the target
  tells every other rover *and* the base station; the base can tell them all to
  stop, come home, or start a new round.
- **No MAC addresses to copy.** Boards find each other by themselves.
- **No router, no WiFi, no internet.** Works on a field with nothing but
  batteries.
- **Cheap.** An ESP32 costs a few dollars. That is the point — a fleet of six
  rovers plus four sensors and a camera costs less than one commercial sensor.

Everything below is also written into the comments at the top of
[`src/PorpoiseNet.h`](src/PorpoiseNet.h), so a student reading the code never
has to go looking for a document.

---

## The picture

```
   FIND THE TARGET (the game)              SECURITY CAMERA (the school system)
   ┌──────────────┐                        ┌──────────────┐
   │ Rover 11     │                        │ Sensor 31    │  PIR + door switch
   │ finds target ├──┐                  ┌──┤ zone 3 trips │
   └──────────────┘  │                  │  └──────────────┘
   ┌──────────────┐  │   ESP-NOW        │  ┌──────────────┐
   │ Rover 12     ├──┤   broadcast      ├──┤ Sensor 32    │
   │ hears it     │  │   channel 1      │  └──────────────┘
   └──────────────┘  │   net id 7       │
                     ▼                  ▼
              ┌─────────────────────────────────┐
              │  every node hears every message │
              └────────┬───────────────┬────────┘
                       │               │
          ┌────────────▼───┐   ┌───────▼────────────┐
          │ Base 1         │   │ Camera 21          │
          │ on laptop USB  │   │ takes a picture,   │
          │ prints + JSON, │   │ saves to SD,       │
          │ sends commands │   │ says what it saved │
          └────────┬───────┘   └────────────────────┘
                   │ serial
          ┌────────▼───────────┐        the image itself NEVER goes over
          │ base_station.py    │        ESP-NOW — only the event and the
          │ → dashboard.html   │        receipt do. See "What the radio
          └────────────────────┘        carries" below.
```

The same base station serves both projects. So does the same camera: point it
at the field and it photographs whatever rover reports a find; point it at a
door and it photographs whatever trips the sensor. Nothing in it knows or cares
which project it is part of today.

---

## Install it

Pick one and tell the whole team which, because mixing them is how you end up
editing a file that is not the one being compiled.

**As a library** (better once more than one sketch uses it)

1. Copy this whole `PorpoiseNet` folder into `Documents/Arduino/libraries/`
2. Restart the Arduino IDE
3. Sketches use `#include <PorpoiseNet.h>`

**As a plain file** (fine for one board, and impossible to get wrong)

1. Copy `src/PorpoiseNet.h` into the sketch folder, next to the `.ino`
2. Sketches use `#include "PorpoiseNet.h"`

The sketches in this repository accept either — they check which one is there.
There is no `.cpp` file, on purpose: nothing to forget to copy.

---

## The three settings that must match

At the top of every sketch:

```cpp
#define MY_NODE_ID   11   // UNIQUE on this network
#define MY_NET_ID     7   // THE SAME on every board
#define MY_CHANNEL    1   // THE SAME on every board
```

| Setting | Rule | What goes wrong if you get it wrong |
|---|---|---|
| `MY_NODE_ID` | different on every board | two boards share an identity; the roster shows one node that keeps contradicting itself, and messages get filtered as duplicates of each other |
| `MY_NET_ID` | identical on every board | total silence between the mismatched boards, with no error printed anywhere |
| `MY_CHANNEL` | identical on every board | total silence, again with no error. **This is the number one cause of "it doesn't work".** |

Node numbering used across this repository:

| Range | Role |
|---|---|
| `1` | base station |
| `10–19` | ground vehicles / rovers |
| `20–29` | cameras |
| `30–39` | fixed sensors |

`0` is reserved and means "everybody".

Write the node number on the board itself with a marker. Ten minutes before a
demo, with six identical boards on a table, this is worth more than any
software feature in here.

---

## Bring-up order

1. **Base station first.** Flash `PorpoiseNet_Base`, open the serial monitor at
   115200. It prints its identity and then waits.
2. **One rover.** Flash `PorpoiseNet_Vehicle` with `MY_NODE_ID 11`. Within a
   few seconds it should appear in the base station's roster, and the base
   should appear in the rover's.
3. **Prove it two ways.** Type `ping` at the base station — the rover answers.
   Press the rover's FIND button — the base prints `TARGET FOUND` and the rover
   prints that it was acknowledged. That is the full round trip in both
   directions, tested in about fifteen seconds.
4. **Then everything else.** Each new board needs only its own `MY_NODE_ID`.
   Nothing about the existing boards changes — no re-flashing, no MAC
   addresses.

If step 3 works, the network is working, and any later problem is in a sketch
rather than in the radio.

---

## Reading the roster

Every node prints this every few seconds:

```
# ROSTER (node 1 base, net 7, ch 1)
#   id 11  vehicle  rssi -47dBm  heard 1s ago  rx 84  lost 0
#   id 12  vehicle  rssi -78dBm  heard 2s ago  rx 79  lost 5
#   id 21  camera   rssi -61dBm  heard 1s ago  rx 22  lost 0
#   links: sent 91  received 185  duplicates ignored 34  not ours 0
```

- **A board that is not in the roster is not on the network.** Do not debug the
  board that is printing the roster; debug the one that is missing.
- **`rssi`** is signal strength in dBm. Always negative, closer to zero is
  better. `-30` is across the table, `-70` is fine, `-85` is about to start
  losing messages.
- **`lost`** counts gaps in that node's message numbering — messages that were
  sent and never arrived. `rx 84 lost 0` is a healthy link. A link can be "up"
  and still be losing a third of everything, and this is the column that says
  so.
- **`duplicates ignored`** is normal and healthy. Radios repeat themselves and
  relays repeat each other; this is the count of echoes that were correctly
  thrown away.
- **`not ours`** counts messages with a different `MY_NET_ID` or a different
  protocol version — another team on your channel, or one board still running
  last term's firmware.

---

## Range, and what to do when the field is bigger than the radio

Expect roughly **100–200 m outdoors with line of sight**, much less through
buildings, and noticeably less with a person standing between two antennas.
Two ways to get more, and they stack:

**Relaying** is on by default. Any node repeats a message it hears, once, with
a hop counter that stops it echoing forever. A rover in the middle of the field
extends the reach of one at the far end without being told to, and without
anybody configuring a route. This is tested — see [`test/`](test/).

**Long range mode** trades data rate for distance, roughly two to four times:

```cpp
PorpoiseNet.enableLongRange();
```

Two hard rules. **Every** node must call it — an LR radio and a normal radio
cannot hear each other at all, and neither reports an error. And an LR node
**cannot join a WiFi router**, so never call it on the camera bridge.

---

## The channel trap

Read this before debugging a camera that "worked yesterday".

An ESP32 has **one** radio and can be on **one** channel. The moment a board
joins a WiFi router, the router decides that channel — and the board silently
stops hearing every other PorpoiseNet node. Nothing prints an error. The mesh
just goes quiet.

This affects every board that uses real WiFi. Today that is **the base station**
(`USE_WIFI 1`, on by default) and the camera when `USE_WIFI_UPLOAD` is on.

**The base station case is the one that matters most**, because everything on
the network talks to it. If the router puts the base on channel 6 while every
rover is still set to `MY_CHANNEL 1`, the whole fleet is deaf to the base and
the base's roster stays empty — with no error printed anywhere. The base sketch
prints the channel it landed on at boot, in capitals, and prints a second louder
warning if that number disagrees with its own `MY_CHANNEL`. Read the first ten
lines of its serial output before debugging anything else.

Three ways out, best first:

1. **Lock the router to a fixed channel.** In the router's admin page set the
   2.4 GHz channel to `1` (not "Auto"), then use `MY_CHANNEL 1` everywhere.
   Simple and stable. Do this. "Auto" routers move channels on their own,
   usually overnight, which produces a fleet that worked yesterday and does not
   work this morning with nothing having changed.
2. **Let the WiFi board announce the channel.** The base station and the camera
   both connect to WiFi first, then start PorpoiseNet with
   `PN_CHANNEL_FOLLOW_WIFI`, which reads the channel the router handed out and
   prints it in capitals. Put that number in `MY_CHANNEL` on every other board.
   Works, but must be re-checked after any router change.
3. **Do not put WiFi and ESP-NOW on the same board.** Use two boards joined by
   two wires. Costs five dollars and removes the entire problem.

Related: a board joined to a router will power-save its radio and miss
messages. PorpoiseNet calls `WiFi.setSleep(false)` for you; do not turn it back
on.

---

## What the radio carries — and what it must never carry

An ESP-NOW frame holds **250 bytes**. A small JPEG is fifteen thousand. This is
not a matter of being patient: images do not go over this network, and the
design does not pretend otherwise.

What travels over ESP-NOW is the **event** and the **receipt**:

> "zone 3, motion, severity 2" → "saved `n21_000007.jpg`, 18 kB"

The picture itself goes to an SD card in the camera, or over ordinary WiFi to a
server you control. Small facts on the mesh, big data on real infrastructure.
That split is the architecture, not a limitation being worked around.

---

## Security and privacy — plainly

**These messages are not encrypted.** Anyone within a few hundred metres with a
five-dollar board can read every alert this network sends, and can forge one.
`MY_NET_ID` keeps two classes from crossing wires; it stops nobody who is
trying.

ESP-NOW *does* support encryption, but only for one-to-one peers — not for the
broadcast that makes "turn it on and it works" possible. Choosing broadcast was
a deliberate trade, and it is the right one for a teaching network and the
wrong one for anything that actually needs to be secure.

What follows, for a system that will run at a school:

- Never put anything in a message you would not shout across the quad. Zone
  **numbers**, never room names. No names of people, ever.
- Never let a message alone unlock, disarm, or disable anything physical.
- Camera images never touch the radio — see above.
- The privacy questions in
  [`homeland-security-camera/README.md`](../../homeland-security-camera/README.md)
  — who owns the footage, who may see it, how long it is kept, what the school
  has agreed to **in writing** — are not answered by any of this code and must
  be settled before anything is mounted on a wall.

And the repository rule that already cost this project once: **never commit a
password.** This repository is public, deleting a secret later does not
unpublish it, and there are real Wi-Fi passwords in its history right now
because of exactly this. Use the `secrets.h` pattern — see
[`secrets.example.h`](../../homeland-security-camera/src/PorpoiseNet_CameraNode/secrets.example.h).

---

## When it does not work

Work down this list in order. It is sorted by how often each one is the answer.

| What you see | Almost always |
|---|---|
| Roster empty, everything looks fine | `MY_CHANNEL` differs between boards. If the base station is on WiFi, the router chose its channel — read the base's boot output and match it. |
| Roster empty, and `not ours` is climbing | `MY_NET_ID` differs — you *are* hearing them, and correctly ignoring them. |
| One board missing from the roster | That board is not running. Open its serial monitor; it will say why. |
| `NO ACK` on every find | The base station is off, out of range, or on another channel. Nothing else answers acknowledgements. |
| Roster fine, messages arrive rarely | Range. Check `rssi` and `lost`. Move a rover into the middle to act as a relay. |
| Worked on the bench, dead in the field | A WiFi board joined a different router and dragged its channel with it. See the channel trap. |
| Worked yesterday, dead this morning | The router is on "Auto" and moved channel overnight. Lock it. |
| Two nodes flickering in and out of one roster row | Two boards share a `MY_NODE_ID`. |
| `DROPPED (loop too slow)` in the stats | Something in `loop()` blocks for too long — usually a `delay()`. |
| Camera captures nothing | It is `DISARMED`, or inside its cooldown window, or its SD card is missing. It prints which. |

---

## Message types

| Type | Direction | Carries | Acknowledged |
|---|---|---|---|
| `PN_HELLO` | everyone → everyone | nothing; keeps rosters fresh | no |
| `PN_TELEMETRY` | nodes → base | position, range, health | no |
| `PN_TARGET` | rover → everyone | the game find: position, confidence, note | **yes** |
| `PN_ALERT` | sensor → everyone | zone number, sensor kind, severity | **yes** |
| `PN_COMMAND` | base → nodes | start / stop / recall / arm / disarm / snapshot / ping / identify | no |
| `PN_EVIDENCE` | camera → everyone | what was saved, and which event it was for | no |
| `PN_ACK` | base → sender | generated automatically | — |
| `PN_TEXT` | any → any | a short string, for debugging | no |

Events that matter are acknowledged and resent until confirmed; streams are
not, because another reading is along in a second. That distinction is the
reason a find is not lost to one noisy moment.

Message types are numbered and **must never be renumbered** — a board that
spent the summer in a cupboard still speaks last term's numbers. Add new ones
at the end.

---

## Testing without hardware

[`test/`](test/) builds three nodes inside one program on a laptop, wires them
through a fake radio, and checks the awkward cases: the base out of range and
reachable only by relay, the same frame arriving four times, nobody answering
at all.

```
cd test && ./run_tests.sh
```

Needs `g++` and nothing else. No ESP32, no field, no batteries.
