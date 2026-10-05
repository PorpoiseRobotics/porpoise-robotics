"""
docxlite.py - just enough of a Word (.docx) writer for the course handouts.

A .docx is a zip of XML files. This writes them directly, so the handouts
need nothing beyond the Python standard library: no python-docx, no Node.
It covers what a printed workbook needs and no more - headings, paragraphs,
bold and colored runs, tables with shading and fixed row heights, writing
lines, boxes to draw in, page breaks, one inline picture, and a page-number
footer. Letter paper, US layout.

Everything it makes opens in Word, Google Docs and LibreOffice, and stays
fully editable there: the files are the deliverable, this is just how the
first version was produced.

    doc = Document()
    doc.heading("Lesson 1", level=1)
    doc.para([run("Name: ", bold=True), run("_____")])
    doc.lines(3)
    doc.save("out.docx")
"""

import os
import struct
import zipfile
from xml.sax.saxutils import escape

TWIP = 1440                     # twentieths of a point per inch
EMU = 914400                    # EMUs per inch

NAVY = "1F3A5F"
TEAL = "0E7C86"
AMBER = "B46A0F"
INK = "222222"
GREY = "666666"
RULE = "C8D2DE"
PANEL = "EEF3F8"
LIGHT_TEAL = "D6ECEE"

FONT = "Calibri"


def run(text, *, bold=False, italic=False, size=None, color=None,
        font=None, highlight=None):
    return {"text": text, "bold": bold, "italic": italic, "size": size,
            "color": color, "font": font, "highlight": highlight}


def _runs_xml(runs, base_size=None, base_color=None, base_bold=False):
    if isinstance(runs, str):
        runs = [run(runs)]
    out = []
    for r in runs:
        if isinstance(r, str):
            r = run(r)
        props = []
        font = r["font"]
        if font:
            props.append('<w:rFonts w:ascii="{0}" w:hAnsi="{0}" w:cs="{0}"/>'
                         .format(font))
        if r["bold"] or base_bold:
            props.append("<w:b/>")
        if r["italic"]:
            props.append("<w:i/>")
        color = r["color"] or base_color
        if color:
            props.append('<w:color w:val="{}"/>'.format(color))
        size = r["size"] or base_size
        if size:
            props.append('<w:sz w:val="{0}"/><w:szCs w:val="{0}"/>'
                         .format(int(size * 2)))
        if r["highlight"]:
            props.append('<w:highlight w:val="{}"/>'.format(r["highlight"]))
        rpr = "<w:rPr>{}</w:rPr>".format("".join(props)) if props else ""
        out.append('<w:r>{}<w:t xml:space="preserve">{}</w:t></w:r>'
                   .format(rpr, escape(r["text"])))
    return "".join(out)


def _para_xml(runs, *, style=None, align=None, size=None, color=None,
              bold=False, before=0, after=6, keep_next=False,
              page_break_before=False, line=None, indent=None):
    ppr = []
    if style:
        ppr.append('<w:pStyle w:val="{}"/>'.format(style))
    if keep_next:
        ppr.append("<w:keepNext/>")
    if page_break_before:
        ppr.append("<w:pageBreakBefore/>")
    spacing = '<w:spacing w:before="{}" w:after="{}"'.format(
        int(before * 20), int(after * 20))
    if line:
        spacing += ' w:line="{}" w:lineRule="auto"'.format(int(line * 240))
    ppr.append(spacing + "/>")
    if indent:
        ppr.append('<w:ind w:left="{}"/>'.format(int(indent * TWIP)))
    if align:
        ppr.append('<w:jc w:val="{}"/>'.format(align))
    return "<w:p><w:pPr>{}</w:pPr>{}</w:p>".format(
        "".join(ppr), _runs_xml(runs, size, color, bold))


class Span:
    """A table cell stretched across `columns` grid columns."""

    def __init__(self, columns, content):
        self.columns = columns
        self.content = content


def _png_size(path):
    with open(path, "rb") as handle:
        head = handle.read(24)
    width, height = struct.unpack(">II", head[16:24])
    return width, height


