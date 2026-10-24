#pragma once
#include <SDL.h>
#include <vector>
#include <deque>
#include <cstdlib>
#include <cstdio>
#include <cmath>

/* ─── 13. SUMO (DOHYO RING OUT) ─────────────────────────────────────── */

struct SumoGame {
    float p1_x, p1_y, p1_vx, p1_vy;
    float p2_x, p2_y, p2_vx, p2_vy;
    float p1_shove_cd, p2_shove_cd;
    int p1_score, p2_score;
    int round_over_timer;
    int ai_delay;

    const float RING_CX = 240.0f;
    const float RING_CY = 136.0f;
    const float RING_R  = 86.0f;
    const float WRESTLER_R = 15.0f;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        p1_x = RING_CX - 40.0f; p1_y = RING_CY;
        p1_vx = 0.0f; p1_vy = 0.0f;
        p1_shove_cd = 0.0f;

        p2_x = RING_CX + 40.0f; p2_y = RING_CY;
        p2_vx = 0.0f; p2_vy = 0.0f;
        p2_shove_cd = 0.0f;

        round_over_timer = 0;
        ai_delay = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 3) match_winner = 1;
                else if (p2_score >= 3) match_winner = 2;
                else reset(false);
            }
            return;
        }

        if (p1_shove_cd > 0.0f) p1_shove_cd -= 1.0f;
        if (p2_shove_cd > 0.0f) p2_shove_cd -= 1.0f;

        // P1 Movement & Shove
        float speed = 1.6f;
        if (pad.p1.up)    p1_vy -= speed;
        if (pad.p1.down)  p1_vy += speed;
        if (pad.p1.left)  p1_vx -= speed;
        if (pad.p1.right) p1_vx += speed;

        if (pad.p1.action && p1_shove_cd <= 0.0f) {
            float dx = p2_x - p1_x, dy = p2_y - p1_y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 0.1f) {
                p1_vx += (dx / dist) * 7.5f;
                p1_vy += (dy / dist) * 7.5f;
                p1_shove_cd = 30.0f;
                play_tone(460.0f, 40);
            }
        }

        // P2 Movement & Shove (Human or AI)
        if (mode == MODE_2P_LOCAL) {
            if (pad.p2.up)    p2_vy -= speed;
            if (pad.p2.down)  p2_vy += speed;
            if (pad.p2.left)  p2_vx -= speed;
            if (pad.p2.right) p2_vx += speed;

            if (pad.p2.action && p2_shove_cd <= 0.0f) {
                float dx = p1_x - p2_x, dy = p1_y - p2_y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 0.1f) {
                    p2_vx += (dx / dist) * 7.5f;
                    p2_vy += (dy / dist) * 7.5f;
                    p2_shove_cd = 30.0f;
                    play_tone(400.0f, 40);
                }
            }
        } else {
            // AI logic: charge towards P1, keep away from ring edge
            float dx = p1_x - p2_x, dy = p1_y - p2_y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 0.1f) {
                p2_vx += (dx / dist) * 1.3f;
                p2_vy += (dy / dist) * 1.3f;
                if (dist < 45.0f && p2_shove_cd <= 0.0f && rand() % 15 == 0) {
                    p2_vx += (dx / dist) * 7.5f;
                    p2_vy += (dy / dist) * 7.5f;
                    p2_shove_cd = 35.0f;
                    play_tone(400.0f, 40);
                }
            }
        }

        // Friction
        p1_vx *= 0.82f; p1_vy *= 0.82f;
        p2_vx *= 0.82f; p2_vy *= 0.82f;

        p1_x += p1_vx; p1_y += p1_vy;
        p2_x += p2_vx; p2_y += p2_vy;

        // Collision between wrestlers
        float cdx = p2_x - p1_x, cdy = p2_y - p1_y;
        float cdist = sqrtf(cdx * cdx + cdy * cdy);
        if (cdist < WRESTLER_R * 2.0f && cdist > 0.1f) {
            float overlap = (WRESTLER_R * 2.0f - cdist) * 0.5f;
            float nx = cdx / cdist, ny = cdy / cdist;
            p1_x -= nx * overlap; p1_y -= ny * overlap;
            p2_x += nx * overlap; p2_y += ny * overlap;

            float rel_vx = p1_vx - p2_vx;
            float rel_vy = p1_vy - p2_vy;
            float imp = (rel_vx * nx + rel_vy * ny) * 1.4f;
            p1_vx -= nx * imp; p1_vy -= ny * imp;
            p2_vx += nx * imp; p2_vy += ny * imp;
            play_tone(240.0f + (rand() % 80), 30);
        }

        // Ring-out detection
        float d1 = sqrtf((p1_x - RING_CX) * (p1_x - RING_CX) + (p1_y - RING_CY) * (p1_y - RING_CY));
        float d2 = sqrtf((p2_x - RING_CX) * (p2_x - RING_CX) + (p2_y - RING_CY) * (p2_y - RING_CY));

        if (d1 > RING_R + WRESTLER_R * 0.5f) {
            p2_score++;
            round_over_timer = 50;
            play_tone(350.0f, 150);
        } else if (d2 > RING_R + WRESTLER_R * 0.5f) {
            p1_score++;
            round_over_timer = 50;
            play_tone(750.0f, 150);
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Dohyo base
        draw_circle(ren, (int)RING_CX, (int)RING_CY, (int)RING_R + 8, Color(120, 95, 60));
        draw_circle(ren, (int)RING_CX, (int)RING_CY, (int)RING_R, Color(190, 160, 110));
        draw_circle(ren, (int)RING_CX, (int)RING_CY, (int)RING_R, COL_GOLD, false);
        draw_circle(ren, (int)RING_CX, (int)RING_CY, (int)RING_R - 1, Color(160, 120, 50), false);

        // Center lines (shikiri-sen)
        set_color(ren, COL_WHITE);
        SDL_RenderDrawLine(ren, (int)RING_CX - 22, (int)RING_CY - 10, (int)RING_CX - 22, (int)RING_CY + 10);
        SDL_RenderDrawLine(ren, (int)RING_CX + 22, (int)RING_CY - 10, (int)RING_CX + 22, (int)RING_CY + 10);

        // P1 Wrestler (Blue)
        draw_circle(ren, (int)p1_x, (int)p1_y, (int)WRESTLER_R, COL_BLUE);
        draw_circle(ren, (int)p1_x, (int)p1_y, (int)WRESTLER_R, COL_WHITE, false);
        draw_circle(ren, (int)p1_x, (int)p1_y, 4, Color(10, 40, 90)); // Mawashi knot

        // P2 Wrestler (Red)
        draw_circle(ren, (int)p2_x, (int)p2_y, (int)WRESTLER_R, COL_RED);
        draw_circle(ren, (int)p2_x, (int)p2_y, (int)WRESTLER_R, COL_WHITE, false);
        draw_circle(ren, (int)p2_x, (int)p2_y, 4, Color(90, 10, 20));

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d  vs  P2: %d", p1_score, p2_score);
        draw_centered_str(ren, (int)RING_CX, 20, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "PLAYER 1", COL_BLUE, 1);
            draw_centered_str_p1(ren, 36, SCR_H / 2, "[L] SHOVE", COL_GOLD, 1);

            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "PLAYER 2", COL_RED, 1);
            draw_centered_str_p2(ren, SCR_W - 36, SCR_H / 2, "[R] SHOVE", COL_GOLD, 1);
        } else {
            draw_str(ren, 14, 40, "P1 (BLUE)", COL_BLUE, 1);
            draw_str(ren, 14, 60, "[L] SHOVE", COL_GOLD, 1);
            draw_str(ren, SCR_W - 90, 40, "CPU (RED)", COL_RED, 1);
        }
    }
};

