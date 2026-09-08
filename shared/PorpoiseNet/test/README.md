# PorpoiseNet tests

**Nothing in this folder is firmware. You never flash any of it.** It runs on a
laptop and proves the network logic works before anyone carries six boards out
to a field.

```
./run_tests.sh
```

Needs `g++`. Prints a PASS/FAIL line per check and exits non-zero if any fail.

## What it does

`meshtest.cpp` builds **three real PorpoiseNet nodes inside one program** —
rover 11, the base station, and rover 12 in the middle — and connects them
through a fake radio in `stubs/fake_hardware.cpp` whose "who can hear whom"
table the test controls. Time is a variable the test advances by hand, so a
retry timeout that takes seconds in the field takes microseconds here.

That makes the miserable cases testable:

| Test | What it proves |
|---|---|
| everyone in range | a find reaches the base, is delivered exactly once, and is acknowledged |
| base out of range | rover 12 relays the find across, and the base still sees rover 11 as the origin — not the relay |
| no base at all | the sender retries, then gives up and says so, instead of hanging forever |
| the same frame four times | the duplicate filter delivers it to the sketch once |

Each node has to live in its own `.cpp` file, because `PorpoiseNet.h` is
header-only and each compilation unit gets its own private copy of the network
object. `run_tests.sh` generates the three files from `node.inc` with a
`sed` substitution.

## What the stubs are

`stubs/` contains just enough fake `Arduino.h`, `WiFi.h` and `esp_now.h` to
satisfy the compiler. They are not simulations of an ESP32 and they do not
behave like one. They exist so that `PorpoiseNet.h` — the real file, unmodified
— can be compiled and run on a computer.

This means the tests cover **logic**: duplicate filtering, relaying, hop
counting, acknowledgement and retry. They cannot tell you anything about radio
range, interference, timing under real load, or whether the channel is right.
Those still need two boards and a field.