class Document:
    def __init__(self, *, margins=0.7, base_size=13, footer=None):
        self.body = []
        self.images = []
        self.margins = margins
        self.base_size = base_size
        self.footer = footer
        self._drawing_id = 1

    # -------------------------------------------------------------
    # text
    # -------------------------------------------------------------

    def para(self, runs, **kwargs):
        self.body.append(_para_xml(runs, **kwargs))

    def heading(self, text, level=1, *, page_break=False, color=None):
        sizes = {1: 24, 2: 17, 3: 14}
        colors = {1: NAVY, 2: TEAL, 3: NAVY}
        self.body.append(_para_xml(
            [run(text)], style="Heading%d" % level, size=sizes[level],
            color=color or colors[level], bold=True,
            before=0 if page_break else (6 if level > 1 else 0), after=4,
            keep_next=True, page_break_before=page_break))

    def bullets(self, items, *, size=None):
        for item in items:
            runs = item if isinstance(item, list) else [run(item)]
            self.body.append(_para_xml(
                [run("•  ", color=TEAL, bold=True)] + runs,
                size=size, after=3, indent=0.25))

    def page_break(self):
        self.body.append('<w:p><w:r><w:br w:type="page"/></w:r></w:p>')

    def spacer(self, points=6):
        self.body.append(_para_xml([run("")], after=0, before=0,
                                   size=max(points * 0.5, 1)))

    # -------------------------------------------------------------
    # tables
    # -------------------------------------------------------------

    def table(self, rows, widths, *, header=False, heights=None,
              size=None, borders="all", fills=None, align=None,
              valign="top", bold_first_col=False, keep_together=True,
              header_rows=None):
        """
        rows    - list of rows; each cell is a string, a list of runs, a
                  list of lists of runs (several paragraphs), or a Span
                  to stretch one cell across several columns
        widths  - inches per column
        header  - first row shaded and bold, repeated on a new page
        header_rows - how many rows count as the header, if more than one
        heights - one row height in inches for every row, or a list; a
                  height is a MINIMUM, so a full cell still grows
        borders - "all", "lines" (horizontal rules only), or "none"
        fills   - {(row, cell): hex color} for individual cells
        """
        size = size or self.base_size
        fills = fills or {}
        if header_rows is None:
            header_rows = 1 if header else 0
        total = sum(widths)
        grid = "".join('<w:gridCol w:w="{}"/>'.format(int(w * TWIP))
                       for w in widths)

        line = '<w:{0} w:val="single" w:sz="8" w:space="0" w:color="{1}"/>'
        nil = '<w:{0} w:val="nil"/>'
        if borders == "all":
            edges = "".join(line.format(e, NAVY) for e in
                            ("top", "left", "bottom", "right",
                             "insideH", "insideV"))
        elif borders == "lines":
            edges = (nil.format("top") + nil.format("left") +
                     line.format("bottom", GREY) + nil.format("right") +
                     line.format("insideH", GREY) + nil.format("insideV"))
        else:
            edges = "".join(nil.format(e) for e in
                            ("top", "left", "bottom", "right",
                             "insideH", "insideV"))

        # The schema fixes the order of these: width, borders, layout, margins.
        # LibreOffice forgives a wrong order; Word calls the file corrupt.
        out = ['<w:tbl><w:tblPr><w:tblW w:w="{}" w:type="dxa"/>'
               '<w:tblBorders>{}</w:tblBorders><w:tblLayout w:type="fixed"/>'
               '<w:tblCellMar><w:top w:w="40" w:type="dxa"/>'
               '<w:left w:w="90" w:type="dxa"/>'
               '<w:bottom w:w="40" w:type="dxa"/>'
               '<w:right w:w="90" w:type="dxa"/></w:tblCellMar>'
               '</w:tblPr><w:tblGrid>{}</w:tblGrid>'
               .format(int(total * TWIP), edges, grid)]

        for r_index, row in enumerate(rows):
            is_header = r_index < header_rows
            trpr = []
            if keep_together:
                trpr.append("<w:cantSplit/>")
            if is_header:
                trpr.append("<w:tblHeader/>")
            height = heights[r_index] if isinstance(heights, list) else heights
            if height and not is_header:
                trpr.append('<w:trHeight w:val="{}" w:hRule="atLeast"/>'
                            .format(int(height * TWIP)))
            out.append("<w:tr><w:trPr>{}</w:trPr>".format("".join(trpr)))

            column = 0          # grid column, which a Span moves on by more than 1
            for c_index, cell in enumerate(row):
                span = 1
                if isinstance(cell, Span):
                    span, cell = cell.columns, cell.content
                fill = fills.get((r_index, c_index))
                if is_header and not fill:
                    fill = LIGHT_TEAL
                tcpr = '<w:tcW w:w="{}" w:type="dxa"/>'.format(
                    int(sum(widths[column:column + span]) * TWIP))
                if span > 1:
                    tcpr += '<w:gridSpan w:val="{}"/>'.format(span)
                if fill:
                    tcpr += ('<w:shd w:val="clear" w:color="auto" '
                             'w:fill="{}"/>'.format(fill))
                tcpr += '<w:vAlign w:val="{}"/>'.format(
                    "center" if is_header else valign)

                paragraphs = cell
                if isinstance(cell, str):
                    paragraphs = [[run(cell)]]
                elif cell and isinstance(cell[0], dict):
                    paragraphs = [cell]
                elif not cell:
                    paragraphs = [[run("")]]

                bold = is_header or (bold_first_col and column == 0)
                cell_align = align[column] if isinstance(align, list) else align
                if span > 1 and is_header:
                    cell_align = "center"
                column += span
                body = "".join(
                    _para_xml(p, size=size, bold=bold,
                              color=NAVY if is_header else None,
                              after=2, align=cell_align)
                    for p in paragraphs)
                out.append("<w:tc><w:tcPr>{}</w:tcPr>{}</w:tc>".format(
                    tcpr, body))
            out.append("</w:tr>")
        out.append("</w:tbl>")
        self.body.append("".join(out))
        # Word needs a paragraph between two tables, or it merges them.
        self.spacer(4)

    def lines(self, count, *, width=None, height=0.42, label=None):
        """Ruled lines to write on."""
        width = width or self.text_width()
        if label:
            self.para([run(label, bold=True)], after=0, keep_next=True)
        self.table([[""] for _ in range(count)], [width], borders="lines",
                   heights=height)

    def box(self, label, height, *, width=None, note=None):
        """A bordered box to draw or write in, label at the top."""
        width = width or self.text_width()
        content = [[run(label, bold=True, color=NAVY)]]
        if note:
            content.append([run(note, italic=True, color=GREY,
                                size=self.base_size - 2)])
        self.table([[content]], [width], heights=height)

    def text_width(self):
        return 8.5 - 2 * self.margins

    # -------------------------------------------------------------
    # pictures
    # -------------------------------------------------------------

    def picture(self, path, width, *, align="center"):
        if not path.lower().endswith(".png"):
            raise ValueError("docxlite only embeds PNG files: %s" % path)
        rid = "rIdImg%d" % (len(self.images) + 1)
        name = "image%d.png" % (len(self.images) + 1)
        self.images.append((rid, name, path))
        px_w, px_h = _png_size(path)
        cx = int(width * EMU)
        cy = int(cx * px_h / px_w)
        did = self._drawing_id
        self._drawing_id += 1
        drawing = (
            '<w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" '
            'distR="0"><wp:extent cx="{cx}" cy="{cy}"/>'
            '<wp:docPr id="{did}" name="Picture {did}"/>'
            '<wp:cNvGraphicFramePr><a:graphicFrameLocks noChangeAspect="1"/>'
            '</wp:cNvGraphicFramePr><a:graphic><a:graphicData '
            'uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            '<pic:pic><pic:nvPicPr><pic:cNvPr id="{did}" name="{name}"/>'
            '<pic:cNvPicPr/></pic:nvPicPr><pic:blipFill>'
            '<a:blip r:embed="{rid}"/><a:stretch><a:fillRect/></a:stretch>'
            '</pic:blipFill><pic:spPr><a:xfrm><a:off x="0" y="0"/>'
            '<a:ext cx="{cx}" cy="{cy}"/></a:xfrm><a:prstGeom prst="rect">'
            '<a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData>'
            '</a:graphic></wp:inline></w:drawing></w:r>'
        ).format(cx=cx, cy=cy, did=did, name=name, rid=rid)
        self.body.append('<w:p><w:pPr><w:spacing w:before="0" w:after="120"/>'
                         '<w:jc w:val="{}"/></w:pPr>{}</w:p>'.format(
                             align, drawing))

    # -------------------------------------------------------------
    # output
    # -------------------------------------------------------------

    def save(self, path):
        margin = int(self.margins * TWIP)
        footer_ref = ('<w:footerReference w:type="default" r:id="rIdFooter"/>'
                      if self.footer else "")
        sect = ('<w:sectPr>{}<w:pgSz w:w="12240" w:h="15840"/>'
                '<w:pgMar w:top="{m}" w:right="{m}" w:bottom="{m}" w:left="{m}" '
                'w:header="432" w:footer="432" w:gutter="0"/></w:sectPr>'
                .format(footer_ref, m=margin))
        document = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:document '
            'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
            'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
            'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
            'xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            '<w:body>{}{}</w:body></w:document>'.format("".join(self.body), sect))

        styles = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
            '<w:docDefaults><w:rPrDefault><w:rPr>'
            '<w:rFonts w:ascii="{f}" w:hAnsi="{f}" w:eastAsia="{f}" w:cs="{f}"/>'
            '<w:color w:val="{ink}"/><w:sz w:val="{sz}"/><w:szCs w:val="{sz}"/>'
            '<w:lang w:val="en-US"/></w:rPr></w:rPrDefault>'
            '<w:pPrDefault><w:pPr><w:spacing w:after="120" w:line="252" '
            'w:lineRule="auto"/></w:pPr></w:pPrDefault></w:docDefaults>'
            '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
            '<w:name w:val="Normal"/><w:qFormat/></w:style>'
            '<w:style w:type="paragraph" w:styleId="Heading1">'
            '<w:name w:val="heading 1"/><w:basedOn w:val="Normal"/>'
            '<w:next w:val="Normal"/><w:qFormat/><w:pPr><w:keepNext/>'
            '<w:outlineLvl w:val="0"/></w:pPr><w:rPr><w:b/>'
            '<w:color w:val="{navy}"/><w:sz w:val="48"/></w:rPr></w:style>'
            '<w:style w:type="paragraph" w:styleId="Heading2">'
            '<w:name w:val="heading 2"/><w:basedOn w:val="Normal"/>'
            '<w:next w:val="Normal"/><w:qFormat/><w:pPr><w:keepNext/>'
            '<w:outlineLvl w:val="1"/></w:pPr><w:rPr><w:b/>'
            '<w:color w:val="{teal}"/><w:sz w:val="34"/></w:rPr></w:style>'
            '<w:style w:type="paragraph" w:styleId="Heading3">'
            '<w:name w:val="heading 3"/><w:basedOn w:val="Normal"/>'
            '<w:next w:val="Normal"/><w:qFormat/><w:pPr><w:keepNext/>'
            '<w:outlineLvl w:val="2"/></w:pPr><w:rPr><w:b/>'
            '<w:color w:val="{navy}"/><w:sz w:val="28"/></w:rPr></w:style>'
            '<w:style w:type="paragraph" w:styleId="Footer">'
            '<w:name w:val="footer"/><w:basedOn w:val="Normal"/></w:style>'
            '</w:styles>').format(f=FONT, ink=INK, sz=self.base_size * 2,
                                  navy=NAVY, teal=TEAL)

        footer = None
        if self.footer:
            footer = (
                '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                '<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
                '<w:p><w:pPr><w:pStyle w:val="Footer"/><w:jc w:val="center"/>'
                '</w:pPr>{}<w:r><w:rPr><w:color w:val="{g}"/><w:sz w:val="18"/>'
                '</w:rPr><w:fldChar w:fldCharType="begin"/></w:r><w:r><w:rPr>'
                '<w:color w:val="{g}"/><w:sz w:val="18"/></w:rPr><w:instrText '
                'xml:space="preserve"> PAGE </w:instrText></w:r><w:r><w:rPr>'
                '<w:color w:val="{g}"/><w:sz w:val="18"/></w:rPr><w:fldChar '
                'w:fldCharType="separate"/></w:r><w:r><w:rPr><w:color w:val="{g}"/>'
                '<w:sz w:val="18"/></w:rPr><w:t>1</w:t></w:r><w:r><w:rPr>'
                '<w:color w:val="{g}"/><w:sz w:val="18"/></w:rPr><w:fldChar '
                'w:fldCharType="end"/></w:r></w:p></w:ftr>').format(
                    _runs_xml([run(self.footer + "   |   page ", color=GREY,
                                   size=9)]), g=GREY)

        rels = ['<Relationship Id="rIdStyles" '
                'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" '
                'Target="styles.xml"/>']
        if footer:
            rels.append('<Relationship Id="rIdFooter" '
                        'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" '
                        'Target="footer1.xml"/>')
        for rid, name, _src in self.images:
            rels.append('<Relationship Id="{}" '
                        'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" '
                        'Target="media/{}"/>'.format(rid, name))
        doc_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                    '{}</Relationships>'.format("".join(rels)))

        content_types = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Default Extension="xml" ContentType="application/xml"/>'
            '<Default Extension="png" ContentType="image/png"/>'
            '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
            '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>'
            '{}'
            '<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>'
            '</Types>').format(
                '<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>'
                if footer else "")

        package_rels = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
            '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>'
            '</Relationships>')

        core = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" '
            'xmlns:dc="http://purl.org/dc/elements/1.1/">'
            '<dc:title>{}</dc:title><dc:creator>Porpoise Robotics</dc:creator>'
            '</cp:coreProperties>').format(escape(self.footer or "Handout"))

        os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
        with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as package:
            package.writestr("[Content_Types].xml", content_types)
            package.writestr("_rels/.rels", package_rels)
            package.writestr("docProps/core.xml", core)
            package.writestr("word/document.xml", document)
            package.writestr("word/styles.xml", styles)
            package.writestr("word/_rels/document.xml.rels", doc_rels)
            if footer:
                package.writestr("word/footer1.xml", footer)
            for _rid, name, src in self.images:
                package.write(src, "word/media/" + name)
        return path
