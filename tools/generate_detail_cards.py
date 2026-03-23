"""
Generate field reference cards from field-cards.json.
Front: physical setup + field adjustments. Back: saved settings + rationale + hints.
Card size: ~iPhone 11. Print front & back, cut, laminate.

Single source of truth: field-cards.json
"""

from fpdf import FPDF
import json
import os

CARD_W = 151
CARD_H = 95

A4_W = 210
A4_H = 297

COLOR_MAP = {
    "blue": (21, 101, 192),
    "green": (46, 125, 50),
    "red": (198, 40, 40),
    "purple": (106, 27, 154),
    "orange": (230, 81, 0),
}

WHITE = (255, 255, 255)
GRAY_BG = (245, 245, 245)
GRAY_TEXT = (100, 100, 100)

MUST_BG = (255, 243, 240)
MUST_BORDER = (210, 130, 120)
MUST_LABEL = (180, 60, 40)
ADJUST_BG = (240, 248, 255)
ADJUST_BORDER = (150, 185, 220)
ADJUST_LABEL = (50, 100, 160)
HINT_BG = (255, 252, 240)
HINT_BORDER = (220, 200, 140)
HINT_LABEL = (160, 140, 60)
LEARN_BG = (245, 248, 255)
LEARN_BORDER = (170, 190, 220)


