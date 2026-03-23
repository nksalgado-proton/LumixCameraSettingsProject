"""
Generate printable mode cards for G9 MkI and G9 MkII.
Each card is iPhone 11 sized (~151mm x 76mm), one per page, centered on A4.
"""

from fpdf import FPDF
import os

# iPhone 11 size approx: 150.9mm x 75.7mm
CARD_W = 151
CARD_H = 76

# A4
A4_W = 210
A4_H = 297

# Colors (R, G, B)
BLUE = (21, 101, 192)
GREEN = (46, 125, 50)
RED = (198, 40, 40)
PURPLE = (106, 27, 154)
ORANGE = (230, 81, 0)
BLACK = (26, 26, 26)
WHITE = (255, 255, 255)
GRAY_BG = (245, 245, 245)
GRAY_TEXT = (100, 100, 100)
GRAY_LINE = (210, 210, 210)


class ModeCardPDF(FPDF):
    def __init__(self):
        super().__init__(orientation='P', unit='mm', format='A4')
        self.set_auto_page_break(auto=False)
        # Add Unicode font (Windows built-in)
        self.add_font('Segoe', '', r'C:\Windows\Fonts\segoeui.ttf', uni=True)
        self.add_font('Segoe', 'B', r'C:\Windows\Fonts\segoeuib.ttf', uni=True)
        self.add_font('Segoe', 'I', r'C:\Windows\Fonts\segoeuii.ttf', uni=True)

    def draw_card_frame(self, x, y):
        """Draw the card border with rounded corners."""
        self.set_draw_color(60, 60, 60)
        self.set_line_width(0.6)
        self.rect(x, y, CARD_W, CARD_H, style='D')

    def draw_header(self, x, y, title, subtitle):
        """Draw card header with camera name."""
        # Title
        self.set_font('Segoe', 'B', 16)
        self.set_text_color(30, 30, 30)
        self.set_xy(x, y + 3)
        self.cell(CARD_W, 7, title, align='C', new_x='LMARGIN')

        # Subtitle
        self.set_font('Segoe', '', 8)
        self.set_text_color(*GRAY_TEXT)
        self.set_xy(x, y + 10)
        self.cell(CARD_W, 5, subtitle, align='C', new_x='LMARGIN')

        # Divider line
        self.set_draw_color(60, 60, 60)
        self.set_line_width(0.4)
        self.line(x + 4, y + 16, x + CARD_W - 4, y + 16)

        return y + 18  # return next Y position

    def draw_mode_row(self, x, y, code, color, name, hint, row_h=9.5):
        """Draw a single mode row with badge + name + hint."""
        badge_w = 16
        badge_h = 6.5
        badge_x = x + 4
        badge_y = y + (row_h - badge_h) / 2

        # Badge background
        self.set_fill_color(*color)
        self.rect(badge_x, badge_y, badge_w, badge_h, style='F')

        # Badge text
        self.set_font('Segoe', 'B', 9)
        self.set_text_color(*WHITE)
        self.set_xy(badge_x, badge_y + 0.3)
        self.cell(badge_w, badge_h, code, align='C', new_x='LMARGIN')

        # Mode name
        self.set_font('Segoe', 'B', 10)
        self.set_text_color(30, 30, 30)
        self.set_xy(badge_x + badge_w + 3, y + 0.5)
        self.cell(CARD_W - badge_w - 14, 5, name, new_x='LMARGIN')

        # Hint
        self.set_font('Segoe', '', 7)
        self.set_text_color(*GRAY_TEXT)
        self.set_xy(badge_x + badge_w + 3, y + 5)
        self.cell(CARD_W - badge_w - 14, 4, hint, new_x='LMARGIN')

        # Separator line
        self.set_draw_color(*GRAY_LINE)
        self.set_line_width(0.15)
        self.line(x + 4, y + row_h, x + CARD_W - 4, y + row_h)

        return y + row_h

    def draw_section_label(self, x, y, text):
        """Draw a small section label."""
        self.set_font('Segoe', 'B', 6)
        self.set_text_color(160, 160, 160)
        self.set_xy(x + 5, y)
        self.cell(CARD_W - 10, 4, text.upper(), new_x='LMARGIN')
        return y + 4.5

    def add_card_page(self, draw_func):
        """Add a page with a centered card."""
        self.add_page()

        # Light gray background
        self.set_fill_color(*GRAY_BG)
        self.rect(0, 0, A4_W, A4_H, style='F')

        # Center the card
        cx = (A4_W - CARD_W) / 2
        cy = (A4_H - CARD_H) / 2

        # White card background
        self.set_fill_color(*WHITE)
        self.rect(cx, cy, CARD_W, CARD_H, style='F')

        # Draw content
        draw_func(cx, cy)

        # Frame
        self.draw_card_frame(cx, cy)

        # Cut instruction at bottom
        self.set_font('Segoe', 'I', 7)
        self.set_text_color(180, 180, 180)
        self.set_xy(0, A4_H - 12)
        self.cell(A4_W, 5, 'Cut along the border and laminate', align='C')


