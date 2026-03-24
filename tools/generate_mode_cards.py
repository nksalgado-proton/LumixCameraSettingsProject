"""
Generate simple mode-name cards (scenario cards) for G9 MkI and G9 MkII.
One card per camera (MkII gets 2 cards: photo + special/video).
iPhone 11 sized (~151mm x 76mm), one per page, centered on A4.
Separate PDFs per camera.
"""

from fpdf import FPDF
import os

CARD_W = 151
CARD_H = 76

A4_W = 210
A4_H = 297

BLUE = (21, 101, 192)
GREEN = (46, 125, 50)
RED = (198, 40, 40)
PURPLE = (106, 27, 154)
ORANGE = (230, 81, 0)
WHITE = (255, 255, 255)
GRAY_BG = (245, 245, 245)
GRAY_TEXT = (100, 100, 100)
GRAY_LINE = (210, 210, 210)


class ModeCardPDF(FPDF):
    def __init__(self):
        super().__init__(orientation='P', unit='mm', format='A4')
        self.set_auto_page_break(auto=False)
        self.add_font('Segoe', '', r'C:\Windows\Fonts\segoeui.ttf', uni=True)
        self.add_font('Segoe', 'B', r'C:\Windows\Fonts\segoeuib.ttf', uni=True)
        self.add_font('Segoe', 'I', r'C:\Windows\Fonts\segoeuii.ttf', uni=True)

    def draw_card_frame(self, x, y):
        self.set_draw_color(60, 60, 60)
        self.set_line_width(0.6)
        self.rect(x, y, CARD_W, CARD_H, style='D')

    def draw_header(self, x, y, title, subtitle):
        self.set_font('Segoe', 'B', 16)
        self.set_text_color(30, 30, 30)
        self.set_xy(x, y + 3)
        self.cell(CARD_W, 7, title, align='C', new_x='LMARGIN')
        self.set_font('Segoe', '', 8)
        self.set_text_color(*GRAY_TEXT)
        self.set_xy(x, y + 10)
        self.cell(CARD_W, 5, subtitle, align='C', new_x='LMARGIN')
        self.set_draw_color(60, 60, 60)
        self.set_line_width(0.4)
        self.line(x + 4, y + 16, x + CARD_W - 4, y + 16)
        return y + 18

    def draw_mode_row(self, x, y, code, color, name, hint, row_h=9.5):
        badge_w = 16
        badge_h = 6.5
        badge_x = x + 4
        badge_y = y + (row_h - badge_h) / 2
        self.set_fill_color(*color)
        self.rect(badge_x, badge_y, badge_w, badge_h, style='F')
        self.set_font('Segoe', 'B', 9)
        self.set_text_color(*WHITE)
        self.set_xy(badge_x, badge_y + 0.3)
        self.cell(badge_w, badge_h, code, align='C', new_x='LMARGIN')
        self.set_font('Segoe', 'B', 10)
        self.set_text_color(30, 30, 30)
        self.set_xy(badge_x + badge_w + 3, y + 0.5)
        self.cell(CARD_W - badge_w - 14, 5, name, new_x='LMARGIN')
        self.set_font('Segoe', '', 7)
        self.set_text_color(*GRAY_TEXT)
        self.set_xy(badge_x + badge_w + 3, y + 5)
        self.cell(CARD_W - badge_w - 14, 4, hint, new_x='LMARGIN')
        self.set_draw_color(*GRAY_LINE)
        self.set_line_width(0.15)
        self.line(x + 4, y + row_h, x + CARD_W - 4, y + row_h)
        return y + row_h

    def add_card_page(self, draw_func):
        """Single card centered on A4."""
        self.add_page()
        self.set_fill_color(*GRAY_BG)
        self.rect(0, 0, A4_W, A4_H, style='F')
        cx = (A4_W - CARD_W) / 2
        cy = (A4_H - CARD_H) / 2
        self.set_fill_color(*WHITE)
        self.rect(cx, cy, CARD_W, CARD_H, style='F')
        draw_func(cx, cy)
        self.draw_card_frame(cx, cy)
        self.set_font('Segoe', 'I', 7)
        self.set_text_color(180, 180, 180)
        self.set_xy(0, A4_H - 12)
        self.cell(A4_W, 5, 'Recortar na borda e laminar', align='C')

    def add_foldable_page(self, draw_front_func, draw_back_func):
        """Front on top half, back on bottom half rotated 180 for folding."""
        HALF_H = A4_H / 2
        self.add_page()
        self.set_fill_color(*GRAY_BG)
        self.rect(0, 0, A4_W, A4_H, style='F')

        # Top half: front
        cx_top = (A4_W - CARD_W) / 2
        cy_top = (HALF_H - CARD_H) / 2
        self.set_fill_color(*WHITE)
        self.rect(cx_top, cy_top, CARD_W, CARD_H, style='F')
        draw_front_func(cx_top, cy_top)
        self.draw_card_frame(cx_top, cy_top)

        # Fold line
        self.set_draw_color(180, 180, 180)
        self.set_line_width(0.2)
        x = 15
        while x < A4_W - 15:
            self.line(x, HALF_H, x + 4, HALF_H)
            x += 7
        self.set_font('Segoe', 'I', 6)
        self.set_text_color(180, 180, 180)
        self.set_xy(0, HALF_H - 4)
        self.cell(A4_W, 3, '--- dobrar aqui --- recortar na borda --- frente acima, verso abaixo (invertido) ---', align='C')

        # Bottom half: back (rotated 180)
        cx_back = (A4_W - CARD_W) / 2
        cy_back = HALF_H + (HALF_H - CARD_H) / 2
        rot_cx = cx_back + CARD_W / 2
        rot_cy = cy_back + CARD_H / 2
        with self.rotation(180, rot_cx, rot_cy):
            self.set_fill_color(*WHITE)
            self.rect(cx_back, cy_back, CARD_W, CARD_H, style='F')
            draw_back_func(cx_back, cy_back)
            self.draw_card_frame(cx_back, cy_back)


