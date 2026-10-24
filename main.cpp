/*
 * 2 Player Games: The Challenge — PSP Edition
 * Native C++11 / PSPSDK / SDL2 Port
 * 1v1 Local Split-Console Multiplayer & Single Player VS AI
 */

#include <SDL.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <psppower.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

/* ─── CONSTANTS & TYPES ──────────────────────────────────────────────── */

#define SCR_W 480
#define SCR_H 272

struct Color {
    Uint8 r, g, b, a;
    Color() : r(255), g(255), b(255), a(255) {}
    Color(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a = 255) : r(_r), g(_g), b(_b), a(_a) {}
};

static const Color COL_BG(14, 18, 36);
static const Color COL_CARD(24, 32, 60);
static const Color COL_BORDER(45, 60, 100);
static const Color COL_BLUE(0, 180, 255);
static const Color COL_RED(255, 60, 80);
static const Color COL_WHITE(255, 255, 255);
static const Color COL_GOLD(255, 200, 30);
static const Color COL_DIM(110, 130, 165);
static const Color COL_SHADOW(10, 16, 36);

inline void set_color(SDL_Renderer *ren, const Color &c) {
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
}

/* ─── FONT 8x8 BITMAP ─────────────────────────────────────────────────── */

static const unsigned char font8x8[59][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* 32 SPC */
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, /* 33 !   */
    {0x6C,0x6C,0x00,0x00,0x00,0x00,0x00,0x00}, /* 34 "   */
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, /* 35 #   */
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, /* 36 $   */
    {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00}, /* 37 %   */
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, /* 38 &   */
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, /* 39 '   */
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, /* 40 (   */
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, /* 41 )   */
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, /* 42 *   */
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, /* 43 +   */
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, /* 44 ,   */
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, /* 45 -   */
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, /* 46 .   */
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, /* 47 /   */
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, /* 48 0   */
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, /* 49 1   */
    {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00}, /* 50 2   */
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, /* 51 3   */
    {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00}, /* 52 4   */
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, /* 53 5   */
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, /* 54 6   */
    {0x7E,0x06,0x0C,0x18,0x18,0x18,0x18,0x00}, /* 55 7   */
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, /* 56 8   */
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}, /* 57 9   */
    {0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00}, /* 58 :   */
    {0x00,0x00,0x18,0x00,0x00,0x18,0x18,0x30}, /* 59 ;   */
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00}, /* 60 <   */
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, /* 61 =   */
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00}, /* 62 >   */
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00}, /* 63 ?   */
    {0x3C,0x66,0x6E,0x6E,0x60,0x62,0x3C,0x00}, /* 64 @   */
    {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00}, /* 65 A   */
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, /* 66 B   */
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, /* 67 C   */
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, /* 68 D   */
    {0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00}, /* 69 E   */
    {0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00}, /* 70 F   */
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00}, /* 71 G   */
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, /* 72 H   */
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, /* 73 I   */
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00}, /* 74 J   */
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, /* 75 K   */
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, /* 76 L   */
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, /* 77 M   */
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, /* 78 N   */
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, /* 79 O   */
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, /* 80 P   */
    {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00}, /* 81 Q   */
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00}, /* 82 R   */
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, /* 83 S   */
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, /* 84 T   */
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, /* 85 U   */
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, /* 86 V   */
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, /* 87 W   */
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, /* 88 X   */
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, /* 89 Y   */
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, /* 90 Z   */
};

/* ─── STANDARD HORIZONTAL TEXT (1P / MENU) ──────────────────────────── */

static void draw_char(SDL_Renderer *ren, int x, int y, char ch, const Color &c, int scale = 1) {
    if (ch >= 'a' && ch <= 'z') ch -= 32;
    if (ch < 32 || ch > 90) return;
    const unsigned char *glyph = font8x8[ch - 32];
    set_color(ren, c);
    for (int row = 0; row < 8; row++) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                if (scale == 1) {
                    SDL_RenderDrawPoint(ren, x + col, y + row);
                } else {
                    SDL_Rect rc = {x + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(ren, &rc);
                }
            }
        }
    }
}

static void draw_str(SDL_Renderer *ren, int x, int y, const char *str, const Color &c, int scale = 1) {
    int cx = x;
    while (*str) {
        draw_char(ren, cx, y, *str, c, scale);
        cx += 8 * scale;
        str++;
    }
}

static void draw_centered_str(SDL_Renderer *ren, int cx, int y, const char *str, const Color &c, int scale = 1) {
    int len = (int)strlen(str);
    int x = cx - (len * 8 * scale) / 2;
    draw_str(ren, x, y, str, c, scale);
}

/* ─── ROTATED TEXT: PLAYER 1 (FACING WEST LOOKING EAST) ─────────────── */

static void draw_char_p1(SDL_Renderer *ren, int x, int y, char ch, const Color &c, int scale = 1) {
    if (ch >= 'a' && ch <= 'z') ch -= 32;
    if (ch < 32 || ch > 90) return;
    const unsigned char *glyph = font8x8[ch - 32];
    set_color(ren, c);
    for (int row = 0; row < 8; row++) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                int px = x + (7 - row) * scale;
                int py = y + col * scale;
                if (scale == 1) {
                    SDL_RenderDrawPoint(ren, px, py);
                } else {
                    SDL_Rect rc = {px, py, scale, scale};
                    SDL_RenderFillRect(ren, &rc);
                }
            }
        }
    }
}

static void draw_str_p1(SDL_Renderer *ren, int x, int y, const char *str, const Color &c, int scale = 1) {
    int cy = y;
    while (*str) {
        draw_char_p1(ren, x, cy, *str, c, scale);
        cy += 8 * scale;
        str++;
    }
}

static void draw_centered_str_p1(SDL_Renderer *ren, int x, int cy, const char *str, const Color &c, int scale = 1) {
    int len = (int)strlen(str);
    int y = cy - (len * 8 * scale) / 2;
    draw_str_p1(ren, x, y, str, c, scale);
}

/* ─── ROTATED TEXT: PLAYER 2 (FACING EAST LOOKING WEST) ─────────────── */

static void draw_char_p2(SDL_Renderer *ren, int x, int y, char ch, const Color &c, int scale = 1) {
    if (ch >= 'a' && ch <= 'z') ch -= 32;
    if (ch < 32 || ch > 90) return;
    const unsigned char *glyph = font8x8[ch - 32];
    set_color(ren, c);
    for (int row = 0; row < 8; row++) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                int px = x + row * scale;
                int py = y - col * scale;
                if (scale == 1) {
                    SDL_RenderDrawPoint(ren, px, py);
                } else {
                    SDL_Rect rc = {px, py, scale, scale};
                    SDL_RenderFillRect(ren, &rc);
                }
            }
        }
    }
}

static void draw_str_p2(SDL_Renderer *ren, int x, int y, const char *str, const Color &c, int scale = 1) {
    int cy = y;
    while (*str) {
        draw_char_p2(ren, x, cy, *str, c, scale);
        cy -= 8 * scale;
        str++;
    }
}

static void draw_centered_str_p2(SDL_Renderer *ren, int x, int cy, const char *str, const Color &c, int scale = 1) {
    int len = (int)strlen(str);
    int y = cy + (len * 8 * scale) / 2 - scale;
    draw_str_p2(ren, x, y, str, c, scale);
}

