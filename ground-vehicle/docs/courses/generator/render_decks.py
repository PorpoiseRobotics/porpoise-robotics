"""
render_decks.py - turn the decks into PDFs, and optionally into contact sheets.

Two uses:

  1. PRINT-READY HANDOUTS. A PDF prints the same everywhere, whether or not
     the machine has Calibri and Consolas installed.

  2. CHECKING THE LAYOUT. lint_decks.py estimates whether text fits. This
     actually renders it, so you can look.

    python render_decks.py                 every deck -> pdf/
    python render_decks.py --sheets        also one contact sheet per deck
    python render_decks.py advanced        just that track

Needs LibreOffice. On Windows it is normally at
C:\\Program Files\\LibreOffice\\program\\soffice.com - set SOFFICE if yours is
somewhere else. Contact sheets additionally need PyMuPDF and Pillow:

    pip install pymupdf pillow
"""

import glob
import math
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
COURSES = os.path.normpath(os.path.join(HERE, ".."))
OUT = os.path.join(COURSES, "pdf")

TRACKS = ("beginner-ps3", "beginner-switch", "advanced")

CANDIDATES = [
    os.environ.get("SOFFICE"),
    r"C:\Program Files\LibreOffice\program\soffice.com",
    r"C:\Program Files (x86)\LibreOffice\program\soffice.com",
    "/Applications/LibreOffice.app/Contents/MacOS/soffice",
    shutil.which("soffice"),
    shutil.which("libreoffice"),
]


def find_soffice():
    for path in CANDIDATES:
        if path and os.path.exists(path):
            return path
    sys.exit("LibreOffice not found. Install it, or set the SOFFICE "
             "environment variable to the full path of soffice.")


def render(soffice, tracks):
    made = []
    for track in tracks:
        src = os.path.join(COURSES, track)
        dst = os.path.join(OUT, track)
        os.makedirs(dst, exist_ok=True)

        decks = sorted(glob.glob(os.path.join(src, "*.pptx")))
        if not decks:
            continue

        print("rendering %s (%d decks)..." % (track, len(decks)))
        subprocess.run([soffice, "--headless", "--norestore",
                        "--convert-to", "pdf", "--outdir", dst] + decks,
                       check=True, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        made += sorted(glob.glob(os.path.join(dst, "*.pdf")))
    return made


def contact_sheet(pdf, cols=6, dpi=42):
    """One PNG showing every slide, for scanning a deck at a glance."""
    import pymupdf
    from PIL import Image, ImageDraw

    doc = pymupdf.open(pdf)
    tiles = []
    for n in range(doc.page_count):
        pix = doc[n].get_pixmap(dpi=dpi)
        tiles.append((n + 1, Image.frombytes("RGB", (pix.width, pix.height),
                                             pix.samples)))
    if not tiles:
        return None

    w, h = tiles[0][1].size
    rows = math.ceil(len(tiles) / cols)
    sheet = Image.new("RGB", (cols * w, rows * (h + 16)), "white")
    draw = ImageDraw.Draw(sheet)
    for i, (n, im) in enumerate(tiles):
        x, y = (i % cols) * w, (i // cols) * (h + 16)
        draw.text((x + 3, y + 3), str(n), fill="black")
        sheet.paste(im, (x, y + 16))
        draw.rectangle([x, y, x + w - 1, y + h + 15], outline="#aaaaaa")

    out = os.path.splitext(pdf)[0] + "_sheet.png"
    sheet.save(out)
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    sheets = "--sheets" in sys.argv

    tracks = args or TRACKS
    for track in tracks:
        if track not in TRACKS:
            sys.exit("Unknown track %r. Choose from: %s"
                     % (track, ", ".join(TRACKS)))

    pdfs = render(find_soffice(), tracks)

    print()
    for pdf in pdfs:
        size = os.path.getsize(pdf) // 1024
        line = "  %-58s %5d KB" % (os.path.relpath(pdf, COURSES), size)
        if sheets:
            made = contact_sheet(pdf)
            if made:
                line += "   + " + os.path.basename(made)
        print(line)

    print("\n%d PDFs in %s" % (len(pdfs), os.path.relpath(OUT, COURSES)))
    print("That folder is gitignored - it is a build output, not a deliverable.")


if __name__ == "__main__":
    main()
