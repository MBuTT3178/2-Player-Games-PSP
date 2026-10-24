#pragma once
#include <SDL.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cmath>

/* ─── 25. PINBALL DUEL (OPPOSING FLIPPERS) ───────────────────────────── */

struct PinballGame {
    float ball_x, ball_y, ball_vx, ball_vy;
    int p1_score, p2_score;
    float p1_flipper_l, p1_flipper_r; // 0.0 (rest) to 1.0 (flipped)
    float p2_flipper_l, p2_flipper_r;
    int bumper_flash[3]; // flash timer for 3 center bumpers
    int reset_timer;

    const int TABLE_X = 140, TABLE_Y = 16, TABLE_W = 200, TABLE_H = 240;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        ball_x = TABLE_X + TABLE_W / 2.0f;
        ball_y = TABLE_Y + TABLE_H / 2.0f;
        ball_vx = (rand() % 2 == 0 ? 1.5f : -1.5f);
        ball_vy = (rand() % 2 == 0 ? 2.5f : -2.5f);
        p1_flipper_l = 0.0f; p1_flipper_r = 0.0f;
        p2_flipper_l = 0.0f; p2_flipper_r = 0.0f;
        bumper_flash[0] = bumper_flash[1] = bumper_flash[2] = 0;
        reset_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (reset_timer > 0) {
            reset_timer--;
            if (reset_timer == 0) {
                if (p1_score >= 3) match_winner = 1;
                else if (p2_score >= 3) match_winner = 2;
                else reset(false);
            }
            return;
        }

        // Flipper controls
        // P1 (Bottom): action flips both or left/right
        bool p1_flip_l = pad.p1.left || pad.p1.action;
        bool p1_flip_r = pad.p1.right || pad.p1.action;
        p1_flipper_l += (p1_flip_l ? 0.35f : -0.25f);
        p1_flipper_r += (p1_flip_r ? 0.35f : -0.25f);
        if (p1_flipper_l < 0.0f) p1_flipper_l = 0.0f;
        if (p1_flipper_l > 1.0f) p1_flipper_l = 1.0f;
        if (p1_flipper_r < 0.0f) p1_flipper_r = 0.0f;
        if (p1_flipper_r > 1.0f) p1_flipper_r = 1.0f;

        // P2 (Top): in 2P mode pad.p2, in 1P mode AI
        bool p2_flip_l = false, p2_flip_r = false;
        if (mode == MODE_2P_LOCAL) {
            p2_flip_l = pad.p2.left || pad.p2.action;
            p2_flip_r = pad.p2.right || pad.p2.action;
        } else {
            // AI flipper triggers when ball is near top flippers
            if (ball_y < TABLE_Y + 70 && ball_vy < 0) {
                if (ball_x < TABLE_X + TABLE_W / 2) p2_flip_l = true;
                else p2_flip_r = true;
            }
        }
        p2_flipper_l += (p2_flip_l ? 0.35f : -0.25f);
        p2_flipper_r += (p2_flip_r ? 0.35f : -0.25f);
        if (p2_flipper_l < 0.0f) p2_flipper_l = 0.0f;
        if (p2_flipper_l > 1.0f) p2_flipper_l = 1.0f;
        if (p2_flipper_r < 0.0f) p2_flipper_r = 0.0f;
        if (p2_flipper_r > 1.0f) p2_flipper_r = 1.0f;

        // Ball movement
        ball_x += ball_vx;
        ball_y += ball_vy;

        // Side wall bounces
        if (ball_x < TABLE_X + 8) {
            ball_x = TABLE_X + 8;
            ball_vx = fabsf(ball_vx);
            play_tone(400.0f, 20);
        } else if (ball_x > TABLE_X + TABLE_W - 8) {
            ball_x = TABLE_X + TABLE_W - 8;
            ball_vx = -fabsf(ball_vx);
            play_tone(400.0f, 20);
        }

        // Center bumpers
        const float bump_x[3] = { (float)TABLE_X + 100, (float)TABLE_X + 60, (float)TABLE_X + 140 };
        const float bump_y[3] = { (float)TABLE_Y + 120, (float)TABLE_Y + 95, (float)TABLE_Y + 145 };
        const float bump_r = 14.0f;

