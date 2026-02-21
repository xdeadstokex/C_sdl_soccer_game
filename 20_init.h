#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "10_data.h"

static void set_footballer(int i, double x, double y, double r,
                            double angle, int team, int goalie, int active){
    footballers[i].x        = x;
    footballers[i].y        = y;
    footballers[i].r        = r;
    footballers[i].vx       = 0;
    footballers[i].vy       = 0;
    footballers[i].angle_rad = angle;
    footballers[i].team     = team;
    footballers[i].is_goalie = goalie;
    footballers[i].has_ball = 0;
    footballers[i].is_active = active;
    reset_clock(&footballers[i].clock, 0);
}

int init(){
    init_window(&window, 1200, 700, "Shaolin Soccer");
    game_mode = MODE_MENU;

    double fw = window.w;
    double fh = window.h;
    double PI = 3.14159265358979;
    double r  = 14.0;

    // Red team (faces right = angle 0)
    set_footballer(0, fw*0.30, fh*0.35, r,   0, 0, 0, 1); // field 1 (active)
    set_footballer(1, fw*0.25, fh*0.65, r,   0, 0, 0, 0); // field 2
    set_footballer(2, fw*0.18, fh*0.50, r,   0, 0, 0, 0); // field 3 (def mid)
    set_footballer(3, fw*0.05, fh*0.50, r,   0, 0, 1, 0); // goalie

    // Blue team (faces left = angle PI)
    set_footballer(4, fw*0.70, fh*0.65, r,  PI, 1, 0, 1); // field 1 (active)
    set_footballer(5, fw*0.75, fh*0.35, r,  PI, 1, 0, 0); // field 2
    set_footballer(6, fw*0.82, fh*0.50, r,  PI, 1, 0, 0); // field 3 (def mid)
    set_footballer(7, fw*0.95, fh*0.50, r,  PI, 1, 1, 0); // goalie

    // Ball
    ball.x = fw*0.5; ball.y = fh*0.5;
    ball.r = 9; ball.vx = 0; ball.vy = 0; ball.owner = -1;

    // Goals
    double gw = 18, gh = 170;
    goals[0].x = 0;       goals[0].y = fh/2 - gh/2; goals[0].w = gw; goals[0].h = gh;
    goals[1].x = fw - gw; goals[1].y = fh/2 - gh/2; goals[1].w = gw; goals[1].h = gh;

    // Goalie zones
    // Red: circle center at right edge of left goal mouth, extends into field (right)
    goalie_zones[0].cx         = goals[0].x + goals[0].w;
    goalie_zones[0].cy         = goals[0].y + goals[0].h / 2.0;
    goalie_zones[0].r          = 120.0;
    goalie_zones[0].field_side = 1;
    // Blue: circle center at left edge of right goal mouth, extends into field (left)
    goalie_zones[1].cx         = goals[1].x;
    goalie_zones[1].cy         = goals[1].y + goals[1].h / 2.0;
    goalie_zones[1].r          = 120.0;
    goalie_zones[1].field_side = -1;

    // Game state
    game.score_red = 0; game.score_blue = 0; game.goal_cooldown = 0;

    // Wind (notched down)
    wind.vx = 0; wind.vy = 0;
    wind.target_vx = 15.0; wind.target_vy = 5.0;
    wind.change_timer = 7.0;

    // Dust
    for(int i = 0; i < MAX_DUST; i++) dust[i].active = 0;

    // Control state
    team0_active = 0;
    team1_active = 4;
    for(int i = 0; i < NUM_FOOTBALLERS; i++){
        kick_charge[i] = 0;
        kick_timer[i]  = 0;
    }

    return 1;
}

void reset_positions(){
    double fw = window.w, fh = window.h;
    double PI = 3.14159265358979;
    double r  = 14.0;

    set_footballer(0, fw*0.30, fh*0.35, r,   0, 0, 0, 1);
    set_footballer(1, fw*0.25, fh*0.65, r,   0, 0, 0, 0);
    set_footballer(2, fw*0.18, fh*0.50, r,   0, 0, 0, 0);
    set_footballer(3, fw*0.05, fh*0.50, r,   0, 0, 1, 0);
    set_footballer(4, fw*0.70, fh*0.65, r,  PI, 1, 0, 1);
    set_footballer(5, fw*0.75, fh*0.35, r,  PI, 1, 0, 0);
    set_footballer(6, fw*0.82, fh*0.50, r,  PI, 1, 0, 0);
    set_footballer(7, fw*0.95, fh*0.50, r,  PI, 1, 1, 0);

    ball.x = fw*0.5; ball.y = fh*0.5;
    ball.vx = 0; ball.vy = 0; ball.owner = -1;

    // Reset active selections to defaults
    team0_active = 0;
    team1_active = 4;

    for(int i = 0; i < NUM_FOOTBALLERS; i++){
        kick_charge[i] = 0;
        kick_timer[i]  = 0;
    }
}

#endif
