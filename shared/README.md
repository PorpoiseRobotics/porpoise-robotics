# Shared code

Code used by more than one of the three projects lives here, so that a fix
made once is a fix everywhere.

Until now the repository had no answer to "where does shared code go?" — it was
listed as an open question in
[`ground-vehicle/README.md`](../ground-vehicle/README.md). This folder is the
answer.

## What is in here

| Folder | What it is | Used by |
|---|---|---|
| [`PorpoiseNet/`](PorpoiseNet/) | Two-way ESP-NOW networking for ESP32 | ground vehicle, security camera |

## The rule for adding something here

Move code into `shared/` when a **second** project needs it, not when you think
one might. Two copies that have drifted apart are easier to untangle than one
shared thing that is being pulled in two directions by projects that never
really wanted the same behaviour.

When you do move something here, say in its README which projects depend on it,
because from then on changing it can break code you are not looking at.