        for (int b = 0; b < 3; b++) {
            if (bumper_flash[b] > 0) bumper_flash[b]--;
            float dx = ball_x - bump_x[b];
            float dy = ball_y - bump_y[b];
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < bump_r + 6.0f && dist > 0.1f) {
                ball_vx = (dx / dist) * 4.2f;
                ball_vy = (dy / dist) * 4.2f;
                bumper_flash[b] = 12;
                play_tone(600.0f + b * 150.0f, 35);
            }
        }

        // Flipper collision checks
        // P1 bottom flippers (y around TABLE_Y + TABLE_H - 25)
        float p1_flip_y = TABLE_Y + TABLE_H - 24.0f;
        if (ball_y >= p1_flip_y - 6 && ball_y <= p1_flip_y + 10) {
            // Left flipper
            if (ball_x >= TABLE_X + 25 && ball_x <= TABLE_X + 85) {
                ball_vy = -(3.5f + p1_flipper_l * 3.0f);
                ball_vx += (p1_flipper_l * 1.5f);
                play_tone(520.0f, 30);
            }
            // Right flipper
            else if (ball_x >= TABLE_X + TABLE_W - 85 && ball_x <= TABLE_X + TABLE_W - 25) {
                ball_vy = -(3.5f + p1_flipper_r * 3.0f);
                ball_vx -= (p1_flipper_r * 1.5f);
                play_tone(520.0f, 30);
            }
        }

        // P2 top flippers (y around TABLE_Y + 24)
        float p2_flip_y = TABLE_Y + 24.0f;
        if (ball_y >= p2_flip_y - 10 && ball_y <= p2_flip_y + 6) {
            // Left flipper
            if (ball_x >= TABLE_X + 25 && ball_x <= TABLE_X + 85) {
                ball_vy = (3.5f + p2_flipper_l * 3.0f);
                ball_vx += (p2_flipper_l * 1.5f);
                play_tone(520.0f, 30);
            }
            // Right flipper
            else if (ball_x >= TABLE_X + TABLE_W - 85 && ball_x <= TABLE_X + TABLE_W - 25) {
                ball_vy = (3.5f + p2_flipper_r * 3.0f);
                ball_vx -= (p2_flipper_r * 1.5f);
                play_tone(520.0f, 30);
            }
        }

        // Speed clamp
        float spd = sqrtf(ball_vx * ball_vx + ball_vy * ball_vy);
        if (spd > 7.0f) { ball_vx = (ball_vx / spd) * 7.0f; ball_vy = (ball_vy / spd) * 7.0f; }
        if (spd < 2.5f && spd > 0.1f) { ball_vx = (ball_vx / spd) * 2.5f; ball_vy = (ball_vy / spd) * 2.5f; }

        // Goals / gutters
        if (ball_y > TABLE_Y + TABLE_H + 8) {
            p2_score++;
            play_tone(240.0f, 150);
            reset_timer = 45;
        } else if (ball_y < TABLE_Y - 8) {
            p1_score++;
            play_tone(750.0f, 150);
            reset_timer = 45;
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Pinball cabinet table
        SDL_Rect cabinet = { TABLE_X - 6, TABLE_Y - 6, TABLE_W + 12, TABLE_H + 12 };
        set_color(ren, Color(24, 28, 48));
        SDL_RenderFillRect(ren, &cabinet);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &cabinet);

        SDL_Rect field = { TABLE_X, TABLE_Y, TABLE_W, TABLE_H };
        set_color(ren, Color(16, 20, 36));
        SDL_RenderFillRect(ren, &field);

        // Center line
        set_color(ren, Color(35, 45, 75));
        SDL_RenderDrawLine(ren, TABLE_X, TABLE_Y + TABLE_H / 2, TABLE_X + TABLE_W, TABLE_Y + TABLE_H / 2);

        // Bumpers
        const float bump_x[3] = { (float)TABLE_X + 100, (float)TABLE_X + 60, (float)TABLE_X + 140 };
        const float bump_y[3] = { (float)TABLE_Y + 120, (float)TABLE_Y + 95, (float)TABLE_Y + 145 };
        for (int b = 0; b < 3; b++) {
            Color bc = (bumper_flash[b] > 0) ? COL_GOLD : Color(80, 180, 255);
            SDL_Rect br = { (int)(bump_x[b] - 12), (int)(bump_y[b] - 12), 24, 24 };
            set_color(ren, bc);
            SDL_RenderFillRect(ren, &br);
            set_color(ren, COL_WHITE);
            SDL_RenderDrawRect(ren, &br);
        }

        // P2 Top flippers (Red)
        set_color(ren, COL_RED);
        int p2_ly = (int)(TABLE_Y + 20 + p2_flipper_l * 8);
        SDL_RenderDrawLine(ren, TABLE_X + 25, TABLE_Y + 20, TABLE_X + 80, p2_ly);
        SDL_RenderDrawLine(ren, TABLE_X + 25, TABLE_Y + 21, TABLE_X + 80, p2_ly + 1);
        int p2_ry = (int)(TABLE_Y + 20 + p2_flipper_r * 8);
        SDL_RenderDrawLine(ren, TABLE_X + TABLE_W - 25, TABLE_Y + 20, TABLE_X + TABLE_W - 80, p2_ry);
        SDL_RenderDrawLine(ren, TABLE_X + TABLE_W - 25, TABLE_Y + 21, TABLE_X + TABLE_W - 80, p2_ry + 1);

        // P1 Bottom flippers (Blue)
        set_color(ren, COL_BLUE);
        int p1_ly = (int)(TABLE_Y + TABLE_H - 20 - p1_flipper_l * 8);
        SDL_RenderDrawLine(ren, TABLE_X + 25, TABLE_Y + TABLE_H - 20, TABLE_X + 80, p1_ly);
        SDL_RenderDrawLine(ren, TABLE_X + 25, TABLE_Y + TABLE_H - 21, TABLE_X + 80, p1_ly - 1);
        int p1_ry = (int)(TABLE_Y + TABLE_H - 20 - p1_flipper_r * 8);
        SDL_RenderDrawLine(ren, TABLE_X + TABLE_W - 25, TABLE_Y + TABLE_H - 20, TABLE_X + TABLE_W - 80, p1_ry);
        SDL_RenderDrawLine(ren, TABLE_X + TABLE_W - 25, TABLE_Y + TABLE_H - 21, TABLE_X + TABLE_W - 80, p1_ry - 1);

        // Ball
        SDL_Rect b_rect = { (int)(ball_x - 5), (int)(ball_y - 5), 10, 10 };
        set_color(ren, COL_WHITE);
        SDL_RenderFillRect(ren, &b_rect);

        // Score display
        char s1[16], s2[16];
        snprintf(s1, sizeof(s1), "P1: %d", p1_score);
        snprintf(s2, sizeof(s2), "P2: %d", p2_score);
        draw_str(ren, 40, TABLE_Y + TABLE_H - 30, s1, COL_BLUE, 2);
        draw_str(ren, 40, TABLE_Y + 20, s2, COL_RED, 2);
        draw_str(ren, 40, TABLE_Y + TABLE_H / 2 - 8, "FIRST TO 3", COL_DIM, 1);
    }
};

/* ─── 26. 8-BALL POOL / MINI BILLIARDS ───────────────────────────────── */

struct PoolBall {
    float x, y, vx, vy;
    bool potted;
    int type; // 0 = Cue, 1 = Blue Solid, 2 = Red Stripe, 3 = Black 8-Ball
};

struct PoolGame {
    PoolBall balls[8];
    int current_turn; // 1 or 2
    float aim_angle;
    float power;
    bool charging;
    int p1_score, p2_score;
    int turn_delay;

    const int TABLE_X = 60, TABLE_Y = 36, TABLE_W = 360, TABLE_H = 200;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        current_turn = 1;
        aim_angle = 0.0f;
        power = 0.0f;
        charging = false;
        turn_delay = 0;

        // Cue ball
        balls[0] = { (float)TABLE_X + 90.0f, (float)TABLE_Y + TABLE_H / 2.0f, 0, 0, false, 0 };