/* ─── 14. SNAKE (TRON LIGHT CYCLES) ─────────────────────────────────── */

struct SnakeGame {
    struct Point { int x, y; };
    std::deque<Point> p1_body, p2_body;
    int p1_dir, p2_dir; // 0=Up, 1=Right, 2=Down, 3=Left
    int p1_next_dir, p2_next_dir;
    int p1_score, p2_score;
    int move_timer;
    int round_over_timer;
    const int COLS = 40;
    const int ROWS = 22;
    const int CELL_SZ = 10;
    const int BX = 40;
    const int BY = 26;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        p1_body.clear(); p2_body.clear();

        for (int i = 0; i < 6; i++) {
            p1_body.push_front({10 + i, 11});
            p2_body.push_front({29 - i, 11});
        }
        p1_dir = 1; p1_next_dir = 1; // Facing Right
        p2_dir = 3; p2_next_dir = 3; // Facing Left
        move_timer = 0;
        round_over_timer = 0;
    }

    bool is_hit(int x, int y, const std::deque<Point> &body, bool skip_head = false) {
        int start = skip_head ? 1 : 0;
        for (size_t i = start; i < body.size(); i++) {
            if (body[i].x == x && body[i].y == y) return true;
        }
        return false;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 3) match_winner = 1;
                else if (p2_score >= 3) match_winner = 2;
                else reset(false);
            }
            return;
        }

        // P1 Input
        if (pad.p1.up    && p1_dir != 2) p1_next_dir = 0;
        if (pad.p1.right && p1_dir != 3) p1_next_dir = 1;
        if (pad.p1.down  && p1_dir != 0) p1_next_dir = 2;
        if (pad.p1.left  && p1_dir != 1) p1_next_dir = 3;

        // P2 Input (Human or AI)
        if (mode == MODE_2P_LOCAL) {
            if (pad.p2.up    && p2_dir != 2) p2_next_dir = 0;
            if (pad.p2.right && p2_dir != 3) p2_next_dir = 1;
            if (pad.p2.down  && p2_dir != 0) p2_next_dir = 2;
            if (pad.p2.left  && p2_dir != 1) p2_next_dir = 3;
        } else {
            // AI lookahead: check if current direction will crash
            int dx[4] = {0, 1, 0, -1};
            int dy[4] = {-1, 0, 1, 0};
            int nx = p2_body.front().x + dx[p2_dir];
            int ny = p2_body.front().y + dy[p2_dir];

            bool will_crash = (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS ||
                               is_hit(nx, ny, p2_body) || is_hit(nx, ny, p1_body));
            if (will_crash) {
                // Try 90 degree turns
                int t1 = (p2_dir + 1) % 4;
                int t2 = (p2_dir + 3) % 4;
                int nx1 = p2_body.front().x + dx[t1], ny1 = p2_body.front().y + dy[t1];
                int nx2 = p2_body.front().x + dx[t2], ny2 = p2_body.front().y + dy[t2];
                bool safe1 = (nx1 >= 0 && nx1 < COLS && ny1 >= 0 && ny1 < ROWS &&
                              !is_hit(nx1, ny1, p2_body) && !is_hit(nx1, ny1, p1_body));
                bool safe2 = (nx2 >= 0 && nx2 < COLS && ny2 >= 0 && ny2 < ROWS &&
                              !is_hit(nx2, ny2, p2_body) && !is_hit(nx2, ny2, p1_body));
                if (safe1) p2_next_dir = t1;
                else if (safe2) p2_next_dir = t2;
            }
        }

        move_timer++;
        if (move_timer >= 6) {
            move_timer = 0;
            p1_dir = p1_next_dir;
            p2_dir = p2_next_dir;

            int dx[4] = {0, 1, 0, -1};
            int dy[4] = {-1, 0, 1, 0};

            Point p1_head = {p1_body.front().x + dx[p1_dir], p1_body.front().y + dy[p1_dir]};
            Point p2_head = {p2_body.front().x + dx[p2_dir], p2_body.front().y + dy[p2_dir]};

            bool p1_dead = (p1_head.x < 0 || p1_head.x >= COLS || p1_head.y < 0 || p1_head.y >= ROWS ||
                            is_hit(p1_head.x, p1_head.y, p1_body) || is_hit(p1_head.x, p1_head.y, p2_body));

            bool p2_dead = (p2_head.x < 0 || p2_head.x >= COLS || p2_head.y < 0 || p2_head.y >= ROWS ||
                            is_hit(p2_head.x, p2_head.y, p2_body) || is_hit(p2_head.x, p2_head.y, p1_body));

            if (p1_head.x == p2_head.x && p1_head.y == p2_head.y) {
                p1_dead = true; p2_dead = true; // Mutual head crash
            }

            if (p1_dead && p2_dead) {
                round_over_timer = 40;
                play_tone(220.0f, 100);
            } else if (p1_dead) {
                p2_score++;
                round_over_timer = 50;
                play_tone(350.0f, 150);
            } else if (p2_dead) {
                p1_score++;
                round_over_timer = 50;
                play_tone(750.0f, 150);
            } else {
                p1_body.push_front(p1_head);
                p2_body.push_front(p2_head);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        SDL_Rect arena = {BX - 3, BY - 3, COLS * CELL_SZ + 6, ROWS * CELL_SZ + 6};
        set_color(ren, Color(16, 22, 40));
        SDL_RenderFillRect(ren, &arena);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &arena);

        // Draw P1 Trail
        set_color(ren, COL_BLUE);
        for (const auto &pt : p1_body) {
            SDL_Rect r = {BX + pt.x * CELL_SZ + 1, BY + pt.y * CELL_SZ + 1, CELL_SZ - 2, CELL_SZ - 2};
            SDL_RenderFillRect(ren, &r);
        }
        if (!p1_body.empty()) {
            draw_circle(ren, BX + p1_body.front().x * CELL_SZ + CELL_SZ / 2,
                             BY + p1_body.front().y * CELL_SZ + CELL_SZ / 2, CELL_SZ / 2, COL_WHITE);
        }

        // Draw P2 Trail
        set_color(ren, COL_RED);
        for (const auto &pt : p2_body) {
            SDL_Rect r = {BX + pt.x * CELL_SZ + 1, BY + pt.y * CELL_SZ + 1, CELL_SZ - 2, CELL_SZ - 2};
            SDL_RenderFillRect(ren, &r);
        }
        if (!p2_body.empty()) {
            draw_circle(ren, BX + p2_body.front().x * CELL_SZ + CELL_SZ / 2,
                             BY + p2_body.front().y * CELL_SZ + CELL_SZ / 2, CELL_SZ / 2, COL_WHITE);
        }

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d  vs  P2: %d", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 10, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "PLAYER 1", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "PLAYER 2", COL_RED, 1);
        }
    }
};

