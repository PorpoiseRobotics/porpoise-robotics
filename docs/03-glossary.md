# 3. Plain-English glossary

Every term explained without using other git jargon. Skim it once, then come back whenever a
word stops making sense.

---

### Repository (repo)

A folder that git is tracking, plus the complete history of every change ever made to it. This
project is a repository. "Repo" is just the short form; people say it constantly.

The history lives in a hidden folder named `.git` inside it. Never edit or delete that folder —
it *is* the history.

---

### Clone

Downloading a full copy of a repository onto your computer. Not just the current files — the
entire history, every past version, all of it.

You clone once per project per computer. After that you use *pull* to stay current.

```bash
git clone https://github.com/ORG-NAME/porpoise-robotics.git
```

---

### Commit

A saved snapshot of your project at one moment, with a message describing what changed and who
changed it. Think of it as a save point in a video game: you can always return to any of them.

Used as both a noun and a verb. "I made a commit" and "I committed that" mean the same thing.

Committing is a **local** action. Your commits are on your computer only until you *push*.

```bash
git commit -m "Add wiring notes for the rover"
```

---

### Staging / `git add`

Choosing which of your changes will go into your next commit. You have edited five files but
only want to save three together? Add those three and commit; the other two wait.

The in-between state is called "the staging area" or "the index". `git add .` stages
everything.

---

### Branch

A separate line of work. Making a branch gives you your own copy of the project to change
freely, without affecting what anyone else sees.

The analogy that works: everyone is writing in a shared notebook. A branch is you photocopying
the current page, scribbling all over your photocopy, and only later asking to have your version
replace the original page. Until then, everyone else's notebook is untouched.

`main` is the shared branch — the official current state of the project. Everyone's work
eventually ends up there.

```bash
git switch -c fix/steering-drift   # create a branch and move to it
git switch main                    # move back to main
git branch                         # list branches; * marks where you are
```

---

### Push

Uploading your commits from your computer to GitHub. Until you push, your work exists in exactly
one place, and a dead laptop takes it with it.

```bash
git push
```

---

### Pull

The opposite: downloading commits other people have pushed, and applying them to your copy.

Pull before you start work, every time. Starting from an out-of-date copy is the number one
cause of merge conflicts.

```bash
git pull
```

---

### Fetch

Like pull, but it only *looks* — it downloads information about what changed on GitHub without
touching your files. Useful when you want to know what is new before committing to taking it.

`git pull` is really `git fetch` followed by `git merge`, done in one step.

---

### Remote

A copy of the repository that lives somewhere other than your computer. For us, that is GitHub.

Your local repo knows the address of the remote so it knows where to push and pull.

```bash
git remote -v   # show the remotes this project knows about
```

---

### Origin

The default nickname for the main remote — the GitHub copy. It is a shorthand, not a keyword:
`git push origin main` means "push my main branch to the copy called origin."

Nearly every project calls its main remote `origin`, purely by convention.

---

### `main`

The primary branch, holding the official current state of the project. Older projects call it
`master`; the modern default is `main`. Same thing, different name.

**Never commit directly to `main`.** Work on a branch and open a pull request.

---

### HEAD

Git's word for "where you are right now" — which commit you are currently looking at, usually
the newest commit on your current branch.

You will mostly see it inside other commands, like `git reset HEAD~1`, where `HEAD~1` means "one
commit before where I am now." Do not overthink it; you rarely need to reason about HEAD directly.

---

### Pull request (PR)

A request on GitHub to merge your branch into `main`. It shows exactly what you changed and
gives teammates a place to comment before it becomes official.

The name is confusing — it has almost nothing to do with `git pull`. Read it as "please pull my
work into the main project." Everyone just says "PR".

PRs are not a formality: they are how the team catches mistakes early and how everyone learns
what the others are doing.

---

### Merge

Combining two branches into one — usually your branch into `main`. When a PR is approved and you
click the green button, git merges it.

Most merges happen automatically with no input from you.

---

### Merge conflict

What happens when two people changed the *same lines of the same file* differently, and git
cannot decide which version is right. So it stops and asks you.

**A conflict is not an error and nothing is broken.** It is git refusing to guess. You open the
file, choose the correct version, and save. Step-by-step instructions:
[When things go wrong](04-when-things-go-wrong.md#i-have-a-merge-conflict).

Conflicts feel alarming the first time and routine by the fifth.

---

### Working tree / working directory

The actual files in your folder as they exist right now — the ones your editor sees. Distinct
from the committed history.

"Working tree clean" means you have no unsaved changes. It is git's way of saying "all tidy."

---

### Untracked file

A file in your folder that git has never been told about. New files are untracked until you
`git add` them, and git will not include them in commits until then.

`git status` lists them under "Untracked files". Check that section before committing, or you
will push half your work.

---

### `.gitignore`

A file listing patterns of files git should permanently pretend do not exist — build output,
editor settings, and above all **secrets**. Anything matching does not appear in `git status`
and cannot be committed by accident.

Ours is at the top of the repository.

---

### `.gitkeep`

An empty placeholder file that exists only to stop git from dropping an empty folder. Not a real
git feature — just a convention. Full explanation:
[Why empty folders have a .gitkeep file](05-gitkeep-and-empty-folders.md).

---

### Diff

The list of exact lines that changed between two versions. Additions are marked `+` and shown in
green, deletions `-` in red.

```bash
git diff
```

---

### Log / history

The record of every commit ever made, newest first.

```bash
git log --oneline -10   # last 10, one line each
```

---

### Checkout / switch

Moving to a different branch or a different point in history. `git checkout` is the old command
that does many unrelated things; `git switch` is the newer, clearer one for changing branches.

Use `git switch`. You will still see `git checkout` in older tutorials online.

---

### Stash

A temporary shelf for uncommitted changes. Use it when you need to jump to another branch right
now but are not ready to commit what you are working on.

```bash
git stash        # put changes on the shelf; folder returns to clean
git stash pop    # take them back off
```

---

### Fork

Your own personal copy of someone else's repository on GitHub, used when you want to contribute
to a project you do not have write access to.

**You probably will not need this.** Team members work on branches within the same repository.
Forks come up when contributing to outside open-source projects.

---

### Upstream / downstream

"Upstream" is where your work is heading (GitHub, `main`); "downstream" is toward your local
copy. Mostly you will meet the word in messages like "set upstream branch", which just means
"remember which remote branch this one pairs with."

That is what `-u` does in `git push -u origin my-branch`.

---

### Rebase

Another way of combining branches that rewrites history to look tidier.

**Ignore this for now.** It is genuinely useful and genuinely easy to get wrong. Merge does the
same job more safely. Revisit in six months.
