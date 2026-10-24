#pragma once
#include <SDL.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cmath>

/* ─── 19. PENALTY SHOOTOUT (STRIKER VS KEEPER) ──────────────────────── */

struct PenaltyGame {
    enum Phase { AIMING = 0, KICK_IN_FLIGHT, ROUND_OVER };
    Phase phase;
    int striker; // 1 = P1 kicks, 2 = P2 kicks
    float aim_x, aim_y, aim_t;
    float ball_x, ball_y, ball_vx, ball_vy, ball_sz;
    float keeper_x, keeper_target_x;
    int p1_score, p2_score;
    int round_over_timer;
    int result_type; // 0=none, 1=GOAL, 2=SAVED, 3=MISS

    const int GOAL_X = 150, GOAL_Y = 50, GOAL_W = 180, GOAL_H = 75;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        striker = 1;
        phase = AIMING;
        aim_x = 240.0f; aim_y = 80.0f; aim_t = 0.0f;
        ball_x = 240.0f; ball_y = 205.0f; ball_vx = 0; ball_vy = 0; ball_sz = 10.0f;
        keeper_x = 240.0f; keeper_target_x = 240.0f;
        round_over_timer = 0;
        result_type = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (phase == ROUND_OVER) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 3 && p1_score > p2_score + 1) match_winner = 1;
                else if (p2_score >= 3 && p2_score > p1_score + 1) match_winner = 2;
                else {
                    striker = (striker == 1) ? 2 : 1;
                    phase = AIMING;
                    ball_x = 240.0f; ball_y = 205.0f; ball_sz = 10.0f;
                    result_type = 0;
                }
            }
            return;
        }

        if (phase == AIMING) {
            aim_t += 0.05f;
            aim_x = 240.0f + sinf(aim_t) * 75.0f;
            aim_y = 65.0f + fabsf(cosf(aim_t * 1.4f)) * 40.0f;

            // Keeper movement
            if (mode == MODE_2P_LOCAL) {
                if (striker == 1) { // P2 is keeper
                    if (pad.p2.left)  keeper_x -= 2.2f;
                    if (pad.p2.right) keeper_x += 2.2f;
                } else { // P1 is keeper
                    if (pad.p1.left)  keeper_x -= 2.2f;
                    if (pad.p1.right) keeper_x += 2.2f;
                }
            } else {
                // AI Keeper tracks slightly
                keeper_x = 240.0f + sinf(aim_t * 0.7f) * 45.0f;
            }

            if (keeper_x < GOAL_X + 20) keeper_x = GOAL_X + 20;
            if (keeper_x > GOAL_X + GOAL_W - 20) keeper_x = GOAL_X + GOAL_W - 20;

            // Kick trigger
            bool kick = (striker == 1) ? pad.p1.action : (mode == MODE_2P_LOCAL ? pad.p2.action : (rand() % 45 == 0));
            if (kick) {
                phase = KICK_IN_FLIGHT;
                float dx = aim_x - ball_x;
                float dy = aim_y - ball_y;
                float dist = sqrtf(dx * dx + dy * dy);
                ball_vx = (dx / dist) * 7.5f;
                ball_vy = (dy / dist) * 7.5f;
                play_tone(440.0f, 35);
            }
        } else if (phase == KICK_IN_FLIGHT) {
            ball_x += ball_vx;
            ball_y += ball_vy;
            ball_sz *= 0.98f;

            // Arrival at goal line
            if (ball_y <= GOAL_Y + GOAL_H - 10) {
                phase = ROUND_OVER;
                round_over_timer = 50;

                // Check saved by keeper
                if (fabsf(ball_x - keeper_x) < 26.0f) {
                    result_type = 2; // SAVED!
                    play_tone(220.0f, 100);
                } else if (ball_x >= GOAL_X + 5 && ball_x <= GOAL_X + GOAL_W - 5 && ball_y >= GOAL_Y) {
                    result_type = 1; // GOAL!
                    if (striker == 1) p1_score++;
                    else p2_score++;
                    play_tone(750.0f, 150);
                } else {
                    result_type = 3; // MISS!
                    play_tone(180.0f, 60);
                }
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Pitch grass
        set_color(ren, Color(35, 110, 50));
        SDL_Rect pitch = {40, 30, SCR_W - 80, 210};
        SDL_RenderFillRect(ren, &pitch);
        set_color(ren, Color(255, 255, 255));
        SDL_RenderDrawRect(ren, &pitch);

        // Penalty box & spot
        SDL_Rect box = {GOAL_X - 20, GOAL_Y, GOAL_W + 40, 110};
        SDL_RenderDrawRect(ren, &box);
        draw_circle(ren, 240, 205, 3, COL_WHITE);

        // Goal net & frame
        SDL_Rect goal = {GOAL_X, GOAL_Y, GOAL_W, GOAL_H};
        set_color(ren, Color(22, 60, 35));
        SDL_RenderFillRect(ren, &goal);
        set_color(ren, COL_WHITE);
        SDL_RenderDrawRect(ren, &goal);

        // Net mesh lines
        set_color(ren, Color(60, 140, 80));
        for (int x = GOAL_X + 15; x < GOAL_X + GOAL_W; x += 15)
            SDL_RenderDrawLine(ren, x, GOAL_Y, x, GOAL_Y + GOAL_H);
        for (int y = GOAL_Y + 15; y < GOAL_Y + GOAL_H; y += 15)
            SDL_RenderDrawLine(ren, GOAL_X, y, GOAL_X + GOAL_W, y);

        // Goalkeeper
        Color kcol = (striker == 1) ? COL_RED : COL_BLUE;
        draw_circle(ren, (int)keeper_x, GOAL_Y + GOAL_H - 18, 10, kcol);
        draw_circle(ren, (int)keeper_x - 12, GOAL_Y + GOAL_H - 18, 4, COL_GOLD); // Gloves
        draw_circle(ren, (int)keeper_x + 12, GOAL_Y + GOAL_H - 18, 4, COL_GOLD);

        // Aim Reticle (when aiming)
        if (phase == AIMING) {
            draw_circle(ren, (int)aim_x, (int)aim_y, 8, COL_GOLD, false);
            SDL_RenderDrawLine(ren, (int)aim_x - 5, (int)aim_y, (int)aim_x + 5, (int)aim_y);
            SDL_RenderDrawLine(ren, (int)aim_x, (int)aim_y - 5, (int)aim_x, (int)aim_y + 5);
        }

        // Soccer ball
        draw_circle(ren, (int)ball_x, (int)ball_y, (int)ball_sz, COL_WHITE);
        draw_circle(ren, (int)ball_x, (int)ball_y, (int)ball_sz, Color(20, 20, 20), false);

        // Outcome message
        if (result_type == 1) draw_centered_str(ren, SCR_W / 2, 90, "GOAL!!", COL_GOLD, 3);
        else if (result_type == 2) draw_centered_str(ren, SCR_W / 2, 90, "SAVED!", COL_RED, 2);
        else if (result_type == 3) draw_centered_str(ren, SCR_W / 2, 90, "MISS!", COL_DIM, 2);

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d  vs  P2: %d", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 14, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, (striker == 1 ? "KICK [L]" : "DEFEND"), COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, (striker == 2 ? "KICK [R]" : "DEFEND"), COL_RED, 1);
        } else {
            draw_str(ren, 12, 12, "STRIKER: P1 (PRESS L TO KICK)", COL_GOLD, 1);
        }
    }
};

