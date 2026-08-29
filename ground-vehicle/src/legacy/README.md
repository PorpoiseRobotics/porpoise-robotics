# Legacy Pathfinder programs

These programs have been replaced. They are kept so that old behaviour can be looked up and so
that a vehicle can be put back on a known-working program if something goes wrong with a newer
one.

**Do not start new work from anything in this folder.** Use the programs one level up in
[`src/`](../).

| Program | Replaced by | Why it was replaced |
|---|---|---|
| `Pathfinder_System_Test24_WORKING` | `pathfinder_ps3` | Motors ran full-speed-or-nothing, and LED patterns blocked the vehicle for two seconds at a time while holding the last motor command. |
| `PRPV_System_Test24_Gen3_Bluepad32` | `pathfinder_nintendoswitch` | Same structure as the PS3 program, ported to Bluepad32. Superseded by a version with proper mixing, edge-detected buttons, and one-controller-per-vehicle pairing. |
| `Pathfinder_Op_Program11.2` | `Pathfinder_Op_Program12` | Would no longer compile: FastLED 3.10.5 enables an ESP-DSP backend that does not build against the esp32_bluepad32 4.1.0 SDK. Op 12 also fixes a servo range that could drive servos past their end stops, and several other bugs. |

## A warning about building these

`Pathfinder_Op_Program11.2` **does not compile** with current library versions. The failure is in
FastLED, not in the sketch:

```
FastLED/src/fl/audio/fft/fft_backend.h
  -> esp_dsp.h  ->  dspm_matrix.h  ->  fatal error: dspm_add.h: No such file or directory
```

`Pathfinder_Op_Program12` replaces FastLED with Adafruit NeoPixel and builds cleanly. If you need
11.2 specifically, you will have to install an older FastLED.

The two System Test programs still build, but they need the board package they were written for —
`Pathfinder_System_Test24_WORKING` wants esp32 3.0.7 by Espressif, the other wants
esp32_bluepad32 4.1.0. The two packages are mutually exclusive in the Arduino IDE.