class DetailCardPDF(FPDF):
    def __init__(self):
        super().__init__(orientation='P', unit='mm', format='A4')
        self.set_auto_page_break(auto=False)
        self.add_font('Segoe', '', r'C:\Windows\Fonts\segoeui.ttf', uni=True)
        self.add_font('Segoe', 'B', r'C:\Windows\Fonts\segoeuib.ttf', uni=True)
        self.add_font('Segoe', 'I', r'C:\Windows\Fonts\segoeuii.ttf', uni=True)

    def _page_setup(self):
        self.add_page()
        self.set_fill_color(*GRAY_BG)
        self.rect(0, 0, A4_W, A4_H, style='F')
        cx = (A4_W - CARD_W) / 2
        cy = (A4_H - CARD_H) / 2
        self.set_fill_color(*WHITE)
        self.rect(cx, cy, CARD_W, CARD_H, style='F')
        return cx, cy

    def _card_border(self, cx, cy):
        self.set_draw_color(60, 60, 60)
        self.set_line_width(0.6)
        self.rect(cx, cy, CARD_W, CARD_H, style='D')
        self.set_font('Segoe', 'I', 7)
        self.set_text_color(180, 180, 180)
        self.set_xy(0, A4_H - 12)
        self.cell(A4_W, 5,
                  'Cut along the border \u2014 laminate front & back together',
                  align='C')

    def _header(self, x, y, camera, code, color, name, desc, side_label=None):
        bar_h = 13
        self.set_fill_color(*color)
        self.rect(x, y, CARD_W, bar_h, style='F')
        self.set_font('Segoe', 'B', 7)
        self.set_text_color(*WHITE)
        self.set_xy(x + 3, y + 0.8)
        self.cell(30, 3.5, camera, new_x='LMARGIN')
        if side_label:
            self.set_font('Segoe', 'B', 6)
            self.set_xy(x + CARD_W - 22, y + 0.8)
            self.cell(19, 3.5, side_label, align='R', new_x='LMARGIN')
        self.set_font('Segoe', 'B', 14)
        self.set_xy(x, y + 0)
        self.cell(CARD_W, 5.5, code, align='C', new_x='LMARGIN')
        self.set_font('Segoe', 'B', 7)
        self.set_xy(x, y + 5.5)
        self.cell(CARD_W, 3.5, name, align='C', new_x='LMARGIN')
        self.set_font('Segoe', '', 5.5)
        self.set_text_color(220, 225, 240)
        self.set_xy(x + 3, y + 9.5)
        self.cell(CARD_W - 6, 3, desc, align='C', new_x='LMARGIN')
        return y + bar_h

    def _mini_header(self, x, y, camera, code, color, name, side_label):
        bar_h = 8
        self.set_fill_color(*color)
        self.rect(x, y, CARD_W, bar_h, style='F')
        self.set_font('Segoe', 'B', 7)
        self.set_text_color(*WHITE)
        self.set_xy(x + 3, y + 1)
        self.cell(25, 3, camera, new_x='LMARGIN')
        self.set_font('Segoe', 'B', 11)
        self.set_xy(x, y + 0.5)
        self.cell(CARD_W, 4, code, align='C', new_x='LMARGIN')
        self.set_font('Segoe', '', 6)
        self.set_xy(x, y + 4.5)
        self.cell(CARD_W, 3, name, align='C', new_x='LMARGIN')
        self.set_font('Segoe', 'B', 6)
        self.set_xy(x + CARD_W - 22, y + 1)
        self.cell(19, 3, side_label, align='R', new_x='LMARGIN')
        return y + bar_h

    def _draw_box_items(self, x, y, w, h, bg, border, label_color,
                        label, items, item_style='check'):
        self.set_fill_color(*bg)
        self.set_draw_color(*border)
        self.set_line_width(0.3)
        self.rect(x, y, w, h, style='FD')
        self.set_font('Segoe', 'B', 6)
        self.set_text_color(*label_color)
        self.set_xy(x + 2.5, y + 0.5)
        self.cell(w - 5, 3.5, label, new_x='LMARGIN')

        iy = y + 4.5
        for item in items:
            if isinstance(item, dict) and 'item' in item:
                # Structured: {item, value, note}
                lbl = item.get('item', '')
                val = item.get('value', '')
                note = item.get('note', '')
                marker = '[ ] ' if item_style == 'check' else '> '
                self.set_font('Segoe', '', 5.5)
                self.set_text_color(120, 120, 120)
                self.set_xy(x + 3, iy)
                self.cell(3, 3.5, marker, new_x='LMARGIN')
                self.set_font('Segoe', '', 5.5)
                self.set_text_color(100, 90, 80)
                self.set_xy(x + 6, iy)
                self.cell(22, 3.5, lbl, new_x='LMARGIN')
                self.set_font('Segoe', 'B', 6)
                self.set_text_color(30, 30, 30)
                self.set_xy(x + 28, iy)
                self.cell(35, 3.5, val, new_x='LMARGIN')
                if note:
                    self.set_font('Segoe', '', 5)
                    self.set_text_color(120, 110, 100)
                    self.set_xy(x + 63, iy)
                    self.cell(w - 67, 3.5, note, new_x='LMARGIN')
                iy += 3.8
            elif isinstance(item, dict) and 'text' in item:
                # Simple text line (tip or warning)
                text = item['text']
                is_warning = item.get('type') == 'warning'
                self.set_font('Segoe', 'B' if is_warning else '', 5.5)
                self.set_text_color(180, 40, 30) if is_warning else \
                    self.set_text_color(80, 70, 50)
                marker = '> ' if item_style == 'arrow' else '\u2022 '
                self.set_xy(x + 3, iy)
                self.cell(3, 3.2, marker, new_x='LMARGIN')
                self.set_xy(x + 6, iy)
                self.cell(w - 10, 3.2, text, new_x='LMARGIN')
                iy += 3.4
            else:
                # Fallback plain string
                text = str(item)
                self.set_font('Segoe', '', 5.5)
                self.set_text_color(80, 70, 50)
                marker = '> ' if item_style == 'arrow' else '\u2022 '
                self.set_xy(x + 3, iy)
                self.cell(3, 3.2, marker, new_x='LMARGIN')
                self.set_xy(x + 6, iy)
                self.cell(w - 10, 3.2, text, new_x='LMARGIN')
                iy += 3.4
        return iy

    def add_front(self, camera, code, color, name, desc, must_set, can_adjust):
        cx, cy = self._page_setup()
        x, y = cx, cy
        cur_y = self._header(x, y, camera, code, color, name, desc, 'FRONT')
        cur_y += 1

        must_h = 4.5 + len(must_set) * 3.8 + 1
        remaining = (y + CARD_H) - cur_y - must_h - 3
        adjust_h = remaining

        self._draw_box_items(
            x + 3, cur_y, CARD_W - 6, must_h,
            MUST_BG, MUST_BORDER, MUST_LABEL,
            '!! PHYSICAL SETUP \u2014 CHECK BEFORE SHOOTING',
            must_set, item_style='check')

        cur_y += must_h + 1.5

        self._draw_box_items(
            x + 3, cur_y, CARD_W - 6, adjust_h,
            ADJUST_BG, ADJUST_BORDER, ADJUST_LABEL,
            'ADJUST IN THE FIELD',
            can_adjust, item_style='arrow')

        self._card_border(cx, cy)

    def add_back(self, camera, code, color, name,
                 software_settings, hints, why_text):
        cx, cy = self._page_setup()
        x, y = cx, cy
        cur_y = self._mini_header(x, y, camera, code, color, name, 'BACK')
        cur_y += 0.5

        # Settings table
        param_x = x + 3
        val_x = x + 27
        rat_x = x + 60

        self.set_font('Segoe', 'B', 5.5)
        self.set_text_color(140, 140, 150)
        self.set_xy(param_x, cur_y)
        self.cell(24, 3, 'SAVED SETTING', new_x='LMARGIN')
        self.set_xy(val_x, cur_y)
        self.cell(33, 3, 'VALUE', new_x='LMARGIN')
        self.set_xy(rat_x, cur_y)
        self.cell(40, 3, 'RATIONALE', new_x='LMARGIN')
        cur_y += 3.2
        self.set_draw_color(200, 200, 200)
        self.set_line_width(0.15)
        self.line(param_x, cur_y, x + CARD_W - 3, cur_y)
        cur_y += 0.3

        row_h = 3.6
        avail = 42
        if len(software_settings) * row_h > avail:
            row_h = avail / len(software_settings)
            row_h = max(row_h, 3.0)

        for i, s in enumerate(software_settings):
            param = s['param']
            value = s['value']
            rationale = s['rationale']
            sy = cur_y + i * row_h
            if i % 2 == 0:
                self.set_fill_color(248, 249, 253)
                self.rect(param_x, sy, CARD_W - 6, row_h, style='F')
            self.set_font('Segoe', '', 5)
            self.set_text_color(*GRAY_TEXT)
            self.set_xy(param_x + 0.5, sy + 0.2)
            self.cell(23, row_h - 0.4, param, new_x='LMARGIN')
            self.set_font('Segoe', 'B', 5)
            self.set_text_color(30, 30, 30)
            self.set_xy(val_x, sy + 0.2)
            self.cell(32, row_h - 0.4, value, new_x='LMARGIN')
            self.set_font('Segoe', '', 4.5)
            self.set_text_color(100, 100, 100)
            self.set_xy(rat_x, sy + 0.2)
            self.cell(CARD_W - 63, row_h - 0.4, rationale, new_x='LMARGIN')

        cur_y += len(software_settings) * row_h + 1

        # Hints
        hint_h = 4 + len(hints) * 3.2 + 0.5
        self.set_fill_color(*HINT_BG)
        self.set_draw_color(*HINT_BORDER)
        self.set_line_width(0.25)
        self.rect(x + 3, cur_y, CARD_W - 6, hint_h, style='FD')
        self.set_font('Segoe', 'B', 5.5)
        self.set_text_color(*HINT_LABEL)
        self.set_xy(x + 5, cur_y + 0.5)
        self.cell(20, 3, 'FIELD HINTS', new_x='LMARGIN')
        hy = cur_y + 3.8
        for hint in hints:
            self.set_font('Segoe', '', 5)
            self.set_text_color(80, 70, 30)
            self.set_xy(x + 5, hy)
            self.cell(3, 3, '\u2022', new_x='LMARGIN')
            self.set_xy(x + 8, hy)
            self.cell(CARD_W - 14, 3, hint, new_x='LMARGIN')
            hy += 3.2
        cur_y += hint_h + 1

        # Why section
        why_h = (y + CARD_H) - cur_y - 1.5
        if why_h > 4:
            self.set_fill_color(*LEARN_BG)
            self.set_draw_color(*LEARN_BORDER)
            self.set_line_width(0.2)
            self.rect(x + 3, cur_y, CARD_W - 6, why_h, style='FD')
            self.set_font('Segoe', 'B', 5.5)
            self.set_text_color(60, 90, 140)
            self.set_xy(x + 5, cur_y + 0.5)
            self.cell(40, 3, 'WHY THIS MODE WORKS', new_x='LMARGIN')
            self.set_font('Segoe', '', 5)
            self.set_text_color(60, 80, 110)
            self.set_xy(x + 5, cur_y + 4)
            self.multi_cell(CARD_W - 14, 2.8, why_text, new_x='LMARGIN')

        self._card_border(cx, cy)


def build_pdf():
    # Load data from JSON
    project_root = os.path.dirname(os.path.dirname(__file__))
    json_path = os.path.join(project_root, 'data', 'field-cards.json')
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    pdf = DetailCardPDF()

    for camera_data in data['cameras']:
        camera_name = camera_data['camera_short']
        for mode in camera_data['modes']:
            code = mode['code']
            color = COLOR_MAP.get(mode['color'], (100, 100, 100))
            name = mode['name']
            desc = mode['description']
            front = mode['front']
            back = mode['back']

            # Front
            pdf.add_front(
                camera_name, code, color, name, desc,
                front['physical_setup'],
                front['field_adjustments'])

            # Back
            pdf.add_back(
                camera_name, code, color, name,
                back['software_settings'],
                back['hints'],
                back['why'])

    out_path = os.path.join(project_root, 'output', 'Mode-Detail-Cards.pdf')
    pdf.output(out_path)
    print(f"PDF saved to: {out_path}")


if __name__ == '__main__':
    build_pdf()