/* ─── 20. MICRO RACERS (CIRCUIT SPRINT) ─────────────────────────────── */

struct MicroRacersGame {
    float p1_x, p1_y, p1_angle, p1_speed;
    float p2_x, p2_y, p2_angle, p2_speed;
    int p1_laps, p2_laps;
    bool p1_chk, p2_chk;
    int round_over_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_laps = 0; p2_laps = 0; }
        p1_x = 220.0f; p1_y = 210.0f; p1_angle = 3.14159f; p1_speed = 0.0f; p1_chk = false;
        p2_x = 220.0f; p2_y = 224.0f; p2_angle = 3.14159f; p2_speed = 0.0f; p2_chk = false;
        round_over_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_laps >= 3) match_winner = 1;
                else match_winner = 2;
            }
            return;
        }

        // P1 Controls
        if (pad.p1.left)  p1_angle -= 0.07f;
        if (pad.p1.right) p1_angle += 0.07f;
        if (pad.p1.action || pad.p1.up) p1_speed += 0.18f;
        else p1_speed *= 0.94f;
        if (p1_speed > 3.4f) p1_speed = 3.4f;

        // P2 Controls
        if (mode == MODE_2P_LOCAL) {
            if (pad.p2.left)  p2_angle -= 0.07f;
            if (pad.p2.right) p2_angle += 0.07f;
            if (pad.p2.action || pad.p2.up) p2_speed += 0.18f;
            else p2_speed *= 0.94f;
        } else {
            // AI Steer along oval circuit
            float target_angle = atan2f(136.0f - p2_y, 240.0f - p2_x) + 1.57f;
            float diff = target_angle - p2_angle;
            while (diff < -3.14159f) diff += 6.283185f;
            while (diff > 3.14159f) diff -= 6.283185f;
            p2_angle += diff * 0.06f;
            p2_speed += 0.17f;
        }
        if (p2_speed > 3.4f) p2_speed = 3.4f;

        // Velocity
        p1_x += cosf(p1_angle) * p1_speed;
        p1_y += sinf(p1_angle) * p1_speed;
        p2_x += cosf(p2_angle) * p2_speed;
        p2_y += sinf(p2_angle) * p2_speed;

        // Outer & inner boundary friction slowdown
        float d1 = (p1_x - 240)*(p1_x - 240)/170.0f/170.0f + (p1_y - 136)*(p1_y - 136)/95.0f/95.0f;
        float d1_in = (p1_x - 240)*(p1_x - 240)/90.0f/90.0f + (p1_y - 136)*(p1_y - 136)/45.0f/45.0f;
        if (d1 > 1.0f || d1_in < 1.0f) p1_speed *= 0.88f; // Off track slowdown

        float d2 = (p2_x - 240)*(p2_x - 240)/170.0f/170.0f + (p2_y - 136)*(p2_y - 136)/95.0f/95.0f;
        float d2_in = (p2_x - 240)*(p2_x - 240)/90.0f/90.0f + (p2_y - 136)*(p2_y - 136)/45.0f/45.0f;
        if (d2 > 1.0f || d2_in < 1.0f) p2_speed *= 0.88f;

        // Checkpoint & Lap completion (Finish line at x=240, y > 136)
        if (p1_y < 80) p1_chk = true;
        if (p1_chk && p1_x > 235 && p1_x < 245 && p1_y > 180) {
            p1_laps++;
            p1_chk = false;
            play_tone(660.0f, 35);
            if (p1_laps >= 3) { round_over_timer = 50; play_tone(750.0f, 150); }
        }

        if (p2_y < 80) p2_chk = true;
        if (p2_chk && p2_x > 235 && p2_x < 245 && p2_y > 180) {
            p2_laps++;
            p2_chk = false;
            play_tone(550.0f, 35);
            if (p2_laps >= 3) { round_over_timer = 50; play_tone(350.0f, 150); }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Track Asphalt Oval
        draw_circle(ren, 240, 136, 105, Color(55, 60, 75));
        draw_circle(ren, 150, 136, 85, Color(55, 60, 75));
        draw_circle(ren, 330, 136, 85, Color(55, 60, 75));

        // Infield Grass
        draw_circle(ren, 240, 136, 50, Color(30, 85, 45));
        draw_circle(ren, 170, 136, 45, Color(30, 85, 45));
        draw_circle(ren, 310, 136, 45, Color(30, 85, 45));

        // Finish Line
        set_color(ren, COL_WHITE);
        SDL_RenderDrawLine(ren, 240, 190, 240, 235);
        set_color(ren, COL_GOLD);
        SDL_RenderDrawLine(ren, 241, 190, 241, 235);

        // P1 Car (Blue)
        SDL_Rect c1 = {(int)p1_x - 5, (int)p1_y - 5, 10, 10};
        set_color(ren, COL_BLUE);
        SDL_RenderFillRect(ren, &c1);
        set_color(ren, COL_WHITE);
        SDL_RenderDrawRect(ren, &c1);

        // P2 Car (Red)
        SDL_Rect c2 = {(int)p2_x - 5, (int)p2_y - 5, 10, 10};
        set_color(ren, COL_RED);
        SDL_RenderFillRect(ren, &c2);
        set_color(ren, COL_WHITE);
        SDL_RenderDrawRect(ren, &c2);

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "LAP: %d/3   vs   %d/3", p1_laps, p2_laps);
        draw_centered_str(ren, SCR_W / 2, 130, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "STEER & [L] GAS", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "STEER & [R] GAS", COL_RED, 1);
        } else {
            draw_str(ren, 12, 12, "STEER: DPAD  |  GAS: [L] / UP", COL_GOLD, 1);
        }
    }
};