static void draw_circle(SDL_Renderer *ren, int cx, int cy, int radius, const Color &c, bool fill = true) {
    set_color(ren, c);
    if (!fill) {
        for (int a = 0; a < 360; a += 4) {
            float rad = a * 0.0174533f;
            int px = cx + (int)(radius * cosf(rad));
            int py = cy + (int)(radius * sinf(rad));
            SDL_RenderDrawPoint(ren, px, py);
        }
        return;
    }
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrtf((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

/* ─── AUDIO ENGINE (SYNTHESIZER) ─────────────────────────────────────── */

struct SynthVoice {
    int active;
    float freq;
    int samples_left;
    int phase;
    int type; // 0=square, 1=noise
};

#define MAX_VOICES 4
static SynthVoice voices[MAX_VOICES];

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    Sint16 *buf = (Sint16*)stream;
    int samples = len / 2;
    for (int i = 0; i < samples; i++) {
        Sint32 mixed = 0;
        for (int v = 0; v < MAX_VOICES; v++) {
            if (!voices[v].active) continue;
            if (voices[v].type == 0) {
                int period = (int)(22050.0f / voices[v].freq);
                if (period <= 0) period = 1;
                Sint16 val = ((voices[v].phase % period) < (period / 2)) ? 4000 : -4000;
                mixed += val;
                voices[v].phase++;
            } else {
                mixed += (rand() % 8000) - 4000;
            }
            voices[v].samples_left--;
            if (voices[v].samples_left <= 0) voices[v].active = 0;
        }
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;
        buf[i] = (Sint16)mixed;
    }
}

static void play_tone(float freq, int ms, int type = 0) {
    for (int v = 0; v < MAX_VOICES; v++) {
        if (!voices[v].active) {
            voices[v].freq = freq;
            voices[v].samples_left = (int)(22050.0f * ms / 1000.0f);
            voices[v].phase = 0;
            voices[v].type = type;
            voices[v].active = 1;
            return;
        }
    }
}

/* ─── ENUMS & GAME STATE ─────────────────────────────────────────────── */

enum MinigameType {
    GAME_PINGPONG = 0,
    GAME_AIRHOCKEY,
    GAME_SPINNERS,
    GAME_TANKS,
    GAME_COUNT
};

enum GameMode {
    MODE_1P_VS_AI = 0,
    MODE_2P_LOCAL = 1
};

enum AppState {
    STATE_MENU = 0,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
};

/* ─── HARDWARE INPUT INTERFACE ───────────────────────────────────────── */

struct PlayerInput {
    bool up, down, left, right;
    bool action; // L or R trigger
};

struct GamePadState {
    PlayerInput p1; // Left side of console
    PlayerInput p2; // Right side of console
    bool start_pressed;
    bool select_pressed;
};

static SceCtrlData pad_curr, pad_prev;

static void read_inputs(GamePadState &out, GameMode mode) {
    pad_prev = pad_curr;
    sceCtrlPeekBufferPositive(&pad_curr, 1);

    if (mode == MODE_2P_LOCAL) {
        /*
         * 2-Player Split-Console Layout:
         * Top of PSP is North.
         * Player 1 sits West looking East:
         *   UP    = D-Pad Right (into screen)
         *   DOWN  = D-Pad Left  (away from screen)
         *   LEFT  = D-Pad Up    (North)
         *   RIGHT = D-Pad Down  (South)
         *   ACTION= L Trigger
         *
         * Player 2 sits East looking West:
         *   UP    = Square   (into screen)
         *   DOWN  = Circle   (away from screen)
         *   LEFT  = Cross    (South)
         *   RIGHT = Triangle (North)
         *   ACTION= R Trigger
         */
        out.p1.up    = (pad_curr.Buttons & PSP_CTRL_RIGHT) || (pad_curr.Lx > 185);
        out.p1.down  = (pad_curr.Buttons & PSP_CTRL_LEFT)  || (pad_curr.Lx < 70);
        out.p1.left  = (pad_curr.Buttons & PSP_CTRL_UP)    || (pad_curr.Ly < 70);
        out.p1.right = (pad_curr.Buttons & PSP_CTRL_DOWN)  || (pad_curr.Ly > 185);
        out.p1.action = (pad_curr.Buttons & PSP_CTRL_LTRIGGER) != 0;

        out.p2.up    = (pad_curr.Buttons & PSP_CTRL_SQUARE)   != 0;
        out.p2.down  = (pad_curr.Buttons & PSP_CTRL_CIRCLE)   != 0;
        out.p2.left  = (pad_curr.Buttons & PSP_CTRL_CROSS)    != 0;
        out.p2.right = (pad_curr.Buttons & PSP_CTRL_TRIANGLE) != 0;
        out.p2.action = (pad_curr.Buttons & PSP_CTRL_RTRIGGER) != 0;
    } else {
        /*
         * 1-Player Standard Handheld Layout:
         * Standard D-Pad + Analog Nub + Cross / L Trigger
         */
        out.p1.up    = (pad_curr.Buttons & PSP_CTRL_UP)    || (pad_curr.Ly < 70);
        out.p1.down  = (pad_curr.Buttons & PSP_CTRL_DOWN)  || (pad_curr.Ly > 185);
        out.p1.left  = (pad_curr.Buttons & PSP_CTRL_LEFT)  || (pad_curr.Lx < 70);
        out.p1.right = (pad_curr.Buttons & PSP_CTRL_RIGHT) || (pad_curr.Lx > 185);
        out.p1.action = ((pad_curr.Buttons & PSP_CTRL_LTRIGGER) || (pad_curr.Buttons & PSP_CTRL_CROSS)) != 0;

        out.p2.up    = (pad_curr.Buttons & PSP_CTRL_TRIANGLE) != 0;
        out.p2.down  = (pad_curr.Buttons & PSP_CTRL_CROSS)    != 0;
        out.p2.left  = (pad_curr.Buttons & PSP_CTRL_SQUARE)   != 0;
        out.p2.right = (pad_curr.Buttons & PSP_CTRL_CIRCLE)   != 0;
        out.p2.action = (pad_curr.Buttons & PSP_CTRL_RTRIGGER) != 0;
    }

    out.start_pressed  = (pad_curr.Buttons & PSP_CTRL_START)  && !(pad_prev.Buttons & PSP_CTRL_START);
    out.select_pressed = (pad_curr.Buttons & PSP_CTRL_SELECT) && !(pad_prev.Buttons & PSP_CTRL_SELECT);
}

/* ─── 1. PING PONG MINIGAME ─────────────────────────────────────────── */

struct PingPong {
    float p1_y, p2_y;
    float ball_x, ball_y;
    float ball_vx, ball_vy;
    int score1, score2;
    int target_score;
    float paddle_h, paddle_w;

    void reset(bool reset_scores = true) {
        if (reset_scores) { score1 = 0; score2 = 0; }
        p1_y = SCR_H / 2.0f;
        p2_y = SCR_H / 2.0f;
        ball_x = SCR_W / 2.0f;
        ball_y = SCR_H / 2.0f;
        ball_vx = (rand() % 2 == 0 ? 3.5f : -3.5f);
        ball_vy = ((rand() % 100) - 50) / 25.0f;
        paddle_h = 50.0f;
        paddle_w = 10.0f;
        target_score = 5;
    }

    void update(const GamePadState &pad, GameMode mode, int &winner) {
        float speed = 4.5f;

        // Player 1 controls (Left)
        if (mode == MODE_2P_LOCAL) {
            if (pad.p1.left || pad.p1.up)    p1_y -= speed;
            if (pad.p1.right || pad.p1.down) p1_y += speed;
        } else {
            if (pad.p1.up)   p1_y -= speed;
            if (pad.p1.down) p1_y += speed;
        }
        if (p1_y < 35.0f) p1_y = 35.0f;
        if (p1_y > SCR_H - 35.0f) p1_y = SCR_H - 35.0f;

        // Player 2 / AI controls (Right)
        if (mode == MODE_1P_VS_AI) {
            float target_y = ball_y;
            float diff = target_y - p2_y;
            float ai_speed = (ball_vx > 0) ? 3.6f : 1.8f;
            if (fabsf(diff) > 8.0f) {
                p2_y += (diff > 0 ? ai_speed : -ai_speed);
            }
        } else {
            if (pad.p2.right || pad.p2.up)   p2_y -= speed;
            if (pad.p2.left || pad.p2.down)  p2_y += speed;
        }
        if (p2_y < 35.0f) p2_y = 35.0f;
        if (p2_y > SCR_H - 35.0f) p2_y = SCR_H - 35.0f;

        // Ball movement
        ball_x += ball_vx;
        ball_y += ball_vy;

        // Top / Bottom wall bounce
        if (ball_y <= 24.0f) { ball_y = 24.0f; ball_vy = -ball_vy; play_tone(320.0f, 30); }
        if (ball_y >= SCR_H - 12.0f) { ball_y = SCR_H - 12.0f; ball_vy = -ball_vy; play_tone(320.0f, 30); }

        // Paddle 1 Collision
        float p1_x = 24.0f;
        if (ball_x - 6.0f <= p1_x + paddle_w && ball_x >= p1_x && ball_vx < 0) {
            if (ball_y >= p1_y - paddle_h / 2.0f && ball_y <= p1_y + paddle_h / 2.0f) {
                ball_vx = -ball_vx * 1.08f;
                float hit_offset = (ball_y - p1_y) / (paddle_h / 2.0f);
                ball_vy = hit_offset * 4.2f;
                play_tone(480.0f, 40);
            }
        }

        // Paddle 2 Collision
        float p2_x = SCR_W - 34.0f;
        if (ball_x + 6.0f >= p2_x && ball_x <= p2_x + paddle_w && ball_vx > 0) {
            if (ball_y >= p2_y - paddle_h / 2.0f && ball_y <= p2_y + paddle_h / 2.0f) {
                ball_vx = -ball_vx * 1.08f;
                float hit_offset = (ball_y - p2_y) / (paddle_h / 2.0f);
                ball_vy = hit_offset * 4.2f;
                play_tone(520.0f, 40);
            }
        }

        // Score Goal Check
        if (ball_x < 0) {
            score2++;
            play_tone(180.0f, 150);
            if (score2 >= target_score) winner = 2;
            else reset(false);
        } else if (ball_x > SCR_W) {
            score1++;
            play_tone(640.0f, 150);
            if (score1 >= target_score) winner = 1;
            else reset(false);
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Center dividing dotted net
        set_color(ren, COL_BORDER);
        for (int y = 20; y < SCR_H - 10; y += 12) {
            SDL_Rect net = {SCR_W / 2 - 1, y, 2, 6};
            SDL_RenderFillRect(ren, &net);
        }

        // Left Paddle (Blue)
        set_color(ren, COL_BLUE);
        SDL_Rect rc1 = {24, (int)(p1_y - paddle_h / 2.0f), (int)paddle_w, (int)paddle_h};
        SDL_RenderFillRect(ren, &rc1);

        // Right Paddle (Red)
        set_color(ren, COL_RED);
        SDL_Rect rc2 = {SCR_W - 34, (int)(p2_y - paddle_h / 2.0f), (int)paddle_w, (int)paddle_h};
        SDL_RenderFillRect(ren, &rc2);

        // Ball
        draw_circle(ren, (int)ball_x, (int)ball_y, 5, COL_WHITE);

        // Scores
        char sbuf1[16], sbuf2[16];
        snprintf(sbuf1, sizeof(sbuf1), "%d", score1);
        snprintf(sbuf2, sizeof(sbuf2), "%d", score2);

        if (mode == MODE_2P_LOCAL) {
            // Rotated UI facing each player's controls
            draw_centered_str_p1(ren, 45, SCR_H / 2, sbuf1, COL_BLUE, 3);
            draw_centered_str_p1(ren, 18, SCR_H / 2, "PLAYER 1", COL_DIM, 1);

            draw_centered_str_p2(ren, SCR_W - 66, SCR_H / 2, sbuf2, COL_RED, 3);
            draw_centered_str_p2(ren, SCR_W - 22, SCR_H / 2, "PLAYER 2", COL_DIM, 1);
        } else {
            // Standard horizontal HUD at top
            draw_str(ren, SCR_W / 2 - 50, 24, sbuf1, COL_BLUE, 3);
            draw_str(ren, SCR_W / 2 + 32, 24, sbuf2, COL_RED, 3);
        }
    }
};

/* ─── 2. AIR HOCKEY MINIGAME ─────────────────────────────────────────── */

struct AirHockey {
    float p1_x, p1_y;
    float p2_x, p2_y;
    float puck_x, puck_y;
    float puck_vx, puck_vy;
    int score1, score2;
    int target_score;
    float mallet_r;
    float puck_r;

    void reset(bool reset_scores = true) {
        if (reset_scores) { score1 = 0; score2 = 0; }
        p1_x = 70.0f;
        p1_y = SCR_H / 2.0f;
        p2_x = SCR_W - 70.0f;
        p2_y = SCR_H / 2.0f;
        puck_x = SCR_W / 2.0f;
        puck_y = SCR_H / 2.0f;
        puck_vx = (rand() % 2 == 0 ? 2.5f : -2.5f);
        puck_vy = ((rand() % 100) - 50) / 30.0f;
        mallet_r = 16.0f;
        puck_r = 9.0f;
        target_score = 5;
    }

    void update(const GamePadState &pad, GameMode mode, int &winner) {
        float speed = 3.8f;

        // P1 2D Control (Restricted to Left Half: x <= 225)
        if (mode == MODE_2P_LOCAL) {
            if (pad.p1.up)    p1_x += speed; // Forward towards opponent
            if (pad.p1.down)  p1_x -= speed; // Retreat to goal
            if (pad.p1.left)  p1_y -= speed; // Left (North)
            if (pad.p1.right) p1_y += speed; // Right (South)
        } else {
            if (pad.p1.up)    p1_y -= speed;
            if (pad.p1.down)  p1_y += speed;
            if (pad.p1.left)  p1_x -= speed;
            if (pad.p1.right) p1_x += speed;
        }
        if (p1_x < 36.0f)  p1_x = 36.0f;
        if (p1_x > 225.0f) p1_x = 225.0f;
        if (p1_y < 40.0f)  p1_y = 40.0f;
        if (p1_y > SCR_H - 40.0f) p1_y = SCR_H - 40.0f;

        // P2 / AI 2D Control (Restricted to Right Half: x >= 255)
        if (mode == MODE_1P_VS_AI) {
            float dest_x = SCR_W - 65.0f;
            float dest_y = puck_y;
            if (puck_x > SCR_W / 2.0f) {
                dest_x = puck_x; // Attack puck
            }
            float dx = dest_x - p2_x;
            float dy = dest_y - p2_y;
            p2_x += dx * 0.08f;
            p2_y += dy * 0.08f;
        } else {
            if (pad.p2.up)    p2_x -= speed; // Forward towards opponent (-X)
            if (pad.p2.down)  p2_x += speed; // Retreat (+X)
            if (pad.p2.left)  p2_y += speed; // Left (+Y)
            if (pad.p2.right) p2_y -= speed; // Right (-Y)
        }
        if (p2_x < 255.0f)        p2_x = 255.0f;
        if (p2_x > SCR_W - 36.0f) p2_x = SCR_W - 36.0f;
        if (p2_y < 40.0f)         p2_y = 40.0f;
        if (p2_y > SCR_H - 40.0f) p2_y = SCR_H - 40.0f;

        // Puck Physics
        puck_x += puck_vx;
        puck_y += puck_vy;
        puck_vx *= 0.992f;
        puck_vy *= 0.992f;

        // Top and Bottom table rebounds
        if (puck_y - puck_r <= 22.0f) {
            puck_y = 22.0f + puck_r;
            puck_vy = -puck_vy;
            play_tone(340.0f, 25);
        }
        if (puck_y + puck_r >= SCR_H - 22.0f) {
            puck_y = SCR_H - 22.0f - puck_r;
            puck_vy = -puck_vy;
            play_tone(340.0f, 25);
        }

        // Left and Right goal zones
        bool in_goal_y = (puck_y >= 96.0f && puck_y <= 176.0f);

        if (puck_x - puck_r <= 20.0f) {
            if (in_goal_y) {
                score2++;
                play_tone(180.0f, 150);
                if (score2 >= target_score) winner = 2;
                else reset(false);
                return;
            } else {
                puck_x = 20.0f + puck_r;
                puck_vx = -puck_vx;
                play_tone(340.0f, 25);
            }
        }

        if (puck_x + puck_r >= SCR_W - 20.0f) {
            if (in_goal_y) {
                score1++;
                play_tone(640.0f, 150);
                if (score1 >= target_score) winner = 1;
                else reset(false);
                return;
            } else {
                puck_x = SCR_W - 20.0f - puck_r;
                puck_vx = -puck_vx;
                play_tone(340.0f, 25);
            }
        }

        // Collision with P1 Mallet
        float d1x = puck_x - p1_x;
        float d1y = puck_y - p1_y;
        float dist1 = sqrtf(d1x * d1x + d1y * d1y);
        if (dist1 < mallet_r + puck_r && dist1 > 0.001f) {
            float nx = d1x / dist1;
            float ny = d1y / dist1;
            puck_x = p1_x + nx * (mallet_r + puck_r);
            puck_y = p1_y + ny * (mallet_r + puck_r);
            float impulse = pad.p1.action ? 7.2f : 5.0f;
            puck_vx = nx * impulse;
            puck_vy = ny * impulse;
            play_tone(pad.p1.action ? 680.0f : 450.0f, 35);
        }

        // Collision with P2 Mallet
        float d2x = puck_x - p2_x;
        float d2y = puck_y - p2_y;
        float dist2 = sqrtf(d2x * d2x + d2y * d2y);
        if (dist2 < mallet_r + puck_r && dist2 > 0.001f) {
            float nx = d2x / dist2;
            float ny = d2y / dist2;
            puck_x = p2_x + nx * (mallet_r + puck_r);
            puck_y = p2_y + ny * (mallet_r + puck_r);
            float impulse = pad.p2.action ? 7.2f : 5.0f;
            puck_vx = nx * impulse;
            puck_vy = ny * impulse;
            play_tone(pad.p2.action ? 720.0f : 480.0f, 35);
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Table Rink Outer Boundary
        SDL_Rect rink = {20, 22, SCR_W - 40, SCR_H - 44};
        set_color(ren, COL_CARD);
        SDL_RenderFillRect(ren, &rink);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &rink);

        // Center line & circle
        SDL_RenderDrawLine(ren, SCR_W / 2, 22, SCR_W / 2, SCR_H - 22);
        draw_circle(ren, SCR_W / 2, SCR_H / 2, 35, COL_BORDER, false);

        // Left Goal Box & Right Goal Box
        SDL_Rect goal1 = {16, 96, 6, 80};
        set_color(ren, COL_RED);
        SDL_RenderFillRect(ren, &goal1);
        SDL_Rect goal2 = {SCR_W - 22, 96, 6, 80};
        set_color(ren, COL_BLUE);
        SDL_RenderFillRect(ren, &goal2);

        // P1 Mallet (Blue)
        draw_circle(ren, (int)p1_x, (int)p1_y, (int)mallet_r, COL_BLUE);
        draw_circle(ren, (int)p1_x, (int)p1_y, 7, COL_WHITE);

        // P2 Mallet (Red)
        draw_circle(ren, (int)p2_x, (int)p2_y, (int)mallet_r, COL_RED);
        draw_circle(ren, (int)p2_x, (int)p2_y, 7, COL_WHITE);

        // Puck
        draw_circle(ren, (int)puck_x, (int)puck_y, (int)puck_r, COL_WHITE);
        draw_circle(ren, (int)puck_x, (int)puck_y, 5, Color(20, 24, 40));

        // Score display
        char sbuf1[16], sbuf2[16];
        snprintf(sbuf1, sizeof(sbuf1), "%d", score1);
        snprintf(sbuf2, sizeof(sbuf2), "%d", score2);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 40, SCR_H / 2, sbuf1, COL_BLUE, 3);
            draw_centered_str_p1(ren, 22, SCR_H / 2, "PLAYER 1", COL_DIM, 1);

            draw_centered_str_p2(ren, SCR_W - 64, SCR_H / 2, sbuf2, COL_RED, 3);
            draw_centered_str_p2(ren, SCR_W - 28, SCR_H / 2, "PLAYER 2", COL_DIM, 1);
        } else {
            draw_str(ren, SCR_W / 2 - 50, 8, sbuf1, COL_BLUE, 2);
            draw_str(ren, SCR_W / 2 + 32, 8, sbuf2, COL_RED, 2);
        }
    }
};

/* ─── 3. SPINNER WAR (BEYBLADE SUMO) ─────────────────────────────────── */

struct SpinnerWar {
    float p1_x, p1_y, p1_vx, p1_vy, p1_rot;
    float p2_x, p2_y, p2_vx, p2_vy, p2_rot;
    int score1, score2;
    int target_score;
    float arena_cx, arena_cy, arena_r;
    float spinner_r;
    float p1_boost, p2_boost;

    void reset(bool reset_scores = true) {
        if (reset_scores) { score1 = 0; score2 = 0; }
        arena_cx = SCR_W / 2.0f;
        arena_cy = SCR_H / 2.0f + 6.0f;
        arena_r = 95.0f;
        spinner_r = 15.0f;
        target_score = 3;

        p1_x = arena_cx - 50.0f;
        p1_y = arena_cy;
        p1_vx = 0; p1_vy = 0; p1_rot = 0; p1_boost = 100.0f;

        p2_x = arena_cx + 50.0f;
        p2_y = arena_cy;
        p2_vx = 0; p2_vy = 0; p2_rot = 0; p2_boost = 100.0f;
    }

    void update(const GamePadState &pad, GameMode mode, int &winner) {
        float accel = 0.35f;

        // P1 Controls
        if (mode == MODE_2P_LOCAL) {
            if (pad.p1.up)    p1_vx += accel; // Forward towards opponent
            if (pad.p1.down)  p1_vx -= accel; // Retreat
            if (pad.p1.left)  p1_vy -= accel; // Left (North)
            if (pad.p1.right) p1_vy += accel; // Right (South)
        } else {
            if (pad.p1.up)    p1_vy -= accel;
            if (pad.p1.down)  p1_vy += accel;
            if (pad.p1.left)  p1_vx -= accel;
            if (pad.p1.right) p1_vx += accel;
        }
        if (pad.p1.action && p1_boost > 20.0f) {
            p1_vx *= 1.45f; p1_vy *= 1.45f; p1_boost -= 4.0f;
            play_tone(550.0f, 20);
        } else if (p1_boost < 100.0f) {
            p1_boost += 0.5f;
        }

        // P2 / AI Controls
        if (mode == MODE_1P_VS_AI) {
            float dx = p1_x - p2_x;
            float dy = p1_y - p2_y;
            float d = sqrtf(dx * dx + dy * dy);
            if (d > 0.01f) {
                p2_vx += (dx / d) * accel;
                p2_vy += (dy / d) * accel;
            }
            if (d < 45.0f && p2_boost > 30.0f) {
                p2_vx *= 1.4f; p2_vy *= 1.4f; p2_boost -= 4.0f;
            } else if (p2_boost < 100.0f) {
                p2_boost += 0.5f;
            }
        } else {
            if (pad.p2.up)    p2_vx -= accel; // Forward towards opponent (-X)
            if (pad.p2.down)  p2_vx += accel; // Retreat (+X)
            if (pad.p2.left)  p2_vy += accel; // Left (+Y)
            if (pad.p2.right) p2_vy -= accel; // Right (-Y)
            if (pad.p2.action && p2_boost > 20.0f) {
                p2_vx *= 1.45f; p2_vy *= 1.45f; p2_boost -= 4.0f;
                play_tone(600.0f, 20);
            } else if (p2_boost < 100.0f) {
                p2_boost += 0.5f;
            }
        }

        // Positions & Damping
        p1_x += p1_vx; p1_y += p1_vy;
        p2_x += p2_vx; p2_y += p2_vy;
        p1_vx *= 0.94f; p1_vy *= 0.94f;
        p2_vx *= 0.94f; p2_vy *= 0.94f;
        p1_rot += 18.0f;
        p2_rot -= 18.0f;

        // Mutual Spinner Collision
        float dx = p2_x - p1_x;
        float dy = p2_y - p1_y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < spinner_r * 2.0f && dist > 0.001f) {
            float nx = dx / dist;
            float ny = dy / dist;
            float overlap = (spinner_r * 2.0f) - dist;
            p1_x -= nx * overlap * 0.5f;
            p1_y -= ny * overlap * 0.5f;
            p2_x += nx * overlap * 0.5f;
            p2_y += ny * overlap * 0.5f;

            float repulse = 6.2f;
            p1_vx = -nx * repulse;
            p1_vy = -ny * repulse;
            p2_vx = nx * repulse;
            p2_vy = ny * repulse;
            play_tone(240.0f, 50, 1);
        }

        // Ring Out / Fall Check
        float d1 = sqrtf((p1_x - arena_cx)*(p1_x - arena_cx) + (p1_y - arena_cy)*(p1_y - arena_cy));
        if (d1 > arena_r) {
            score2++;
            play_tone(150.0f, 200);
            if (score2 >= target_score) winner = 2;
            else reset(false);
            return;
        }

        float d2 = sqrtf((p2_x - arena_cx)*(p2_x - arena_cx) + (p2_y - arena_cy)*(p2_y - arena_cy));
        if (d2 > arena_r) {
            score1++;
            play_tone(600.0f, 200);
            if (score1 >= target_score) winner = 1;
            else reset(false);
            return;
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Void backdrop with circular floating ring
        draw_circle(ren, (int)arena_cx, (int)arena_cy, (int)arena_r + 4, Color(30, 40, 70));
        draw_circle(ren, (int)arena_cx, (int)arena_cy, (int)arena_r, Color(20, 26, 50));
        draw_circle(ren, (int)arena_cx, (int)arena_cy, (int)arena_r - 18, Color(25, 34, 65), false);

        // Spinner 1 (Blue with spinning blades)
        draw_circle(ren, (int)p1_x, (int)p1_y, (int)spinner_r, COL_BLUE);
        for (int i = 0; i < 3; i++) {
            float ang = (p1_rot + i * 120.0f) * 0.0174533f;
            int bx = (int)(p1_x + cosf(ang) * (spinner_r + 4.0f));
            int by = (int)(p1_y + sinf(ang) * (spinner_r + 4.0f));
            set_color(ren, COL_WHITE);
            SDL_RenderDrawLine(ren, (int)p1_x, (int)p1_y, bx, by);
        }

        // Spinner 2 (Red with spinning blades)
        draw_circle(ren, (int)p2_x, (int)p2_y, (int)spinner_r, COL_RED);
        for (int i = 0; i < 3; i++) {
            float ang = (p2_rot + i * 120.0f) * 0.0174533f;
            int bx = (int)(p2_x + cosf(ang) * (spinner_r + 4.0f));
            int by = (int)(p2_y + sinf(ang) * (spinner_r + 4.0f));
            set_color(ren, COL_WHITE);
            SDL_RenderDrawLine(ren, (int)p2_x, (int)p2_y, bx, by);
        }

        // Scores & Boost Gauges
        char sbuf1[16], sbuf2[16];
        snprintf(sbuf1, sizeof(sbuf1), "%d", score1);
        snprintf(sbuf2, sizeof(sbuf2), "%d", score2);

        if (mode == MODE_2P_LOCAL) {
            // Left margin UI (facing P1)
            draw_centered_str_p1(ren, 40, SCR_H / 2, sbuf1, COL_BLUE, 3);
            draw_centered_str_p1(ren, 18, SCR_H / 2, "BOOST: L", COL_BLUE, 1);
            SDL_Rect b1 = {62, (int)(SCR_H / 2 - 25), 4, (int)(p1_boost * 0.5f)};
            set_color(ren, COL_BLUE);
            SDL_RenderFillRect(ren, &b1);

            // Right margin UI (facing P2)
            draw_centered_str_p2(ren, SCR_W - 64, SCR_H / 2, sbuf2, COL_RED, 3);
            draw_centered_str_p2(ren, SCR_W - 24, SCR_H / 2, "BOOST: R", COL_RED, 1);
            SDL_Rect b2 = {SCR_W - 66, (int)(SCR_H / 2 - 25), 4, (int)(p2_boost * 0.5f)};
            set_color(ren, COL_RED);
            SDL_RenderFillRect(ren, &b2);
        } else {
            draw_str(ren, 40, 16, sbuf1, COL_BLUE, 3);
            draw_str(ren, SCR_W - 60, 16, sbuf2, COL_RED, 3);

            SDL_Rect b1 = {40, 46, (int)(p1_boost * 0.5f), 4};
            set_color(ren, COL_BLUE);
            SDL_RenderFillRect(ren, &b1);

            SDL_Rect b2 = {SCR_W - 90, 46, (int)(p2_boost * 0.5f), 4};
            set_color(ren, COL_RED);
            SDL_RenderFillRect(ren, &b2);
        }
    }
};

/* ─── 4. TANK BATTLE MINIGAME ────────────────────────────────────────── */

struct Bullet {
    float x, y, vx, vy;
    int bounces;
    int owner; // 1 or 2
    bool active;
};

struct TankBattle {
    float t1_x, t1_y, t1_angle;
    float t2_x, t2_y, t2_angle;
    int score1, score2;
    int target_score;
    int t1_reload, t2_reload;
    std::vector<Bullet> bullets;
    std::vector<SDL_Rect> obstacles;

    void reset(bool reset_scores = true) {
        if (reset_scores) { score1 = 0; score2 = 0; }
        t1_x = 55.0f;  t1_y = SCR_H / 2.0f; t1_angle = 0.0f;   t1_reload = 0;
        t2_x = SCR_W - 55.0f; t2_y = SCR_H / 2.0f; t2_angle = 3.14159f; t2_reload = 0;
        target_score = 5;
        bullets.clear();

        obstacles.clear();
        obstacles.push_back({130, 60, 20, 60});
        obstacles.push_back({130, SCR_H - 120, 20, 60});
        obstacles.push_back({SCR_W - 150, 60, 20, 60});
        obstacles.push_back({SCR_W - 150, SCR_H - 120, 20, 60});
        obstacles.push_back({SCR_W / 2 - 10, 40, 20, 50});
        obstacles.push_back({SCR_W / 2 - 10, SCR_H - 90, 20, 50});
    }

    bool check_wall_collision(float x, float y, float radius) {
        if (x - radius < 14 || x + radius > SCR_W - 14 || y - radius < 14 || y + radius > SCR_H - 14) return true;
        for (const auto &obs : obstacles) {
            if (x + radius >= obs.x && x - radius <= obs.x + obs.w &&
                y + radius >= obs.y && y - radius <= obs.y + obs.h) {
                return true;
            }
        }
        return false;
    }

    void update(const GamePadState &pad, GameMode mode, int &winner) {
        float speed = 2.4f;

        // P1 Tank Controls
        float prev1_x = t1_x, prev1_y = t1_y;
        if (pad.p1.left)  t1_angle -= 0.07f;
        if (pad.p1.right) t1_angle += 0.07f;
        if (pad.p1.up) {
            t1_x += cosf(t1_angle) * speed;
            t1_y += sinf(t1_angle) * speed;
        }
        if (pad.p1.down) {
            t1_x -= cosf(t1_angle) * speed * 0.6f;
            t1_y -= sinf(t1_angle) * speed * 0.6f;
        }
        if (check_wall_collision(t1_x, t1_y, 11.0f)) { t1_x = prev1_x; t1_y = prev1_y; }

        if (t1_reload > 0) t1_reload--;
        if (pad.p1.action && t1_reload == 0) {
            Bullet b;
            b.x = t1_x + cosf(t1_angle) * 14.0f;
            b.y = t1_y + sinf(t1_angle) * 14.0f;
            b.vx = cosf(t1_angle) * 4.8f;
            b.vy = sinf(t1_angle) * 4.8f;
            b.bounces = 0; b.owner = 1; b.active = true;
            bullets.push_back(b);
            t1_reload = 20;
            play_tone(600.0f, 35, 1);
        }

        // P2 Tank / AI Controls
        float prev2_x = t2_x, prev2_y = t2_y;
        if (mode == MODE_1P_VS_AI) {
            float dx = t1_x - t2_x;
            float dy = t1_y - t2_y;
            float target_ang = atan2f(dy, dx);
            float diff_ang = target_ang - t2_angle;
            while (diff_ang > 3.14159f) diff_ang -= 6.28318f;
            while (diff_ang < -3.14159f) diff_ang += 6.28318f;
            t2_angle += diff_ang * 0.06f;

            t2_x += cosf(t2_angle) * (speed * 0.75f);
            t2_y += sinf(t2_angle) * (speed * 0.75f);

            if (t2_reload == 0 && fabsf(diff_ang) < 0.25f) {
                Bullet b;
                b.x = t2_x + cosf(t2_angle) * 14.0f;
                b.y = t2_y + sinf(t2_angle) * 14.0f;
                b.vx = cosf(t2_angle) * 4.8f;
                b.vy = sinf(t2_angle) * 4.8f;
                b.bounces = 0; b.owner = 2; b.active = true;
                bullets.push_back(b);
                t2_reload = 26;
                play_tone(520.0f, 35, 1);
            }
        } else {
            if (pad.p2.left)  t2_angle -= 0.07f;
            if (pad.p2.right) t2_angle += 0.07f;
            if (pad.p2.up) {
                t2_x += cosf(t2_angle) * speed;
                t2_y += sinf(t2_angle) * speed;
            }
            if (pad.p2.down) {
                t2_x -= cosf(t2_angle) * speed * 0.6f;
                t2_y -= sinf(t2_angle) * speed * 0.6f;
            }
            if (pad.p2.action && t2_reload == 0) {
                Bullet b;
                b.x = t2_x + cosf(t2_angle) * 14.0f;
                b.y = t2_y + sinf(t2_angle) * 14.0f;
                b.vx = cosf(t2_angle) * 4.8f;
                b.vy = sinf(t2_angle) * 4.8f;
                b.bounces = 0; b.owner = 2; b.active = true;
                bullets.push_back(b);
                t2_reload = 20;
                play_tone(520.0f, 35, 1);
            }
        }
        if (check_wall_collision(t2_x, t2_y, 11.0f)) { t2_x = prev2_x; t2_y = prev2_y; }
        if (t2_reload > 0) t2_reload--;

        // Bullets Physics & Bounce
        for (auto &b : bullets) {
            if (!b.active) continue;
            b.x += b.vx;
            b.y += b.vy;

            // Wall bounce
            if (b.x <= 16 || b.x >= SCR_W - 16) {
                b.vx = -b.vx; b.bounces++;
                play_tone(400.0f, 15);
            }
            if (b.y <= 16 || b.y >= SCR_H - 16) {
                b.vy = -b.vy; b.bounces++;
                play_tone(400.0f, 15);
            }
            for (const auto &obs : obstacles) {
                if (b.x >= obs.x && b.x <= obs.x + obs.w && b.y >= obs.y && b.y <= obs.y + obs.h) {
                    b.vx = -b.vx; b.vy = -b.vy; b.bounces++;
                    play_tone(400.0f, 15);
                    break;
                }
            }

            if (b.bounces > 2) { b.active = false; continue; }

            // Hit Tank 1
            float d1 = sqrtf((b.x - t1_x)*(b.x - t1_x) + (b.y - t1_y)*(b.y - t1_y));
            if (d1 < 12.0f && (b.owner != 1 || b.bounces > 0)) {
                b.active = false;
                score2++;
                play_tone(150.0f, 180, 1);
                if (score2 >= target_score) winner = 2;
                else reset(false);
                return;
            }

            // Hit Tank 2
            float d2 = sqrtf((b.x - t2_x)*(b.x - t2_x) + (b.y - t2_y)*(b.y - t2_y));
            if (d2 < 12.0f && (b.owner != 2 || b.bounces > 0)) {
                b.active = false;
                score1++;
                play_tone(650.0f, 180, 1);
                if (score1 >= target_score) winner = 1;
                else reset(false);
                return;
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Outer arena wall
        SDL_Rect arena = {14, 14, SCR_W - 28, SCR_H - 28};
        set_color(ren, COL_CARD);
        SDL_RenderFillRect(ren, &arena);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &arena);

        // Barricades
        for (const auto &obs : obstacles) {
            set_color(ren, COL_BORDER);
            SDL_RenderFillRect(ren, &obs);
            set_color(ren, COL_DIM);
            SDL_RenderDrawRect(ren, &obs);
        }

        // Bullets
        for (const auto &b : bullets) {
            if (!b.active) continue;
            draw_circle(ren, (int)b.x, (int)b.y, 3, COL_GOLD);
        }

        // Tank 1 (Blue)
        draw_circle(ren, (int)t1_x, (int)t1_y, 10, COL_BLUE);
        set_color(ren, COL_WHITE);
        SDL_RenderDrawLine(ren, (int)t1_x, (int)t1_y, (int)(t1_x + cosf(t1_angle) * 14.0f), (int)(t1_y + sinf(t1_angle) * 14.0f));

        // Tank 2 (Red)
        draw_circle(ren, (int)t2_x, (int)t2_y, 10, COL_RED);
        set_color(ren, COL_WHITE);
        SDL_RenderDrawLine(ren, (int)t2_x, (int)t2_y, (int)(t2_x + cosf(t2_angle) * 14.0f), (int)(t2_y + sinf(t2_angle) * 14.0f));

        // Scores
        char sbuf1[16], sbuf2[16];
        snprintf(sbuf1, sizeof(sbuf1), "%d", score1);
        snprintf(sbuf2, sizeof(sbuf2), "%d", score2);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 38, 45, sbuf1, COL_BLUE, 2);
            draw_centered_str_p1(ren, 20, 45, "P1", COL_BLUE, 1);

            draw_centered_str_p2(ren, SCR_W - 54, 45, sbuf2, COL_RED, 2);
            draw_centered_str_p2(ren, SCR_W - 26, 45, "P2", COL_RED, 1);
        } else {
            draw_str(ren, 40, 20, sbuf1, COL_BLUE, 2);
            draw_str(ren, SCR_W - 55, 20, sbuf2, COL_RED, 2);
        }
    }
};

