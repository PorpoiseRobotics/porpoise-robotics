# 4. When things go wrong

## Read this part first

**Almost nothing in git is truly unrecoverable.**

Git was built by people who were paranoid about losing work. Once you have made a commit, that
snapshot is kept, and git will fight hard to hang onto it. Even when you appear to have deleted
something, it is usually still there — git keeps a private log of everywhere you have been for
at least 30 days, and there is a command below that shows it to you.

The realistic ways to actually lose work are short:

1. Changes you never committed at all, thrown away with `git reset --hard` or `git checkout .`
2. Files you never committed, deleted from your computer
3. Deleting the project folder itself, including the hidden `.git` directory

Notice what those have in common: **uncommitted** work. So the rule that protects you is
simply *commit often*. Every commit is a save point you can return to.

Everything else on this page is a routine problem with a known fix. None of them are
emergencies, and none of them mean you did something stupid. Experienced engineers hit every
single one of these regularly.

### Two habits that prevent most of this

**Run `git status` whenever you are confused.** It tells you which branch you are on, what has
changed, and very often the exact command you need next. It is safe — it only looks, it never
changes anything.

**When genuinely panicked, make a backup copy of the whole project folder before doing
anything.** Copy and paste it to your desktop. It costs ten seconds and makes every subsequent
step risk-free. There is no shame in this; do it every time if it helps.

---

## "I committed to main by accident"

Extremely common, completely harmless, and fixable in three commands. Your work is not lost —
it is just sitting on the wrong branch.

