# 1. Installing git and setting it up

**After this guide you will have:** git installed on your computer, git knowing your name and
email, a GitHub account, and a copy of this project on your machine that you can actually edit.

**Time:** about 20 minutes, once per computer. You never do this again on that machine.

---

## First: what are we even installing?

**Git** is a program that runs on your computer. It keeps a complete history of a folder of
files — every version, every change, who made it, and when. It runs entirely offline.

**GitHub** is a website that stores copies of git projects online so a team can share them.
Git and GitHub are separate things with confusingly similar names. Git is the tool; GitHub is
the place the team's copy lives.

You need both: git on your computer, and a GitHub account.

---

## Choose your path: terminal or app?

There are two ways to use git, and **both are legitimate**. Nobody is a better engineer for
choosing one over the other.

| | **GitHub Desktop** (an app with buttons) | **The terminal** (typed commands) |
|---|---|---|
| Learning curve | Gentle. You can see everything. | Steeper. You type exact words. |
| Speed once fluent | Good | Faster |
| Shows you your changes | Visually, side by side, automatically | You ask for it with commands |
| Available help online | Some | Enormous |
| Works on a robot / Raspberry Pi over SSH | No | Yes |

**Our recommendation:** install both. Use GitHub Desktop for everyday work, and keep the
terminal available for when you are logged into a vehicle's onboard computer, where no graphical
app exists. Many experienced developers work exactly this way.

This guide covers both. Every later guide shows both.

---

## Step 1: Install git

### Windows