/* ─── 21. BASKETBALL (ARC SHOOTOUT) ─────────────────────────────────── */

struct BasketballGame {
    float p1_ball_x, p1_ball_y, p1_vx, p1_vy;
    float p2_ball_x, p2_ball_y, p2_vx, p2_vy;
    bool p1_shooting, p2_shooting;
    float p1_power, p2_power;
    int p1_score, p2_score;
    int round_over_timer;
    int ai_timer;

    const int HOOP_X = 240, HOOP_Y = 70;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        p1_ball_x = 120.0f; p1_ball_y = 190.0f; p1_vx = 0; p1_vy = 0; p1_shooting = false; p1_power = 0;
        p2_ball_x = 360.0f; p2_ball_y = 190.0f; p2_vx = 0; p2_vy = 0; p2_shooting = false; p2_power = 0;
        round_over_timer = 0;
        ai_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 5) match_winner = 1;
                else match_winner = 2;
            }
            return;
        }

        // P1 Shoot
        if (!p1_shooting) {
            p1_power += 0.08f;
            if (pad.p1.action) {
                p1_shooting = true;
                float p = 7.0f + fabsf(sinf(p1_power)) * 4.2f;
                p1_vx = 4.2f;
                p1_vy = -p;
                play_tone(460.0f, 30);
            }
        } else {
            p1_ball_x += p1_vx;
            p1_ball_y += p1_vy;
            p1_vy += 0.28f; // Gravity

            // Hoop collision
            if (fabsf(p1_ball_x - HOOP_X) < 14.0f && fabsf(p1_ball_y - HOOP_Y) < 10.0f && p1_vy > 0) {
                p1_score++;
                p1_shooting = false;
                p1_ball_x = 120.0f; p1_ball_y = 190.0f;
                play_tone(750.0f, 120);
                if (p1_score >= 5) { round_over_timer = 50; }
            } else if (p1_ball_y > 220.0f) {
                p1_shooting = false;
                p1_ball_x = 120.0f; p1_ball_y = 190.0f;
            }
        }

        // P2 Shoot (Human or AI)
        if (mode == MODE_2P_LOCAL) {
            if (!p2_shooting) {
                p2_power += 0.08f;
                if (pad.p2.action) {
                    p2_shooting = true;
                    float p = 7.0f + fabsf(sinf(p2_power)) * 4.2f;
                    p2_vx = -4.2f;
                    p2_vy = -p;
                    play_tone(420.0f, 30);
                }
            } else {
                p2_ball_x += p2_vx;
                p2_ball_y += p2_vy;
                p2_vy += 0.28f;

                if (fabsf(p2_ball_x - HOOP_X) < 14.0f && fabsf(p2_ball_y - HOOP_Y) < 10.0f && p2_vy > 0) {
                    p2_score++;
                    p2_shooting = false;
                    p2_ball_x = 360.0f; p2_ball_y = 190.0f;
                    play_tone(750.0f, 120);
                    if (p2_score >= 5) { round_over_timer = 50; }
                } else if (p2_ball_y > 220.0f) {
                    p2_shooting = false;
                    p2_ball_x = 360.0f; p2_ball_y = 190.0f;
                }
            }
        } else {
            // AI auto shooting
            ai_timer++;
            if (!p2_shooting && ai_timer > 35) {
                ai_timer = 0;
                p2_shooting = true;
                p2_vx = -4.2f;
                p2_vy = -(8.2f + (rand() % 15) * 0.1f);
                play_tone(420.0f, 30);
            } else if (p2_shooting) {
                p2_ball_x += p2_vx;
                p2_ball_y += p2_vy;
                p2_vy += 0.28f;
                if (fabsf(p2_ball_x - HOOP_X) < 14.0f && fabsf(p2_ball_y - HOOP_Y) < 10.0f && p2_vy > 0) {
                    p2_score++;
                    p2_shooting = false;
                    p2_ball_x = 360.0f; p2_ball_y = 190.0f;
                    play_tone(750.0f, 120);
                    if (p2_score >= 5) { round_over_timer = 50; }
                } else if (p2_ball_y > 220.0f) {
                    p2_shooting = false;
                    p2_ball_x = 360.0f; p2_ball_y = 190.0f;
                }
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Court hardwood floor
        set_color(ren, Color(185, 135, 75));
        SDL_Rect court = {40, 180, SCR_W - 80, 50};
        SDL_RenderFillRect(ren, &court);

        // Hoop Backboard & Rim
        set_color(ren, COL_WHITE);
        SDL_Rect backboard = {HOOP_X - 18, HOOP_Y - 30, 36, 25};
        SDL_RenderFillRect(ren, &backboard);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &backboard);

        // Rim
        set_color(ren, Color(225, 80, 30));
        SDL_Rect rim = {HOOP_X - 12, HOOP_Y, 24, 4};
        SDL_RenderFillRect(ren, &rim);

        // Net cords
        set_color(ren, COL_WHITE);
        SDL_RenderDrawLine(ren, HOOP_X - 10, HOOP_Y + 4, HOOP_X - 4, HOOP_Y + 16);
        SDL_RenderDrawLine(ren, HOOP_X + 10, HOOP_Y + 4, HOOP_X + 4, HOOP_Y + 16);

        // Balls
        draw_circle(ren, (int)p1_ball_x, (int)p1_ball_y, 8, Color(230, 110, 30));
        draw_circle(ren, (int)p2_ball_x, (int)p2_ball_y, 8, Color(230, 110, 30));

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d/5   vs   P2: %d/5", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 20, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "SHOOT [L]", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "SHOOT [R]", COL_RED, 1);
        } else {
            draw_str(ren, 12, 12, "TIMED RELEASE: PRESS [L] TO SHOOT", COL_GOLD, 1);
        }
    }
};