def build_pdf():
    pdf = ModeCardPDF()

    # --- Card 1: G9 MkI ---
    def draw_mki(x, y):
        row_y = pdf.draw_header(x, y, 'LUMIX G9', 'Custom Modes \u2014 What Each One Does')
        modes = [
            ('C1',   BLUE,  'Street / General',  'Walking around, everyday photos'),
            ('C2',   BLUE,  'Portrait / People',  'Photos of people, blurred background'),
            ('C3-1', BLUE,  'Landscape',          'Scenery, everything in focus'),
            ('C3-2', GREEN, 'Macro + Flash',      'Insects, frogs, close-ups with flash'),
            ('C3-3', RED,   'Wildlife / Action',  'Birds, animals, fast burst'),
        ]
        row_h = (CARD_H - (row_y - y) - 2) / len(modes)
        for code, color, name, hint in modes:
            row_y = pdf.draw_mode_row(x, row_y, code, color, name, hint, row_h)

    pdf.add_card_page(draw_mki)

    # --- Card 2: G9 MkII (Photo modes) ---
    def draw_mkii_photo(x, y):
        row_y = pdf.draw_header(x, y, 'LUMIX G9II', 'Photo Modes (1 of 2)')
        modes = [
            ('C1',   BLUE,   'Street / General',   'Walking around, everyday'),
            ('C2',   BLUE,   'Portrait / People',   'People, blurred background'),
            ('C3-1', BLUE,   'Landscape',            'Scenery, everything sharp'),
            ('C3-2', GREEN,  'Macro + Flash',        'Insects, frogs, close-ups'),
            ('C3-3', RED,    'Wildlife / Action',    'Birds & animals, burst'),
            ('C3-4', GREEN,  'Tripod Macro Stack',   'Focus stacking on tripod'),
        ]
        row_h = (CARD_H - (row_y - y) - 2) / len(modes)
        for code, color, name, hint in modes:
            row_y = pdf.draw_mode_row(x, row_y, code, color, name, hint, row_h)

    pdf.add_card_page(draw_mkii_photo)

    # --- Card 3: G9 MkII (Special + Video modes) ---
    def draw_mkii_special(x, y):
        row_y = pdf.draw_header(x, y, 'LUMIX G9II', 'Special & Video Modes (2 of 2)')
        modes = [
            ('C3-5', RED,    'Birds + Crop Zoom',   'Extra reach, JPEG only'),
            ('C3-6', PURPLE, 'Lightning',            'Live View Composite, tripod'),
            ('C3-7', PURPLE, 'Indoor / Low Light',   'Museums, quiet places, silent'),
            ('C3-8', RED,    'Birds in Flight',      'Fast tracking, 1/4000s'),
            ('C3-9', ORANGE, 'Video \u2014 Travel',  '4K 30p, auto exposure'),
            ('C3-10',ORANGE, 'Video \u2014 Wildlife', '4K 30p, manual, tracking'),
        ]
        row_h = (CARD_H - (row_y - y) - 2) / len(modes)
        for code, color, name, hint in modes:
            row_y = pdf.draw_mode_row(x, row_y, code, color, name, hint, row_h)

    pdf.add_card_page(draw_mkii_special)

    # Save
    project_root = os.path.dirname(os.path.dirname(__file__))
    out_path = os.path.join(project_root, 'output', 'Mode-Cards-iPhone-Size.pdf')
    pdf.output(out_path)
    print(f"PDF saved to: {out_path}")


if __name__ == '__main__':
    build_pdf()