1. Go to [git-scm.com/download/win](https://git-scm.com/download/win). The download starts
   automatically.
2. Run the installer. It asks a lot of questions. **Click Next on all of them.** The defaults
   are correct.
   - One exception worth noticing: on the screen asking about the default editor, if you do not
     already know what Vim is, change the dropdown to **"Use Notepad as Git's default editor"**.
     This will save you a genuinely frustrating moment later.
3. When it finishes, open the Start menu and search for **Git Bash**. That is your terminal for
   this project. Use Git Bash rather than Command Prompt or PowerShell — the commands in these
   guides are written for it.

### macOS

Open **Terminal** (press Cmd+Space, type "terminal", press Enter) and type:

```bash
git --version
```

If git is already installed you will see something like `git version 2.39.5`. Done.

If instead a window appears offering to install "command line developer tools", click
**Install** and wait a few minutes. That installs git.

If you use [Homebrew](https://brew.sh/), `brew install git` gets you a newer version.

### Linux

```bash
# Debian / Ubuntu / Raspberry Pi OS
sudo apt update && sudo apt install git

# Fedora
sudo dnf install git

# Arch
sudo pacman -S git
```

### Confirm it worked (all systems)

In your terminal (Git Bash on Windows), type:

```bash
git --version
```

You should see a version number:

```
git version 2.43.0
```

The exact number does not matter as long as it is 2.30 or higher. If instead you see
`command not found`, git did not install — close the terminal, open a new one, and try again
before reinstalling. A newly installed program is often not visible to an already-open terminal.

---

## Step 2: Tell git who you are

Git stamps your name and email onto every change you save so the team knows who did what. It
will refuse to work until you tell it. Do this once.

```bash
git config --global user.name "Your Actual Name"
git config --global user.email "you@example.com"
```

Use the same email you will use for your GitHub account, so GitHub can connect your commits to
your profile.

**Note that this email becomes public.** It is embedded in every commit, and this repository is
public. If you would rather not publish a personal address, GitHub can give you a private
no-reply one: sign in, go to **Settings > Emails**, check "Keep my email addresses private",
and use the `ID+username@users.noreply.github.com` address it shows you.

Check that it took:

```bash
git config --global --list
```

```
user.name=Your Actual Name
user.email=you@example.com
init.defaultbranch=main
```

While you are here, set two things that make life easier:

```bash
# New projects start on a branch called "main" (the modern default)
git config --global init.defaultBranch main

# When a text editor pops up unexpectedly, use a simple one
git config --global core.editor "nano"     # macOS / Linux
git config --global core.editor "notepad"  # Windows
```

---

## Step 3: Create a GitHub account

1. Go to [github.com/signup](https://github.com/signup).
2. Use the same email you configured above.
3. Pick a username you will not be embarrassed by — it appears next to your work publicly.
4. Verify your email address when GitHub sends the confirmation.
5. **Turn on two-factor authentication.** GitHub requires it for most accounts now, and you
   will be locked out later if you skip it. Go to **Settings > Password and authentication >
   Two-factor authentication** and follow the prompts. An authenticator app on your phone (Google
   Authenticator, Authy, 1Password) is the easiest option. **Save the recovery codes it gives you
   somewhere other than your phone** — they are how you get back in if you lose the device.

6. Ask whoever runs the Porpoise Robotics GitHub organization to invite you. You will get an
   email invitation; accept it. Until then you can read the project but not contribute to it.

---

## Step 4: Get authenticated, and get the project onto your computer

Your computer has to prove it is you before GitHub will accept your work. Pick the path
matching your choice above.

### Path A: GitHub Desktop (recommended if you are new)

1. Download from [desktop.github.com](https://desktop.github.com/) — Windows and macOS only.
   (On Linux, use Path B, or the community build at
   [github.com/shiftkey/desktop](https://github.com/shiftkey/desktop).)
2. Install and open it.
3. Click **Sign in to GitHub.com** and log in through the browser window that opens. That
   single sign-in handles authentication permanently. There is nothing else to configure.
4. Choose **Clone a repository from the Internet**, find `porpoise-robotics` in the list, pick
   a folder on your computer to keep it in, and click **Clone**.

"Clone" means "download a full copy of the project, with all of its history, onto my computer."
You now have the project. Skip to the end of this guide.

### Path B: Terminal

You cannot use your GitHub password from the terminal — GitHub stopped accepting it in 2021.
You need one of these instead.

**Option 1: GitHub CLI (easiest).** Install [cli.github.com](https://cli.github.com/), then:

```bash
gh auth login
```

Answer the prompts: choose `GitHub.com`, then `HTTPS`, then `Login with a web browser`. It shows
you an eight-character code, you press Enter, your browser opens, you paste the code. Done — it
configures git for you.

**Option 2: Personal access token.** In GitHub go to **Settings > Developer settings > Personal
access tokens > Tokens (classic) > Generate new token**. Check the `repo` box, generate it, and
**copy the token immediately — GitHub never shows it again.** When git asks for a password, paste
the token instead. Store it in a password manager, not in a file in a project folder.

**Option 3: SSH key.** More setup up front, then nothing to type ever again. GitHub's guide:
[Generating a new SSH key](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent).

Now clone the project. On the GitHub page for the repository, click the green **Code** button and
copy the URL, then:

```bash
cd ~/Documents                                  # or wherever you keep your work
git clone https://github.com/ORG-NAME/porpoise-robotics.git
cd porpoise-robotics
```

You should see:

```
Cloning into 'porpoise-robotics'...
remote: Enumerating objects: 24, done.
remote: Counting objects: 100% (24/24), done.
remote: Compressing objects: 100% (18/18), done.
Receiving objects: 100% (24/24), 12.847 KiB | 4.28 MiB/s, done.
Resolving deltas: 100% (2/2), done.
```

Confirm you are in the right place:

```bash
git status
```

```
On branch main
Your branch is up to date with 'origin/main'.

nothing to commit, working tree clean
```

That message — "nothing to commit, working tree clean" — means everything is in order. You will
see it a lot. It is a good message.

---

## You are set up

You now have git installed, git knowing your name, a GitHub account, and a copy of the project
on your computer.

**Next: [The everyday workflow](02-everyday-workflow.md)** — the actual loop of doing work.

---

## If something did not work

**`git: command not found`** — Close every terminal window and open a fresh one. Terminals only
learn about newly installed programs when they start. If it persists on Windows, make sure you
are in **Git Bash**, not Command Prompt.

**`Permission denied (publickey)`** — Your SSH key is not set up or not registered with GitHub.
Easiest fix: use `gh auth login` (Option 1) instead, which sidesteps SSH entirely.

**`Authentication failed` when pushing** — You typed your GitHub password. Use a personal access
token instead (Option 2), or run `gh auth login`.

**`fatal: not a git repository`** — You are in the wrong folder. Run `pwd` to see where you are
and `cd` into the `porpoise-robotics` folder.

**`Support for password authentication was removed`** — Same as above. Token, not password.

Anything else: [When things go wrong](04-when-things-go-wrong.md), or ask a teammate. Getting
stuck at setup is normal and says nothing about you.