/* ─── 22. SOCCER PHYSICS (BOUNCY 1V1 FOOTBALL) ──────────────────────── */

struct SoccerGame {
    float p1_x, p1_y, p1_vy;
    float p2_x, p2_y, p2_vy;
    float ball_x, ball_y, ball_vx, ball_vy;
    int p1_score, p2_score;
    int round_over_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        p1_x = 120.0f; p1_y = 190.0f; p1_vy = 0;
        p2_x = 360.0f; p2_y = 190.0f; p2_vy = 0;
        ball_x = 240.0f; ball_y = 140.0f; ball_vx = 0; ball_vy = 0;
        round_over_timer = 0;
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

        // P1 Controls: run and jump
        if (mode == MODE_2P_LOCAL) {
            if (pad.p1.up)    p1_x += 2.0f; // forward towards opponent
            if (pad.p1.down)  p1_x -= 2.0f; // back towards own goal
            if ((pad.p1.action || pad.p1.left) && p1_y >= 189.0f) {
                p1_vy = -6.5f;
                play_tone(420.0f, 25);
            }
        } else {
            if (pad.p1.left)  p1_x -= 2.0f;
            if (pad.p1.right) p1_x += 2.0f;
            if ((pad.p1.action || pad.p1.up) && p1_y >= 189.0f) {
                p1_vy = -6.5f;
                play_tone(420.0f, 25);
            }
        }

