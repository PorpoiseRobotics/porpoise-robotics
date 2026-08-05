# Contributing

This is a short guide on purpose. There are four rules. If you follow them, you are contributing
correctly.

If you have never used git before, read [`docs/01-install-git.md`](docs/01-install-git.md) and
[`docs/02-everyday-workflow.md`](docs/02-everyday-workflow.md) first — this page assumes you can
already make a commit.

---

## Rule 1: Never commit directly to `main`

`main` is the branch everyone shares. It should always work. Make your changes on a **branch**
instead, then open a pull request.

Name your branch with a category, a slash, and a few words joined by dashes:

```
feature/camera-motion-detection
fix/rover-steering-drift
docs/update-submersible-readme
```

Use `feature/` for something new, `fix/` for something broken, `docs/` for documentation.
That is the whole naming system. Do not overthink it.

Create one like this:

```bash
git switch -c docs/update-submersible-readme
```

Already committed to `main` by accident? It is fixable and nothing is lost —
see [When things go wrong](docs/04-when-things-go-wrong.md#i-committed-to-main-by-accident).

---

## Rule 2: Write commit messages that finish this sentence

Every commit message should complete the sentence *"If applied, this commit will..."*

```
Add motion detection to camera pipeline
Fix steering drift when the rover turns left
Update submersible README with the ballast design
```

Keep the first line under about 70 characters, start with a verb, and skip the period at the
end. If you need to explain *why*, leave a blank line and write a paragraph underneath.

Avoid messages like `update`, `fix`, `stuff`, or `asdf`. Six months from now someone will read
your message while trying to find a bug. Write it for that person.

---

## Rule 3: Open a pull request and let someone look at it

When your branch is pushed, open a pull request (PR) on GitHub. A PR is a request to merge your
branch into `main`, and it gives teammates a place to read the change before it lands.

The full click-by-click walkthrough is in
[the everyday workflow guide](docs/02-everyday-workflow.md#step-7-open-a-pull-request).

A few habits that make PRs pleasant:

- **Keep them small.** One idea per PR. A 40-line PR gets reviewed today; a 4,000-line PR sits
  for a week.
- **Fill in the template.** GitHub will pre-fill a short form. It asks what you changed, why, and
  how you tested it. Answering it is the fastest way to get approved.
- **Do not merge your own PR without a review** unless the change is trivially safe, like fixing
  a typo.
- **Comments on your PR are about the code, not about you.** Everyone's code gets comments,
  including people who have done this for twenty years.

---

## Rule 4: Never commit secrets

**This repository is public. Anyone on the internet can read it, including its entire history.**

Never put any of these in a file you commit:

- Passwords, API keys, tokens, or camera login credentials
- IP addresses, network diagrams, Wi-Fi names, router configuration, or VPN details
- School floor plans, campus maps, or camera placement locations
- Any information about students, staff, or recorded footage

Deleting a secret in a later commit **does not remove it** — git keeps the old version forever,
and anyone can read it. If a secret does get committed, treat it as leaked: change the password
or revoke the key immediately, then tell whoever runs the system. Do not try to quietly rewrite
history and hope nobody noticed.

### The safe pattern for configuration

Real values go in a file named `.env`, which is listed in `.gitignore` and therefore never gets
committed:

```
# .env  — stays on your computer only, never committed
CAMERA_HOST=192.168.1.42
CAMERA_PASSWORD=the-real-password
```

Then commit a file named `.env.example` next to it, with the same keys and fake values, so the
next person knows what settings they need:

```
# .env.example — safe to commit, shows the shape without the secrets
CAMERA_HOST=
CAMERA_PASSWORD=
```

Anyone setting up the project copies `.env.example` to `.env` and fills in the real values
locally. The real values never leave their machine.

---

## Quick reference

```bash
git switch main                     # go to the shared branch
git pull                            # get everyone else's latest work
git switch -c feature/my-thing      # make your own branch
# ... edit files ...
git add .                           # stage your changes
git commit -m "Add my thing"        # save them with a message
git push -u origin feature/my-thing # send the branch to GitHub
# ... then open a pull request in the browser ...
```

Stuck? [When things go wrong](docs/04-when-things-go-wrong.md) covers the common problems, and
almost none of them are permanent.