/* ─── 15. RED HANDS (SLAP & DODGE) ──────────────────────────────────── */

struct RedHandsGame {
    int attacker; // 1 = P1 attacking, 2 = P2 attacking
    int p1_score, p2_score;
    float p1_hand_pos; // 0.0f (rest) to 60.0f (slapped)
    float p2_hand_pos;
    bool p1_dodging, p2_dodging;
    int dodge_timer_p1, dodge_timer_p2;
    int round_over_timer;
    int ai_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        attacker = 1;
        p1_hand_pos = 0.0f; p2_hand_pos = 0.0f;
        p1_dodging = false; p2_dodging = false;
        dodge_timer_p1 = 0; dodge_timer_p2 = 0;
        round_over_timer = 0;
        ai_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 5) match_winner = 1;
                else if (p2_score >= 5) match_winner = 2;
                else {
                    p1_hand_pos = 0.0f; p2_hand_pos = 0.0f;
                }
            }
            return;
        }

        if (dodge_timer_p1 > 0) {
            dodge_timer_p1--;
            if (dodge_timer_p1 == 0) p1_dodging = false;
        }
        if (dodge_timer_p2 > 0) {
            dodge_timer_p2--;
            if (dodge_timer_p2 == 0) p2_dodging = false;
        }

        // P1 Action
        static bool p1_prev = false;
        if (pad.p1.action && !p1_prev && round_over_timer == 0) {
            if (attacker == 1) { // Slap!
                p1_hand_pos = 70.0f;
                play_tone(420.0f, 30);
                if (!p2_dodging) { // HIT!
                    p1_score++;
                    round_over_timer = 40;
                    play_tone(750.0f, 120);
                } else { // DODGED!
                    attacker = 2; // Role swap
                    play_tone(240.0f, 40);
                }
            } else { // Dodge!
                p1_dodging = true;
                dodge_timer_p1 = 20;
            }
        }
        p1_prev = pad.p1.action;

        // P2 Action (Human or AI)
        if (mode == MODE_2P_LOCAL) {
            static bool p2_prev = false;
            if (pad.p2.action && !p2_prev && round_over_timer == 0) {
                if (attacker == 2) { // Slap!
                    p2_hand_pos = 70.0f;
                    play_tone(420.0f, 30);
                    if (!p1_dodging) { // HIT!
                        p2_score++;
                        round_over_timer = 40;
                        play_tone(750.0f, 120);
                    } else { // DODGED!
                        attacker = 1; // Role swap
                        play_tone(240.0f, 40);
                    }
                } else { // Dodge!
                    p2_dodging = true;
                    dodge_timer_p2 = 20;
                }
            }
            p2_prev = pad.p2.action;
        } else {
            // AI Action
            ai_timer++;
            if (attacker == 2) {
                // AI Attacking: surprise slap
                if (ai_timer > 35 + rand() % 40) {
                    ai_timer = 0;
                    p2_hand_pos = 70.0f;
                    play_tone(420.0f, 30);
                    if (!p1_dodging) {
                        p2_score++;
                        round_over_timer = 40;
                        play_tone(750.0f, 120);
                    } else {
                        attacker = 1;
                        play_tone(240.0f, 40);
                    }
                }
            } else {
                // AI Defending: dodge when P1 hand moves
                if (p1_hand_pos > 20.0f && rand() % 3 == 0) {
                    p2_dodging = true;
                    dodge_timer_p2 = 20;
                }
            }
        }

        // Ease hands back to resting position
        p1_hand_pos *= 0.82f;
        p2_hand_pos *= 0.82f;
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        int cy = SCR_H / 2;

        // Mat table
        SDL_Rect mat = {110, 45, 260, 180};
        set_color(ren, Color(30, 40, 70));
        SDL_RenderFillRect(ren, &mat);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &mat);

        // Center strike threshold
        set_color(ren, COL_GOLD);
        SDL_RenderDrawLine(ren, SCR_W / 2, 45, SCR_W / 2, 225);

        // P1 Hand (Blue)
        int p1_x = 130 + (int)p1_hand_pos - (p1_dodging ? 25 : 0);
        SDL_Rect h1 = {p1_x, cy - 25, 55, 50};
        set_color(ren, COL_BLUE);
        SDL_RenderFillRect(ren, &h1);
        set_color(ren, COL_WHITE);
        SDL_RenderDrawRect(ren, &h1);

        // P2 Hand (Red)
        int p2_x = 295 - (int)p2_hand_pos + (p2_dodging ? 25 : 0);
        SDL_Rect h2 = {p2_x, cy - 25, 55, 50};
        set_color(ren, COL_RED);
        SDL_RenderFillRect(ren, &h2);
        set_color(ren, COL_WHITE);
        SDL_RenderDrawRect(ren, &h2);

        // Role labels
        draw_centered_str(ren, 160, 60, (attacker == 1 ? "ATTACKER" : "DEFENDER"), COL_GOLD, 1);
        draw_centered_str(ren, 320, 60, (attacker == 2 ? "ATTACKER" : "DEFENDER"), COL_GOLD, 1);

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d/5   vs   P2: %d/5", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 25, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 20, SCR_H / 2, (attacker == 1 ? "SLAP [L]!" : "DODGE [L]!"), COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 20, SCR_H / 2, (attacker == 2 ? "SLAP [R]!" : "DODGE [R]!"), COL_RED, 1);
        } else {
            draw_str(ren, 12, 100, (attacker == 1 ? "SLAP [L]!" : "DODGE [L]!"), COL_BLUE, 1);
        }
    }
};