        // P2 Controls
        if (mode == MODE_2P_LOCAL) {
            if (pad.p2.up)    p2_x -= 2.0f; // forward towards opponent
            if (pad.p2.down)  p2_x += 2.0f; // back towards own goal
            if ((pad.p2.action || pad.p2.right) && p2_y >= 189.0f) {
                p2_vy = -6.5f;
                play_tone(400.0f, 25);
            }
        } else {
            // AI tracker
            if (ball_x < p2_x - 10) p2_x -= 1.8f;
            if (ball_x > p2_x + 10) p2_x += 1.8f;
            if (fabsf(ball_x - p2_x) < 30.0f && p2_y >= 189.0f && rand() % 8 == 0) {
                p2_vy = -6.5f;
                play_tone(400.0f, 25);
            }
        }

        // Gravity
        p1_vy += 0.32f; p1_y += p1_vy;
        if (p1_y > 190.0f) { p1_y = 190.0f; p1_vy = 0; }

        p2_vy += 0.32f; p2_y += p2_vy;
        if (p2_y > 190.0f) { p2_y = 190.0f; p2_vy = 0; }

        // Ball physics
        ball_vy += 0.24f;
        ball_x += ball_vx;
        ball_y += ball_vy;
        ball_vx *= 0.98f;

        if (ball_y > 196.0f) { ball_y = 196.0f; ball_vy = -ball_vy * 0.75f; }
        if (ball_x < 40.0f) { ball_x = 40.0f; ball_vx = -ball_vx; }
        if (ball_x > 440.0f) { ball_x = 440.0f; ball_vx = -ball_vx; }

