# 5. Why empty folders have a `.gitkeep` file

## The short version

Look inside `ground-vehicle/src/` and you will find a single empty file named `.gitkeep`. Same
in `docs/` and `hardware/`, in every project. They look like clutter or like something went
wrong. They are neither.

**Git cannot store an empty folder.** The `.gitkeep` file is a harmless placeholder that gives
the folder something to hold, so it survives being committed.

That is the whole thing. The rest of this page explains why, because "the tool cannot do the
obvious thing" is unsatisfying without a reason.

---

## Why git cannot store an empty folder

Git does not actually track folders. It tracks **files**, and each file's path happens to
include the folders it sits in.

So when git records a file at `ground-vehicle/src/motor_control.py`, it is not storing "a folder
called `ground-vehicle`, containing a folder called `src`, containing a file." It is storing one
entry whose name is the entire path. The folders exist only as a side effect of that path.

Which means an empty folder has no file, therefore no path, therefore nothing for git to record.
When someone clones the project, git recreates folders by creating the files inside them — and
an empty folder has nothing to recreate.

This is not a bug or an oversight. It is a consequence of how git was designed, and it has
worked this way since 2005. It is also not going to change.

---

## What actually happens without a placeholder

Say you create the folders and commit:

```bash
mkdir -p ground-vehicle/src
git add .
git status
```

```
On branch main
nothing to commit, working tree clean
```

Git says there is nothing to commit, because from its point of view there genuinely is nothing.
The folder is on your computer and will simply never appear on GitHub — and never appear for
anyone who clones the project.

That is exactly the confusing outcome this convention prevents.

---

## What `.gitkeep` actually is

Here is the part that surprises people: **`.gitkeep` is not a git feature.** Git has never heard
of it. There is no code in git that looks for that name.

It is purely a convention the community agreed on. Any file would work — `placeholder.txt`,
`README.md`, `nothing-here` — because the trick is only that *a file exists*. The name
`.gitkeep` was chosen because it signals intent: "this exists to keep the folder."

The leading dot follows Unix convention for hidden files, so it stays out of your way in file
listings.

Two useful consequences:

- **If you cannot see them, that is normal.** Files starting with `.` are hidden by default.
  On macOS press `Cmd+Shift+.` in Finder; on Windows, enable "Hidden items" in File Explorer's
  View tab; in a terminal use `ls -a`.
- **Nothing in the project reads them.** They are inert. No script imports them, no build depends
  on them. They just sit there.

---

## What to do with them

**Leave them alone until the folder has real content.**

Once a folder has actual files in it, the `.gitkeep` has done its job and can be deleted:

```bash
git rm ground-vehicle/src/.gitkeep
git commit -m "Remove .gitkeep now that src/ has code"
```

There is no harm in leaving it either. Plenty of projects never bother. It is tidiness, not
correctness.

**When you create a new empty folder that others will need,** add one:

```bash
mkdir submersible-vehicle/calibration
touch submersible-vehicle/calibration/.gitkeep
git add submersible-vehicle/calibration/.gitkeep
git commit -m "Add calibration folder for submersible"
```

On Windows in Git Bash, `touch` works the same way. In PowerShell, use
`New-Item -ItemType File .gitkeep`.

---

## The related trap: a folder that vanishes when you delete files

You will hit this eventually. You delete the last file out of a folder, commit, and a teammate
pulls — and the folder is gone on their machine.

Same cause. Git removed the only file, so the folder had no reason to exist, so git did not
recreate it. If that folder needs to stay, add a `.gitkeep` before you commit the deletion.

---

## In short

| Question | Answer |
|---|---|
| Can git store an empty folder? | No. It tracks files, not folders. |
| Is `.gitkeep` a git feature? | No. Just a naming convention. |
| Does the name matter? | No. Any file works. `.gitkeep` says why it is there. |
| Why can I not see the files? | Names starting with `.` are hidden by default. |
| Should I delete them? | Once the folder has real files, optionally. No rush. |
| Do I add them to new empty folders? | Yes, if other people need the folder to exist. |

---

Back to [the guides index](README.md).