/* ─── 16. KNIFE HIT (ROTATING TARGET DUEL) ──────────────────────────── */

struct KnifeHitGame {
    float log_angle_p1, log_angle_p2;
    std::vector<float> knives_p1, knives_p2;
    int p1_knives_left, p2_knives_left;
    int p1_score, p2_score;
    int round_over_timer;
    int ai_throw_timer;

    const float LOG_R = 34.0f;
    const int T1_CX = 140, T1_CY = 85;
    const int T2_CX = 340, T2_CY = 85;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        log_angle_p1 = 0.0f; log_angle_p2 = 0.0f;
        knives_p1.clear(); knives_p2.clear();
        p1_knives_left = 6; p2_knives_left = 6;
        round_over_timer = 0;
        ai_throw_timer = 0;
    }

    bool throw_knife(int player) {
        float angle = (player == 1) ? -log_angle_p1 : -log_angle_p2;
        // Normalize angle into [0, 2*PI)
        while (angle < 0.0f) angle += 6.283185f;
        while (angle >= 6.283185f) angle -= 6.283185f;

        const auto &list = (player == 1) ? knives_p1 : knives_p2;
        for (float k : list) {
            float diff = fabsf(k - angle);
            if (diff > 3.14159f) diff = 6.283185f - diff;
            if (diff < 0.28f) return false; // Knife clash!
        }
        if (player == 1) knives_p1.push_back(angle);
        else knives_p2.push_back(angle);
        return true;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 3) match_winner = 1;
                else if (p2_score >= 3) match_winner = 2;
                else reset(false);
            }
            return;
        }

        log_angle_p1 += 0.035f;
        log_angle_p2 += 0.035f;

        // P1 Throw
        static bool p1_prev = false;
        if (pad.p1.action && !p1_prev && p1_knives_left > 0) {
            bool ok = throw_knife(1);
            if (ok) {
                p1_knives_left--;
                play_tone(540.0f, 30);
                if (p1_knives_left == 0) {
                    p1_score++;
                    round_over_timer = 50;
                    play_tone(750.0f, 150);
                }
            } else {
                play_tone(200.0f, 80); // Clash bounce
            }
        }
        p1_prev = pad.p1.action;

        // P2 Throw (Human or AI)
        if (mode == MODE_2P_LOCAL) {
            static bool p2_prev = false;
            if (pad.p2.action && !p2_prev && p2_knives_left > 0) {
                bool ok = throw_knife(2);
                if (ok) {
                    p2_knives_left--;
                    play_tone(540.0f, 30);
                    if (p2_knives_left == 0) {
                        p2_score++;
                        round_over_timer = 50;
                        play_tone(750.0f, 150);
                    }
                } else {
                    play_tone(200.0f, 80);
                }
            }
            p2_prev = pad.p2.action;
        } else {
            ai_throw_timer++;
            if (ai_throw_timer > 24 + rand() % 20 && p2_knives_left > 0) {
                ai_throw_timer = 0;
                bool ok = throw_knife(2);
                if (ok) {
                    p2_knives_left--;
                    play_tone(540.0f, 30);
                    if (p2_knives_left == 0) {
                        p2_score++;
                        round_over_timer = 50;
                        play_tone(350.0f, 150);
                    }
                }
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Target 1 (P1 Left)
        draw_circle(ren, T1_CX, T1_CY, (int)LOG_R, Color(150, 100, 50));
        draw_circle(ren, T1_CX, T1_CY, (int)LOG_R - 6, Color(175, 125, 75));
        draw_circle(ren, T1_CX, T1_CY, 8, Color(110, 70, 30));

        // Embedded Knives P1
        for (float k : knives_p1) {
            float a = k + log_angle_p1 + 1.570796f;
            int x1 = T1_CX + (int)(cosf(a) * LOG_R);
            int y1 = T1_CY + (int)(sinf(a) * LOG_R);
            int x2 = T1_CX + (int)(cosf(a) * (LOG_R + 18));
            int y2 = T1_CY + (int)(sinf(a) * (LOG_R + 18));
            set_color(ren, COL_BLUE);
            SDL_RenderDrawLine(ren, x1, y1, x2, y2);
        }

        // Target 2 (P2 Right)
        draw_circle(ren, T2_CX, T2_CY, (int)LOG_R, Color(150, 100, 50));
        draw_circle(ren, T2_CX, T2_CY, (int)LOG_R - 6, Color(175, 125, 75));
        draw_circle(ren, T2_CX, T2_CY, 8, Color(110, 70, 30));

        // Embedded Knives P2
        for (float k : knives_p2) {
            float a = k + log_angle_p2 + 1.570796f;
            int x1 = T2_CX + (int)(cosf(a) * LOG_R);
            int y1 = T2_CY + (int)(sinf(a) * LOG_R);
            int x2 = T2_CX + (int)(cosf(a) * (LOG_R + 18));
            int y2 = T2_CY + (int)(sinf(a) * (LOG_R + 18));
            set_color(ren, COL_RED);
            SDL_RenderDrawLine(ren, x1, y1, x2, y2);
        }

        // Ready Knives at bottom
        for (int i = 0; i < p1_knives_left; i++) {
            SDL_Rect r = {T1_CX - 25 + i * 10, 210, 5, 20};
            set_color(ren, COL_BLUE);
            SDL_RenderFillRect(ren, &r);
        }
        for (int i = 0; i < p2_knives_left; i++) {
            SDL_Rect r = {T2_CX - 25 + i * 10, 210, 5, 20};
            set_color(ren, COL_RED);
            SDL_RenderFillRect(ren, &r);
        }

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d  vs  P2: %d", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 20, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "THROW [L]", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "THROW [R]", COL_RED, 1);
        } else {
            draw_str(ren, 20, 100, "PRESS [L] TO THROW", COL_BLUE, 1);
        }
    }
};

