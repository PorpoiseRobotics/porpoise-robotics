# Find the Target

> A STEM mission activity: camera-equipped vehicles hunt for targets on a field while each
> team's laptop shows what its vehicle sees and where it is.

---

## What this project is

A classroom mission rather than a vehicle platform. Up to **nine stations** share one field.
A *station* is one laptop plus one vehicle, run by a team or by one person. Each vehicle
carries three ESP32 boards:

| Board | Program | Job |
|---|---|---|
| Drive board | Op Program 13 (in [`ground-vehicle/`](../ground-vehicle/README.md)) | Driving, with a game controller |
| Sensor board | `VehicleSensors_v5` | GPS, compass, range finder, temperature, LED ring |
| Camera | `VehicleCamera_v5` | Live video over WiFi |

The laptop has a fourth board on its USB port, the **station radio** (`StationRadio_v5`), and
runs the **dashboard**: live video, a map of the field with every vehicle, the sensor numbers,
and a chat where stations can message each other and drop marks on each other's maps.

**Start here: [`src/FindTheTarget_v5/README.md`](src/FindTheTarget_v5/README.md).** It walks
through setting everything up, step by step, for someone who has not done it before.

The teaching material for the activity lives in [`docs/`](docs/): the slide deck and the setup
guide students follow. They were written for v3/v4 (one vehicle) and have not been updated
for v5 yet.

## Layout

```
find-the-target/
├── README.md
├── src/
│   ├── FindTheTarget_v5/      current version: several stations, chat and shared map marks
│   │   ├── StationRadio_v5/     board on the laptop's USB port
│   │   ├── VehicleSensors_v5/   sensor board on the vehicle
│   │   ├── VehicleCamera_v5/    camera on the vehicle
│   │   └── Dashboard/           double-click "Start Dashboard.bat"
│   └── legacy/                superseded versions (v3, v4, the first mesh demo)
├── docs/                      slides, setup guide
└── hardware/                  sensor circuit board (KiCad), order files, parts list
```

Everything in `src/legacy/` is kept only so old builds can be looked up; do not start anything
new from it. See [`src/legacy/README.md`](src/legacy/README.md).

## How it works, in one picture

```
 STATION 1                         THE FIELD                       STATION 2
 laptop + dashboard                                               laptop + dashboard
      │ USB                                                             │ USB
 station radio (node 1) ◄──── PorpoiseNet radio, channel 1 ────► station radio (node 2)
                          ▲               ▲               ▲
                 vehicle 1 sensors   vehicle 2 sensors   ...  (node 11, 12, ...)

 vehicle 1 camera ──WiFi──► field router ◄──WiFi── laptops   (video only)
```

- The radio network is [PorpoiseNet](../shared/PorpoiseNet/README.md), shared with the security
  camera project. No router is needed for it, and every board repeats messages for the others,
  which stretches the range.
- Video is too big for that radio, so the cameras use a WiFi router. A small router with no
  internet is enough; set its 2.4 GHz channel to 6 or 11.
- Every board stores its own station number. Every station uploads exactly the same programs.

## WiFi names and passwords

They are **not** on GitHub (this repository is public). They live in the team's shared
OneDrive folder **Porpoise Robotics Private → WiFi networks.txt**. You type them into the
camera's own setup page (or the dashboard's Setup page) once; the camera remembers them.
Never type a real password into a file you commit.

## Safety and privacy

This project streams video from a camera and lets stations chat. Before running it anywhere
students are present, settle who may view the streams, whether anything is recorded, and where
recordings go. Chat messages travel over open radio: anyone nearby with an ESP32 can read them,
so stations use numbers, never names. The dashboard says so next to the chat box and keeps a
chat log on each laptop (`Dashboard/data/chat-log/`) so an adult can look back at what was said.
The [homeland security camera project](../homeland-security-camera/README.md) has stricter
rules for cameras — read those before pointing a camera at a campus.
