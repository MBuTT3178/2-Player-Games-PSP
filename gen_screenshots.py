#!/usr/bin/env python3
import struct
import zlib
import math

W = 480
H = 272
SCALE = 2 # 960x544 crisp 2x integer scale for retina / high-DPI GitHub rendering

font8x8 = [
    [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00], # 32 SPC
    [0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00], # 33 !
    [0x6C,0x6C,0x00,0x00,0x00,0x00,0x00,0x00], # 34 "
    [0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00], # 35 #
    [0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00], # 36 $
    [0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00], # 37 %
    [0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00], # 38 &
    [0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00], # 39 '
    [0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00], # 40 (
    [0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00], # 41 )
    [0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00], # 42 *
    [0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00], # 43 +
    [0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30], # 44 ,
    [0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00], # 45 -
    [0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00], # 46 .
    [0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00], # 47 /
    [0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00], # 48 0
    [0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00], # 49 1
    [0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00], # 50 2
    [0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00], # 51 3
    [0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00], # 52 4
    [0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00], # 53 5
    [0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00], # 54 6
    [0x7E,0x06,0x0C,0x18,0x18,0x18,0x18,0x00], # 55 7
    [0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00], # 56 8
    [0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00], # 57 9
    [0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00], # 58 :
    [0x00,0x00,0x18,0x00,0x00,0x18,0x18,0x30], # 59 ;
    [0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00], # 60 <
    [0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00], # 61 =
    [0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00], # 62 >
    [0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00], # 63 ?
    [0x3C,0x66,0x6E,0x6E,0x60,0x62,0x3C,0x00], # 64 @
    [0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00], # 65 A
    [0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00], # 66 B
    [0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00], # 67 C
    [0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00], # 68 D
    [0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00], # 69 E
    [0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00], # 70 F
    [0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00], # 71 G
    [0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00], # 72 H
    [0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00], # 73 I
    [0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00], # 74 J
    [0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00], # 75 K
    [0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00], # 76 L
    [0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00], # 77 M
    [0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00], # 78 N
    [0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00], # 79 O
    [0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00], # 80 P
    [0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00], # 81 Q
    [0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00], # 82 R
    [0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00], # 83 S
    [0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00], # 84 T
    [0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00], # 85 U
    [0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00], # 86 V
    [0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00], # 87 W
    [0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00], # 88 X
    [0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00], # 89 Y
    [0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00], # 90 Z
]

COL_BG     = (14, 18, 36)
COL_CARD   = (24, 32, 60)
COL_BORDER = (45, 60, 100)
COL_BLUE   = (0, 180, 255)
COL_RED    = (255, 60, 80)
COL_WHITE  = (255, 255, 255)
COL_GOLD   = (255, 200, 30)
COL_DIM    = (110, 130, 165)
COL_SHADOW = (10, 16, 36)