/* ─── 17. WESTERN QUICK DRAW (HIGH NOON SHOOTOUT) ────────────────────── */

struct QuickDrawGame {
    enum Phase { READY = 0, FIRE_SIGNAL, ROUND_OVER };
    Phase phase;
    int signal_delay;
    int p1_score, p2_score;
    int winner_shot; // 1 or 2, or -1 for foul
    int round_over_timer;
    int ai_reaction_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        phase = READY;
        signal_delay = 90 + rand() % 120;
        winner_shot = 0;
        round_over_timer = 0;
        ai_reaction_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (phase == ROUND_OVER) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 3) match_winner = 1;
                else if (p2_score >= 3) match_winner = 2;
                else reset(false);
            }
            return;
        }

        if (phase == READY) {
            signal_delay--;
            if (signal_delay <= 0) {
                phase = FIRE_SIGNAL;
                play_tone(880.0f, 60); // Gong signal!
                ai_reaction_timer = 14 + rand() % 8;
            }

            // Early draw check (Foul!)
            if (pad.p1.action) {
                p2_score++;
                winner_shot = 2; // P2 wins by foul
                phase = ROUND_OVER;
                round_over_timer = 50;
                play_tone(220.0f, 150);
                return;
            }
            if (mode == MODE_2P_LOCAL && pad.p2.action) {
                p1_score++;
                winner_shot = 1; // P1 wins by foul
                phase = ROUND_OVER;
                round_over_timer = 50;
                play_tone(220.0f, 150);
                return;
            }
        } else if (phase == FIRE_SIGNAL) {
            if (pad.p1.action) {
                p1_score++;
                winner_shot = 1;
                phase = ROUND_OVER;
                round_over_timer = 50;
                play_tone(750.0f, 120);
                return;
            }

            if (mode == MODE_2P_LOCAL) {
                if (pad.p2.action) {
                    p2_score++;
                    winner_shot = 2;
                    phase = ROUND_OVER;
                    round_over_timer = 50;
                    play_tone(350.0f, 120);
                    return;
                }
            } else {
                ai_reaction_timer--;
                if (ai_reaction_timer <= 0) {
                    p2_score++;
                    winner_shot = 2;
                    phase = ROUND_OVER;
                    round_over_timer = 50;
                    play_tone(350.0f, 120);
                    return;
                }
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Desert sand
        set_color(ren, Color(160, 125, 75));
        SDL_Rect sand = {0, 160, SCR_W, SCR_H - 160};
        SDL_RenderFillRect(ren, &sand);

        // Setting sun
        draw_circle(ren, SCR_W / 2, 130, 40, Color(225, 120, 50));

        // Cowboy 1 (Left Blue)
        int c1_x = 90, c1_y = 150;
        draw_circle(ren, c1_x, c1_y - 28, 8, COL_BLUE); // Hat
        set_color(ren, COL_BLUE);
        SDL_RenderDrawLine(ren, c1_x, c1_y - 20, c1_x, c1_y + 12);
        SDL_RenderDrawLine(ren, c1_x, c1_y + 12, c1_x - 8, c1_y + 30);
        SDL_RenderDrawLine(ren, c1_x, c1_y + 12, c1_x + 8, c1_y + 30);
        SDL_RenderDrawLine(ren, c1_x, c1_y - 8, c1_x + 14, c1_y); // Gun arm

        // Cowboy 2 (Right Red)
        int c2_x = SCR_W - 90, c2_y = 150;
        draw_circle(ren, c2_x, c2_y - 28, 8, COL_RED);
        set_color(ren, COL_RED);
        SDL_RenderDrawLine(ren, c2_x, c2_y - 20, c2_x, c2_y + 12);
        SDL_RenderDrawLine(ren, c2_x, c2_y + 12, c2_x - 8, c2_y + 30);
        SDL_RenderDrawLine(ren, c2_x, c2_y + 12, c2_x + 8, c2_y + 30);
        SDL_RenderDrawLine(ren, c2_x, c2_y - 8, c2_x - 14, c2_y);

        if (phase == READY) {
            draw_centered_str(ren, SCR_W / 2, 50, "READY... STEADY...", COL_DIM, 2);
            draw_centered_str(ren, SCR_W / 2, 75, "DON'T DRAW EARLY!", COL_RED, 1);
        } else if (phase == FIRE_SIGNAL) {
            draw_centered_str(ren, SCR_W / 2, 50, ">> FIRE!! <<", COL_GOLD, 3);
        } else if (phase == ROUND_OVER) {
            char wbuf[32];
            snprintf(wbuf, sizeof(wbuf), "PLAYER %d DRAW!", winner_shot);
            draw_centered_str(ren, SCR_W / 2, 50, wbuf, COL_GOLD, 2);
        }

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d/3   vs   P2: %d/3", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 20, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "DRAW [L]", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "DRAW [R]", COL_RED, 1);
        }
    }
};

