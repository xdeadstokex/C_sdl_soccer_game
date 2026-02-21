#ifndef PROCESS_H
#define PROCESS_H
#include <stdio.h>
#include <stdlib.h>

#include "10_data.h"
#include "41_process_helper.h"
#include "external_lib/common_logic.h"

static double last_proc_time = 0;

// Edge-detection state (previous frame key state)
static int tab_last   = 0;
static int m_last     = 0;
static int space_last = 0;
static int n_last     = 0;
static int esc_last   = 0;

// Menu button geometry — MUST match draw_menu() in 51_render_helper.h exactly.
// Buttons are screen-coord rectangles centered horizontally.
// PvA: top-left y = window.h/2 - 80,  height 50
// PvP: top-left y = window.h/2 + 10,  height 50
// Both have width MENU_BTN_W=260, centered at window.w/2
static inline int menu_btn_x(){ return window.w/2 - MENU_BTN_W/2; }
static inline int menu_pva_y(){ return window.h/2 - 80; }
static inline int menu_pvp_y(){ return window.h/2 + 10; }

void process(){
    double now = get_time_sec();
    if(last_proc_time == 0) last_proc_time = now;
    double dt = now - last_proc_time;
    if(dt > 0.1) dt = 0.1;
    last_proc_time = now;

    // ESC edge
    int esc_clicked = window.key_escape && !esc_last;
    esc_last = window.key_escape;

    // --------------------------------------------------
    // MENU
    // --------------------------------------------------
    if(game_mode == MODE_MENU){
        if(mouse.left.click){
            int mx = mouse.x;
            int my = mouse.y;  // SDL gives screen coords directly
            int bx = menu_btn_x();
            if(mx > bx && mx < bx + MENU_BTN_W){
                if(my > menu_pva_y() && my < menu_pva_y() + MENU_BTN_H){
                    game_mode = MODE_PVA;
                    reset_positions();
                    printf("Mode: Player vs AI\n");
                }
                if(my > menu_pvp_y() && my < menu_pvp_y() + MENU_BTN_H){
                    game_mode = MODE_PVP;
                    reset_positions();
                    printf("Mode: Player vs Player\n");
                }
            }
        }
        return;
    }

    // --------------------------------------------------
    // BACK TO MENU: ESC or click back button
    // --------------------------------------------------
    if(esc_clicked){
        game_mode = MODE_MENU;
        reset_positions();
        return;
    }
    if(mouse.left.click){
        int mx = mouse.x, my = mouse.y;
        if(mx > BACK_BTN_X && mx < BACK_BTN_X + BACK_BTN_W &&
           my > BACK_BTN_Y && my < BACK_BTN_Y + BACK_BTN_H){
            game_mode = MODE_MENU;
            reset_positions();
            return;
        }
    }

    // --------------------------------------------------
    // GOAL FREEZE
    // --------------------------------------------------
    if(game.goal_cooldown > 0){ game.goal_cooldown--; return; }

    // --------------------------------------------------
    // EDGE DETECTION
    // --------------------------------------------------
    int tab_clicked    = window.key_tab    && !tab_last;
    int m_clicked      = window.key_m      && !m_last;
    int space_released = !window.key_space && space_last;
    int n_released     = !window.key_n     && n_last;
    tab_last   = window.key_tab;
    m_last     = window.key_m;
    space_last = window.key_space;
    n_last     = window.key_n;

    // --------------------------------------------------
    // TEAM 0 (RED): WASD + SPACE + TAB
    // Active field player is human; others run AI.
    // Goalie always AI (but still clamp-zone-enforced).
    // --------------------------------------------------
    {
        int idx  = team0_active;
        int next = (idx == 0) ? 1 : (idx == 1) ? 2 : 0;
        handle_human(idx,
            window.key_a, window.key_d,
            window.key_w, window.key_s,
            window.key_space, space_released,
            tab_clicked,
            &team0_active, next,
            dt);
        // Non-active red field players
        for(int i = 0; i <= 2; i++)
            if(i != team0_active) ai_footballer(i, dt);
        // Red goalie always AI
        ai_footballer(RED_GOALIE_IDX, dt);
    }

    // --------------------------------------------------
    // TEAM 1 (BLUE)
    // PvA: all AI.
    // PvP: active field player is human (ARROWS+N+M); others AI.
    // --------------------------------------------------
    if(game_mode == MODE_PVP){
        int idx  = team1_active;
        int next = (idx == 4) ? 5 : (idx == 5) ? 6 : 4;
        handle_human(idx,
            window.key_arrow_left, window.key_arrow_right,
            window.key_arrow_up,   window.key_arrow_down,
            window.key_n, n_released,
            m_clicked,
            &team1_active, next,
            dt);
        for(int i = 4; i <= 6; i++)
            if(i != team1_active) ai_footballer(i, dt);
        ai_footballer(BLUE_GOALIE_IDX, dt);
    } else {
        for(int i = 4; i < NUM_FOOTBALLERS; i++) ai_footballer(i, dt);
    }

    // --------------------------------------------------
    // PHYSICS
    // --------------------------------------------------
    update_wind(dt);
    spawn_dust(dt);
    update_dust(dt);
    resolve_player_collisions();
    update_ball_dt(dt);
    check_ball_attract(dt);
    check_ball_reflect();

    // --------------------------------------------------
    // GOAL CHECK
    // --------------------------------------------------
    int goal = check_goal();
    if(goal == 1){
        game.score_red++;
        printf("RED SCORES! %d - %d\n", game.score_red, game.score_blue);
        game.goal_cooldown = 150;
        reset_positions();
    } else if(goal == 2){
        game.score_blue++;
        printf("BLUE SCORES! %d - %d\n", game.score_red, game.score_blue);
        game.goal_cooldown = 150;
        reset_positions();
    }
}

#endif