        // Triangle cluster at right side
        float rx = TABLE_X + 250.0f;
        float ry = TABLE_Y + TABLE_H / 2.0f;
        int types[7] = { 1, 2, 3, 1, 2, 1, 2 }; // 3 blue, 3 red, 1 black
        int idx = 1;
        for (int col = 0; col < 3; col++) {
            for (int row = 0; row <= col; row++) {
                if (idx > 7) break;
                float bx = rx + col * 16.0f;
                float by = ry + (row - col * 0.5f) * 18.0f;
                balls[idx] = { bx, by, 0, 0, false, types[idx - 1] };
                idx++;
            }
        }
    }

    bool any_moving() {
        for (int i = 0; i < 8; i++) {
            if (!balls[i].potted && (fabsf(balls[i].vx) > 0.08f || fabsf(balls[i].vy) > 0.08f)) return true;
        }
        return false;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (p1_score >= 3) { match_winner = 1; return; }
        if (p2_score >= 3) { match_winner = 2; return; }

        bool moving = any_moving();

        if (moving) {
            // Update physics
            for (int i = 0; i < 8; i++) {
                if (balls[i].potted) continue;
                balls[i].x += balls[i].vx;
                balls[i].y += balls[i].vy;
                balls[i].vx *= 0.982f;
                balls[i].vy *= 0.982f;
                if (fabsf(balls[i].vx) < 0.08f) balls[i].vx = 0;
                if (fabsf(balls[i].vy) < 0.08f) balls[i].vy = 0;

                // Cushion bounces
                if (balls[i].x < TABLE_X + 16) { balls[i].x = TABLE_X + 16; balls[i].vx = -balls[i].vx * 0.88f; }
                if (balls[i].x > TABLE_X + TABLE_W - 16) { balls[i].x = TABLE_X + TABLE_W - 16; balls[i].vx = -balls[i].vx * 0.88f; }
                if (balls[i].y < TABLE_Y + 16) { balls[i].y = TABLE_Y + 16; balls[i].vy = -balls[i].vy * 0.88f; }
                if (balls[i].y > TABLE_Y + TABLE_H - 16) { balls[i].y = TABLE_Y + TABLE_H - 16; balls[i].vy = -balls[i].vy * 0.88f; }

                // Pocket checks (6 pockets)
                float pockets[6][2] = {
                    { (float)TABLE_X + 14, (float)TABLE_Y + 14 },
                    { (float)TABLE_X + TABLE_W / 2, (float)TABLE_Y + 10 },
                    { (float)TABLE_X + TABLE_W - 14, (float)TABLE_Y + 14 },
                    { (float)TABLE_X + 14, (float)TABLE_Y + TABLE_H - 14 },
                    { (float)TABLE_X + TABLE_W / 2, (float)TABLE_Y + TABLE_H - 10 },
                    { (float)TABLE_X + TABLE_W - 14, (float)TABLE_Y + TABLE_H - 14 }
                };

                for (int p = 0; p < 6; p++) {
                    float pdx = balls[i].x - pockets[p][0];
                    float pdy = balls[i].y - pockets[p][1];
                    if (sqrtf(pdx * pdx + pdy * pdy) < 16.0f) {
                        balls[i].potted = true;
                        balls[i].vx = 0; balls[i].vy = 0;
                        play_tone(280.0f, 60);

                        if (balls[i].type == 0) {
                            // Cue scratch: respawn
                            balls[i].x = TABLE_X + 90.0f;
                            balls[i].y = TABLE_Y + TABLE_H / 2.0f;
                            balls[i].potted = false;
                        } else if (balls[i].type == 1) {
                            p1_score++;
                        } else if (balls[i].type == 2) {
                            p2_score++;
                        } else if (balls[i].type == 3) {
                            // 8-ball potted
                            if (current_turn == 1) p1_score = 3;
                            else p2_score = 3;
                        }
                    }
                }
            }

            // Ball to ball elastic collisions
            for (int i = 0; i < 8; i++) {
                if (balls[i].potted) continue;
                for (int j = i + 1; j < 8; j++) {
                    if (balls[j].potted) continue;
                    float dx = balls[j].x - balls[i].x;
                    float dy = balls[j].y - balls[i].y;
                    float dist = sqrtf(dx * dx + dy * dy);
                    if (dist < 16.0f && dist > 0.01f) {
                        float nx = dx / dist;
                        float ny = dy / dist;
                        float kx = balls[i].vx - balls[j].vx;
                        float ky = balls[i].vy - balls[j].vy;
                        float p = 2.0f * (nx * kx + ny * ky) / 2.0f;
                        balls[i].vx -= p * nx * 0.95f;
                        balls[i].vy -= p * ny * 0.95f;
                        balls[j].vx += p * nx * 0.95f;
                        balls[j].vy += p * ny * 0.95f;

                        // Separate
                        float overlap = 0.5f * (16.0f - dist);
                        balls[i].x -= overlap * nx;
                        balls[i].y -= overlap * ny;
                        balls[j].x += overlap * nx;
                        balls[j].y += overlap * ny;

                        play_tone(440.0f, 15);
                    }
                }
            }
        } else {
            // Aiming & shooting phase
            if (mode == MODE_2P_LOCAL || current_turn == 1) {
                const auto &p = (current_turn == 1) ? pad.p1 : pad.p2;
                if (p.left)  aim_angle -= 0.04f;
                if (p.right) aim_angle += 0.04f;

                if (p.action) {
                    charging = true;
                    power += 0.25f;
                    if (power > 10.0f) power = 10.0f;
                } else if (charging) {
                    // Shoot
                    balls[0].vx = cosf(aim_angle) * power;
                    balls[0].vy = sinf(aim_angle) * power;
                    charging = false;
                    power = 0.0f;
                    current_turn = (current_turn == 1) ? 2 : 1;
                    play_tone(350.0f, 30);
                }
            } else {
                // AI Turn: find closest valid target ball and shoot
                turn_delay++;
                if (turn_delay > 40) {
                    float best_dist = 9999.0f;
                    int target_idx = 1;
                    for (int i = 1; i < 8; i++) {
                        if (!balls[i].potted) {
                            float d = hypotf(balls[i].x - balls[0].x, balls[i].y - balls[0].y);
                            if (d < best_dist) { best_dist = d; target_idx = i; }
                        }
                    }
                    aim_angle = atan2f(balls[target_idx].y - balls[0].y, balls[target_idx].x - balls[0].x);
                    balls[0].vx = cosf(aim_angle) * 7.0f;
                    balls[0].vy = sinf(aim_angle) * 7.0f;
                    turn_delay = 0;
                    current_turn = 1;
                    play_tone(350.0f, 30);
                }
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Wooden Table Border
        SDL_Rect border = { TABLE_X - 12, TABLE_Y - 12, TABLE_W + 24, TABLE_H + 24 };
        set_color(ren, Color(80, 50, 30));
        SDL_RenderFillRect(ren, &border);
        set_color(ren, Color(50, 30, 18));
        SDL_RenderDrawRect(ren, &border);

        // Green Felt Cloth
        SDL_Rect felt = { TABLE_X, TABLE_Y, TABLE_W, TABLE_H };
        set_color(ren, Color(24, 110, 50));
        SDL_RenderFillRect(ren, &felt);

        // Pockets
        float pockets[6][2] = {
            { (float)TABLE_X + 14, (float)TABLE_Y + 14 },
            { (float)TABLE_X + TABLE_W / 2, (float)TABLE_Y + 10 },
            { (float)TABLE_X + TABLE_W - 14, (float)TABLE_Y + 14 },
            { (float)TABLE_X + 14, (float)TABLE_Y + TABLE_H - 14 },
            { (float)TABLE_X + TABLE_W / 2, (float)TABLE_Y + TABLE_H - 10 },
            { (float)TABLE_X + TABLE_W - 14, (float)TABLE_Y + TABLE_H - 14 }
        };
        for (int p = 0; p < 6; p++) {
            SDL_Rect pr = { (int)(pockets[p][0] - 12), (int)(pockets[p][1] - 12), 24, 24 };
            set_color(ren, Color(10, 14, 18));
            SDL_RenderFillRect(ren, &pr);
        }

        // Aim line
        if (!any_moving() && !balls[0].potted) {
            set_color(ren, Color(255, 255, 255, 160));
            int lx = (int)(balls[0].x + cosf(aim_angle) * 45.0f);
            int ly = (int)(balls[0].y + sinf(aim_angle) * 45.0f);
            SDL_RenderDrawLine(ren, (int)balls[0].x, (int)balls[0].y, lx, ly);

            if (charging) {
                SDL_Rect pbar = { (int)balls[0].x - 16, (int)balls[0].y - 20, (int)(power * 3.2f), 5 };
                set_color(ren, COL_GOLD);
                SDL_RenderFillRect(ren, &pbar);
            }
        }

        // Balls
        for (int i = 0; i < 8; i++) {
            if (balls[i].potted) continue;
            SDL_Rect br = { (int)(balls[i].x - 7), (int)(balls[i].y - 7), 14, 14 };
            Color bc = COL_WHITE;
            if (balls[i].type == 1) bc = COL_BLUE;
            else if (balls[i].type == 2) bc = COL_RED;
            else if (balls[i].type == 3) bc = Color(20, 20, 20);

            set_color(ren, bc);
            SDL_RenderFillRect(ren, &br);
            set_color(ren, COL_SHADOW);
            SDL_RenderDrawRect(ren, &br);
        }

        // HUD
        char h1[32], h2[32];
        snprintf(h1, sizeof(h1), "P1 (BLUE): %d", p1_score);
        snprintf(h2, sizeof(h2), "P2 (RED): %d", p2_score);
        draw_str(ren, 64, 14, h1, COL_BLUE, 1);
        draw_str(ren, 300, 14, h2, COL_RED, 1);

        const char *turn_str = (current_turn == 1) ? "< P1 TURN: AIM & SHOOT >" : "< P2 TURN: AIM & SHOOT >";
        Color tc = (current_turn == 1) ? COL_BLUE : COL_RED;
        draw_centered_str(ren, SCR_W / 2, TABLE_Y + TABLE_H + 12, turn_str, tc, 1);
    }
};

/* ─── 27. MINI GOLF (OBSTACLE GREEN & SWING METER) ───────────────────── */

struct MiniGolfGame {
    float p1_x, p1_y, p1_vx, p1_vy;
    float p2_x, p2_y, p2_vx, p2_vy;
    int p1_strokes, p2_strokes;
    int p1_holes, p2_holes;
    int current_player; // 1 or 2
    float aim_angle;
    float swing_power, swing_dir;
    bool swinging;
    float obstacle_t;
    int round_over_timer;

    const int HOLE_X = 390, HOLE_Y = 136;
    const int TEE_X = 80, TEE_Y = 136;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_holes = 0; p2_holes = 0; }
        p1_x = TEE_X; p1_y = TEE_Y - 15; p1_vx = 0; p1_vy = 0; p1_strokes = 0;
        p2_x = TEE_X; p2_y = TEE_Y + 15; p2_vx = 0; p2_vy = 0; p2_strokes = 0;
        current_player = 1;
        aim_angle = 0.0f;
        swing_power = 0.0f; swing_dir = 1.0f;
        swinging = false;
        obstacle_t = 0.0f;
        round_over_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_holes >= 2) match_winner = 1;
                else if (p2_holes >= 2) match_winner = 2;
                else reset(false);
            }
            return;
        }

        obstacle_t += 0.04f;

        // Ball physics
        p1_x += p1_vx; p1_y += p1_vy;
        p1_vx *= 0.97f; p1_vy *= 0.97f;
        if (fabsf(p1_vx) < 0.06f) p1_vx = 0;
        if (fabsf(p1_vy) < 0.06f) p1_vy = 0;

        p2_x += p2_vx; p2_y += p2_vy;
        p2_vx *= 0.97f; p2_vy *= 0.97f;
        if (fabsf(p2_vx) < 0.06f) p2_vx = 0;
        if (fabsf(p2_vy) < 0.06f) p2_vy = 0;

        // Green boundary limits: 50 <= x <= 430, 40 <= y <= 232
        auto clamp_ball = [](float &x, float &y, float &vx, float &vy) {
            if (x < 56)  { x = 56;  vx = -vx * 0.8f; }
            if (x > 424) { x = 424; vx = -vx * 0.8f; }
            if (y < 46)  { y = 46;  vy = -vy * 0.8f; }
            if (y > 226) { y = 226; vy = -vy * 0.8f; }
        };
        clamp_ball(p1_x, p1_y, p1_vx, p1_vy);
        clamp_ball(p2_x, p2_y, p2_vx, p2_vy);

        // Obstacle barrier at x = 240 with moving opening
        float barrier_open_y = 136.0f + sinf(obstacle_t) * 45.0f;
        auto check_barrier = [barrier_open_y](float &x, float &vx, float y) {
            if (x >= 235 && x <= 245) {
                if (fabsf(y - barrier_open_y) > 28.0f) {
                    vx = -vx * 0.85f;
                    play_tone(320.0f, 20);
                }
            }
        };
        check_barrier(p1_x, p1_vx, p1_y);
        check_barrier(p2_x, p2_vx, p2_y);

        // Check holed out
        if (p1_vx == 0 && p1_vy == 0 && hypotf(p1_x - HOLE_X, p1_y - HOLE_Y) < 9.0f) {
            p1_holes++;
            play_tone(750.0f, 150);
            round_over_timer = 50;
            return;
        }
        if (p2_vx == 0 && p2_vy == 0 && hypotf(p2_x - HOLE_X, p2_y - HOLE_Y) < 9.0f) {
            p2_holes++;
            play_tone(750.0f, 150);
            round_over_timer = 50;
            return;
        }

        bool moving = (p1_vx != 0 || p1_vy != 0 || p2_vx != 0 || p2_vy != 0);
        if (!moving) {
            if (mode == MODE_2P_LOCAL || current_player == 1) {
                const auto &p = (current_player == 1) ? pad.p1 : pad.p2;
                if (p.left)  aim_angle -= 0.04f;
                if (p.right) aim_angle += 0.04f;

                if (p.action) {
                    swinging = true;
                    swing_power += swing_dir * 0.28f;
                    if (swing_power > 8.0f) { swing_power = 8.0f; swing_dir = -1.0f; }
                    if (swing_power < 1.0f) { swing_power = 1.0f; swing_dir = 1.0f; }
                } else if (swinging) {
                    if (current_player == 1) {
                        p1_vx = cosf(aim_angle) * swing_power;
                        p1_vy = sinf(aim_angle) * swing_power;
                        p1_strokes++;
                    } else {
                        p2_vx = cosf(aim_angle) * swing_power;
                        p2_vy = sinf(aim_angle) * swing_power;
                        p2_strokes++;
                    }
                    swinging = false;
                    swing_power = 0;
                    current_player = (current_player == 1) ? 2 : 1;
                    play_tone(480.0f, 30);
                }
            } else {
                // AI Putt
                aim_angle = atan2f(HOLE_Y - p2_y, HOLE_X - p2_x);
                p2_vx = cosf(aim_angle) * 5.5f;
                p2_vy = sinf(aim_angle) * 5.5f;
                p2_strokes++;
                current_player = 1;
                play_tone(480.0f, 30);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Course green
        SDL_Rect green = { 50, 40, 380, 192 };
        set_color(ren, Color(30, 140, 50));
        SDL_RenderFillRect(ren, &green);
        set_color(ren, Color(80, 180, 70));
        SDL_RenderDrawRect(ren, &green);

        // Sand bunker at center bottom
        SDL_Rect sand = { 150, 150, 65, 55 };
        set_color(ren, Color(210, 180, 110));
        SDL_RenderFillRect(ren, &sand);

        // Barrier with opening
        float open_y = 136.0f + sinf(obstacle_t) * 45.0f;
        set_color(ren, Color(160, 40, 40));
        SDL_Rect wall_top = { 238, 40, 8, (int)(open_y - 28 - 40) };
        SDL_Rect wall_bot = { 238, (int)(open_y + 28), 8, (int)(232 - (open_y + 28)) };
        SDL_RenderFillRect(ren, &wall_top);
        SDL_RenderFillRect(ren, &wall_bot);

        // Cup / Hole & Flag
        SDL_Rect hole = { HOLE_X - 6, HOLE_Y - 6, 12, 12 };
        set_color(ren, Color(10, 15, 20));
        SDL_RenderFillRect(ren, &hole);
        set_color(ren, COL_GOLD);
        SDL_RenderDrawLine(ren, HOLE_X, HOLE_Y, HOLE_X, HOLE_Y - 18);
        SDL_Rect flag = { HOLE_X, HOLE_Y - 18, 10, 7 };
        set_color(ren, COL_RED);
        SDL_RenderFillRect(ren, &flag);

        // Balls
        SDL_Rect b1 = { (int)(p1_x - 5), (int)(p1_y - 5), 10, 10 };
        set_color(ren, COL_BLUE);
        SDL_RenderFillRect(ren, &b1);

        SDL_Rect b2 = { (int)(p2_x - 5), (int)(p2_y - 5), 10, 10 };
        set_color(ren, COL_RED);
        SDL_RenderFillRect(ren, &b2);

        // Aim indicator
        float cur_x = (current_player == 1) ? p1_x : p2_x;
        float cur_y = (current_player == 1) ? p1_y : p2_y;
        set_color(ren, COL_WHITE);
        SDL_RenderDrawLine(ren, (int)cur_x, (int)cur_y, (int)(cur_x + cosf(aim_angle) * 35.0f), (int)(cur_y + sinf(aim_angle) * 35.0f));

        if (swinging) {
            SDL_Rect sw = { (int)cur_x - 14, (int)cur_y - 16, (int)(swing_power * 3.5f), 4 };
            set_color(ren, COL_GOLD);
            SDL_RenderFillRect(ren, &sw);
        }

        // HUD
        char h1[32], h2[32];
        snprintf(h1, sizeof(h1), "P1 HOLES: %d", p1_holes);
        snprintf(h2, sizeof(h2), "P2 HOLES: %d", p2_holes);
        draw_str(ren, 54, 18, h1, COL_BLUE, 1);
        draw_str(ren, 330, 18, h2, COL_RED, 1);
        draw_centered_str(ren, SCR_W / 2, 246, "FIRST TO 2 HOLES WINS", COL_GOLD, 1);
    }
};