/* ─── 18. SLING PUCK (FINGER HOCKEY) ────────────────────────────────── */

struct SlingPuckGame {
    struct Puck {
        float x, y, vx, vy;
        int owner; // 1 = P1 Blue, 2 = P2 Red
    };
    std::vector<Puck> pucks;
    int p1_score, p2_score;
    int round_over_timer;
    int ai_pull_timer;

    const float ARENA_X = 70.0f;
    const float ARENA_Y = 40.0f;
    const float ARENA_W = 340.0f;
    const float ARENA_H = 192.0f;
    const float PUCK_R  = 10.0f;
    const float GATE_Y1 = 110.0f;
    const float GATE_Y2 = 162.0f;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        pucks.clear();
        float mid_x = ARENA_X + ARENA_W * 0.5f;

        // 3 Pucks on P1 side (left)
        pucks.push_back({mid_x - 90.0f, ARENA_Y + 45.0f, 0, 0, 1});
        pucks.push_back({mid_x - 110.0f, ARENA_Y + 96.0f, 0, 0, 1});
        pucks.push_back({mid_x - 90.0f, ARENA_Y + 145.0f, 0, 0, 1});

        // 3 Pucks on P2 side (right)
        pucks.push_back({mid_x + 90.0f, ARENA_Y + 45.0f, 0, 0, 2});
        pucks.push_back({mid_x + 110.0f, ARENA_Y + 96.0f, 0, 0, 2});
        pucks.push_back({mid_x + 90.0f, ARENA_Y + 145.0f, 0, 0, 2});

