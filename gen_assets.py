#!/usr/bin/env python3
import struct
import zlib

def make_png(width, height, get_pixel_func):
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        for x in range(width):
            r, g, b, a = get_pixel_func(x, y, width, height)
            raw.extend((r, g, b, a))
    compressed = zlib.compress(bytes(raw), 9)

    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)

    png = bytearray(b"\x89PNG\r\n\x1a\n")
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    png.extend(chunk(b"IHDR", ihdr))
    png.extend(chunk(b"IDAT", compressed))
    png.extend(chunk(b"IEND", b""))
    return bytes(png)

# ICON0.PNG: 144x80 — Blue vs Red 1v1 Split Badge
def icon_pixel(x, y, w, h):
    # Rounded badge border
    dx = min(x, w - 1 - x)
    dy = min(y, h - 1 - y)
    if dx < 2 and dy < 2: return (0, 0, 0, 0)
    if dx < 3 or dy < 3: return (40, 50, 75, 255)

    # Left half: Deep Blue / Cyan
    # Right half: Deep Crimson / Red
    center_split = w // 2
    if x < center_split - 1:
        # Blue gradient
        base_r = 16 + (x * 12 // center_split)
        base_g = 36 + (y * 20 // h)
        base_b = 90 + (x * 40 // center_split)
        # Left circular token (P1 Blue)
        cx, cy = 40, 40
        dist_sq = (x - cx)**2 + (y - cy)**2
        if dist_sq < 256: # R=16
            if dist_sq < 144: return (0, 210, 255, 255) # cyan center
            return (20, 120, 240, 255) # blue ring
        return (base_r, base_g, base_b, 255)
    elif x > center_split + 1:
        # Red gradient
        base_r = 90 + ((w - x) * 40 // center_split)
        base_g = 20 + (y * 20 // h)
        base_b = 26 + ((w - x) * 12 // center_split)
        # Right circular token (P2 Red)
        cx, cy = 104, 40
        dist_sq = (x - cx)**2 + (y - cy)**2
        if dist_sq < 256: # R=16
            if dist_sq < 144: return (255, 80, 80, 255) # bright red center
            return (210, 30, 40, 255) # red ring
        return (base_r, base_g, base_b, 255)
    else:
        # Dividing white neon laser seam
        return (255, 255, 255, 255)

# PIC1.PNG: 480x272 Wallpaper
def pic1_pixel(x, y, w, h):
    # Midnight arena gradient with split neon glow
    center_x = w // 2
    dist_center = abs(x - center_x)
    
    # Left is cool navy/cyan, right is deep indigo/red
    if x < center_x:
        r = 12 + (y * 10 // h)
        g = 20 + ((center_x - dist_center) * 15 // center_x)
        b = 48 + ((center_x - dist_center) * 40 // center_x)
    else:
        r = 48 + ((center_x - dist_center) * 40 // center_x)
        g = 16 + (y * 10 // h)
        b = 28 + (y * 10 // h)
    
    # Center dividing subtle line
    if dist_center == 0:
        return (100, 180, 255, 255)
    
    # Subtle arena grid
    if (x % 32 == 0) or (y % 32 == 0):
        return (r + 10, g + 10, b + 14, 255)
    return (r, g, b, 255)

with open("ICON0.PNG", "wb") as f:
    f.write(make_png(144, 80, icon_pixel))
with open("PIC1.PNG", "wb") as f:
    f.write(make_png(480, 272, pic1_pixel))
print("Assets generated.")