/* ─── 28. MEMORY CARDS (4x4 PAIR MATCH) ──────────────────────────────── */

struct MemoryGame {
    struct Card {
        int symbol; // 0..7
        bool revealed;
        bool matched;
    } cards[16];

    int cursor_x, cursor_y;
    int first_pick; // -1 or 0..15
    int second_pick; // -1 or 0..15
    int reveal_timer;
    int current_player; // 1 or 2
    int p1_pairs, p2_pairs;
    int ai_memory[16]; // remembers symbols seen

    void reset(bool reset_match = true) {
        if (reset_match) { p1_pairs = 0; p2_pairs = 0; }
        cursor_x = 0; cursor_y = 0;
        first_pick = -1; second_pick = -1;
        reveal_timer = 0;
        current_player = 1;

        // Populate pairs
        int pool[16];
        for (int i = 0; i < 8; i++) { pool[i * 2] = i; pool[i * 2 + 1] = i; }
        for (int i = 15; i > 0; i--) {
            int j = rand() % (i + 1);
            int tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
        }

        for (int i = 0; i < 16; i++) {
            cards[i].symbol = pool[i];
            cards[i].revealed = false;
            cards[i].matched = false;
            ai_memory[i] = -1;
        }
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (p1_pairs + p2_pairs == 8) {
            match_winner = (p1_pairs > p2_pairs) ? 1 : 2;
            return;
        }

        if (reveal_timer > 0) {
            reveal_timer--;
            if (reveal_timer == 0) {
                // Check match
                if (cards[first_pick].symbol == cards[second_pick].symbol) {
                    cards[first_pick].matched = true;
                    cards[second_pick].matched = true;
                    if (current_player == 1) p1_pairs++;
                    else p2_pairs++;
                    play_tone(660.0f, 100);
                } else {
                    cards[first_pick].revealed = false;
                    cards[second_pick].revealed = false;
                    current_player = (current_player == 1) ? 2 : 1;
                    play_tone(240.0f, 40);
                }
                first_pick = -1;
                second_pick = -1;
            }
            return;
        }

        // Active turn
        if (mode == MODE_2P_LOCAL || current_player == 1) {
            const auto &p = (current_player == 1) ? pad.p1 : pad.p2;
            if (p.left && cursor_x > 0)  { cursor_x--; play_tone(400.0f, 15); }
            if (p.right && cursor_x < 3) { cursor_x++; play_tone(400.0f, 15); }
            if (p.up && cursor_y > 0)    { cursor_y--; play_tone(400.0f, 15); }
            if (p.down && cursor_y < 3)  { cursor_y++; play_tone(400.0f, 15); }

            if (p.action) {
                int idx = cursor_y * 4 + cursor_x;
                if (!cards[idx].matched && !cards[idx].revealed) {
                    cards[idx].revealed = true;
                    ai_memory[idx] = cards[idx].symbol;
                    play_tone(520.0f, 25);

                    if (first_pick == -1) {
                        first_pick = idx;
                    } else {
                        second_pick = idx;
                        reveal_timer = 40;
                    }
                }
            }
        } else {
            // AI Turn: Pick two cards
            int pick1 = -1, pick2 = -1;
            // Check if AI knows any match in memory
            for (int i = 0; i < 16; i++) {
                if (cards[i].matched) continue;
                for (int j = i + 1; j < 16; j++) {
                    if (cards[j].matched) continue;
                    if (ai_memory[i] != -1 && ai_memory[i] == ai_memory[j]) {
                        pick1 = i; pick2 = j; break;
                    }
                }
                if (pick1 != -1) break;
            }
            if (pick1 == -1) {
                for (int tries = 0; tries < 20; tries++) {
                    int r = rand() % 16;
                    if (!cards[r].matched && !cards[r].revealed) { pick1 = r; break; }
                }
                for (int tries = 0; tries < 20; tries++) {
                    int r = rand() % 16;
                    if (!cards[r].matched && !cards[r].revealed && r != pick1) { pick2 = r; break; }
                }
            }
            if (pick1 != -1 && pick2 != -1) {
                cards[pick1].revealed = true;
                cards[pick2].revealed = true;
                ai_memory[pick1] = cards[pick1].symbol;
                ai_memory[pick2] = cards[pick2].symbol;
                first_pick = pick1;
                second_pick = pick2;
                reveal_timer = 45;
                play_tone(520.0f, 25);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        const char *symbols[8] = { "STAR", "HEART", "DIAM", "MOON", "RING", "CLUB", "SPADE", "CROWN" };
        const Color sym_cols[8] = {
            COL_GOLD, COL_RED, COL_BLUE, Color(220, 240, 255),
            Color(80, 220, 140), Color(255, 140, 40), Color(180, 100, 240), COL_GOLD
        };

        int start_x = 120, start_y = 36;
        int card_w = 52, card_h = 48, gap = 8;

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                int idx = r * 4 + c;
                int cx = start_x + c * (card_w + gap);
                int cy = start_y + r * (card_h + gap);

                SDL_Rect card_rect = { cx, cy, card_w, card_h };

                if (cards[idx].matched) {
                    set_color(ren, Color(20, 28, 44));
                    SDL_RenderFillRect(ren, &card_rect);
                    set_color(ren, Color(35, 50, 75));
                    SDL_RenderDrawRect(ren, &card_rect);
                    draw_centered_str(ren, cx + card_w / 2, cy + card_h / 2 - 4, "OK", COL_DIM, 1);
                } else if (cards[idx].revealed) {
                    set_color(ren, Color(40, 55, 95));
                    SDL_RenderFillRect(ren, &card_rect);
                    set_color(ren, sym_cols[cards[idx].symbol]);
                    SDL_RenderDrawRect(ren, &card_rect);
                    draw_centered_str(ren, cx + card_w / 2, cy + card_h / 2 - 4, symbols[cards[idx].symbol], sym_cols[cards[idx].symbol], 1);
                } else {
                    set_color(ren, Color(30, 40, 70));
                    SDL_RenderFillRect(ren, &card_rect);
                    set_color(ren, COL_BORDER);
                    SDL_RenderDrawRect(ren, &card_rect);
                    draw_centered_str(ren, cx + card_w / 2, cy + card_h / 2 - 4, "?", COL_DIM, 2);
                }

                // Cursor highlight
                if (c == cursor_x && r == cursor_y) {
                    SDL_Rect cur = { cx - 2, cy - 2, card_w + 4, card_h + 4 };
                    set_color(ren, (current_player == 1) ? COL_BLUE : COL_RED);
                    SDL_RenderDrawRect(ren, &cur);
                }
            }
        }

        // HUD
        char h1[32], h2[32];
        snprintf(h1, sizeof(h1), "P1: %d PAIRS", p1_pairs);
        snprintf(h2, sizeof(h2), "P2: %d PAIRS", p2_pairs);
        draw_str(ren, 20, 20, h1, COL_BLUE, 1);
        draw_str(ren, 370, 20, h2, COL_RED, 1);

        const char *t_str = (current_player == 1) ? "P1 TURN: FLIP CARD" : "P2 TURN: FLIP CARD";
        Color tc = (current_player == 1) ? COL_BLUE : COL_RED;
        draw_centered_str(ren, SCR_W / 2, 252, t_str, tc, 1);
    }
};

