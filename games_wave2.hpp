#pragma once
#include <SDL.h>
#include <vector>
#include <tuple>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <cmath>

/* ─── 7. TIC-TAC-TOE (TRIS) ─────────────────────────────────────────── */

struct TicTacToeGame {
    int board[3][3]; // 0=empty, 1=P1 Blue, 2=P2 Red
    int turn; // 1 or 2
    int cursor_r, cursor_c;
    int p1_score, p2_score;
    int input_delay;
    int ai_delay;
    int win_line; // 0=none, 1..3=rows, 4..6=cols, 7=diag1, 8=diag2
    int round_over_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) board[r][c] = 0;
        turn = 1;
        cursor_r = 1; cursor_c = 1;
        input_delay = 0;
        ai_delay = 0;
        win_line = 0;
        round_over_timer = 0;
    }

    int check_winner(int &line) {
        line = 0;
        for (int r = 0; r < 3; r++) {
            if (board[r][0] != 0 && board[r][0] == board[r][1] && board[r][1] == board[r][2]) {
                line = 1 + r;
                return board[r][0];
            }
        }
        for (int c = 0; c < 3; c++) {
            if (board[0][c] != 0 && board[0][c] == board[1][c] && board[1][c] == board[2][c]) {
                line = 4 + c;
                return board[0][c];
            }
        }
        if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
            line = 7;
            return board[0][0];
        }
        if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
            line = 8;
            return board[0][2];
        }
        return 0;
    }

    bool is_full() {
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                if (board[r][c] == 0) return false;
        return true;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (input_delay > 0) input_delay--;

        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 3) { match_winner = 1; return; }
                if (p2_score >= 3) { match_winner = 2; return; }
                reset(false);
            }
            return;
        }

        if (mode == MODE_1P_VS_AI && turn == 2) {
            if (ai_delay < 20) { ai_delay++; return; }
            ai_delay = 0;

            int best_r = -1, best_c = -1;
            for (int r = 0; r < 3 && best_r == -1; r++) {
                for (int c = 0; c < 3; c++) {
                    if (board[r][c] == 0) {
                        board[r][c] = 2;
                        int l = 0;
                        if (check_winner(l) == 2) { best_r = r; best_c = c; }
                        board[r][c] = 0;
                    }
                }
            }
            if (best_r == -1) {
                for (int r = 0; r < 3 && best_r == -1; r++) {
                    for (int c = 0; c < 3; c++) {
                        if (board[r][c] == 0) {
                            board[r][c] = 1;
                            int l = 0;
                            if (check_winner(l) == 1) { best_r = r; best_c = c; }
                            board[r][c] = 0;
                        }
                    }
                }
            }
            if (best_r == -1 && board[1][1] == 0) { best_r = 1; best_c = 1; }
            if (best_r == -1) {
                std::vector<std::pair<int,int>> opts;
                for (int r = 0; r < 3; r++)
                    for (int c = 0; c < 3; c++)
                        if (board[r][c] == 0) opts.push_back({r, c});
                if (!opts.empty()) {
                    auto pick = opts[rand() % opts.size()];
                    best_r = pick.first; best_c = pick.second;
                }
            }

            if (best_r != -1) {
                board[best_r][best_c] = 2;
                play_tone(420.0f, 30);
                int l = 0;
                int w = check_winner(l);
                if (w == 2) {
                    p2_score++;
                    win_line = l;
                    round_over_timer = 50;
                    play_tone(350.0f, 150);
                } else if (is_full()) {
                    round_over_timer = 40;
                    play_tone(220.0f, 60);
                } else {
                    turn = 1;
                }
            }
            return;
        }

        bool u = false, d = false, l = false, r = false, act = false;
        if (turn == 1) {
            u = pad.p1.up; d = pad.p1.down; l = pad.p1.left; r = pad.p1.right; act = pad.p1.action;
        } else {
            u = pad.p2.up; d = pad.p2.down; l = pad.p2.left; r = pad.p2.right; act = pad.p2.action;
        }

        if (input_delay == 0) {
            if (u) { cursor_r = (cursor_r + 2) % 3; input_delay = 10; play_tone(350.0f, 15); }
            else if (d) { cursor_r = (cursor_r + 1) % 3; input_delay = 10; play_tone(350.0f, 15); }
            else if (l) { cursor_c = (cursor_c + 2) % 3; input_delay = 10; play_tone(350.0f, 15); }
            else if (r) { cursor_c = (cursor_c + 1) % 3; input_delay = 10; play_tone(350.0f, 15); }
        }

        if (act && input_delay == 0) {
            input_delay = 14;
            if (board[cursor_r][cursor_c] == 0) {
                board[cursor_r][cursor_c] = turn;
                play_tone(turn == 1 ? 520.0f : 420.0f, 30);
                int l = 0;
                int w = check_winner(l);
                if (w != 0) {
                    if (w == 1) p1_score++; else p2_score++;
                    win_line = l;
                    round_over_timer = 50;
                    play_tone(w == 1 ? 750.0f : 350.0f, 150);
                } else if (is_full()) {
                    round_over_timer = 40;
                    play_tone(220.0f, 60);
                } else {
                    turn = (turn == 1) ? 2 : 1;
                }
            } else {
                play_tone(180.0f, 25);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        int bx = 168, by = 40;
        int sz = 48;

        SDL_Rect frame = {bx - 4, by - 4, sz * 3 + 8, sz * 3 + 8};
        set_color(ren, Color(22, 28, 48));
        SDL_RenderFillRect(ren, &frame);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &frame);

        set_color(ren, Color(50, 65, 105));
        SDL_RenderDrawLine(ren, bx + sz, by, bx + sz, by + sz * 3);
        SDL_RenderDrawLine(ren, bx + sz * 2, by, bx + sz * 2, by + sz * 3);
        SDL_RenderDrawLine(ren, bx, by + sz, bx + sz * 3, by + sz);
        SDL_RenderDrawLine(ren, bx, by + sz * 2, bx + sz * 3, by + sz * 2);

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                int cx = bx + c * sz + sz / 2;
                int cy = by + r * sz + sz / 2;
                int val = board[r][c];
                if (val == 1) {
                    set_color(ren, COL_BLUE);
                    for (int w = -1; w <= 1; w++) {
                        SDL_RenderDrawLine(ren, cx - 14 + w, cy - 14, cx + 14 + w, cy + 14);
                        SDL_RenderDrawLine(ren, cx - 14 + w, cy + 14, cx + 14 + w, cy - 14);
                    }
                } else if (val == 2) {
                    draw_circle(ren, cx, cy, 15, COL_RED, false);
                    draw_circle(ren, cx, cy, 14, COL_RED, false);
                }
            }
        }

        if (round_over_timer == 0) {
            SDL_Rect cur = {bx + cursor_c * sz + 3, by + cursor_r * sz + 3, sz - 6, sz - 6};
            set_color(ren, COL_GOLD);
            SDL_RenderDrawRect(ren, &cur);
        }

        if (win_line != 0) {
            set_color(ren, COL_GOLD);
            if (win_line >= 1 && win_line <= 3) {
                int r = win_line - 1;
                int cy = by + r * sz + sz / 2;
                for (int w = -1; w <= 1; w++)
                    SDL_RenderDrawLine(ren, bx + 6, cy + w, bx + sz * 3 - 6, cy + w);
            } else if (win_line >= 4 && win_line <= 6) {
                int c = win_line - 4;
                int cx = bx + c * sz + sz / 2;
                for (int w = -1; w <= 1; w++)
                    SDL_RenderDrawLine(ren, cx + w, by + 6, cx + w, by + sz * 3 - 6);
            } else if (win_line == 7) {
                for (int w = -1; w <= 1; w++)
                    SDL_RenderDrawLine(ren, bx + 6 + w, by + 6, bx + sz * 3 - 6 + w, by + sz * 3 - 6);
            } else if (win_line == 8) {
                for (int w = -1; w <= 1; w++)
                    SDL_RenderDrawLine(ren, bx + sz * 3 - 6 + w, by + 6, bx + 6 + w, by + sz * 3 - 6);
            }
        }

        char c1buf[16], c2buf[16];
        snprintf(c1buf, sizeof(c1buf), "WINS: %d/3", p1_score);
        snprintf(c2buf, sizeof(c2buf), "WINS: %d/3", p2_score);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 20, SCR_H / 2, "PLAYER 1 (X)", COL_BLUE, 1);
            draw_centered_str_p1(ren, 45, SCR_H / 2, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p1(ren, 75, SCR_H / 2, c1buf, COL_WHITE, 1);

            draw_centered_str_p2(ren, SCR_W - 20, SCR_H / 2, "PLAYER 2 (O)", COL_RED, 1);
            draw_centered_str_p2(ren, SCR_W - 45, SCR_H / 2, (turn == 2 ? "YOUR TURN" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p2(ren, SCR_W - 75, SCR_H / 2, c2buf, COL_WHITE, 1);
        } else {
            draw_str(ren, 14, 45, "PLAYER 1 (X)", COL_BLUE, 1);
            draw_str(ren, 14, 65, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, 14, 95, c1buf, COL_WHITE, 1);

            draw_str(ren, SCR_W - 116, 45, "CPU (O)", COL_RED, 1);
            draw_str(ren, SCR_W - 116, 65, (turn == 2 ? "THINKING" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, SCR_W - 116, 95, c2buf, COL_WHITE, 1);
        }
    }
};

/* ─── 8. CONNECT 4 (FOUR IN A ROW) ──────────────────────────────────── */

struct Connect4Game {
    int grid[6][7]; // 0=empty, 1=Blue, 2=Red
    int turn;
    int cursor_col;
    int p1_score, p2_score;
    int input_delay;
    int ai_delay;
    int round_over_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        for (int r = 0; r < 6; r++)
            for (int c = 0; c < 7; c++) grid[r][c] = 0;
        turn = 1;
        cursor_col = 3;
        input_delay = 0;
        ai_delay = 0;
        round_over_timer = 0;
    }

    int check_win() {
        for (int r = 0; r < 6; r++) {
            for (int c = 0; c <= 3; c++) {
                int p = grid[r][c];
                if (p != 0 && p == grid[r][c+1] && p == grid[r][c+2] && p == grid[r][c+3]) return p;
            }
        }
        for (int c = 0; c < 7; c++) {
            for (int r = 0; r <= 2; r++) {
                int p = grid[r][c];
                if (p != 0 && p == grid[r+1][c] && p == grid[r+2][c] && p == grid[r+3][c]) return p;
            }
        }
        for (int r = 0; r <= 2; r++) {
            for (int c = 0; c <= 3; c++) {
                int p = grid[r][c];
                if (p != 0 && p == grid[r+1][c+1] && p == grid[r+2][c+2] && p == grid[r+3][c+3]) return p;
            }
        }
        for (int r = 3; r < 6; r++) {
            for (int c = 0; c <= 3; c++) {
                int p = grid[r][c];
                if (p != 0 && p == grid[r-1][c+1] && p == grid[r-2][c+2] && p == grid[r-3][c+3]) return p;
            }
        }
        return 0;
    }

    bool is_full() {
        for (int c = 0; c < 7; c++) if (grid[0][c] == 0) return false;
        return true;
    }

    int get_drop_row(int col) {
        for (int r = 5; r >= 0; r--) {
            if (grid[r][col] == 0) return r;
        }
        return -1;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (input_delay > 0) input_delay--;

        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score >= 3) { match_winner = 1; return; }
                if (p2_score >= 3) { match_winner = 2; return; }
                reset(false);
            }
            return;
        }

        if (mode == MODE_1P_VS_AI && turn == 2) {
            if (ai_delay < 25) { ai_delay++; return; }
            ai_delay = 0;

            int best_col = -1;
            for (int c = 0; c < 7 && best_col == -1; c++) {
                int r = get_drop_row(c);
                if (r != -1) {
                    grid[r][c] = 2;
                    if (check_win() == 2) best_col = c;
                    grid[r][c] = 0;
                }
            }
            if (best_col == -1) {
                for (int c = 0; c < 7 && best_col == -1; c++) {
                    int r = get_drop_row(c);
                    if (r != -1) {
                        grid[r][c] = 1;
                        if (check_win() == 1) best_col = c;
                        grid[r][c] = 0;
                    }
                }
            }
            if (best_col == -1 && get_drop_row(3) != -1) {
                best_col = 3;
            }
            if (best_col == -1) {
                std::vector<int> valids;
                for (int c = 0; c < 7; c++) {
                    int r = get_drop_row(c);
                    if (r != -1) {
                        if (r > 0) {
                            grid[r-1][c] = 1;
                            bool gives_win = (check_win() == 1);
                            grid[r-1][c] = 0;
                            if (!gives_win) valids.push_back(c);
                        } else valids.push_back(c);
                    }
                }
                if (!valids.empty()) best_col = valids[rand() % valids.size()];
                else {
                    for (int c = 0; c < 7; c++) if (get_drop_row(c) != -1) { best_col = c; break; }
                }
            }

            if (best_col != -1) {
                int r = get_drop_row(best_col);
                grid[r][best_col] = 2;
                play_tone(380.0f, 35);
                int w = check_win();
                if (w == 2) {
                    p2_score++;
                    round_over_timer = 50;
                    play_tone(320.0f, 150);
                } else if (is_full()) {
                    round_over_timer = 40;
                    play_tone(220.0f, 60);
                } else {
                    turn = 1;
                }
            }
            return;
        }

        bool l = false, r = false, act = false;
        if (turn == 1) {
            l = pad.p1.left; r = pad.p1.right; act = pad.p1.action;
        } else {
            l = pad.p2.left; r = pad.p2.right; act = pad.p2.action;
        }

        if (input_delay == 0) {
            if (l) { cursor_col = (cursor_col + 6) % 7; input_delay = 10; play_tone(360.0f, 15); }
            else if (r) { cursor_col = (cursor_col + 1) % 7; input_delay = 10; play_tone(360.0f, 15); }
        }

        if (act && input_delay == 0) {
            input_delay = 14;
            int drop_r = get_drop_row(cursor_col);
            if (drop_r != -1) {
                grid[drop_r][cursor_col] = turn;
                play_tone(turn == 1 ? 520.0f : 400.0f, 35);
                int w = check_win();
                if (w != 0) {
                    if (w == 1) p1_score++; else p2_score++;
                    round_over_timer = 50;
                    play_tone(w == 1 ? 750.0f : 350.0f, 150);
                } else if (is_full()) {
                    round_over_timer = 40;
                    play_tone(220.0f, 60);
                } else {
                    turn = (turn == 1) ? 2 : 1;
                }
            } else {
                play_tone(180.0f, 25);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        int bx = 135, by = 48;
        int col_w = 30, row_h = 32;

        SDL_Rect frame = {bx - 6, by - 6, col_w * 7 + 12, row_h * 6 + 12};
        set_color(ren, Color(24, 60, 130));
        SDL_RenderFillRect(ren, &frame);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &frame);

        if (round_over_timer == 0) {
            int cx = bx + cursor_col * col_w + col_w / 2;
            int cy = by - 14;
            Color cur_col = (turn == 1) ? COL_BLUE : COL_RED;
            draw_circle(ren, cx, cy, 9, cur_col);
            draw_circle(ren, cx, cy, 9, COL_WHITE, false);
            set_color(ren, COL_GOLD);
            SDL_RenderDrawLine(ren, cx, cy + 9, cx, cy + 13);
            SDL_RenderDrawLine(ren, cx - 3, cy + 10, cx, cy + 13);
            SDL_RenderDrawLine(ren, cx + 3, cy + 10, cx, cy + 13);
        }

        for (int r = 0; r < 6; r++) {
            for (int c = 0; c < 7; c++) {
                int cx = bx + c * col_w + col_w / 2;
                int cy = by + r * row_h + row_h / 2;
                int val = grid[r][c];
                if (val == 0) {
                    draw_circle(ren, cx, cy, 11, Color(14, 18, 36));
                    draw_circle(ren, cx, cy, 11, Color(16, 40, 90), false);
                } else {
                    Color token_col = (val == 1) ? COL_BLUE : COL_RED;
                    draw_circle(ren, cx, cy, 11, token_col);
                    draw_circle(ren, cx, cy, 11, COL_WHITE, false);
                    draw_circle(ren, cx, cy, 6, Color(255, 255, 255), false);
                }
            }
        }

        char c1buf[16], c2buf[16];
        snprintf(c1buf, sizeof(c1buf), "WINS: %d/3", p1_score);
        snprintf(c2buf, sizeof(c2buf), "WINS: %d/3", p2_score);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 20, SCR_H / 2, "PLAYER 1", COL_BLUE, 1);
            draw_centered_str_p1(ren, 45, SCR_H / 2, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p1(ren, 75, SCR_H / 2, c1buf, COL_WHITE, 1);

            draw_centered_str_p2(ren, SCR_W - 20, SCR_H / 2, "PLAYER 2", COL_RED, 1);
            draw_centered_str_p2(ren, SCR_W - 45, SCR_H / 2, (turn == 2 ? "YOUR TURN" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p2(ren, SCR_W - 75, SCR_H / 2, c2buf, COL_WHITE, 1);
        } else {
            draw_str(ren, 12, 45, "PLAYER 1", COL_BLUE, 1);
            draw_str(ren, 12, 65, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, 12, 95, c1buf, COL_WHITE, 1);

            draw_str(ren, SCR_W - 110, 45, "CPU (AI)", COL_RED, 1);
            draw_str(ren, SCR_W - 110, 65, (turn == 2 ? "THINKING" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, SCR_W - 110, 95, c2buf, COL_WHITE, 1);
        }
    }
};

/* ─── 9. DOTS & BOXES ───────────────────────────────────────────────── */

struct DotsBoxesGame {
    bool h_lines[4][3];
    bool v_lines[3][4];
    int boxes[3][3];
    int turn;
    bool cur_is_vert;
    int cur_r, cur_c;
    int p1_score, p2_score;
    int input_delay;
    int ai_delay;
    int round_over_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 3; c++) h_lines[r][c] = false;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 4; c++) v_lines[r][c] = false;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) boxes[r][c] = 0;
        turn = 1;
        cur_is_vert = false;
        cur_r = 0; cur_c = 0;
        input_delay = 0;
        ai_delay = 0;
        round_over_timer = 0;
    }

    int check_and_capture_boxes(int player) {
        int newly_captured = 0;
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (boxes[r][c] == 0) {
                    bool top    = h_lines[r][c];
                    bool bottom = h_lines[r + 1][c];
                    bool left   = v_lines[r][c];
                    bool right  = v_lines[r][c + 1];
                    if (top && bottom && left && right) {
                        boxes[r][c] = player;
                        newly_captured++;
                    }
                }
            }
        }
        return newly_captured;
    }

    bool is_board_complete() {
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                if (boxes[r][c] == 0) return false;
        return true;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (input_delay > 0) input_delay--;

        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (p1_score > p2_score) match_winner = 1;
                else if (p2_score > p1_score) match_winner = 2;
                else reset(false);
            }
            return;
        }

        if (mode == MODE_1P_VS_AI && turn == 2) {
            if (ai_delay < 25) { ai_delay++; return; }
            ai_delay = 0;

            bool found_vert = false;
            int found_r = -1, found_c = -1;

            for (int r = 0; r < 4 && found_r == -1; r++) {
                for (int c = 0; c < 3; c++) {
                    if (!h_lines[r][c]) {
                        h_lines[r][c] = true;
                        if (check_and_capture_boxes(2) > 0) {
                            found_vert = false; found_r = r; found_c = c;
                        }
                        h_lines[r][c] = false;
                        for (int br = 0; br < 3; br++) for (int bc = 0; bc < 3; bc++) if (boxes[br][bc] == 2) boxes[br][bc] = 0;
                    }
                }
            }
            for (int r = 0; r < 3 && found_r == -1; r++) {
                for (int c = 0; c < 4; c++) {
                    if (!v_lines[r][c]) {
                        v_lines[r][c] = true;
                        if (check_and_capture_boxes(2) > 0) {
                            found_vert = true; found_r = r; found_c = c;
                        }
                        v_lines[r][c] = false;
                        for (int br = 0; br < 3; br++) for (int bc = 0; bc < 3; bc++) if (boxes[br][bc] == 2) boxes[br][bc] = 0;
                    }
                }
            }

            if (found_r == -1) {
                std::vector<std::tuple<bool,int,int>> opts;
                for (int r = 0; r < 4; r++)
                    for (int c = 0; c < 3; c++)
                        if (!h_lines[r][c]) opts.push_back(std::make_tuple(false, r, c));
                for (int r = 0; r < 3; r++)
                    for (int c = 0; c < 4; c++)
                        if (!v_lines[r][c]) opts.push_back(std::make_tuple(true, r, c));
                if (!opts.empty()) {
                    auto pick = opts[rand() % opts.size()];
                    found_vert = std::get<0>(pick);
                    found_r = std::get<1>(pick);
                    found_c = std::get<2>(pick);
                }
            }

            if (found_r != -1) {
                if (found_vert) v_lines[found_r][found_c] = true;
                else h_lines[found_r][found_c] = true;

                play_tone(420.0f, 30);
                int caps = check_and_capture_boxes(2);
                if (caps > 0) {
                    p2_score += caps;
                    play_tone(540.0f, 60);
                    if (is_board_complete()) {
                        round_over_timer = 50;
                        play_tone(350.0f, 150);
                    }
                } else {
                    turn = 1;
                }
            }
            return;
        }

        bool u = false, d = false, l = false, r = false, act = false;
        if (turn == 1) {
            u = pad.p1.up; d = pad.p1.down; l = pad.p1.left; r = pad.p1.right; act = pad.p1.action;
        } else {
            u = pad.p2.up; d = pad.p2.down; l = pad.p2.left; r = pad.p2.right; act = pad.p2.action;
        }

        if (input_delay == 0) {
            if (u) {
                if (cur_is_vert) {
                    if (cur_r > 0) cur_r--;
                    else { cur_is_vert = false; cur_r = 0; if (cur_c > 2) cur_c = 2; }
                } else {
                    if (cur_r > 0) cur_r--;
                    else { cur_is_vert = true; cur_r = 0; }
                }
                input_delay = 10; play_tone(350.0f, 15);
            } else if (d) {
                if (!cur_is_vert) {
                    if (cur_r < 3) cur_r++;
                    else { cur_is_vert = true; cur_r = 2; }
                } else {
                    if (cur_r < 2) cur_r++;
                    else { cur_is_vert = false; cur_r = 3; if (cur_c > 2) cur_c = 2; }
                }
                input_delay = 10; play_tone(350.0f, 15);
            } else if (l) {
                if (cur_c > 0) cur_c--;
                else cur_is_vert = !cur_is_vert;
                input_delay = 10; play_tone(350.0f, 15);
            } else if (r) {
                int max_c = cur_is_vert ? 3 : 2;
                if (cur_c < max_c) cur_c++;
                else cur_is_vert = !cur_is_vert;
                input_delay = 10; play_tone(350.0f, 15);
            }
        }

        if (act && input_delay == 0) {
            input_delay = 14;
            bool already = cur_is_vert ? v_lines[cur_r][cur_c] : h_lines[cur_r][cur_c];
            if (!already) {
                if (cur_is_vert) v_lines[cur_r][cur_c] = true;
                else h_lines[cur_r][cur_c] = true;

                play_tone(turn == 1 ? 520.0f : 420.0f, 30);
                int caps = check_and_capture_boxes(turn);
                if (caps > 0) {
                    if (turn == 1) p1_score += caps; else p2_score += caps;
                    play_tone(640.0f, 60);
                    if (is_board_complete()) {
                        round_over_timer = 50;
                        play_tone(turn == 1 ? 750.0f : 350.0f, 150);
                    }
                } else {
                    turn = (turn == 1) ? 2 : 1;
                }
            } else {
                play_tone(180.0f, 25);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        int bx = 165, by = 50;
        int step = 50;

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                int owner = boxes[r][c];
                if (owner != 0) {
                    SDL_Rect rc = {bx + c * step + 4, by + r * step + 4, step - 8, step - 8};
                    Color bcol = (owner == 1) ? Color(20, 60, 110) : Color(110, 25, 35);
                    set_color(ren, bcol);
                    SDL_RenderFillRect(ren, &rc);
                    draw_centered_str(ren, bx + c * step + step / 2, by + r * step + step / 2 - 4, (owner == 1 ? "P1" : "P2"), COL_WHITE, 1);
                }
            }
        }

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 3; c++) {
                int x1 = bx + c * step, x2 = bx + (c + 1) * step;
                int y = by + r * step;
                if (h_lines[r][c]) {
                    set_color(ren, COL_WHITE);
                    SDL_RenderDrawLine(ren, x1, y - 1, x2, y - 1);
                    SDL_RenderDrawLine(ren, x1, y,     x2, y);
                    SDL_RenderDrawLine(ren, x1, y + 1, x2, y + 1);
                }
            }
        }

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 4; c++) {
                int y1 = by + r * step, y2 = by + (r + 1) * step;
                int x = bx + c * step;
                if (v_lines[r][c]) {
                    set_color(ren, COL_WHITE);
                    SDL_RenderDrawLine(ren, x - 1, y1, x - 1, y2);
                    SDL_RenderDrawLine(ren, x,     y1, x,     y2);
                    SDL_RenderDrawLine(ren, x + 1, y1, x + 1, y2);
                }
            }
        }

        if (round_over_timer == 0) {
            set_color(ren, COL_GOLD);
            if (!cur_is_vert) {
                int x1 = bx + cur_c * step, x2 = bx + (cur_c + 1) * step;
                int y = by + cur_r * step;
                SDL_RenderDrawLine(ren, x1, y - 2, x2, y - 2);
                SDL_RenderDrawLine(ren, x1, y + 2, x2, y + 2);
            } else {
                int y1 = by + cur_r * step, y2 = by + (cur_r + 1) * step;
                int x = bx + cur_c * step;
                SDL_RenderDrawLine(ren, x - 2, y1, x - 2, y2);
                SDL_RenderDrawLine(ren, x + 2, y1, x + 2, y2);
            }
        }

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                draw_circle(ren, bx + c * step, by + r * step, 4, COL_GOLD);
                draw_circle(ren, bx + c * step, by + r * step, 2, COL_WHITE);
            }
        }

        char c1buf[16], c2buf[16];
        snprintf(c1buf, sizeof(c1buf), "BOXES: %d", p1_score);
        snprintf(c2buf, sizeof(c2buf), "BOXES: %d", p2_score);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 20, SCR_H / 2, "PLAYER 1", COL_BLUE, 1);
            draw_centered_str_p1(ren, 45, SCR_H / 2, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p1(ren, 75, SCR_H / 2, c1buf, COL_WHITE, 1);

            draw_centered_str_p2(ren, SCR_W - 20, SCR_H / 2, "PLAYER 2", COL_RED, 1);
            draw_centered_str_p2(ren, SCR_W - 45, SCR_H / 2, (turn == 2 ? "YOUR TURN" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p2(ren, SCR_W - 75, SCR_H / 2, c2buf, COL_WHITE, 1);
        } else {
            draw_str(ren, 12, 45, "PLAYER 1", COL_BLUE, 1);
            draw_str(ren, 12, 65, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, 12, 95, c1buf, COL_WHITE, 1);

            draw_str(ren, SCR_W - 110, 45, "CPU (AI)", COL_RED, 1);
            draw_str(ren, SCR_W - 110, 65, (turn == 2 ? "THINKING" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, SCR_W - 110, 95, c2buf, COL_WHITE, 1);
        }
    }
};

/* ─── 10. REVERSI (OTHELLO) ─────────────────────────────────────────── */

struct ReversiGame {
    int board[8][8];
    int turn;
    int cursor_r, cursor_c;
    int p1_discs, p2_discs;
    int input_delay;
    int ai_delay;
    int round_over_timer;

    void reset(bool reset_match = true) {
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++) board[r][c] = 0;
        board[3][3] = 2; board[3][4] = 1;
        board[4][3] = 1; board[4][4] = 2;
        turn = 1;
        cursor_r = 3; cursor_c = 2;
        p1_discs = 2; p2_discs = 2;
        input_delay = 0;
        ai_delay = 0;
        round_over_timer = 0;
    }

    int count_flips(int r, int c, int player, bool do_flip = false) {
        if (board[r][c] != 0) return 0;
        int opp = (player == 1) ? 2 : 1;
        int total_flips = 0;
        int dirs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};

        for (int d = 0; d < 8; d++) {
            int dr = dirs[d][0], dc = dirs[d][1];
            int step = 1;
            while (true) {
                int nr = r + dr * step, nc = c + dc * step;
                if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) break;
                if (board[nr][nc] == opp) { step++; continue; }
                if (board[nr][nc] == player) {
                    if (step > 1) {
                        int flips = step - 1;
                        total_flips += flips;
                        if (do_flip) {
                            for (int s = 1; s < step; s++) {
                                board[r + dr * s][c + dc * s] = player;
                            }
                        }
                    }
                }
                break;
            }
        }
        if (do_flip && total_flips > 0) board[r][c] = player;
        return total_flips;
    }

    bool has_any_move(int player) {
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++)
                if (count_flips(r, c, player, false) > 0) return true;
        return false;
    }

    void tally_discs() {
        p1_discs = 0; p2_discs = 0;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (board[r][c] == 1) p1_discs++;
                else if (board[r][c] == 2) p2_discs++;
            }
        }
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (input_delay > 0) input_delay--;

        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                tally_discs();
                if (p1_discs > p2_discs) match_winner = 1;
                else if (p2_discs > p1_discs) match_winner = 2;
                else reset(false);
            }
            return;
        }

        tally_discs();

        if (mode == MODE_1P_VS_AI && turn == 2) {
            if (ai_delay < 25) { ai_delay++; return; }
            ai_delay = 0;

            if (!has_any_move(2)) {
                if (!has_any_move(1)) round_over_timer = 50;
                else turn = 1;
                return;
            }

            int best_score = -99999;
            int best_r = -1, best_c = -1;
            const int W_REV[8][8] = {
                {100, -25,  10,   5,   5,  10, -25, 100},
                {-25, -45,  -3,  -3,  -3,  -3, -45, -25},
                { 10,  -3,   5,   1,   1,   5,  -3,  10},
                {  5,  -3,   1,   1,   1,   1,  -3,   5},
                {  5,  -3,   1,   1,   1,   1,  -3,   5},
                { 10,  -3,   5,   1,   1,   5,  -3,  10},
                {-25, -45,  -3,  -3,  -3,  -3, -45, -25},
                {100, -25,  10,   5,   5,  10, -25, 100}
            };

            for (int r = 0; r < 8; r++) {
                for (int c = 0; c < 8; c++) {
                    int flips = count_flips(r, c, 2, false);
                    if (flips > 0) {
                        int score = flips * 2 + W_REV[r][c];
                        if (score > best_score) {
                            best_score = score;
                            best_r = r; best_c = c;
                        }
                    }
                }
            }

            if (best_r != -1) {
                count_flips(best_r, best_c, 2, true);
                play_tone(420.0f, 35);
                tally_discs();
                if (!has_any_move(1)) {
                    if (!has_any_move(2)) round_over_timer = 50;
                } else {
                    turn = 1;
                }
            }
            return;
        }

        if (!has_any_move(turn)) {
            int other = (turn == 1) ? 2 : 1;
            if (!has_any_move(other)) round_over_timer = 50;
            else turn = other;
            return;
        }

        bool u = false, d = false, l = false, r = false, act = false;
        if (turn == 1) {
            u = pad.p1.up; d = pad.p1.down; l = pad.p1.left; r = pad.p1.right; act = pad.p1.action;
        } else {
            u = pad.p2.up; d = pad.p2.down; l = pad.p2.left; r = pad.p2.right; act = pad.p2.action;
        }

        if (input_delay == 0) {
            if (u) { cursor_r = (cursor_r + 7) % 8; input_delay = 9; play_tone(340.0f, 15); }
            else if (d) { cursor_r = (cursor_r + 1) % 8; input_delay = 9; play_tone(340.0f, 15); }
            else if (l) { cursor_c = (cursor_c + 7) % 8; input_delay = 9; play_tone(340.0f, 15); }
            else if (r) { cursor_c = (cursor_c + 1) % 8; input_delay = 9; play_tone(340.0f, 15); }
        }

        if (act && input_delay == 0) {
            input_delay = 14;
            int flips = count_flips(cursor_r, cursor_c, turn, true);
            if (flips > 0) {
                play_tone(turn == 1 ? 520.0f : 420.0f, 35);
                tally_discs();
                int next_p = (turn == 1) ? 2 : 1;
                if (!has_any_move(next_p)) {
                    if (!has_any_move(turn)) round_over_timer = 50;
                } else {
                    turn = next_p;
                }
            } else {
                play_tone(180.0f, 25);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        int bx = 136, by = 30;
        int sz = 26;

        SDL_Rect frame = {bx - 4, by - 4, sz * 8 + 8, sz * 8 + 8};
        set_color(ren, Color(22, 70, 40));
        SDL_RenderFillRect(ren, &frame);
        set_color(ren, COL_BORDER);
        SDL_RenderDrawRect(ren, &frame);

        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                SDL_Rect sq = {bx + c * sz, by + r * sz, sz, sz};
                set_color(ren, ((r + c) % 2 == 0) ? Color(28, 95, 52) : Color(25, 85, 46));
                SDL_RenderFillRect(ren, &sq);
                set_color(ren, Color(20, 60, 35));
                SDL_RenderDrawRect(ren, &sq);

                int val = board[r][c];
                int cx = bx + c * sz + sz / 2;
                int cy = by + r * sz + sz / 2;
                if (val != 0) {
                    Color dcol = (val == 1) ? COL_BLUE : COL_RED;
                    draw_circle(ren, cx, cy, 10, dcol);
                    draw_circle(ren, cx, cy, 10, COL_WHITE, false);
                } else if (count_flips(r, c, turn, false) > 0) {
                    draw_circle(ren, cx, cy, 3, COL_GOLD);
                }
            }
        }

        if (round_over_timer == 0) {
            SDL_Rect cur = {bx + cursor_c * sz - 1, by + cursor_r * sz - 1, sz + 2, sz + 2};
            set_color(ren, COL_GOLD);
            SDL_RenderDrawRect(ren, &cur);
        }

        char c1buf[16], c2buf[16];
        snprintf(c1buf, sizeof(c1buf), "DISCS: %d", p1_discs);
        snprintf(c2buf, sizeof(c2buf), "DISCS: %d", p2_discs);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 20, SCR_H / 2, "PLAYER 1", COL_BLUE, 1);
            draw_centered_str_p1(ren, 45, SCR_H / 2, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p1(ren, 75, SCR_H / 2, c1buf, COL_WHITE, 1);

            draw_centered_str_p2(ren, SCR_W - 20, SCR_H / 2, "PLAYER 2", COL_RED, 1);
            draw_centered_str_p2(ren, SCR_W - 45, SCR_H / 2, (turn == 2 ? "YOUR TURN" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_centered_str_p2(ren, SCR_W - 75, SCR_H / 2, c2buf, COL_WHITE, 1);
        } else {
            draw_str(ren, 12, 45, "PLAYER 1", COL_BLUE, 1);
            draw_str(ren, 12, 65, (turn == 1 ? "YOUR TURN" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, 12, 95, c1buf, COL_WHITE, 1);

            draw_str(ren, SCR_W - 110, 45, "CPU (AI)", COL_RED, 1);
            draw_str(ren, SCR_W - 110, 65, (turn == 2 ? "THINKING" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
            draw_str(ren, SCR_W - 110, 95, c2buf, COL_WHITE, 1);
        }
    }
};

/* ─── 11. BATTLESHIP (NAVAL FLEET SALVO) ────────────────────────────── */

struct BattleshipGame {
    int ships[2][6][6];
    int shots[2][6][6];
    int hits_remaining[2];
    int turn;
    int cursor_r, cursor_c;
    int input_delay;
    int ai_delay;
    int round_over_timer;

    void place_ship(int p, int len) {
        bool placed = false;
        int attempts = 0;
        while (!placed && attempts < 100) {
            attempts++;
            bool horiz = (rand() % 2 == 0);
            int r = rand() % 6;
            int c = rand() % 6;
            if (horiz && c + len <= 6) {
                bool clash = false;
                for (int i = 0; i < len; i++) if (ships[p][r][c + i] != 0) clash = true;
                if (!clash) {
                    for (int i = 0; i < len; i++) ships[p][r][c + i] = 1;
                    placed = true;
                }
            } else if (!horiz && r + len <= 6) {
                bool clash = false;
                for (int i = 0; i < len; i++) if (ships[p][r + i][c] != 0) clash = true;
                if (!clash) {
                    for (int i = 0; i < len; i++) ships[p][r + i][c] = 1;
                    placed = true;
                }
            }
        }
    }

    void reset(bool reset_match = true) {
        for (int p = 0; p < 2; p++) {
            for (int r = 0; r < 6; r++) {
                for (int c = 0; c < 6; c++) {
                    ships[p][r][c] = 0;
                    shots[p][r][c] = 0;
                }
            }
            place_ship(p, 3);
            place_ship(p, 2);
            place_ship(p, 2);
            hits_remaining[p] = 7;
        }
        turn = 1;
        cursor_r = 2; cursor_c = 2;
        input_delay = 0;
        ai_delay = 0;
        round_over_timer = 0;
    }

    void update(const GamePadState &pad, GameMode mode, int &match_winner) {
        if (input_delay > 0) input_delay--;

        if (round_over_timer > 0) {
            round_over_timer--;
            if (round_over_timer == 0) {
                if (hits_remaining[1] <= 0) match_winner = 1;
                else match_winner = 2;
            }
            return;
        }

        int target_idx = (turn == 1) ? 1 : 0;

        if (mode == MODE_1P_VS_AI && turn == 2) {
            if (ai_delay < 25) { ai_delay++; return; }
            ai_delay = 0;

            int best_r = -1, best_c = -1;
            int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
            for (int r = 0; r < 6 && best_r == -1; r++) {
                for (int c = 0; c < 6 && best_r == -1; c++) {
                    if (shots[0][r][c] == 2) {
                        for (int d = 0; d < 4; d++) {
                            int nr = r + dirs[d][0], nc = c + dirs[d][1];
                            if (nr >= 0 && nr < 6 && nc >= 0 && nc < 6 && shots[0][nr][nc] == 0) {
                                best_r = nr; best_c = nc;
                                break;
                            }
                        }
                    }
                }
            }

            if (best_r == -1) {
                std::vector<std::pair<int,int>> opts;
                for (int r = 0; r < 6; r++)
                    for (int c = 0; c < 6; c++)
                        if (shots[0][r][c] == 0 && (r + c) % 2 == 0) opts.push_back({r, c});
                if (!opts.empty()) {
                    auto pick = opts[rand() % opts.size()];
                    best_r = pick.first; best_c = pick.second;
                }
            }

            if (best_r == -1) {
                for (int r = 0; r < 6 && best_r == -1; r++)
                    for (int c = 0; c < 6; c++)
                        if (shots[0][r][c] == 0) { best_r = r; best_c = c; }
            }

            if (best_r != -1) {
                if (ships[0][best_r][best_c] == 1) {
                    shots[0][best_r][best_c] = 2;
                    hits_remaining[0]--;
                    play_tone(220.0f, 120, 1);
                    if (hits_remaining[0] <= 0) {
                        round_over_timer = 50;
                        play_tone(300.0f, 200);
                    }
                } else {
                    shots[0][best_r][best_c] = 1;
                    play_tone(450.0f, 40);
                    turn = 1;
                }
            }
            return;
        }

        bool u = false, d = false, l = false, r = false, act = false;
        if (turn == 1) {
            u = pad.p1.up; d = pad.p1.down; l = pad.p1.left; r = pad.p1.right; act = pad.p1.action;
        } else {
            u = pad.p2.up; d = pad.p2.down; l = pad.p2.left; r = pad.p2.right; act = pad.p2.action;
        }

        if (input_delay == 0) {
            if (u) { cursor_r = (cursor_r + 5) % 6; input_delay = 9; play_tone(350.0f, 15); }
            else if (d) { cursor_r = (cursor_r + 1) % 6; input_delay = 9; play_tone(350.0f, 15); }
            else if (l) { cursor_c = (cursor_c + 5) % 6; input_delay = 9; play_tone(350.0f, 15); }
            else if (r) { cursor_c = (cursor_c + 1) % 6; input_delay = 9; play_tone(350.0f, 15); }
        }

        if (act && input_delay == 0) {
            input_delay = 14;
            if (shots[target_idx][cursor_r][cursor_c] == 0) {
                if (ships[target_idx][cursor_r][cursor_c] == 1) {
                    shots[target_idx][cursor_r][cursor_c] = 2;
                    hits_remaining[target_idx]--;
                    play_tone(240.0f, 140, 1);
                    if (hits_remaining[target_idx] <= 0) {
                        round_over_timer = 50;
                        play_tone(turn == 1 ? 750.0f : 300.0f, 250);
                    }
                } else {
                    shots[target_idx][cursor_r][cursor_c] = 1;
                    play_tone(480.0f, 40);
                    turn = (turn == 1) ? 2 : 1;
                }
            } else {
                play_tone(180.0f, 20);
            }
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        int sz = 24;
        int p1_bx = 80, p2_bx = 270, by = 50;

        SDL_Rect f1 = {p1_bx - 4, by - 4, sz * 6 + 8, sz * 6 + 8};
        set_color(ren, Color(16, 28, 52));
        SDL_RenderFillRect(ren, &f1);
        set_color(ren, (turn == 1 ? COL_BLUE : COL_BORDER));
        SDL_RenderDrawRect(ren, &f1);

        SDL_Rect f2 = {p2_bx - 4, by - 4, sz * 6 + 8, sz * 6 + 8};
        set_color(ren, Color(16, 28, 52));
        SDL_RenderFillRect(ren, &f2);
        set_color(ren, (turn == 2 ? COL_RED : COL_BORDER));
        SDL_RenderDrawRect(ren, &f2);

        for (int r = 0; r < 6; r++) {
            for (int c = 0; c < 6; c++) {
                int x1 = p1_bx + c * sz, y1 = by + r * sz;
                SDL_Rect r1 = {x1, y1, sz, sz};
                set_color(ren, Color(22, 38, 70));
                SDL_RenderDrawRect(ren, &r1);
                int s1 = shots[1][r][c];
                if (s1 == 1) {
                    draw_circle(ren, x1 + sz / 2, y1 + sz / 2, 4, COL_WHITE, false);
                } else if (s1 == 2) {
                    draw_circle(ren, x1 + sz / 2, y1 + sz / 2, 8, COL_RED);
                    draw_circle(ren, x1 + sz / 2, y1 + sz / 2, 8, COL_GOLD, false);
                }

                int x2 = p2_bx + c * sz, y2 = by + r * sz;
                SDL_Rect r2 = {x2, y2, sz, sz};
                set_color(ren, Color(22, 38, 70));
                SDL_RenderDrawRect(ren, &r2);
                int s2 = shots[0][r][c];
                if (s2 == 1) {
                    draw_circle(ren, x2 + sz / 2, y2 + sz / 2, 4, COL_WHITE, false);
                } else if (s2 == 2) {
                    draw_circle(ren, x2 + sz / 2, y2 + sz / 2, 8, COL_RED);
                    draw_circle(ren, x2 + sz / 2, y2 + sz / 2, 8, COL_GOLD, false);
                }
            }
        }

        if (round_over_timer == 0) {
            int cur_bx = (turn == 1) ? p1_bx : p2_bx;
            SDL_Rect cur = {cur_bx + cursor_c * sz - 1, by + cursor_r * sz - 1, sz + 2, sz + 2};
            set_color(ren, COL_GOLD);
            SDL_RenderDrawRect(ren, &cur);
            int cx = cur_bx + cursor_c * sz + sz / 2;
            int cy = by + cursor_r * sz + sz / 2;
            SDL_RenderDrawLine(ren, cx - 4, cy, cx + 4, cy);
            SDL_RenderDrawLine(ren, cx, cy - 4, cx, cy + 4);
        }

        draw_centered_str(ren, p1_bx + sz * 3, by - 14, "P1 RADAR (TARGET P2)", COL_BLUE, 1);
        draw_centered_str(ren, p2_bx + sz * 3, by - 14, "P2 RADAR (TARGET P1)", COL_RED, 1);

        char c1buf[24], c2buf[24];
        snprintf(c1buf, sizeof(c1buf), "ENEMY HP: %d/7", hits_remaining[1]);
        snprintf(c2buf, sizeof(c2buf), "ENEMY HP: %d/7", hits_remaining[0]);
        draw_centered_str(ren, p1_bx + sz * 3, by + sz * 6 + 12, c1buf, COL_WHITE, 1);
        draw_centered_str(ren, p2_bx + sz * 3, by + sz * 6 + 12, c2buf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "PLAYER 1", COL_BLUE, 1);
            draw_centered_str_p1(ren, 38, SCR_H / 2, (turn == 1 ? "AIM & FIRE" : "WAITING"), (turn == 1 ? COL_GOLD : COL_DIM), 1);

            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "PLAYER 2", COL_RED, 1);
            draw_centered_str_p2(ren, SCR_W - 38, SCR_H / 2, (turn == 2 ? "AIM & FIRE" : "WAITING"), (turn == 2 ? COL_GOLD : COL_DIM), 1);
        }
    }
};

/* ─── 12. TUG OF WAR (ROPE PULL) ────────────────────────────────────── */

struct TugOfWarGame {
    float rope_pos;
    float rope_vel;
    float p1_mash, p2_mash;
    int p1_score, p2_score;
    int ai_mash_timer;
    int round_over_timer;

    void reset(bool reset_match = true) {
        if (reset_match) { p1_score = 0; p2_score = 0; }
        rope_pos = 0.0f;
        rope_vel = 0.0f;
        p1_mash = 0.0f;
        p2_mash = 0.0f;
        ai_mash_timer = 0;
        round_over_timer = 0;
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

        static bool p1_prev_action = false;
        static bool p1_prev_up = false;
        if ((pad.p1.action && !p1_prev_action) || (pad.p1.up && !p1_prev_up)) {
            p1_mash += 14.0f;
            if (p1_mash > 80.0f) p1_mash = 80.0f;
            play_tone(420.0f + (rand() % 60), 15);
        }
        p1_prev_action = pad.p1.action;
        p1_prev_up = pad.p1.up;

        if (mode == MODE_2P_LOCAL) {
            static bool p2_prev_action = false;
            static bool p2_prev_up = false;
            if ((pad.p2.action && !p2_prev_action) || (pad.p2.up && !p2_prev_up)) {
                p2_mash += 14.0f;
                if (p2_mash > 80.0f) p2_mash = 80.0f;
                play_tone(360.0f + (rand() % 60), 15);
            }
            p2_prev_action = pad.p2.action;
            p2_prev_up = pad.p2.up;
        } else {
            ai_mash_timer++;
            if (ai_mash_timer % (3 + rand() % 3) == 0) {
                p2_mash += 12.0f;
                if (p2_mash > 75.0f) p2_mash = 75.0f;
                if (rand() % 4 == 0) play_tone(360.0f, 15);
            }
        }

        p1_mash *= 0.90f;
        p2_mash *= 0.90f;

        rope_vel += (p2_mash - p1_mash) * 0.05f;
        rope_vel *= 0.88f;
        rope_pos += rope_vel;

        if (rope_pos <= -80.0f) {
            p1_score++;
            round_over_timer = 50;
            play_tone(750.0f, 180);
        } else if (rope_pos >= 80.0f) {
            p2_score++;
            round_over_timer = 50;
            play_tone(350.0f, 180);
        }
    }

    void render(SDL_Renderer *ren, GameMode mode) {
        int pit_w = 60, pit_h = 130;
        SDL_Rect pit = {SCR_W / 2 - pit_w / 2, SCR_H / 2 - pit_h / 2, pit_w, pit_h};
        set_color(ren, Color(90, 60, 30));
        SDL_RenderFillRect(ren, &pit);
        set_color(ren, Color(70, 45, 20));
        SDL_RenderDrawRect(ren, &pit);

        set_color(ren, COL_BLUE);
        SDL_RenderDrawLine(ren, SCR_W / 2 - 80, 50, SCR_W / 2 - 80, SCR_H - 50);
        set_color(ren, COL_RED);
        SDL_RenderDrawLine(ren, SCR_W / 2 + 80, 50, SCR_W / 2 + 80, SCR_H - 50);

        set_color(ren, COL_BORDER);
        SDL_RenderDrawLine(ren, SCR_W / 2, 50, SCR_W / 2, SCR_H - 50);

        int rope_y = SCR_H / 2;
        int flag_x = SCR_W / 2 + (int)rope_pos;

        set_color(ren, Color(210, 180, 120));
        for (int w = -2; w <= 2; w++)
            SDL_RenderDrawLine(ren, 40, rope_y + w, SCR_W - 40, rope_y + w);

        SDL_Rect ribbon = {flag_x - 4, rope_y - 12, 8, 24};
        set_color(ren, COL_GOLD);
        SDL_RenderFillRect(ren, &ribbon);
        set_color(ren, COL_RED);
        SDL_RenderDrawRect(ren, &ribbon);

        int p1_x = 90 + (int)(rope_pos * 0.4f);
        int lean1 = (int)(p1_mash * 0.2f);
        draw_circle(ren, p1_x - lean1, rope_y - 28, 8, COL_BLUE);
        set_color(ren, COL_BLUE);
        SDL_RenderDrawLine(ren, p1_x - lean1, rope_y - 20, p1_x, rope_y + 12);
        SDL_RenderDrawLine(ren, p1_x, rope_y + 12, p1_x - 14, rope_y + 30);
        SDL_RenderDrawLine(ren, p1_x, rope_y + 12, p1_x + 8,  rope_y + 30);
        SDL_RenderDrawLine(ren, p1_x - lean1 / 2, rope_y - 8, p1_x + 16, rope_y);

        int p2_x = SCR_W - 90 + (int)(rope_pos * 0.4f);
        int lean2 = (int)(p2_mash * 0.2f);
        draw_circle(ren, p2_x + lean2, rope_y - 28, 8, COL_RED);
        set_color(ren, COL_RED);
        SDL_RenderDrawLine(ren, p2_x + lean2, rope_y - 20, p2_x, rope_y + 12);
        SDL_RenderDrawLine(ren, p2_x, rope_y + 12, p2_x + 14, rope_y + 30);
        SDL_RenderDrawLine(ren, p2_x, rope_y + 12, p2_x - 8,  rope_y + 30);
        SDL_RenderDrawLine(ren, p2_x + lean2 / 2, rope_y - 8, p2_x - 16, rope_y);

        draw_centered_str(ren, 90, 48, "MASH [L] / UP!", (p1_mash > 20.0f ? COL_GOLD : COL_BLUE), 1);
        draw_centered_str(ren, SCR_W - 90, 48, (mode == MODE_1P_VS_AI ? "CPU PULLING" : "MASH [R] / SQ!"), (p2_mash > 20.0f ? COL_GOLD : COL_RED), 1);

        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "P1: %d   vs   P2: %d", p1_score, p2_score);
        draw_centered_str(ren, SCR_W / 2, 28, sbuf, COL_WHITE, 1);

        if (mode == MODE_2P_LOCAL) {
            draw_centered_str_p1(ren, 16, SCR_H / 2, "PLAYER 1", COL_BLUE, 1);
            draw_centered_str_p2(ren, SCR_W - 16, SCR_H / 2, "PLAYER 2", COL_RED, 1);
        }
    }
};