        round_over_timer = 0;
        ai_pull_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 2) match_winner = 1;
                else if (p2_score >= 2) match_winner = 2;
                else reset(false);
            }
            return;
        }

        float mid_x = ARENA_X + ARENA_W * 0.5f;

        // P1 Slingshot (Fires puck from left side towards center gate)
        static bool p1_prev = false;
        if (pad.p1.action && !p1_prev) {
            for (auto &p : pucks) {
                if (p.x < mid_x) { // Left half
                    float target_y = (GATE_Y1 + GATE_Y2) * 0.5f + ((rand() % 16) - 8);
                    float dx = mid_x - p.x;
                    float dy = target_y - p.y;
                    float d = sqrtf(dx * dx + dy * dy);
                    if (d > 1.0f) {
                        p.vx = (dx / d) * 11.5f;
                        p.vy = (dy / d) * 11.5f;
                        play_tone(480.0f, 25);
                        break;
                    }
                }
            }
        }
        p1_prev = pad.p1.action;

        // P2 Slingshot (Human or AI)
        if (mode == MODE_2P_LOCAL) {
            static bool p2_prev = false;
            if (pad.p2.action && !p2_prev) {
                for (auto &p : pucks) {
                    if (p.x > mid_x) { // Right half
                        float target_y = (GATE_Y1 + GATE_Y2) * 0.5f + ((rand() % 16) - 8);
                        float dx = mid_x - p.x;
                        float dy = target_y - p.y;
                        float d = sqrtf(dx * dx + dy * dy);
                        if (d > 1.0f) {
                            p.vx = (dx / d) * 11.5f;
                            p.vy = (dy / d) * 11.5f;
                            play_tone(420.0f, 25);
                            break;
                        }
                    }
                }
            }
            p2_prev = pad.p2.action;
        } else {
            ai_pull_timer++;
            if (ai_pull_timer > 22 + rand() % 16) {
                ai_pull_timer = 0;
                for (auto &p : pucks) {
                    if (p.x > mid_x) {
                        float target_y = (GATE_Y1 + GATE_Y2) * 0.5f + ((rand() % 16) - 8);
                        float dx = mid_x - p.x;
                        float dy = target_y - p.y;
                        float d = sqrtf(dx * dx + dy * dy);
                        if (d > 1.0f) {
                            p.vx = (dx / d) * 11.0f;
                            p.vy = (dy / d) * 11.0f;
                            play_tone(420.0f, 25);
                            break;
                        }
                    }
                }
            }
        }

        // Physics integration & boundary rebounds
        for (auto &p : pucks) {
            p.x += p.vx;
            p.y += p.vy;
            p.vx *= 0.965f;
            p.vy *= 0.965f;

            // Outer boundary bounds
            if (p.x < ARENA_X + PUCK_R) { p.x = ARENA_X + PUCK_R; p.vx = -p.vx; }
            if (p.x > ARENA_X + ARENA_W - PUCK_R) { p.x = ARENA_X + ARENA_W - PUCK_R; p.vx = -p.vx; }
            if (p.y < ARENA_Y + PUCK_R) { p.y = ARENA_Y + PUCK_R; p.vy = -p.vy; }
            if (p.y > ARENA_Y + ARENA_H - PUCK_R) { p.y = ARENA_Y + ARENA_H - PUCK_R; p.vy = -p.vy; }

            // Center partition wall (only blocks outside the gate window)
            if (p.y < GATE_Y1 || p.y > GATE_Y2) {
                if (p.x + PUCK_R > mid_x && p.x - PUCK_R < mid_x) {
                    p.vx = -p.vx;
                    if (p.x < mid_x) p.x = mid_x - PUCK_R;
                    else p.x = mid_x + PUCK_R;
                }
            }
        }

        // Win check: are all pucks on one side?
        int left_count = 0, right_count = 0;
        for (const auto &p : pucks) {
            if (p.x < mid_x) left_count++;
            else right_count++;
        }

        if (left_count == 0) {
            p1_score++;
            round_over_timer = 50;
            play_tone(750.0f, 150);
        } else if (right_count == 0) {
            p2_score++;
            round_over_timer = 50;
            play_tone(350.0f, 150);
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        float mid_x = ARENA_X + ARENA_W * 0.5f;

        // Wooden board table
        SDL_Rect board = {(int)ARENA_X - 4, (int)ARENA_Y - 4, (int)ARENA_W + 8, (int)ARENA_H + 8};
        set_color(ren, Color(175, 135, 90));
        SDL_RenderFillRect(ren, &board);
        set_color(ren, Color(90, 60, 30));
        SDL_RenderDrawRect(ren, &board);

        // Center partition walls (top & bottom)
        set_color(ren, Color(90, 60, 30));
        SDL_Rect w_top = {(int)mid_x - 3, (int)ARENA_Y, 6, (int)(GATE_Y1 - ARENA_Y)};
        SDL_RenderFillRect(ren, &w_top);
        SDL_Rect w_bot = {(int)mid_x - 3, (int)GATE_Y2, 6, (int)(ARENA_Y + ARENA_H - GATE_Y2)};
        SDL_RenderFillRect(ren, &w_bot);

        // Elastic cords on left and right
        set_color(ren, Color(50, 50, 50));
        SDL_RenderDrawLine(ren, (int)ARENA_X + 24, (int)ARENA_Y, (int)ARENA_X + 24, (int)ARENA_Y + (int)ARENA_H);
        SDL_RenderDrawLine(ren, (int)ARENA_X + (int)ARENA_W - 24, (int)ARENA_Y, (int)ARENA_X + (int)ARENA_W - 24, (int)ARENA_Y + (int)ARENA_H);

        // Pucks
        for (const auto &p : pucks) {
            Color pcol = (p.x < mid_x) ? COL_BLUE : COL_RED;
            draw_circle(ren, (int)p.x, (int)p.y, (int)PUCK_R, pcol);
            draw_circle(ren, (int)p.x, (int)p.y, (int)PUCK_R, COL_WHITE, false);
            draw_circle(ren, (int)p.x, (int)p.y, 4, Color(255, 255, 255), false);
        }

        // Scores
        int left_count = 0, right_count = 0;
        for (const auto &p : pucks) {
            if (p.x < mid_x) left_count++; else right_count++;
        }
        char lbuf[24], rbuf[24];
        snprintf(lbuf, sizeof(lbuf), "PUCKS: %d", left_count);
        snprintf(rbuf, sizeof(rbuf), "PUCKS: %d", right_count);
        draw_centered_str(ren, (int)ARENA_X + 80, 22, lbuf, COL_BLUE, 1);
        draw_centered_str(ren, (int)ARENA_X + (int)ARENA_W - 80, 22, rbuf, COL_RED, 1);

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "WINS: %d  vs  %d", p1_score, p2_score);
        draw_centered_str(ren, (int)mid_x, 22, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "SLING [L]", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "SLING [R]", COL_RED, 1);
        } else {
            draw_str(ren, 16, 248, "PRESS [L] TO FLING PUCK THROUGH CENTER GATE", COL_WHITE, 1);
        }
    }
};