/* ─── 29. ARM WRESTLING (RAPID TAP STRUGGLE) ─────────────────────────── */

struct ArmWrestlingGame {
    float arm_pos; // -100.0 (P1 slammed) to +100.0 (P2 slammed)
    float p1_stamina, p2_stamina;
    int p1_rounds, p2_rounds;
    int surge_timer;
    int slam_hold_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_rounds = 0; p2_rounds = 0; }
        arm_pos = 0.0f;
        p1_stamina = 100.0f;
        p2_stamina = 100.0f;
        surge_timer = 0;
        slam_hold_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (slam_hold_timer > 0) {
            slam_hold_timer--;
            if (slam_hold_timer == 0) {
                if (p1_rounds >= 2) match_winner = 1;
                else if (p2_rounds >= 2) match_winner = 2;
                else reset(false);
            }
            return;
        }

        surge_timer++;
        bool surge_active = ((surge_timer / 120) % 2 == 1);

        // Natural center decay
        arm_pos *= 0.985f;

        // Stamina recovery
        p1_stamina += 0.45f; if (p1_stamina > 100.0f) p1_stamina = 100.0f;
        p2_stamina += 0.45f; if (p2_stamina > 100.0f) p2_stamina = 100.0f;

        // P1 Action
        if (pad.p1.action) {
            float pwr = (p1_stamina > 20.0f) ? 3.8f : 1.6f;
            if (surge_active) pwr *= 1.8f;
            arm_pos += pwr;
            p1_stamina -= 3.2f;
            if (p1_stamina < 0) p1_stamina = 0;
            play_tone(420.0f, 15);
        }

        // P2 Action
        bool p2_mash = false;
        if (mode == MODE_2P_LOCAL) {
            p2_mash = pad.p2.action;
        } else {
            // AI mashing rhythm
            p2_mash = (rand() % 4 != 0);
        }

        if (p2_mash) {
            float pwr = (p2_stamina > 20.0f) ? 3.6f : 1.5f;
            if (surge_active) pwr *= 1.8f;
            arm_pos -= pwr;
            p2_stamina -= 3.0f;
            if (p2_stamina < 0) p2_stamina = 0;
            play_tone(380.0f, 15);
        }

        // Check victory slam
        if (arm_pos >= 88.0f) {
            arm_pos = 90.0f;
            p1_rounds++;
            slam_hold_timer = 50;
            play_tone(750.0f, 200);
        } else if (arm_pos <= -88.0f) {
            arm_pos = -90.0f;
            p2_rounds++;
            slam_hold_timer = 50;
            play_tone(260.0f, 200);
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Wooden wrestling table
        SDL_Rect table = { 80, 80, 320, 130 };
        set_color(ren, Color(70, 45, 25));
        SDL_RenderFillRect(ren, &table);
        set_color(ren, Color(45, 28, 15));
        SDL_RenderDrawRect(ren, &table);

        // Center elbow pads
        SDL_Rect pad1 = { 180, 160, 30, 20 };
        set_color(ren, COL_BLUE);
        SDL_RenderFillRect(ren, &pad1);

        SDL_Rect pad2 = { 270, 160, 30, 20 };
        set_color(ren, COL_RED);
        SDL_RenderFillRect(ren, &pad2);

        // Arm struggle visualization: pivot at (240, 160)
        int pivot_x = 240, pivot_y = 160;
        float angle_rad = (arm_pos / 100.0f) * 0.9f; // tilt angle
        int hand_x = pivot_x + (int)(sinf(angle_rad) * 65.0f);
        int hand_y = pivot_y - (int)(cosf(angle_rad) * 65.0f);

        // Forearms
        set_color(ren, Color(220, 170, 120));
        SDL_RenderDrawLine(ren, pivot_x - 30, pivot_y, hand_x, hand_y);
        SDL_RenderDrawLine(ren, pivot_x - 29, pivot_y, hand_x + 1, hand_y);
        SDL_RenderDrawLine(ren, pivot_x + 30, pivot_y, hand_x, hand_y);
        SDL_RenderDrawLine(ren, pivot_x + 31, pivot_y, hand_x - 1, hand_y);

        // Locked hands
        SDL_Rect hands = { hand_x - 8, hand_y - 8, 16, 16 };
        set_color(ren, COL_GOLD);
        SDL_RenderFillRect(ren, &hands);

        // Power struggle meter at top
        SDL_Rect meter_bg = { 100, 35, 280, 16 };
        set_color(ren, Color(20, 25, 45));
        SDL_RenderFillRect(ren, &meter_bg);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &meter_bg);

        // Pin marker
        int marker_x = 240 + (int)(arm_pos * 1.35f);
        SDL_Rect marker = { marker_x - 4, 31, 8, 24 };
        set_color(ren, (arm_pos > 0) ? COL_BLUE : COL_RED);
        SDL_RenderFillRect(ren, &marker);

        // Surge alert
        if (((surge_timer / 120) % 2 == 1)) {
            draw_centered_str(ren, SCR_W / 2, 58, ">>> PUSH SURGE! DOUBLE POWER <<<", COL_GOLD, 1);
        } else {
            draw_centered_str(ren, SCR_W / 2, 58, "RAPIDLY MASH ACTION TO SLAM!", COL_WHITE, 1);
        }

        // HUD Rounds
        char r1[16], r2[16];
        snprintf(r1, sizeof(r1), "P1: %d", p1_rounds);
        snprintf(r2, sizeof(r2), "P2: %d", p2_rounds);
        draw_str(ren, 40, 35, r1, COL_BLUE, 2);
        draw_str(ren, 400, 35, r2, COL_RED, 2);
        draw_centered_str(ren, SCR_W / 2, 235, "FIRST TO 2 SLAMS WINS", COL_DIM, 1);
    }
};

