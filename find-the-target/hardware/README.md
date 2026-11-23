# Find the Target hardware

The circuit boards for Find the Target, drawn in [KiCad](https://www.kicad.org/) (free).

## Sensor board ("Find The Target Sender")

This is the board on the vehicle that carries the sensor ESP32. It runs
`VehicleSensors_v5` (or the older `ESPNow_Sender_v4`). A 38-pin ESP32 plugs into its sockets,
and the board connects it to:

- the FK-A1 GPS and compass (JST-SH cable, J4);
- the HC-SR04 range finder (the board divides the 5 V echo down to 3.3 V);
- the BMP280 temperature and pressure sensor on the same I2C pins as the compass;
- the NeoPixel LED ring, through a 74AHCT125 level shifter;
- 5 V power through a polyfuse, reverse-polarity MOSFET and surge diode.

The **latest revision is Rev 0.5**: 2 layers, 85 × 85 mm, all parts on the top side.

| Folder | What is in it |
|---|---|
| [`sensor-board/rev-0.5/kicad/`](sensor-board/rev-0.5/kicad/) | The KiCad project. Open `FindTheTarget_PCB_v1.kicad_pro`. (The file name still says v1; the title block says Rev 0.5.) The project's own symbol and footprint libraries sit next to it. |
| [`sensor-board/rev-0.5/order-files/`](sensor-board/rev-0.5/order-files/) | Everything a board house needs: Gerbers and drill files (zip), the parts list with manufacturer part numbers, the pick-and-place file, and **assembly notes to paste into the order**. |
| [`sensor-board/rev-0.5/review/`](sensor-board/rev-0.5/review/) | PDFs and pictures to look at without KiCad: schematic, board layout, 3D renders, and the clean design-rule (DRC) and electrical-rule (ERC) reports. |
| [`sensor-board/legacy/v1/`](sensor-board/legacy/v1/) | The first board (August 2026). Kept for reference; do not order it. |

### To order more boards

1. Upload `order-files/FindTheTarget_Rev0.5_Gerbers.zip` to the board house.
2. Choose 2 layers, 1.6 mm, FR-4. The other settings are in `ASSEMBLY_NOTES.txt`.
3. For assembled boards, also upload the BOM and Centroid files, and paste the
   "Notes for the assembler" section of `ASSEMBLY_NOTES.txt` into the order comments.
   **Solder jumper JP1 must be bridged to "B"** (level-shifted LED data).
4. Buy the parts listed under "Not on the board" in the same file: the ESP32, the FK-A1
   with its cable, JST-XH plugs, the LED ring, and M3 screws and standoffs.

The Rev 0.5 Gerbers were checked on 2026-11-22: exporting them again from the KiCad project
in this folder gives identical copper, mask, paste, silkscreen and outline layers. The last
change (D3 swapped to the MBR230LSFT1G diode on 2026-11-06) kept the same footprint, so it
only changed the parts list.

### If you change the board

1. Make the change in KiCad, then run **Inspect → Design Rules Checker** and fix what it finds.
2. Bump the revision in **File → Page Settings** (the title block) and on the silkscreen.
3. Make a new `rev-X.Y/` folder next to `rev-0.5/` with fresh Gerbers, BOM, centroid file,
   schematic PDF and DRC report, and move the old revision into `legacy/`.

KiCad also writes backup folders, a `.kicad_prl` file (your window layout) and lock files.
Git ignores those on purpose: only the project, schematic, board and library files belong here.