        // Player to ball collision
        if (hypotf(ball_x - p1_x, ball_y - p1_y) < 22.0f) {
            ball_vx = (ball_x - p1_x) * 0.5f + 3.0f;
            ball_vy = -5.0f;
            play_tone(520.0f, 30);
        }
        if (hypotf(ball_x - p2_x, ball_y - p2_y) < 22.0f) {
            ball_vx = (ball_x - p2_x) * 0.5f - 3.0f;
            ball_vy = -5.0f;
            play_tone(520.0f, 30);
        }

        // Goal check
        if (ball_x <= 55.0f && ball_y > 140.0f) {
            p2_score++;
            round_over_timer = 50;
            play_tone(350.0f, 150);
        } else if (ball_x >= 425.0f && ball_y > 140.0f) {
            p1_score++;
            round_over_timer = 50;
            play_tone(750.0f, 150);
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Pitch
        set_color(ren, Color(40, 115, 55));
        SDL_Rect pitch = {40, 200, 400, 30};
        SDL_RenderFillRect(ren, &pitch);

        // Goal Posts
        set_color(ren, COL_WHITE);
        SDL_Rect g1 = {40, 140, 15, 60};
        SDL_RenderDrawRect(ren, &g1);
        SDL_Rect g2 = {425, 140, 15, 60};
        SDL_RenderDrawRect(ren, &g2);

        // P1 Player (Blue)
        draw_circle(ren, (int)p1_x, (int)p1_y - 14, 8, COL_BLUE);
        SDL_Rect b1 = {(int)p1_x - 6, (int)p1_y - 6, 12, 14};
        set_color(ren, COL_BLUE);
        SDL_RenderFillRect(ren, &b1);

        // P2 Player (Red)
        draw_circle(ren, (int)p2_x, (int)p2_y - 14, 8, COL_RED);
        SDL_Rect b2 = {(int)p2_x - 6, (int)p2_y - 6, 12, 14};
        set_color(ren, COL_RED);
        SDL_RenderFillRect(ren, &b2);

        // Soccer ball
        draw_circle(ren, (int)ball_x, (int)ball_y, 8, COL_WHITE);
        draw_circle(ren, (int)ball_x, (int)ball_y, 8, Color(20, 20, 20), false);

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d  vs  P2: %d", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 20, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "JUMP & KICK [L]", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "JUMP & KICK [R]", COL_RED, 1);
        }
    }
};