/* ─── 30. CURVE RACER (ACHTUNG TRAIL DUEL) ───────────────────────────── */

struct CurveRacerGame {
    float p1_x, p1_y, p1_angle;
    float p2_x, p2_y, p2_angle;
    int p1_rounds, p2_rounds;
    int p1_gap_timer, p2_gap_timer;
    bool p1_alive, p2_alive;
    int round_over_timer;

    // 2x downscaled grid: 480/2 x 272/2 = 240 x 136
    uint8_t arena_grid[136][240];

    void reset(bool reset_match = true) {
        if (reset_match) { p1_rounds = 0; p2_rounds = 0; }
        p1_x = 80.0f; p1_y = 136.0f; p1_angle = 0.0f;
        p2_x = 400.0f; p2_y = 136.0f; p2_angle = 3.14159f;
        p1_alive = true; p2_alive = true;
        p1_gap_timer = rand() % 80;
        p2_gap_timer = rand() % 80;
        round_over_timer = 0;
        memset(arena_grid, 0, sizeof(arena_grid));
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_rounds >= 3) match_winner = 1;
                else if (p2_rounds >= 3) match_winner = 2;
                else reset(false);
            }
            return;
        }

        float turn_spd = 0.065f;
        float speed = 2.1f;

        // P1 Controls
        if (pad.p1.left)  p1_angle -= turn_spd;
        if (pad.p1.right) p1_angle += turn_spd;

        // P2 Controls
        if (mode == MODE_2P_LOCAL) {
            if (pad.p2.left)  p2_angle -= turn_spd;
            if (pad.p2.right) p2_angle += turn_spd;
        } else {
            // AI Steer: check forward cell, steer if wall or trail ahead
            int fwd_x = (int)((p2_x + cosf(p2_angle) * 16.0f) / 2.0f);
            int fwd_y = (int)((p2_y + sinf(p2_angle) * 16.0f) / 2.0f);
            if (fwd_x < 15 || fwd_x > 225 || fwd_y < 15 || fwd_y > 120 ||
                (fwd_x >= 0 && fwd_x < 240 && fwd_y >= 0 && fwd_y < 136 && arena_grid[fwd_y][fwd_x] != 0)) {
                p2_angle += turn_spd * 1.5f;
            }
        }

        // Advance P1
        if (p1_alive) {
            p1_x += cosf(p1_angle) * speed;
            p1_y += sinf(p1_angle) * speed;

            int gx = (int)(p1_x / 2.0f);
            int gy = (int)(p1_y / 2.0f);

            // Bounds check
            if (p1_x < 24 || p1_x > 456 || p1_y < 24 || p1_y > 248) {
                p1_alive = false;
            } else if (gx >= 0 && gx < 240 && gy >= 0 && gy < 136) {
                p1_gap_timer++;
                bool is_gap = (p1_gap_timer % 90 > 78);
                if (!is_gap) {
                    if (arena_grid[gy][gx] != 0) {
                        p1_alive = false;
                    } else {
                        arena_grid[gy][gx] = 1;
                    }
                }
            }
        }

        // Advance P2
        if (p2_alive) {
            p2_x += cosf(p2_angle) * speed;
            p2_y += sinf(p2_angle) * speed;

            int gx = (int)(p2_x / 2.0f);
            int gy = (int)(p2_y / 2.0f);

            if (p2_x < 24 || p2_x > 456 || p2_y < 24 || p2_y > 248) {
                p2_alive = false;
            } else if (gx >= 0 && gx < 240 && gy >= 0 && gy < 136) {
                p2_gap_timer++;
                bool is_gap = (p2_gap_timer % 90 > 78);
                if (!is_gap) {
                    if (arena_grid[gy][gx] != 0) {
                        p2_alive = false;
                    } else {
                        arena_grid[gy][gx] = 2;
                    }
                }
            }
        }

        // Round outcome
        if (!p1_alive || !p2_alive) {
            round_over_timer = 50;
            if (!p1_alive && p2_alive) { p2_rounds++; play_tone(280.0f, 150); }
            else if (p1_alive && !p2_alive) { p1_rounds++; play_tone(750.0f, 150); }
            else { play_tone(350.0f, 150); } // tie crash
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Arena Boundary
        SDL_Rect arena = { 24, 24, 432, 224 };
        set_color(ren, Color(16, 20, 36));
        SDL_RenderFillRect(ren, &arena);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &arena);

        // Render trails from downscaled grid
        for (int y = 12; y < 125; y++) {
            for (int x = 12; x < 229; x++) {
                if (arena_grid[y][x] == 1) {
                    set_color(ren, COL_BLUE);
                    SDL_Rect tr = { x * 2, y * 2, 2, 2 };
                    SDL_RenderFillRect(ren, &tr);
                } else if (arena_grid[y][x] == 2) {
                    set_color(ren, COL_RED);
                    SDL_Rect tr = { x * 2, y * 2, 2, 2 };
                    SDL_RenderFillRect(ren, &tr);
                }
            }
        }

        // Player Heads
        if (p1_alive) {
            SDL_Rect h1 = { (int)(p1_x - 3), (int)(p1_y - 3), 6, 6 };
            set_color(ren, COL_WHITE);
            SDL_RenderFillRect(ren, &h1);
        }
        if (p2_alive) {
            SDL_Rect h2 = { (int)(p2_x - 3), (int)(p2_y - 3), 6, 6 };
            set_color(ren, COL_GOLD);
            SDL_RenderFillRect(ren, &h2);
        }

        // HUD
        char h1[16], h2[16];
        snprintf(h1, sizeof(h1), "P1: %d", p1_rounds);
        snprintf(h2, sizeof(h2), "P2: %d", p2_rounds);
        draw_str(ren, 30, 8, h1, COL_BLUE, 1);
        draw_str(ren, 410, 8, h2, COL_RED, 1);
        draw_centered_str(ren, SCR_W / 2, 8, "CURVE RACER - FIRST TO 3", COL_GOLD, 1);
    }
};