def build_pdf():
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.makedirs(os.path.join(project_root, 'output'), exist_ok=True)

    # === G9 MkI ===
    pdf_g9 = ModeCardPDF()

    def draw_mki(x, y):
        row_y = pdf_g9.draw_header(x, y, 'LUMIX G9', 'Modos Customizados -- O Que Cada Um Faz')
        modes = [
            ('C1',   BLUE,  'Street / General',  'Passeio, fotos do dia a dia'),
            ('C2',   BLUE,  'Portrait / People',  'Fotos de pessoas, fundo desfocado'),
            ('C3-1', BLUE,  'Landscape',          'Paisagens, tudo em foco'),
            ('C3-2', GREEN, 'Macro + Flash',      'Insetos, sapos, close-ups com flash'),
            ('C3-3', RED,   'Wildlife / Action',  'Aves, animais, rajada rapida'),
        ]
        row_h = (CARD_H - (row_y - y) - 2) / len(modes)
        for code, color, name, hint in modes:
            row_y = pdf_g9.draw_mode_row(x, row_y, code, color, name, hint, row_h)

    pdf_g9.add_card_page(draw_mki)
    out_g9 = os.path.join(project_root, 'output', 'G9-Mode-Cards.pdf')
    pdf_g9.output(out_g9)
    print(f"  G9-Mode-Cards.pdf: 1 card (1 page)")

    # === G9 MkII ===
    pdf_g9ii = ModeCardPDF()

    def draw_mkii_photo(x, y):
        row_y = pdf_g9ii.draw_header(x, y, 'LUMIX G9II', 'Modos Foto (1 de 2)')
        modes = [
            ('C1',   BLUE,   'Street / General',   'Passeio, fotos do dia a dia'),
            ('C2',   BLUE,   'Portrait / People',   'Pessoas, fundo desfocado'),
            ('C3-1', BLUE,   'Landscape',            'Paisagens, tudo em foco'),
            ('C3-2', GREEN,  'Macro + Flash',        'Insetos, sapos, close-ups'),
            ('C3-3', RED,    'Wildlife / Action',    'Aves e animais, rajada'),
            ('C3-4', GREEN,  'Tripod Macro Stack',   'Focus stacking no tripe'),
        ]
        row_h = (CARD_H - (row_y - y) - 2) / len(modes)
        for code, color, name, hint in modes:
            row_y = pdf_g9ii.draw_mode_row(x, row_y, code, color, name, hint, row_h)

    def draw_mkii_special(x, y):
        row_y = pdf_g9ii.draw_header(x, y, 'LUMIX G9II', 'Modos Especiais e Video (2 de 2)')
        modes = [
            ('C3-5', RED,    'Birds + Crop Zoom',   'Alcance extra, somente JPEG'),
            ('C3-6', PURPLE, 'Lightning',            'Live Composite, tripe'),
            ('C3-7', PURPLE, 'Indoor / Low Light',   'Museus, silencioso'),
            ('C3-8', RED,    'Birds in Flight',      'Tracking rapido, 1/4000s'),
            ('C3-9', ORANGE, 'Video -- Travel',      '4K 30p, exposicao auto'),
            ('C3-10',ORANGE, 'Video -- Wildlife',    '4K 30p, manual, tracking'),
        ]
        row_h = (CARD_H - (row_y - y) - 2) / len(modes)
        for code, color, name, hint in modes:
            row_y = pdf_g9ii.draw_mode_row(x, row_y, code, color, name, hint, row_h)

    pdf_g9ii.add_foldable_page(draw_mkii_photo, draw_mkii_special)
    out_g9ii = os.path.join(project_root, 'output', 'G9II-Mode-Cards.pdf')
    pdf_g9ii.output(out_g9ii)
    print(f"  G9II-Mode-Cards.pdf: 2 cards (1 foldable page)")

    print(f"\nPDFs saved to: {os.path.join(project_root, 'output')}")


if __name__ == '__main__':
    build_pdf()