/* ─── 23. BOWLING (10-PIN LANE) ─────────────────────────────────────── */

struct BowlingGame {
    float ball_x, ball_y, ball_vx, ball_vy;
    bool ball_rolling;
    float aim_x, aim_t;
    bool pins[10];
    int p1_pins, p2_pins;
    int turn; // 1 or 2
    int frames_played;
    int round_over_timer;

    void reset_pins() {
        for (int i = 0; i < 10; i++) pins[i] = true;
    }

    void reset(bool reset_match = true) {
        if (reset_match) { p1_pins = 0; p2_pins = 0; frames_played = 0; }
        turn = 1;
        ball_rolling = false;
        aim_x = 240.0f; aim_t = 0.0f;
        reset_pins();
        round_over_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (frames_played >= 4) {
                    if (p1_pins > p2_pins) match_winner = 1;
                    else match_winner = 2;
                } else {
                    turn = (turn == 1) ? 2 : 1;
                    ball_rolling = false;
                    reset_pins();
                }
            }
            return;
        }

        if (!ball_rolling) {
            aim_t += 0.06f;
            aim_x = 240.0f + sinf(aim_t) * 35.0f;

            bool roll = (turn == 1) ? pad.p1.action : (mode == MODE_2P_LOCAL ? pad.p2.action : (rand() % 35 == 0));
            if (roll) {
                ball_rolling = true;
                ball_x = aim_x;
                ball_y = 210.0f;
                ball_vx = (aim_x - 240.0f) * 0.04f;
                ball_vy = -4.5f;
                play_tone(380.0f, 40);
            }
        } else {
            ball_x += ball_vx;
            ball_y += ball_vy;

            // Pin deck collision at y = 70
            if (ball_y <= 70.0f) {
                int knocked = 0;
                for (int i = 0; i < 10; i++) {
                    if (pins[i] && fabsf(ball_x - 240.0f) < 40.0f) {
                        if (rand() % 100 < 80) { pins[i] = false; knocked++; }
                    }
                }
                if (turn == 1) p1_pins += knocked;
                else p2_pins += knocked;

                frames_played++;
                round_over_timer = 50;
                play_tone(640.0f, 150, 1);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Perspective Lane
        set_color(ren, Color(175, 130, 80));
        SDL_Rect lane = {190, 50, 100, 175};
        SDL_RenderFillRect(ren, &lane);
        set_color(ren, Color(90, 50, 20));
        SDL_RenderDrawRect(ren, &lane);

        // Pins (10-pin triangle)
        int pin_positions[10][2] = {
            {240, 55},
            {233, 62}, {247, 62},
            {226, 69}, {240, 69}, {254, 69},
            {219, 76}, {233, 76}, {247, 76}, {261, 76}
        };
        for (int i = 0; i < 10; i++) {
            if (pins[i]) {
                draw_circle(ren, pin_positions[i][0], pin_positions[i][1], 4, COL_WHITE);
                draw_circle(ren, pin_positions[i][0], pin_positions[i][1], 2, COL_RED);
            }
        }

        // Aim indicator / Ball
        if (!ball_rolling) {
            draw_circle(ren, (int)aim_x, 210, 8, (turn == 1 ? COL_BLUE : COL_RED));
            draw_circle(ren, (int)aim_x, 210, 8, COL_WHITE, false);
        } else {
            draw_circle(ren, (int)ball_x, (int)ball_y, 8, (turn == 1 ? COL_BLUE : COL_RED));
            draw_circle(ren, (int)ball_x, (int)ball_y, 8, COL_WHITE, false);
        }

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d PINS   vs   P2: %d PINS", p1_pins, p2_pins);
        draw_centered_str(ren, SCR_W / 2, 20, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, (turn == 1 ? "ROLL [L]" : "WAITING"), COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, (turn == 2 ? "ROLL [R]" : "WAITING"), COL_RED, 1);
        }
    }
};

