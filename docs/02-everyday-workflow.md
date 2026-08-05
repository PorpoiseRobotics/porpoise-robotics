# 2. The everyday workflow

**This is the guide to keep open in a tab.** Everything below is one loop that you will repeat
for the rest of your time on this project. It does not get more complicated than this.

Before starting, finish [Installing git and setting it up](01-install-git.md).

---

## The loop, in one picture

```
  1. pull      get everyone else's latest work
  2. branch    make your own workspace
  3. edit      do the actual work
  4. add       choose which changes to save
  5. commit    save them, with a message
  6. push      send your branch to GitHub
  7. PR        ask for your work to be added to main
  8. merge     it becomes part of the project
              ↺ back to 1
```

That is it. Eight steps, most of which are one command. Everything else in git is a variation
on this or a way to fix a mistake.

**Why the branch step?** So your half-finished work never breaks anyone else's. You work in your
own sandbox, and it only joins the real project once someone has looked at it. See
[the glossary](03-glossary.md#branch) if that is still fuzzy.

---

## Step 1: Pull the latest work

Always start here. Someone else has probably changed something since you last looked, and
starting from stale code is the single most common cause of merge conflicts.

**Terminal:**

```bash
git switch main
git pull
```

Expected output when there is something new:

```
remote: Enumerating objects: 8, done.
Unpacking objects: 100% (5/5), 1.21 KiB | 620.00 KiB/s, done.
From https://github.com/ORG-NAME/porpoise-robotics
   a1b2c3d..e4f5g6h  main       -> origin/main
Updating a1b2c3d..e4f5g6h
Fast-forward
 ground-vehicle/README.md | 12 ++++++++----
 1 file changed, 8 insertions(+), 4 deletions(-)
```

Or, when you are already current:

```
Already up to date.
```

Both are good. `Already up to date.` is not an error.

**GitHub Desktop:** make sure the "Current branch" dropdown says `main`, then click
**Fetch origin** at the top. If it changes to "Pull origin", click it again.

---

## Step 2: Make a branch

Never work directly on `main`. Make a branch named after what you are about to do:

**Terminal:**

```bash
git switch -c docs/add-rover-wiring-notes
```

```
Switched to a new branch 'docs/add-rover-wiring-notes'
```

`-c` means "create". Without it, `git switch` moves to a branch that already exists.

Branch names use a category, a slash, and dashes: `feature/...` for something new, `fix/...`
for something broken, `docs/...` for documentation. See
[CONTRIBUTING.md](../CONTRIBUTING.md#rule-1-never-commit-directly-to-main).

**GitHub Desktop:** click **Current branch > New branch**, type the name, click
**Create branch**.

Check where you are any time:

```bash
git status
```

```
On branch docs/add-rover-wiring-notes
nothing to commit, working tree clean
```

The first line always tells you which branch you are on. Get in the habit of glancing at it.

---

## Step 3: Do the work

Open the files in whatever editor you like — VS Code, Notepad++, nano, anything. Edit them, save
them, as normal. Git is not involved yet and does not care what tool you use.

When you want to see what you have changed:

```bash
git status
```

```
On branch docs/add-rover-wiring-notes
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git restore <file>..." to discard changes in working directory)
        modified:   ground-vehicle/docs/wiring.md

Untracked files:
  (use "git add <file>..." to include in what will be committed)
        ground-vehicle/docs/power-budget.md

no changes added to commit (use "git add" and/or "git commit -a")
```

Two categories to notice:

- **Changes not staged for commit** — files git already knows about that you have edited.
- **Untracked files** — brand new files git has never seen. Git will ignore them until you
  explicitly add them, so watch this section or you will commit half your work.

To see the exact lines you changed:

```bash
git diff
```

Lines starting with `+` are additions, lines with `-` are deletions. Press `q` to exit if it
takes over your terminal.

**GitHub Desktop** shows all of this automatically in the Changes tab, with additions in green
and deletions in red. This is the single best reason to use the app.

---

## Step 4: Stage your changes with `add`

"Staging" means choosing which changes go into your next save. It is a separate step because
sometimes you have edited five files but only want to save three of them together.

```bash
git add ground-vehicle/docs/wiring.md          # one specific file
git add ground-vehicle/                        # everything in one folder
git add .                                      # everything you have changed
```

`git add .` is what you will use most of the time. Nothing visible happens — that is normal.
Confirm it worked:

```bash
git status
```

```
On branch docs/add-rover-wiring-notes
Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
        modified:   ground-vehicle/docs/wiring.md
        new file:   ground-vehicle/docs/power-budget.md
```

"Changes to be committed" (usually shown in green) means staged and ready.

Before you run `git add .`, glance at `git status` and make sure nothing surprising is listed.
This is where secrets accidentally get committed. If you see a `.env` file or anything with a
password in it, stop and read
[CONTRIBUTING.md](../CONTRIBUTING.md#rule-4-never-commit-secrets).

**GitHub Desktop:** the checkboxes next to each file *are* staging. Checked means it will be
included.

---

## Step 5: Commit — save your work with a message

A commit is a permanent, labeled snapshot of the project at this moment. It lives on your
computer until you push it.

```bash
git commit -m "Add wiring notes and power budget for the rover"
```

```
[docs/add-rover-wiring-notes 7f3a9c2] Add wiring notes and power budget for the rover
 2 files changed, 47 insertions(+)
 create mode 100644 ground-vehicle/docs/power-budget.md
```

`7f3a9c2` is the commit's ID. Every commit gets a unique one, and it is how you refer to this
exact snapshot forever.

Write messages that complete *"If applied, this commit will..."* — see
[CONTRIBUTING.md](../CONTRIBUTING.md#rule-2-write-commit-messages-that-finish-this-sentence).

**Commit often.** Small, frequent commits are much better than one giant one. Every commit is a
point you can safely return to, so more commits means more safety nets. There is no cost.

**GitHub Desktop:** type a summary in the box at the bottom left, click **Commit to
&lt;branch-name&gt;**.

---

## Step 6: Push — send your branch to GitHub

Your commits are still only on your computer. Push copies them to GitHub so the team can see
them.

The very first push of a new branch needs extra words, because GitHub does not have this branch
yet and git wants you to say where it goes:

```bash
git push -u origin docs/add-rover-wiring-notes
```

```
Enumerating objects: 9, done.
Counting objects: 100% (9/9), done.
Compressing objects: 100% (5/5), done.
Writing objects: 100% (6/6), 1.42 KiB | 1.42 MiB/s, done.
remote: 
remote: Create a pull request for 'docs/add-rover-wiring-notes' on GitHub by visiting:
remote:      https://github.com/ORG-NAME/porpoise-robotics/pull/new/docs/add-rover-wiring-notes
remote: 
To https://github.com/ORG-NAME/porpoise-robotics.git
 * [new branch]      docs/add-rover-wiring-notes -> docs/add-rover-wiring-notes
branch 'docs/add-rover-wiring-notes' set up to track 'origin/docs/add-rover-wiring-notes'.
```

Notice that git prints the exact link to open your pull request. That is the fastest way to get
to Step 7.

After that first time, on this branch, just:

```bash
git push
```

`origin` means "the copy on GitHub" and `-u` means "remember this pairing so I do not have to
type it again." Full definitions in [the glossary](03-glossary.md).

**GitHub Desktop:** click **Publish branch** (first time) or **Push origin** (after that).

---

## Step 7: Open a pull request

A pull request (PR) is you saying: *"here is my work, please review it and add it to `main`."*
It is a conversation attached to your changes.

1. Go to the repository on GitHub in your browser. A yellow banner appears at the top:
   **"docs/add-rover-wiring-notes had recent pushes"** with a green
   **Compare & pull request** button. Click it. (If the banner is gone, click the
   **Pull requests** tab, then **New pull request**, and pick your branch.)
2. Check the two dropdowns at the top: it should say `base: main` on the left and your branch
   on the right. That reads as "merge my branch into main," which is what you want.
3. A form appears, pre-filled with our template asking what you changed, why, and how you tested
   it. Fill it in. It takes two minutes and makes review dramatically faster.
4. Click **Create pull request**.
5. Scroll down and use the **Reviewers** box on the right to request a specific teammate, or
   just tell them in chat.

Now wait. Someone will read it and either approve it or leave comments.

**If they leave comments:** you do not open a new PR. Fix the code on the same branch, then
`git add`, `git commit`, and `git push` again — the PR updates itself automatically.

Comments on your PR are normal and are about the code, not about you. Every engineer's work gets
comments.

---

## Step 8: Merge

Once approved, click the green **Merge pull request** button, then **Confirm merge**. Your work
is now part of `main` and everyone else will get it on their next `git pull`.

GitHub then offers **Delete branch**. Click it. The branch has served its purpose and the commits
are safely in `main` — deleting it is tidying up, not throwing anything away.

Then clean up on your own machine and start the loop again:

```bash
git switch main
git pull                                  # brings in your own merged work
git branch -d docs/add-rover-wiring-notes # delete the local copy of the finished branch
```

```
Deleted branch docs/add-rover-wiring-notes (was 7f3a9c2).
```

If git refuses with "not fully merged", it thinks the branch has work that never made it into
`main` — worth checking before you force it. Usually it just means you forgot to pull first.

---

## The whole loop, condensed

Once this is muscle memory, a full cycle is:

```bash
git switch main && git pull              # 1
git switch -c fix/steering-drift         # 2
# ... edit files ...                     # 3
git add .                                # 4
git commit -m "Fix steering drift on left turns"  # 5
git push -u origin fix/steering-drift    # 6
# ... open the PR in the browser ...     # 7, 8
```

Print it, tape it to the wall, whatever helps.

---

## Four commands worth knowing beyond the loop

```bash
git status              # where am I, what have I changed?     (use constantly)
git diff                # what exactly did I change?
git log --oneline -10   # the last 10 commits, one line each
git switch main         # go back to the shared branch
```

`git status` is the answer to "what is going on?" roughly ninety percent of the time. When
confused, run it first. It usually tells you the exact command you need next.

---

**Next: [Plain-English glossary](03-glossary.md)** for any word that is still fuzzy, and
**[When things go wrong](04-when-things-go-wrong.md)** for the moment something breaks.