/* ─── APP INSTANCE & MENU ────────────────────────────────────────────── */

struct App {
    AppState state;
    MinigameType selected_game;
    GameMode mode;
    int menu_cursor;
    int winner; // 1 or 2

    PingPong pingpong;
    AirHockey airhockey;
    SpinnerWar spinners;
    TankBattle tanks;

    void init() {
        state = STATE_MENU;
        selected_game = GAME_PINGPONG;
        mode = MODE_1P_VS_AI;
        menu_cursor = 0;
        winner = 0;
    }

    void start_selected_game() {
        winner = 0;
        state = STATE_PLAYING;
        if (selected_game == GAME_PINGPONG)  pingpong.reset(true);
        if (selected_game == GAME_AIRHOCKEY) airhockey.reset(true);
        if (selected_game == GAME_SPINNERS)  spinners.reset(true);
        if (selected_game == GAME_TANKS)     tanks.reset(true);
    }

    void update(const GamePadState &pad) {
        if (state == STATE_MENU) {
            static int rep = 0;
            // Allow browsing using D-Pad Left/Right or Shape Buttons
            bool left_req  = pad.p1.left  || pad.p2.left  || ((pad_curr.Buttons & PSP_CTRL_LEFT) != 0);
            bool right_req = pad.p1.right || pad.p2.right || ((pad_curr.Buttons & PSP_CTRL_RIGHT) != 0);

            if (left_req) {
                if (rep == 0) {
                    menu_cursor = (menu_cursor + GAME_COUNT - 1) % GAME_COUNT;
                    selected_game = (MinigameType)menu_cursor;
                    play_tone(440.0f, 25);
                    rep = 12;
                } else rep--;
            } else if (right_req) {
                if (rep == 0) {
                    menu_cursor = (menu_cursor + 1) % GAME_COUNT;
                    selected_game = (MinigameType)menu_cursor;
                    play_tone(440.0f, 25);
                    rep = 12;
                } else rep--;
            } else {
                rep = 0;
            }

            // Mode Toggle via SELECT
            if (pad.select_pressed) {
                mode = (mode == MODE_1P_VS_AI) ? MODE_2P_LOCAL : MODE_1P_VS_AI;
                play_tone(580.0f, 30);
            }

            // Start Game via Action, Cross, Circle, or Start
            bool start_req = pad.p1.action || pad.p2.action || pad.start_pressed ||
                             ((pad_curr.Buttons & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE)) != 0);
            if (start_req) {
                play_tone(660.0f, 50);
                start_selected_game();
            }
        } else if (state == STATE_PLAYING) {
            if (pad.start_pressed) {
                state = STATE_PAUSED;
                play_tone(350.0f, 30);
                return;
            }
            if (pad.select_pressed) {
                mode = (mode == MODE_1P_VS_AI) ? MODE_2P_LOCAL : MODE_1P_VS_AI;
                play_tone(580.0f, 30);
            }

            if (selected_game == GAME_PINGPONG)  pingpong.update(pad, mode, winner);
            if (selected_game == GAME_AIRHOCKEY) airhockey.update(pad, mode, winner);
            if (selected_game == GAME_SPINNERS)  spinners.update(pad, mode, winner);
            if (selected_game == GAME_TANKS)     tanks.update(pad, mode, winner);

            if (winner != 0) {
                state = STATE_GAMEOVER;
                play_tone(winner == 1 ? 750.0f : 300.0f, 250);
            }
        } else if (state == STATE_PAUSED) {
            if (pad.start_pressed) {
                state = STATE_PLAYING;
                play_tone(450.0f, 30);
            } else if (pad.select_pressed || pad.p1.action || pad.p2.action) {
                state = STATE_MENU;
                play_tone(300.0f, 40);
            }
        } else if (state == STATE_GAMEOVER) {
            bool any_btn = pad.start_pressed || pad.p1.action || pad.p2.action ||
                           ((pad_curr.Buttons & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_SQUARE | PSP_CTRL_TRIANGLE)) != 0);
            if (any_btn) {
                state = STATE_MENU;
                play_tone(500.0f, 40);
            }
        }
    }

    void render(SDL_Renderer *ren) {
        if (state == STATE_MENU) {
            // Header Title
            draw_centered_str(ren, SCR_W / 2 + 1, 19, "2 PLAYER GAMES", COL_SHADOW, 3);
            draw_centered_str(ren, SCR_W / 2, 18, "2 PLAYER GAMES", COL_GOLD, 3);
            draw_centered_str(ren, SCR_W / 2, 46, "THE CHALLENGE - PSP EDITION", COL_DIM, 1);

            // Minigames Carousel Cards
            const char *games[GAME_COUNT] = {"PING PONG", "AIR HOCKEY", "SPINNER WAR", "TANK BATTLE"};
            const char *descs[GAME_COUNT] = {
                "CLASSIC PADDLE RALLY BATTLE",
                "2D AIR HOCKEY TABLE & SMASHES",
                "BEYBLADE SUMO RING KNOCKOUT",
                "TACTICAL BOUNCING SHELL COMBAT"
            };

            for (int i = 0; i < GAME_COUNT; i++) {
                int cx = 20 + i * 112;
                int cy = 88;
                int cw = 104;
                int ch = 76;

                if (i == menu_cursor) {
                    SDL_Rect glow = {cx - 2, cy - 2, cw + 4, ch + 4};
                    set_color(ren, COL_BLUE);
                    SDL_RenderFillRect(ren, &glow);
                }

                SDL_Rect card = {cx, cy, cw, ch};
                set_color(ren, COL_CARD);
                SDL_RenderFillRect(ren, &card);
                set_color(ren, i == menu_cursor ? COL_WHITE : COL_BORDER);
                SDL_RenderDrawRect(ren, &card);

                char num[8];
                snprintf(num, sizeof(num), "#%d", i + 1);
                draw_centered_str(ren, cx + cw / 2, cy + 10, num, COL_GOLD, 1);
                draw_centered_str(ren, cx + cw / 2, cy + 34, games[i], i == menu_cursor ? COL_WHITE : COL_DIM, 1);
            }

            draw_centered_str(ren, SCR_W / 2, 174, descs[menu_cursor], COL_WHITE, 1);

            // Mode Selector Pill
            int mode_y = 196;
            SDL_Rect mode_pill = {SCR_W / 2 - 135, mode_y, 270, 24};
            set_color(ren, COL_CARD);
            SDL_RenderFillRect(ren, &mode_pill);
            set_color(ren, COL_GOLD);
            SDL_RenderDrawRect(ren, &mode_pill);

            const char *mstr = (mode == MODE_1P_VS_AI) ? "< MODE: 1 PLAYER (VS AI) >" : "< MODE: 2 PLAYERS (SPLIT OPPS) >";
            draw_centered_str(ren, SCR_W / 2, mode_y + 8, mstr, COL_GOLD, 1);

            // Control scheme instructions per mode
            if (mode == MODE_1P_VS_AI) {
                draw_centered_str(ren, SCR_W / 2, 232, "P1 (LEFT): D-PAD + L / X  |  VS AI CPU", COL_DIM, 1);
            } else {
                draw_centered_str(ren, SCR_W / 2, 232, "P1 (WEST): DPAD RIGHT=UP + L  |  P2 (EAST): SQUARE=UP + R", COL_DIM, 1);
            }
            draw_centered_str(ren, SCR_W / 2, 248, "SELECT: TOGGLE MODE   |   START / X / L / R: BEGIN", COL_WHITE, 1);

        } else if (state == STATE_PLAYING) {
            if (selected_game == GAME_PINGPONG)  pingpong.render(ren, mode);
            if (selected_game == GAME_AIRHOCKEY) airhockey.render(ren, mode);
            if (selected_game == GAME_SPINNERS)  spinners.render(ren, mode);
            if (selected_game == GAME_TANKS)     tanks.render(ren, mode);

            if (mode == MODE_1P_VS_AI) {
                draw_str(ren, 16, 6, "P1 (BLUE)", COL_BLUE, 1);
                draw_centered_str(ren, SCR_W / 2, 6, "1P VS AI", COL_DIM, 1);
                draw_str(ren, SCR_W - 88, 6, "CPU (RED)", COL_RED, 1);
            } else {
                // In 2P mode, center dividing tick on top and bottom border
                set_color(ren, COL_BORDER);
                SDL_RenderDrawLine(ren, SCR_W / 2, 0, SCR_W / 2, 8);
                SDL_RenderDrawLine(ren, SCR_W / 2, SCR_H - 8, SCR_W / 2, SCR_H);
                draw_centered_str(ren, SCR_W / 2, 2, "2P", COL_DIM, 1);
            }

        } else if (state == STATE_PAUSED) {
            if (selected_game == GAME_PINGPONG)  pingpong.render(ren, mode);
            if (selected_game == GAME_AIRHOCKEY) airhockey.render(ren, mode);
            if (selected_game == GAME_SPINNERS)  spinners.render(ren, mode);
            if (selected_game == GAME_TANKS)     tanks.render(ren, mode);

            if (mode == MODE_2P_LOCAL) {
                // Dual rotated pause boxes facing each player
                SDL_Rect pop1 = {40, SCR_H / 2 - 40, 160, 80};
                set_color(ren, COL_CARD);
                SDL_RenderFillRect(ren, &pop1);
                set_color(ren, COL_GOLD);
                SDL_RenderDrawRect(ren, &pop1);
                draw_centered_str_p1(ren, 130, SCR_H / 2, "PAUSED", COL_GOLD, 2);
                draw_centered_str_p1(ren, 105, SCR_H / 2, "START: RESUME", COL_WHITE, 1);
                draw_centered_str_p1(ren, 85, SCR_H / 2, "SELECT: QUIT", COL_DIM, 1);

                SDL_Rect pop2 = {280, SCR_H / 2 - 40, 160, 80};
                set_color(ren, COL_CARD);
                SDL_RenderFillRect(ren, &pop2);
                set_color(ren, COL_GOLD);
                SDL_RenderDrawRect(ren, &pop2);
                draw_centered_str_p2(ren, SCR_W - 150, SCR_H / 2, "PAUSED", COL_GOLD, 2);
                draw_centered_str_p2(ren, SCR_W - 125, SCR_H / 2, "START: RESUME", COL_WHITE, 1);
                draw_centered_str_p2(ren, SCR_W - 105, SCR_H / 2, "SELECT: QUIT", COL_DIM, 1);
            } else {
                SDL_Rect pop = {SCR_W / 2 - 140, SCR_H / 2 - 50, 280, 100};
                set_color(ren, COL_CARD);
                SDL_RenderFillRect(ren, &pop);
                set_color(ren, COL_GOLD);
                SDL_RenderDrawRect(ren, &pop);

                draw_centered_str(ren, SCR_W / 2, SCR_H / 2 - 32, "PAUSED", COL_GOLD, 2);
                draw_centered_str(ren, SCR_W / 2, SCR_H / 2, "START: RESUME GAME", COL_WHITE, 1);
                draw_centered_str(ren, SCR_W / 2, SCR_H / 2 + 18, "SELECT / ACTION: QUIT TO MENU", COL_DIM, 1);
            }

        } else if (state == STATE_GAMEOVER) {
            if (mode == MODE_2P_LOCAL) {
                // Center dividing line between player zones
                set_color(ren, COL_BORDER);
                SDL_RenderDrawLine(ren, SCR_W / 2, 20, SCR_W / 2, SCR_H - 20);

                // Player 1 Result Box (facing West/P1)
                SDL_Rect pop1 = {30, SCR_H / 2 - 55, 180, 110};
                set_color(ren, COL_CARD);
                SDL_RenderFillRect(ren, &pop1);
                set_color(ren, winner == 1 ? COL_BLUE : COL_BORDER);
                SDL_RenderDrawRect(ren, &pop1);

                if (winner == 1) {
                    draw_centered_str_p1(ren, 130, SCR_H / 2, "YOU WIN!", COL_GOLD, 2);
                    draw_centered_str_p1(ren, 105, SCR_H / 2, "CHAMPION", COL_BLUE, 1);
                } else {
                    draw_centered_str_p1(ren, 130, SCR_H / 2, "YOU LOSE", COL_RED, 2);
                    draw_centered_str_p1(ren, 105, SCR_H / 2, "DEFEAT", COL_DIM, 1);
                }
                draw_centered_str_p1(ren, 70, SCR_H / 2, "PRESS L TO RETRY", COL_WHITE, 1);

                // Player 2 Result Box (facing East/P2)
                SDL_Rect pop2 = {270, SCR_H / 2 - 55, 180, 110};
                set_color(ren, COL_CARD);
                SDL_RenderFillRect(ren, &pop2);
                set_color(ren, winner == 2 ? COL_RED : COL_BORDER);
                SDL_RenderDrawRect(ren, &pop2);

                if (winner == 2) {
                    draw_centered_str_p2(ren, SCR_W - 150, SCR_H / 2, "YOU WIN!", COL_GOLD, 2);
                    draw_centered_str_p2(ren, SCR_W - 125, SCR_H / 2, "CHAMPION", COL_RED, 1);
                } else {
                    draw_centered_str_p2(ren, SCR_W - 150, SCR_H / 2, "YOU LOSE", COL_BLUE, 2);
                    draw_centered_str_p2(ren, SCR_W - 125, SCR_H / 2, "DEFEAT", COL_DIM, 1);
                }
                draw_centered_str_p2(ren, SCR_W - 90, SCR_H / 2, "PRESS R TO RETRY", COL_WHITE, 1);

            } else {
                SDL_Rect pop = {SCR_W / 2 - 160, SCR_H / 2 - 60, 320, 120};
                set_color(ren, COL_CARD);
                SDL_RenderFillRect(ren, &pop);
                set_color(ren, winner == 1 ? COL_BLUE : COL_RED);
                SDL_RenderDrawRect(ren, &pop);

                const char *wstr = (winner == 1) ? "PLAYER 1 (BLUE) WINS!" : "AI (RED) WINS!";
                Color wc = (winner == 1 ? COL_BLUE : COL_RED);

                draw_centered_str(ren, SCR_W / 2, SCR_H / 2 - 36, "MATCH OVER", COL_GOLD, 2);
                draw_centered_str(ren, SCR_W / 2, SCR_H / 2 - 6, wstr, wc, 2);
                draw_centered_str(ren, SCR_W / 2, SCR_H / 2 + 26, "PRESS ANY BUTTON TO CONTINUE", COL_WHITE, 1);
            }
        }
    }
};

/* ─── ENTRY POINT ────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    scePowerSetClockFrequency(333, 333, 166);
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        sceKernelExitGame();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "2 Player Games PSP",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCR_W, SCR_H, 0
    );
    if (!window) {
        SDL_Quit();
        sceKernelExitGame();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        sceKernelExitGame();
        return 1;
    }

    // Audio Setup
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 22050;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = audio_callback;
    if (SDL_OpenAudio(&want, &have) == 0) {
        SDL_PauseAudio(0);
    }

    srand(12345);
    App app;
    app.init();

    GamePadState pad;
    memset(&pad_curr, 0, sizeof(pad_curr));
    memset(&pad_prev, 0, sizeof(pad_prev));

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        read_inputs(pad, app.mode);
        app.update(pad);

        set_color(renderer, COL_BG);
        SDL_RenderClear(renderer);

        app.render(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    sceKernelExitGame();
    return 0;
}
