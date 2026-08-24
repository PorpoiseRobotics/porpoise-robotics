# Porpoise Robotics

> Code, hardware notes, and documentation for the Porpoise Robotics vehicle and camera projects.

Porpoise Robotics is an educational robotics organization based in Rancho Bernardo, California,
built around the idea of teaching **STEM and AI through unmanned vehicles**. In the
organization's own words, its purpose is "to revolutionize STEM education through innovative
robotic platforms that combine cutting-edge technology with hands-on learning experiences,"
serving students and educators. Learn more at
[porpoiserobotics.org](https://www.porpoiserobotics.org/).

This repository is where the engineering work lives: source code, design documents, and build
notes for three projects.

---

## New to git? Start here.

**If you have never used git or GitHub before, do not start by reading this file.**
Go to the guides in [`docs/`](docs/) and work through them in order. They assume zero prior
experience — no terminal, no GitHub account, nothing.

| Read this | When |
|---|---|
| [Installing git and setting it up](docs/01-install-git.md) | First, once per computer |
| [The everyday workflow](docs/02-everyday-workflow.md) | Second, then keep it open while you work |
| [Plain-English glossary](docs/03-glossary.md) | Any time a word doesn't make sense |
| [When things go wrong](docs/04-when-things-go-wrong.md) | The moment anything looks scary |
| [Why empty folders have a `.gitkeep` file](docs/05-gitkeep-and-empty-folders.md) | When you notice those odd empty files |

Also read [CONTRIBUTING.md](CONTRIBUTING.md) before your first change. It is short on purpose.

---

## What is in this repository

Three projects, each in its own top-level folder — plus one folder holding code they share:

### [`homeland-security-camera/`](homeland-security-camera/README.md)
A security camera system intended for **deployment at a school**. Because it involves a camera
system on a campus, this project has stricter rules about what may be written down here — see
its README before contributing. It now has its first code: a camera node and a perimeter sensor
node that talk to each other over radio, with no router or server between them.

### [`ground-vehicle/`](ground-vehicle/README.md)
A ground-based unmanned vehicle platform — a rover that drives on land. Holds the Pathfinder
vehicle firmware and the "Find the Target" mission, in which a rover that spots the target
announces it to every other rover on the field.

### [`submersible-vehicle/`](submersible-vehicle/README.md)
An underwater vehicle platform. Porpoise Robotics publicly lists two ROV platforms, Explorer ROV
and Pathfinder ROV; the team should confirm which of these this folder covers and record that in
the project README.

### [`shared/`](shared/README.md)
Not a project — code that more than one project uses, so a fix made once is a fix everywhere.
Today that is [**PorpoiseNet**](shared/PorpoiseNet/README.md), the two-way ESP-NOW radio layer
that both the Find the Target game and the security camera system run on.

Every *project* folder has the same starter layout:

```
project-name/
├── README.md      what this project is and how to work on it
├── src/           source code
├── docs/          design notes, diagrams, decisions
└── hardware/      wiring notes, bill of materials, mechanical files
```

**Several of those folders are still empty.** That is expected — the code has not been written
yet. Each empty one contains a file called `.gitkeep`, which exists purely so that git will keep
the empty folder. [Here is why that is necessary.](docs/05-gitkeep-and-empty-folders.md) Delete a
`.gitkeep` once its folder has real files in it.

`ground-vehicle/` and `homeland-security-camera/` both have working code in `src/`; see their
READMEs for what is in there.

---

## This repository is public

Anyone on the internet can read everything in here, including its full history. That is a
deliberate decision by the organization. It means one rule matters more than any other:

**Do not commit anything sensitive.** No passwords, no API keys, no camera credentials, no
network or IP addresses, no school floor plans or camera placement maps, no student information.
Details and the safe way to handle configuration are in
[CONTRIBUTING.md](CONTRIBUTING.md#rule-4-never-commit-secrets).

---

## Status

Early. `ground-vehicle/` holds the team's existing ESP32 and Python code, moved in from a
OneDrive export. `homeland-security-camera/` has its radio layer and nothing above it — and its
privacy questions are still unanswered, which blocks any actual deployment. `submersible-vehicle/`
is still structure and documentation only. Sections marked **TBD** are waiting on the team.

## License

Not yet chosen — see [LICENSE](LICENSE). The organization needs to decide this before the
repository is published publicly.