/* ─── 24. DARTS (TARGET 301 DUEL) ───────────────────────────────────── */

struct DartsGame {
    int p1_score, p2_score;
    float crosshair_x, crosshair_y, ch_t;
    bool locked_x;
    int turn; // 1 or 2
    int round_over_timer;
    bool prev_act;

    const int BOARD_X = 240, BOARD_Y = 115, BOARD_R = 60;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 301; p2_score = 301; }
        turn = 1;
        crosshair_x = 240.0f; crosshair_y = 115.0f; ch_t = 0.0f;
        locked_x = false;
        round_over_timer = 0;
        prev_act = true;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score == 0) match_winner = 1;
                else if (p2_score == 0) match_winner = 2;
                else {
                    turn = (turn == 1) ? 2 : 1;
                    locked_x = false;
                    prev_act = true;
                }
            }
            return;
        }

        ch_t += 0.06f;
        if (!locked_x) {
            crosshair_x = BOARD_X + sinf(ch_t) * 45.0f;
        } else {
            crosshair_y = BOARD_Y + cosf(ch_t * 1.3f) * 45.0f;
        }

        bool act = (turn == 1) ? pad.p1.action : (mode == MODE_2P_LOCAL ? pad.p2.action : (rand() % 30 == 0));
        if (act && !prev_act) {
            if (!locked_x) {
                locked_x = true;
                play_tone(450.0f, 25);
            } else {
                // Dart lands!
                float dist = hypotf(crosshair_x - BOARD_X, crosshair_y - BOARD_Y);
                int points = 0;
                if (dist < 8.0f) points = 50; // Bullseye!
                else if (dist < 20.0f) points = 25;
                else if (dist < BOARD_R) points = 10 + rand() % 20;

                if (turn == 1) {
                    if (p1_score - points >= 0) p1_score -= points;
                } else {
                    if (p2_score - points >= 0) p2_score -= points;
                }

                round_over_timer = 45;
                play_tone(660.0f, 60);
            }
        }
        prev_act = act;
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        // Dartboard rings
        draw_circle(ren, BOARD_X, BOARD_Y, BOARD_R + 6, Color(20, 20, 20));
        draw_circle(ren, BOARD_X, BOARD_Y, BOARD_R, Color(220, 215, 185));
        draw_circle(ren, BOARD_X, BOARD_Y, BOARD_R, COL_BORDER, false);
        draw_circle(ren, BOARD_X, BOARD_Y, BOARD_R - 15, Color(180, 40, 40), false);
        draw_circle(ren, BOARD_X, BOARD_Y, 20, Color(40, 140, 60), false);
        draw_circle(ren, BOARD_X, BOARD_Y, 8, Color(200, 30, 30)); // Bullseye

        // Targeting Crosshair
        draw_circle(ren, (int)crosshair_x, (int)crosshair_y, 6, COL_GOLD, false);
        SDL_RenderDrawLine(ren, (int)crosshair_x - 4, (int)crosshair_y, (int)crosshair_x + 4, (int)crosshair_y);
        SDL_RenderDrawLine(ren, (int)crosshair_x, (int)crosshair_y - 4, (int)crosshair_x, (int)crosshair_y + 4);

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d   vs   P2: %d", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 20, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, (turn == 1 ? "LOCK [L]" : "WAITING"), COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, (turn == 2 ? "LOCK [R]" : "WAITING"), COL_RED, 1);
        } else {
            draw_str(ren, 12, 12, "AIM: PRESS [L] TO LOCK AXES", COL_GOLD, 1);
        }
    }
};
