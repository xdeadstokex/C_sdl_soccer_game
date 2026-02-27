#ifndef PROCESS_H
#define PROCESS_H
#include <stdio.h>
#include <stdlib.h>

#include "10_data.h"
#include "41_process_helper.h"
#include "external_lib/common_logic.h"

static double last_proc_time = 0;

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

					stop_sound(&bgm_menu);
					play_sound_loop(&bgm_match);
                }
                if(my > menu_pvp_y() && my < menu_pvp_y() + MENU_BTN_H){
                    game_mode = MODE_PVP;
                    reset_positions();
                    printf("Mode: Player vs Player\n");

					stop_sound(&bgm_menu);
					play_sound_loop(&bgm_match);
                }
            }
        }
        return;
    }

    // --------------------------------------------------
    // BACK TO MENU: ESC or click back button
    // --------------------------------------------------
	int return_menu = 0;
    if(kb.key_escape.click){ return_menu = 1; }
	if( check_point_in_box_2d(mouse.x, mouse.y, BACK_BTN_X, BACK_BTN_Y, BACK_BTN_W, BACK_BTN_H) && mouse.left.click){ return_menu = 1; }
    if(return_menu){
        game_mode = MODE_MENU;
        reset_positions();
		stop_sound(&bgm_match);
		play_sound_loop(&bgm_menu);
        return;
    }
    // --------------------------------------------------
    // GOAL FREEZE
    // --------------------------------------------------
    if(game.goal_cooldown > 0){ game.goal_cooldown--; return; }


    // --------------------------------------------------
    // TEAM 0 (RED): WASD + SPACE + TAB
    // Active field player is human; others run AI.
    // Goalie always AI (but still clamp-zone-enforced).
    // --------------------------------------------------
    {
        int idx  = team0_active;
        // Thêm role 3 vào vòng lặp next
        int next = (idx == 0) ? 1 : (idx == 1) ? 2 : (idx == 2) ? 3 : 0;
		handle_human(idx,
			kb.key_a.hold, kb.key_d.hold,
			kb.key_w.hold, kb.key_s.hold,
			kb.key_space.hold, kb.key_space.unclick,
			kb.key_shift_l.click,
			kb.key_ctrl_l.hold,
			&team0_active, next, dt);
        // Non-active red field players (từ 0 đến 3)
        for(int i = 0; i <= 3; i++)
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
        // Logic next cho index 5, 6, 7, 8
        int next = (idx == 5) ? 6 : (idx == 6) ? 7 : (idx == 7) ? 8 : 5;
		handle_human(idx,
			kb.key_arrow_left.hold, kb.key_arrow_right.hold,
			kb.key_arrow_up.hold,   kb.key_arrow_down.hold,
			kb.key_n.hold, kb.key_n.unclick,
			kb.key_m.click,
			kb.key_shift_r.hold,
			&team1_active, next, dt);

        for(int i = 5; i <= 8; i++)
            if(i != team1_active) ai_footballer(i, dt);
        ai_footballer(BLUE_GOALIE_IDX, dt);
    } else {
        for(int i = 5; i < NUM_FOOTBALLERS; i++) ai_footballer(i, dt);
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

		play_sound(&sfx_goal);
    }
	else if(goal == 2){
        game.score_blue++;
        printf("BLUE SCORES! %d - %d\n", game.score_red, game.score_blue);
        game.goal_cooldown = 150;
        reset_positions();
		
	play_sound(&sfx_goal);
    }
}

#endif