**This assumes you have not pushed yet.** If you have already pushed, skip to
[the pushed version](#i-committed-to-main-and-already-pushed) below.

```bash
# 1. Make a branch here. Your commits come with it.
git switch -c fix/my-actual-branch-name

# 2. Go back to main.
git switch main

# 3. Move main back to match GitHub, dropping the commits from main only.
git reset --hard origin/main
```

```
HEAD is now at e4f5g6h Update ground vehicle README
```

Your commits are safe on `fix/my-actual-branch-name`. Switch to it and carry on:

```bash
git switch fix/my-actual-branch-name
git log --oneline -3   # your commits are right there
```

**Why this works:** step 1 creates a branch pointing at your commits, so they now have a home.
Step 3 rewinds `main` to where GitHub thinks it should be. The commits still exist — they just
belong to the branch now instead of to `main`.

**What if you only edited files and did not commit yet?** Even easier — uncommitted changes
follow you between branches automatically:

```bash
git switch -c fix/my-actual-branch-name   # your edits come along
```

### "I committed to main and already pushed"

Do not use `reset` — rewriting shared history causes problems for everyone else. Use `revert`,
which undoes the change by adding a new commit that reverses it:

```bash
git switch main
git pull
git revert <the-commit-id>   # get the id from: git log --oneline
git push
```

Git opens an editor with a pre-written message; save and close it. (If a strange full-screen
editor appears, see [the editor question](#git-opened-a-weird-editor-and-i-cannot-get-out) below.)

Then tell the team, so nobody is confused by the back-and-forth. Nobody will be annoyed.

---

## "I have a merge conflict"

**Nothing is broken and you have not damaged anything.** A conflict means two people edited the
same lines of the same file, and git is refusing to guess which version is right. It is asking
you, a human, to decide.

You will see something like:

```
Auto-merging ground-vehicle/README.md
CONFLICT (content): Merge conflict in ground-vehicle/README.md
Automatic merge failed; fix conflicts and then commit the result.
```

### If you want to back out entirely

You can always cancel and return to exactly how things were before:

```bash
git merge --abort
```

Nothing is lost. Do this if you want a teammate present before continuing. It is a legitimate
choice, not a retreat.

### Fixing it

**Step 1: find out which files are conflicted.**

```bash
git status
```

```
You have unmerged paths.
  (fix conflicts and run "git commit")
  (use "git merge --abort" to abort the merge)

Unmerged paths:
  (use "git add <file>..." to mark resolution)
        both modified:   ground-vehicle/README.md
```

**Step 2: open the file.** Git has written markers into it showing both versions:

```
<<<<<<< HEAD
The rover uses a differential drive system.
=======
The rover uses a four-wheel independent drive system.
>>>>>>> feature/drive-update
```

Read it like this:

- `<<<<<<< HEAD` — everything below this line is **your** version
- `=======` — the divider
- `>>>>>>> feature/drive-update` — everything above this, below the divider, is **their**
  version, and this line names where it came from

**Step 3: edit the file so it says what it should say.** Keep yours, keep theirs, or write
something new that combines both. Then **delete all three marker lines** — the `<<<<<<<`, the
`=======`, and the `>>>>>>>`. They are notes to you, not part of the file.

The result should read as a normal file that you would be happy to have in the project:

```
The rover uses a four-wheel independent drive system.
```

If you are unsure which version is correct, **ask the person who wrote the other one.** That is
the right move, not a failure. Conflicts frequently mean two people were solving the same
problem without knowing it, which is worth a conversation regardless.

**Step 4: tell git you have resolved it, and finish.**

```bash
git add ground-vehicle/README.md
git commit
```

Git pre-writes the commit message; save and close the editor. Done.

Repeat for each conflicted file if there are several. `git status` tracks which remain.

**GitHub Desktop** shows conflicts with a clear panel and often gives you "Use my version" /
"Use their version" buttons for simple cases. This is one of the places the app really earns its
keep.

### Avoiding conflicts

- `git pull` before starting work, every single time
- Keep branches short-lived — merge within a day or two, not a month
- Tell the team which files you are working in, especially for shared docs

---

## "I want to undo my last commit"

Pick the row matching your situation.

| Situation | Command | What happens |
|---|---|---|
| Wrong message, right changes | `git commit --amend -m "Better message"` | Rewrites the message |
| Undo commit, **keep** the changes | `git reset --soft HEAD~1` | Files unchanged, changes staged again |
| Undo commit, keep files, unstage | `git reset HEAD~1` | Files unchanged, changes unstaged |
| Undo commit and **discard** the work | `git reset --hard HEAD~1` | **Deletes those changes.** Careful. |
| Already pushed it | `git revert HEAD` | Adds a new commit that reverses it |

`HEAD~1` means "one commit before the current one".

**Most of the time you want `git reset --soft HEAD~1`.** It removes the commit but leaves every
file exactly as it is, so you can fix something and commit again.

```bash
git reset --soft HEAD~1
git status
```

```
On branch feature/camera-setup
Changes to be committed:
        modified:   homeland-security-camera/README.md
```

The commit is gone; your work is untouched and ready to re-commit.

**Only `--hard` actually destroys anything.** Read the row twice before typing it. And even then,
[reflog](#i-think-i-lost-work-the-command-that-saves-you) can usually get the commit back.

**If you already pushed, use `git revert`, not `reset`.** Reset rewrites history that other
people already have, which creates a mess for them. Revert is honest: it leaves the original
commit in place and adds a new one undoing it.

---

## "I pushed something I shouldn't have"

### If it was a password, API key, token, or credential

**Treat it as leaked, immediately.** This repository is public. Assume a stranger has already
copied it — automated bots scan public GitHub for credentials within seconds of a push. This is
not hypothetical.

**Do this in order:**

1. **Change the password or revoke the key. Right now, before touching git.** This is the only
   step that actually protects anything. Removing it from git afterwards is cleanup, not a fix.
2. **Tell whoever runs the affected system.** Immediately, even if it is embarrassing. A
   credential quietly leaked for a week is far worse than an awkward two-minute conversation.
3. Then clean up the repository — see below.

**Deleting the file in a new commit does not remove it.** Git keeps every past version. Anyone
can read the old commit. This is the single most important thing to understand about a public
repository.

Actually scrubbing it from history requires rewriting history with a tool like
[git-filter-repo](https://github.com/newren/git-filter-repo) or
[BFG Repo-Cleaner](https://rtyley.github.io/bfg-repo-cleaner/), coordinated with everyone who
has a copy. **Do not attempt this alone if you are new.** Get help. And remember it changes
nothing about step 1 — the credential is still compromised and still has to be rotated.

Prevention: keep real values in a `.env` file, which our `.gitignore` already blocks, and commit
a `.env.example` with the keys but no values. See
[CONTRIBUTING.md](../CONTRIBUTING.md#rule-4-never-commit-secrets).

### If it was just a mistake — wrong file, broken code, something embarrassing

Much simpler. Add a commit that undoes it:

```bash
git revert <the-commit-id>   # find the id with: git log --oneline
git push
```

Nobody will care. This is a normal Tuesday.

### If it was a large file that is now making the repo slow

Ask for help rather than improvising. Removing large files needs the same history rewriting as
above. Going forward, keep video, model weights, and datasets out of git — our `.gitignore`
already excludes the common ones.

---

## "Git says my branch is behind"

```
Your branch is behind 'origin/main' by 3 commits, and can be fast-forwarded.
  (use "git pull" to update your local branch)
```

This is **not an error**. It is git saying "other people have done work you do not have yet."
Git even tells you the fix in the message:

```bash
git pull
```

Related messages you will see:

**"Your branch is ahead of 'origin/main' by 2 commits"** — you have commits you have not pushed
yet. Run `git push`.

**"Your branch and 'origin/main' have diverged"** — you both made different commits. `git pull`
will merge them, possibly producing a conflict, which is handled above.

**"Updates were rejected because the remote contains work that you do not have locally"** — you
tried to push while behind. Pull first, then push:

```bash
git pull
git push
```

**Never use `git push --force` to get past this.** It overwrites other people's work on GitHub.
It has a legitimate use in narrow circumstances, and this is not one of them. If someone tells
you to force push, ask why first.

---

## "I'm scared I'll break something"

Good instinct, but you can relax. Some things that are genuinely true:

**You cannot break other people's work by experimenting locally.** Everything you do stays on
your computer until you `push`. Make a branch and try anything you like.

**You cannot break `main` on GitHub by accident.** Changes reach `main` only through a pull
request that someone reviews and merges. There is a human step in the way.

**Reading commands cannot change anything.** `git status`, `git log`, `git diff`, `git branch`,
and `git show` only look. Run them as often as you want.

**Committed work is hard to destroy.** Even deleted branches and "lost" commits stay recoverable
for weeks via reflog, below.

**Every single person on this team will make these mistakes.** So has every engineer who has
ever used git. The commands are unforgiving in wording and forgiving in effect.

### If you want a safety net before trying something

```bash
git branch backup-before-i-try-this
```

That takes a snapshot of exactly where you are. If the experiment goes badly:

```bash
git switch backup-before-i-try-this
```

You are back. Delete the backup branch when you no longer need it, or leave it — branches cost
nothing.

Or just copy the entire project folder somewhere else first. Unsophisticated, completely
effective.

---

## "I think I lost work" — the command that saves you

`git reflog` is git's private diary of every position you have been in, including commits from
branches you deleted and commits you reset away. It keeps roughly 30 days of history.

```bash
git reflog
```

```
7f3a9c2 (HEAD -> main) HEAD@{0}: reset: moving to origin/main
9d8e7f6 HEAD@{1}: commit: Add camera mounting notes
a1b2c3d HEAD@{2}: commit: Update rover wiring diagram
e4f5g6h HEAD@{3}: checkout: moving from main to feature/camera
```

Find the commit you thought you lost — it is usually right there — and get back to it:

```bash
git switch -c recovered-work 9d8e7f6
```

You are now on a new branch containing that "lost" commit, with everything intact.

**If your work was never committed, reflog cannot help.** That is the whole argument for
committing frequently.

---

## Other things that will happen to you

### Git opened a weird editor and I cannot get out

You are almost certainly in Vim, which some git installs use by default. To leave:

1. Press `Esc`
2. Type `:wq`
3. Press `Enter`

That means "save and quit". To quit *without* saving, use `:q!` instead.

Prevent it happening again:

```bash
git config --global core.editor "nano"     # macOS / Linux
git config --global core.editor "notepad"  # Windows
```

### I need to switch branches but have unfinished work

Put it on a shelf:

```bash
git stash            # changes set aside, folder goes clean
git switch main      # go do the urgent thing
git switch -         # come back ("-" means the previous branch)
git stash pop        # take your work back off the shelf
```

### I deleted a file by accident

If you had not committed the deletion:

```bash
git restore path/to/file.md
```

If you had already committed it, find the commit before the deletion and restore from it:

```bash
git log --oneline -- path/to/file.md    # find when it existed
git restore --source=<commit-id> path/to/file.md
```

### "You are in 'detached HEAD' state"

Alarming wording, harmless situation. You are looking at a specific old commit rather than
sitting on a branch. Nothing is broken. Get back with:

```bash
git switch main
```

If you made commits while detached and want to keep them, name them first:

```bash
git switch -c my-recovered-work
```

### `git status` shows hundreds of files I did not touch

Usually build output that should be ignored — a `build/` folder, `__pycache__/`, `.pio/`. Check
whether the pattern is in `.gitignore`; if not, add it and commit that change. Do not commit the
build output itself.

Occasionally it is a line-ending difference between Windows and Mac/Linux. If every file in the
project looks modified, mention it to a teammate rather than committing all of it.

### I cannot push — "failed to push some refs"

Almost always means you are behind:

```bash
git pull
git push
```

---

## When to just ask someone

Ask for help immediately, without trying anything first, when:

- A **credential or password was committed** (after rotating it — that comes first)
- Someone suggests `git push --force`, `git rebase`, or `git filter-repo` and you do not know
  what those do
- You are about to type any command containing `--hard` or `--force` and are unsure
- `git status` says something you have never seen and this page does not cover it

Asking takes two minutes. Guessing with `--force` can cost the team a day. There is no
expectation that you memorize any of this — this page exists precisely so you do not have to.