class Canvas:
    def __init__(self):
        self.buf = [[COL_BG for _ in range(W)] for _ in range(H)]

    def clear(self, col=COL_BG):
        for y in range(H):
            for x in range(W):
                self.buf[y][x] = col

    def set_pixel(self, x, y, col):
        if 0 <= x < W and 0 <= y < H:
            self.buf[y][x] = col

    def fill_rect(self, x, y, w, h, col):
        for j in range(max(0, y), min(H, y + h)):
            for i in range(max(0, x), min(W, x + w)):
                self.buf[j][i] = col

    def draw_rect(self, x, y, w, h, col, thick=1):
        self.fill_rect(x, y, w, thick, col)
        self.fill_rect(x, y + h - thick, w, thick, col)
        self.fill_rect(x, y, thick, h, col)
        self.fill_rect(x + w - thick, y, thick, h, col)

    def draw_line(self, x0, y0, x1, y1, col):
        dx = abs(x1 - x0)
        dy = abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx - dy
        while True:
            self.set_pixel(x0, y0, col)
            if x0 == x1 and y0 == y1:
                break
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x0 += sx
            if e2 < dx:
                err += dx
                y0 += sy

    def draw_circle(self, cx, cy, radius, col, fill=True):
        for dy in range(-radius, radius + 1):
            dx_max = int(math.sqrt(max(0, radius * radius - dy * dy)))
            if fill:
                for dx in range(-dx_max, dx_max + 1):
                    self.set_pixel(cx + dx, cy + dy, col)
            else:
                self.set_pixel(cx - dx_max, cy + dy, col)
                self.set_pixel(cx + dx_max, cy + dy, col)

    # Standard Horizontal Text
    def draw_char(self, x, y, ch, col, scale=1):
        c = ord(ch.upper())
        if c < 32 or c > 90: return
        glyph = font8x8[c - 32]
        for row in range(8):
            bits = glyph[row]
            for col_idx in range(8):
                if bits & (0x80 >> col_idx):
                    self.fill_rect(x + col_idx * scale, y + row * scale, scale, scale, col)

    def draw_str(self, x, y, text, col, scale=1):
        cx = x
        for ch in text.upper():
            self.draw_char(cx, y, ch, col, scale)
            cx += 8 * scale

    def draw_centered_str(self, cx, y, text, col, scale=1):
        w = len(text) * 8 * scale
        self.draw_str(cx - w // 2, y, text, col, scale)

    # Rotated Text Player 1 (Facing West looking East)
    def draw_char_p1(self, x, y, ch, col, scale=1):
        c = ord(ch.upper())
        if c < 32 or c > 90: return
        glyph = font8x8[c - 32]
        for row in range(8):
            bits = glyph[row]
            for col_idx in range(8):
                if bits & (0x80 >> col_idx):
                    px = x + (7 - row) * scale
                    py = y + col_idx * scale
                    self.fill_rect(px, py, scale, scale, col)

    def draw_str_p1(self, x, y, text, col, scale=1):
        cy = y
        for ch in text.upper():
            self.draw_char_p1(x, cy, ch, col, scale)
            cy += 8 * scale

    def draw_centered_str_p1(self, x, cy, text, col, scale=1):
        total_len = len(text) * 8 * scale
        y = cy - total_len // 2
        self.draw_str_p1(x, y, text, col, scale)

    # Rotated Text Player 2 (Facing East looking West)
    def draw_char_p2(self, x, y, ch, col, scale=1):
        c = ord(ch.upper())
        if c < 32 or c > 90: return
        glyph = font8x8[c - 32]
        for row in range(8):
            bits = glyph[row]
            for col_idx in range(8):
                if bits & (0x80 >> col_idx):
                    px = x + row * scale
                    py = y - col_idx * scale
                    self.fill_rect(px, py, scale, scale, col)

    def draw_str_p2(self, x, y, text, col, scale=1):
        cy = y
        for ch in text.upper():
            self.draw_char_p2(x, cy, ch, col, scale)
            cy -= 8 * scale

    def draw_centered_str_p2(self, x, cy, text, col, scale=1):
        total_len = len(text) * 8 * scale
        y = cy + total_len // 2 - scale
        self.draw_str_p2(x, y, text, col, scale)

    def save_png(self, filename):
        out_w = W * SCALE
        out_h = H * SCALE
        raw = bytearray()
        for y in range(out_h):
            raw.append(0)
            src_y = y // SCALE
            row = self.buf[src_y]
            for x in range(out_w):
                src_x = x // SCALE
                r, g, b = row[src_x]
                raw.extend((r, g, b, 255))
        compressed = zlib.compress(bytes(raw), 9)

        def chunk(tag, data):
            return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

        png = bytearray(b"\x89PNG\r\n\x1a\n")
        ihdr = struct.pack(">IIBBBBB", out_w, out_h, 8, 6, 0, 0, 0)
        png.extend(chunk(b"IHDR", ihdr))
        png.extend(chunk(b"IDAT", compressed))
        png.extend(chunk(b"IEND", b""))

        with open(filename, "wb") as f:
            f.write(png)
        print(f"Saved: {filename} ({out_w}x{out_h})")


# ─── 1. MAIN MENU SCREENSHOT (6 GAMES) ────────────────────────────────
def make_screenshot_menu():
    c = Canvas()
    for y in range(H):
        t = y * 100 // H
        r = (14 * (100 - t) + 8 * t) // 100
        g = (18 * (100 - t) + 12 * t) // 100
        b = (36 * (100 - t) + 24 * t) // 100
        for x in range(W):
            c.buf[y][x] = (r, g, b)

    c.draw_centered_str(W // 2 + 1, 19, "2 PLAYER GAMES", COL_SHADOW, 3)
    c.draw_centered_str(W // 2, 18, "2 PLAYER GAMES", COL_GOLD, 3)
    c.draw_centered_str(W // 2, 46, "THE CHALLENGE - PSP EDITION", COL_DIM, 1)

    titles1 = ["PING", "AIR", "SPINNER", "TANK", "CHECKERS", "ROYAL"]
    titles2 = ["PONG", "HOCKEY", "WAR", "BATTLE", "DAMA", "CHESS"]
    descs = [
        "CLASSIC PADDLE RALLY BATTLE",
        "2D AIR HOCKEY TABLE & SMASHES",
        "BEYBLADE SUMO RING KNOCKOUT",
        "TACTICAL BOUNCING SHELL COMBAT",
        "CLASSIC 8x8 DAMA & KING CROWNS",
        "TACTICAL ROYAL BOARD CHECKMATE"
    ]
    selected = 5 # Chess selected

    for i in range(6):
        cx = 16 + i * 76
        cy = 88
        cw = 68
        ch = 76
        if i == selected:
            c.fill_rect(cx - 2, cy - 2, cw + 4, ch + 4, COL_BLUE)
        c.fill_rect(cx, cy, cw, ch, COL_CARD)
        c.draw_rect(cx, cy, cw, ch, COL_WHITE if i == selected else COL_BORDER)
        c.draw_centered_str(cx + cw // 2, cy + 10, f"#{i+1}", COL_GOLD, 1)
        c.draw_centered_str(cx + cw // 2, cy + 30, titles1[i], COL_WHITE if i == selected else COL_DIM, 1)
        c.draw_centered_str(cx + cw // 2, cy + 44, titles2[i], COL_WHITE if i == selected else COL_DIM, 1)

    c.draw_centered_str(W // 2, 174, descs[selected], COL_WHITE, 1)

    mode_y = 196
    c.fill_rect(W // 2 - 135, mode_y, 270, 24, COL_CARD)
    c.draw_rect(W // 2 - 135, mode_y, 270, 24, COL_GOLD)
    c.draw_centered_str(W // 2, mode_y + 8, "< MODE: 2 PLAYERS (SPLIT OPPS) >", COL_GOLD, 1)

    c.draw_centered_str(W // 2, 232, "P1 (WEST): DPAD RIGHT=UP + L  |  P2 (EAST): SQUARE=UP + R", COL_DIM, 1)
    c.draw_centered_str(W // 2, 248, "SELECT: TOGGLE MODE   |   START / X / L / R: BEGIN", COL_WHITE, 1)

    c.save_png("screenshot_menu.png")
    c.save_png("screenshot.png")


# ─── 2. PING PONG GAMEPLAY SCREENSHOT ─────────────────────────────────
def make_screenshot_pingpong():
    c = Canvas()
    for y in range(20, H - 10, 12):
        c.fill_rect(W // 2 - 1, y, 2, 6, COL_BORDER)

    c.fill_rect(10, 20, W - 20, 2, COL_BORDER)
    c.fill_rect(10, H - 20, W - 20, 2, COL_BORDER)

    p1_y = 120
    c.fill_rect(24, p1_y - 25, 10, 50, COL_BLUE)

    p2_y = 150
    c.fill_rect(W - 34, p2_y - 25, 10, 50, COL_RED)

    ball_x, ball_y = 310, 142
    c.draw_circle(ball_x - 12, ball_y - 3, 2, (100, 100, 120))
    c.draw_circle(ball_x - 6, ball_y - 1, 3, (180, 180, 200))
    c.draw_circle(ball_x, ball_y, 5, COL_WHITE)

    c.draw_centered_str_p1(45, H // 2, "3", COL_BLUE, 3)
    c.draw_centered_str_p1(18, H // 2, "PLAYER 1", COL_DIM, 1)

    c.draw_centered_str_p2(W - 66, H // 2, "2", COL_RED, 3)
    c.draw_centered_str_p2(W - 22, H // 2, "PLAYER 2", COL_DIM, 1)

    c.draw_line(W // 2, 0, W // 2, 8, COL_BORDER)
    c.draw_line(W // 2, H - 8, W // 2, H, COL_BORDER)
    c.draw_centered_str(W // 2, 2, "2P MATCH", COL_DIM, 1)

    c.save_png("screenshot_pingpong.png")


# ─── 3. AIR HOCKEY GAMEPLAY SCREENSHOT ────────────────────────────────
def make_screenshot_airhockey():
    c = Canvas()
    c.fill_rect(20, 22, W - 40, H - 44, COL_CARD)
    c.draw_rect(20, 22, W - 40, H - 44, COL_BORDER)

    c.draw_line(W // 2, 22, W // 2, H - 22, COL_BORDER)
    c.draw_circle(W // 2, H // 2, 35, COL_BORDER, False)

    c.fill_rect(16, 96, 6, 80, COL_RED)
    c.fill_rect(W - 22, 96, 6, 80, COL_BLUE)

    p1_x, p1_y = 175, 125
    c.draw_circle(p1_x, p1_y, 16, COL_BLUE)
    c.draw_circle(p1_x, p1_y, 7, COL_WHITE)

    puck_x, puck_y = 230, 105
    c.draw_circle(puck_x, puck_y, 9, COL_WHITE)
    c.draw_circle(puck_x, puck_y, 5, COL_BG)

    p2_x, p2_y = W - 85, 140
    c.draw_circle(p2_x, p2_y, 16, COL_RED)
    c.draw_circle(p2_x, p2_y, 7, COL_WHITE)

    c.draw_centered_str_p1(40, H // 2, "4", COL_BLUE, 3)
    c.draw_centered_str_p1(22, H // 2, "PLAYER 1", COL_DIM, 1)

    c.draw_centered_str_p2(W - 64, H // 2, "3", COL_RED, 3)
    c.draw_centered_str_p2(W - 28, H // 2, "PLAYER 2", COL_DIM, 1)

    c.save_png("screenshot_airhockey.png")


# ─── 4. SPINNER WAR GAMEPLAY SCREENSHOT ───────────────────────────────
def make_screenshot_spinners():
    c = Canvas()
    arena_cx = W // 2
    arena_cy = H // 2 + 6
    arena_r = 95

    c.draw_circle(arena_cx, arena_cy, arena_r + 4, (30, 40, 70))
    c.draw_circle(arena_cx, arena_cy, arena_r, (20, 26, 50))
    c.draw_circle(arena_cx, arena_cy, arena_r - 18, (25, 34, 65), False)

    p1_x, p1_y, p1_rot = arena_cx - 28, arena_cy - 10, 45.0
    c.draw_circle(p1_x, p1_y, 15, COL_BLUE)
    for i in range(3):
        ang = math.radians(p1_rot + i * 120.0)
        bx = int(p1_x + math.cos(ang) * 19.0)
        by = int(p1_y + math.sin(ang) * 19.0)
        c.draw_line(p1_x, p1_y, bx, by, COL_WHITE)

    p2_x, p2_y, p2_rot = arena_cx + 28, arena_cy + 10, 110.0
    c.draw_circle(p2_x, p2_y, 15, COL_RED)
    for i in range(3):
        ang = math.radians(p2_rot + i * 120.0)
        bx = int(p2_x + math.cos(ang) * 19.0)
        by = int(p2_y + math.sin(ang) * 19.0)
        c.draw_line(p2_x, p2_y, bx, by, COL_WHITE)

    spark_x = (p1_x + p2_x) // 2
    spark_y = (p1_y + p2_y) // 2
    c.draw_circle(spark_x, spark_y, 4, COL_GOLD)
    c.draw_line(spark_x - 7, spark_y - 5, spark_x + 7, spark_y + 5, COL_WHITE)
    c.draw_line(spark_x + 6, spark_y - 6, spark_x - 6, spark_y + 6, COL_GOLD)

    c.draw_centered_str_p1(40, H // 2, "2", COL_BLUE, 3)
    c.draw_centered_str_p1(18, H // 2, "BOOST: L", COL_BLUE, 1)
    c.fill_rect(62, H // 2 - 25, 4, 38, COL_BLUE)

    c.draw_centered_str_p2(W - 64, H // 2, "1", COL_RED, 3)
    c.draw_centered_str_p2(W - 24, H // 2, "BOOST: R", COL_RED, 1)
    c.fill_rect(W - 66, H // 2 - 25, 4, 45, COL_RED)

    c.save_png("screenshot_spinners.png")


# ─── 5. TANK BATTLE GAMEPLAY SCREENSHOT ───────────────────────────────
def make_screenshot_tanks():
    c = Canvas()
    c.fill_rect(14, 14, W - 28, H - 28, COL_CARD)
    c.draw_rect(14, 14, W - 28, H - 28, COL_BORDER)

    obstacles = [
        (130, 60, 20, 60),
        (130, H - 120, 20, 60),
        (W - 150, 60, 20, 60),
        (W - 150, H - 120, 20, 60),
        (W // 2 - 10, 40, 20, 50),
        (W // 2 - 10, H - 90, 20, 50),
    ]
    for ox, oy, ow, oh in obstacles:
        c.fill_rect(ox, oy, ow, oh, COL_BORDER)
        c.draw_rect(ox, oy, ow, oh, COL_DIM)

    t1_x, t1_y, t1_ang = 85, 136, math.radians(25)
    c.draw_circle(t1_x, t1_y, 10, COL_BLUE)
    c.draw_line(t1_x, t1_y, int(t1_x + math.cos(t1_ang) * 15), int(t1_y + math.sin(t1_ang) * 15), COL_WHITE)

    t2_x, t2_y, t2_ang = W - 85, 120, math.radians(160)
    c.draw_circle(t2_x, t2_y, 10, COL_RED)
    c.draw_line(t2_x, t2_y, int(t2_x + math.cos(t2_ang) * 15), int(t2_y + math.sin(t2_ang) * 15), COL_WHITE)

    b1_x, b1_y = 195, 100
    c.draw_circle(b1_x, b1_y, 3, COL_GOLD)
    c.draw_line(b1_x - 8, b1_y - 4, b1_x, b1_y, (180, 140, 20))

    b2_x, b2_y = 280, 175
    c.draw_circle(b2_x, b2_y, 3, COL_GOLD)
    c.draw_line(b2_x + 8, b2_y + 4, b2_x, b2_y, (180, 140, 20))

    c.draw_centered_str_p1(38, 45, "3", COL_BLUE, 2)
    c.draw_centered_str_p1(20, 45, "P1", COL_BLUE, 1)

    c.draw_centered_str_p2(W - 54, 45, "2", COL_RED, 2)
    c.draw_centered_str_p2(W - 26, 45, "P2", COL_RED, 1)

    c.save_png("screenshot_tanks.png")


# ─── 6. CHECKERS (DAMA) SCREENSHOT ────────────────────────────────────
def make_screenshot_checkers():
    c = Canvas()
    bx, by = 128, 24
    sz = 28

    c.fill_rect(bx - 3, by - 3, sz * 8 + 6, sz * 8 + 6, (35, 45, 65))
    c.draw_rect(bx - 3, by - 3, sz * 8 + 6, sz * 8 + 6, COL_BORDER)

    # Board state with active mid-game match
    board = [[0]*8 for _ in range(8)]
    # Red pieces (top)
    for (r, cl) in [(0, 1), (0, 3), (0, 5), (0, 7), (1, 0), (1, 4), (1, 6), (2, 1), (2, 5), (3, 4)]:
        board[r][cl] = -1
    board[1][2] = -2 # Red King

    # Blue pieces (bottom)
    for (r, cl) in [(5, 0), (5, 2), (5, 6), (6, 1), (6, 3), (6, 7), (7, 0), (7, 2), (7, 4), (7, 6)]:
        board[r][cl] = 1
    board[4][3] = 1 # Blue piece selected
    board[6][5] = 2 # Blue King

    selected_r, selected_c = 4, 3

    for r in range(8):
        for cl in range(8):
            sq_x = bx + cl * sz
            sq_y = by + r * sz
            is_dark = ((r + cl) % 2 == 1)
            c.fill_rect(sq_x, sq_y, sz, sz, (70, 95, 130) if is_dark else (220, 215, 195))

            if r == selected_r and cl == selected_c:
                c.fill_rect(sq_x, sq_y, sz, sz, (50, 160, 100))

            p = board[r][cl]
            if p != 0:
                cx = sq_x + sz // 2
                cy = sq_y + sz // 2
                col = COL_BLUE if p > 0 else COL_RED
                c.draw_circle(cx, cy, 10, col)
                c.draw_circle(cx, cy, 10, COL_WHITE, False)
                if abs(p) == 2:
                    c.draw_circle(cx, cy, 4, COL_GOLD)
                    c.draw_char(cx - 4, cy - 4, 'K', COL_WHITE, 1)

    # Move dots for selected piece at (4,3): jump over (3,4) to (2,5)!
    dest_x = bx + 5 * sz + sz // 2
    dest_y = by + 2 * sz + sz // 2
    c.draw_circle(dest_x, dest_y, 11, COL_RED, False) # Jump capture indicator

    dest_reg_x = bx + 2 * sz + sz // 2
    dest_reg_y = by + 3 * sz + sz // 2
    c.draw_circle(dest_reg_x, dest_reg_y, 4, COL_GOLD)

    # Cursor
    c.draw_rect(bx + 5 * sz - 1, by + 2 * sz - 1, sz + 2, sz + 2, COL_GOLD)

    # Dual Facing UI
    c.draw_centered_str_p1(20, H // 2, "PLAYER 1", COL_BLUE, 1)
    c.draw_centered_str_p1(45, H // 2, "YOUR TURN", COL_GOLD, 1)
    c.draw_centered_str_p1(75, H // 2, "CAPT: 2", COL_WHITE, 1)

    c.draw_centered_str_p2(W - 20, H // 2, "PLAYER 2", COL_RED, 1)
    c.draw_centered_str_p2(W - 45, H // 2, "WAITING", COL_DIM, 1)
    c.draw_centered_str_p2(W - 75, H // 2, "CAPT: 1", COL_WHITE, 1)

    c.save_png("screenshot_checkers.png")


# ─── 7. CHESS SCREENSHOT ──────────────────────────────────────────────
def make_screenshot_chess():
    c = Canvas()
    bx, by = 128, 24
    sz = 28

    c.fill_rect(bx - 3, by - 3, sz * 8 + 6, sz * 8 + 6, (35, 45, 65))
    c.draw_rect(bx - 3, by - 3, sz * 8 + 6, sz * 8 + 6, COL_BORDER)

    # Standard mid-game Sicilian/Italian opening position
    board = [[0]*8 for _ in range(8)]
    board[0] = [-4, 0, -3, -5, -6, -3, 0, -4]
    board[1] = [-1, -1, -1, 0, -1, -1, -1, -1]
    board[2] = [0, 0, -2, 0, 0, -2, 0, 0] # Knights out
    board[3] = [0, 0, 0, -1, 0, 0, 0, 0] # Black pawn on d5
    board[4] = [0, 0, 0, 1, 1, 0, 0, 0]  # White pawns on d4, e4
    board[5] = [0, 0, 2, 0, 0, 2, 0, 0]  # White Knights on c3, f3
    board[6] = [1, 1, 1, 0, 0, 1, 1, 1]
    board[7] = [4, 0, 3, 5, 6, 3, 0, 4]

    selected_r, selected_c = 5, 5 # White Knight on f3 selected!
    p_names = {1:'P', 2:'N', 3:'B', 4:'R', 5:'Q', 6:'K'}

    for r in range(8):
        for cl in range(8):
            sq_x = bx + cl * sz
            sq_y = by + r * sz
            is_dark = ((r + cl) % 2 == 1)
            c.fill_rect(sq_x, sq_y, sz, sz, (70, 95, 130) if is_dark else (220, 215, 195))

            if r == selected_r and cl == selected_c:
                c.fill_rect(sq_x, sq_y, sz, sz, (50, 160, 100))

            p = board[r][cl]
            if p != 0:
                cx = sq_x + sz // 2
                cy = sq_y + sz // 2
                col = COL_BLUE if p > 0 else COL_RED
                c.draw_circle(cx, cy, 11, col)
                c.draw_circle(cx, cy, 11, COL_WHITE, False)
                symbol_col = COL_GOLD if abs(p) >= 5 else COL_WHITE
                c.draw_char(cx - 4, cy - 4, p_names[abs(p)], symbol_col, 1)

    # Valid destination targets for Knight on f3: d4 (pawn), e5 (strike), g5 (jump)
    for (tr, tc, cap) in [(3, 3, True), (3, 6, False), (4, 7, False), (2, 4, False)]:
        cx = bx + tc * sz + sz // 2
        cy = by + tr * sz + sz // 2
        if cap:
            c.draw_circle(cx, cy, 11, COL_RED, False)
        else:
            c.draw_circle(cx, cy, 4, COL_GOLD)

    # Cursor on capture target (3, 3)
    c.draw_rect(bx + 3 * sz - 1, by + 3 * sz - 1, sz + 2, sz + 2, COL_GOLD)

    # Dual Facing UI
    c.draw_centered_str_p1(20, H // 2, "PLAYER 1", COL_BLUE, 1)
    c.draw_centered_str_p1(45, H // 2, "YOUR TURN", COL_GOLD, 1)
    c.draw_centered_str_p1(75, H // 2, "SCORE: 3", COL_WHITE, 1)

    c.draw_centered_str_p2(W - 20, H // 2, "PLAYER 2", COL_RED, 1)
    c.draw_centered_str_p2(W - 45, H // 2, "WAITING", COL_DIM, 1)
    c.draw_centered_str_p2(W - 75, H // 2, "SCORE: 1", COL_WHITE, 1)

    c.save_png("screenshot_chess.png")


# ─── 8. DUAL FACING GAME OVER SCREENSHOT ──────────────────────────────
def make_screenshot_gameover():
    c = Canvas()
    c.fill_rect(20, 22, W - 40, H - 44, (18, 24, 46))
    c.draw_rect(20, 22, W - 40, H - 44, (35, 45, 75))

    c.draw_line(W // 2, 20, W // 2, H - 20, COL_BORDER)

    pop1_x, pop1_y, pop1_w, pop1_h = 30, H // 2 - 55, 180, 110
    c.fill_rect(pop1_x, pop1_y, pop1_w, pop1_h, COL_CARD)
    c.draw_rect(pop1_x, pop1_y, pop1_w, pop1_h, COL_BLUE)

    c.draw_centered_str_p1(130, H // 2, "YOU WIN!", COL_GOLD, 2)
    c.draw_centered_str_p1(105, H // 2, "CHAMPION", COL_BLUE, 1)
    c.draw_centered_str_p1(70, H // 2, "PRESS L TO RETRY", COL_WHITE, 1)

    pop2_x, pop2_y, pop2_w, pop2_h = 270, H // 2 - 55, 180, 110
    c.fill_rect(pop2_x, pop2_y, pop2_w, pop2_h, COL_CARD)
    c.draw_rect(pop2_x, pop2_y, pop2_w, pop2_h, COL_BORDER)

    c.draw_centered_str_p2(W - 150, H // 2, "YOU LOSE", COL_BLUE, 2)
    c.draw_centered_str_p2(W - 125, H // 2, "DEFEAT", COL_DIM, 1)
    c.draw_centered_str_p2(W - 90, H // 2, "PRESS R TO RETRY", COL_WHITE, 1)

    c.save_png("screenshot_gameover.png")


if __name__ == "__main__":
    make_screenshot_menu()
    make_screenshot_pingpong()
    make_screenshot_airhockey()
    make_screenshot_spinners()
    make_screenshot_tanks()
    make_screenshot_checkers()
    make_screenshot_chess()
    make_screenshot_gameover()
    print("All 8 screenshots generated successfully.")
