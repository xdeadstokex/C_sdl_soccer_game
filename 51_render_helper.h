#ifndef RENDER_HELPER_H
#define RENDER_HELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "10_data.h"

// Physics Y  ->  screen Y  (SDL origin is top-left)
static inline int py(double y){ return (int)get_physic_y_cor(window.h, y); }
static inline int px(double x){ return (int)x; }

// Menu button geometry — MUST match process.h inline functions
static inline int menu_btn_x_r(){ return window.w/2 - MENU_BTN_W/2; }
static inline int menu_pva_y_r(){ return window.h/2 - 80; }
static inline int menu_pvp_y_r(){ return window.h/2 + 10; }


// -------------------------------------------------------
// PRIMITIVE HELPERS
// -------------------------------------------------------

void draw_circle(int cx, int cy, int r, int color){
    for(int dy = -r; dy <= r; dy++){
        int dx = (int)sqrt((double)(r*r - dy*dy));
        draw_rect(&window, cx - dx, cy + dy, dx*2, 1, color);
    }
}

void draw_circle_outline(int cx, int cy, int r, int color){
    for(int i = 0; i < 64; i++){
        double a0 = (i      / 64.0) * 2.0 * M_PI;
        double a1 = ((i+1)  / 64.0) * 2.0 * M_PI;
        int x0 = cx + (int)(cos(a0) * r);
        int y0 = cy + (int)(sin(a0) * r);
        int x1 = cx + (int)(cos(a1) * r);
        int y1 = cy + (int)(sin(a1) * r);
        draw_line(&window, x0, y0, x1, y1, color);
    }
}

// Arc from a_start to a_end (radians), screen coords, y already flipped by caller
void draw_arc_screen(int scx, int scy, int r, double a_start, double a_end, int segs, int color){
    for(int i = 0; i < segs; i++){
        double a0 = a_start + (a_end - a_start) * (i       / (double)segs);
        double a1 = a_start + (a_end - a_start) * ((i+1.0) / (double)segs);
        int x0 = scx + (int)(cos(a0) * r);
        int y0 = scy + (int)(sin(a0) * r);
        int x1 = scx + (int)(cos(a1) * r);
        int y1 = scy + (int)(sin(a1) * r);
        draw_line(&window, x0, y0, x1, y1, color);
    }
}


// -------------------------------------------------------
// FIELD
// -------------------------------------------------------
void draw_field(){
    draw_rect(&window, 0, 0, window.w, window.h, 0x2D7A2DFF);
    // Alternating dark stripes
    for(int i = 0; i < 8; i++){
        if(i % 2 == 0)
            draw_rect(&window, i * (window.w/8), 0, window.w/8, window.h, 0x286A28FF);
    }
    // Boundary lines
    draw_rect(&window, 0,            0,            window.w, 3, 0xFFFFFFAA);
    draw_rect(&window, 0,            window.h - 3, window.w, 3, 0xFFFFFFAA);
    draw_rect(&window, 0,            0,            3, window.h, 0xFFFFFFAA);
    draw_rect(&window, window.w - 3, 0,            3, window.h, 0xFFFFFFAA);
    // Centre line + circle
    draw_rect(&window, window.w/2 - 1, 0, 2, window.h, 0xFFFFFF66);
    draw_circle_outline(window.w/2, window.h/2, 70, 0xFFFFFF66);
    draw_circle(window.w/2, window.h/2, 4, 0xFFFFFF88);
}


// -------------------------------------------------------
// GOALS
// -------------------------------------------------------
void draw_goals(){
    // Left goal (red defends)
    {
        int gx = px(goals[0].x);
        int gy = py(goals[0].y + goals[0].h);
        int gw = (int)goals[0].w;
        int gh = (int)goals[0].h;
        draw_rect(&window, gx, gy, gw, gh, 0x44444488);
        draw_rect(&window, gx,      gy,      2,  gh, 0xFFFFFFFF);
        draw_rect(&window, gx,      gy,      gw, 2,  0xFFFFFFFF);
        draw_rect(&window, gx,      gy+gh-2, gw, 2,  0xFFFFFFFF);
    }
    // Right goal (blue defends)
    {
        int gx = px(goals[1].x);
        int gy = py(goals[1].y + goals[1].h);
        int gw = (int)goals[1].w;
        int gh = (int)goals[1].h;
        draw_rect(&window, gx, gy, gw, gh, 0x44444488);
        draw_rect(&window, gx+gw-2, gy,      2,  gh, 0xFFFFFFFF);
        draw_rect(&window, gx,      gy,      gw, 2,  0xFFFFFFFF);
        draw_rect(&window, gx,      gy+gh-2, gw, 2,  0xFFFFFFFF);
    }
}


// -------------------------------------------------------
// GOALIE ZONES (half-circle arcs on field)
// Physics Y coords converted to screen Y for the arc center.
// Red  zone: right half-circle  (arc spans -PI/2 .. PI/2 in screen space)
// Blue zone: left  half-circle  (arc spans  PI/2 .. 3PI/2)
// -------------------------------------------------------
void draw_goalie_zones(){
    // Red goalie zone
    {
        int scx = px(goalie_zones[0].cx);
        int scy = py(goalie_zones[0].cy);
        int zr  = (int)goalie_zones[0].r;
        draw_arc_screen(scx, scy, zr, -M_PI/2.0, M_PI/2.0, 32, 0xFF6644AA);
    }
    // Blue goalie zone
    {
        int scx = px(goalie_zones[1].cx);
        int scy = py(goalie_zones[1].cy);
        int zr  = (int)goalie_zones[1].r;
        draw_arc_screen(scx, scy, zr, M_PI/2.0, 3.0*M_PI/2.0, 32, 0x4488BBAA);
    }
}


// -------------------------------------------------------
// FOOTBALLER
// -------------------------------------------------------
void draw_footballer(struct footballer_data* f){
    int sx = px(f->x);
    int sy = py(f->y);
    int r  = (int)f->r;

    int body_col, shirt_col;
    if(f->is_goalie){
        body_col  = (f->team == 0) ? 0xFF8800FF : 0x00BBFFFF;
        shirt_col = (f->team == 0) ? 0xFFAA33FF : 0x33CCFFFF;
    } else {
        body_col  = (f->team == 0) ? 0xDD2222FF : 0x2244DDFF;
        shirt_col = (f->team == 0) ? 0xFF5555FF : 0x4466FFFF;
    }

    draw_circle(sx, sy, r, body_col);
    draw_circle(sx, sy, r - 4, shirt_col);

    // White outline = currently active (human-controlled)
    if(f->is_active)
        draw_circle_outline(sx, sy, r + 2, 0xFFFFFFFF);

    // Direction arrow (sin negated because screen y flips)
    int ex = sx + (int)( cos(f->angle_rad) * (r + 12));
    int ey = sy - (int)( sin(f->angle_rad) * (r + 12));
    draw_line(&window, sx, sy, ex, ey, 0xFFFFFF88);
    // Arrowhead
    double p1 = f->angle_rad + M_PI * 0.8;
    double p2 = f->angle_rad - M_PI * 0.8;
    draw_line(&window, ex, ey, ex + (int)(cos(p1)*6), ey - (int)(sin(p1)*6), 0xFFFFFFCC);
    draw_line(&window, ex, ey, ex + (int)(cos(p2)*6), ey - (int)(sin(p2)*6), 0xFFFFFFCC);

    // Kick charge bar (only when player has ball and is charging)
    int ki = (int)(f - footballers);
    if(kick_charge[ki] > 0.01 && f->has_ball){
        int bw  = (int)(f->r * 2.0);
        int bx  = sx - (int)f->r;
        int by2 = sy - r - 8;
        draw_rect(&window, bx, by2, bw, 5, 0x333333FF);
        int bc = (kick_charge[ki] < 0.5) ? 0x44FF44FF : 0xFF4422FF;
        draw_rect(&window, bx, by2, (int)(bw * kick_charge[ki]), 5, bc);
    }
}


// -------------------------------------------------------
// BALL
// -------------------------------------------------------
void draw_ball(){
    int sx = px(ball.x);
    int sy = py(ball.y);
    int r  = (int)ball.r;
    draw_circle(sx, sy, r, 0xEEEEEEFF);
    draw_rect(&window, sx - r, sy - 1, r*2, 2, 0x222222FF);
    draw_rect(&window, sx - 1, sy - r, 2, r*2, 0x222222FF);
    draw_circle_outline(sx, sy, r, 0x444444FF);
}


// -------------------------------------------------------
// WIND INDICATOR (top-right compass)
// -------------------------------------------------------
void draw_wind_indicator(){
    double ws = sqrt(wind.vx*wind.vx + wind.vy*wind.vy);
    if(ws < 2.0) return;
    int cx = window.w - 60;
    int cy = 50;
    draw_circle(cx, cy, 28, 0x00000066);
    draw_circle_outline(cx, cy, 28, 0xFFFFFF55);

    double norm = ws / WIND_MAX;
    double ax   =  wind.vx / (ws + 0.001);
    double ay   = -wind.vy / (ws + 0.001);  // screen y flip
    int tip_x = cx + (int)(ax * 22.0 * norm);
    int tip_y = cy + (int)(ay * 22.0 * norm);
    int tail_x = cx - (int)(ax * 12.0);
    int tail_y = cy - (int)(ay * 12.0);
    draw_line(&window, tail_x, tail_y, tip_x, tip_y, 0xFFFF88FF);
    double pa  = atan2(ay, ax) + M_PI * 0.75;
    double pa2 = atan2(ay, ax) - M_PI * 0.75;
    draw_line(&window, tip_x, tip_y, tip_x+(int)(cos(pa)*7),  tip_y+(int)(sin(pa)*7),  0xFFFF88FF);
    draw_line(&window, tip_x, tip_y, tip_x+(int)(cos(pa2)*7), tip_y+(int)(sin(pa2)*7), 0xFFFF88FF);
    // Strength bar
    int bar_len = (int)(50.0 * norm);
    draw_rect(&window, cx - 25, cy + 32, 50, 5, 0x333333FF);
    draw_rect(&window, cx - 25, cy + 32, bar_len, 5, 0xFFFF44FF);
}


// -------------------------------------------------------
// DUST
// -------------------------------------------------------
void draw_dust(){
    for(int i = 0; i < MAX_DUST; i++){
        if(!dust[i].active) continue;
        int alpha = (int)(dust[i].life * 180);
        int color = 0xCCAA6600 | (alpha & 0xFF);
        draw_rect(&window, (int)dust[i].x, (int)dust[i].y, 4, 2, color);
    }
}


// -------------------------------------------------------
// SCORE (top-center)
// -------------------------------------------------------
void draw_score(){
    int cx = window.w / 2;
    draw_rect(&window, cx - 80, 8, 160, 30, 0x00000099);
    for(int i = 0; i < game.score_red  && i < 9; i++)
        draw_rect(&window, cx - 70 + i*13,  14, 10, 18, 0xFF4444FF);
    draw_rect(&window, cx - 3, 10, 6, 26, 0xFFFFFFCC);
    for(int i = 0; i < game.score_blue && i < 9; i++)
        draw_rect(&window, cx + 60 - i*13, 14, 10, 18, 0x4466FFFF);
}


// -------------------------------------------------------
// BACK BUTTON (top-left during play)
// -------------------------------------------------------
void draw_back_button(){
    int bx = BACK_BTN_X, by = BACK_BTN_Y;
    int bw = BACK_BTN_W, bh = BACK_BTN_H;
    int hov = (mouse.x > bx && mouse.x < bx+bw && mouse.y > by && mouse.y < by+bh);
    draw_rect(&window, bx, by, bw, bh, hov ? 0xCC333399 : 0x66222288);
    draw_rect(&window, bx,      by,      bw, 2,  0xFFFFFFAA);
    draw_rect(&window, bx,      by+bh-2, bw, 2,  0xFFFFFFAA);
    draw_rect(&window, bx,      by,      2,  bh, 0xFFFFFFAA);
    draw_rect(&window, bx+bw-2, by,      2,  bh, 0xFFFFFFAA);
    // Left-arrow icon
    int ax = bx + 20, ay = by + bh/2;
    draw_line(&window, ax, ay, ax+10, ay-7, 0xFFFFFFDD);
    draw_line(&window, ax, ay, ax+10, ay+7, 0xFFFFFFDD);
    draw_line(&window, ax, ay, ax+28, ay,   0xFFFFFFDD);
}


// -------------------------------------------------------
// MENU
// -------------------------------------------------------
void draw_menu(){
    // Dim overlay over field
    //draw_rect(&window, 0, 0, window.w, window.h, 0x00000099);

    int cx  = window.w / 2;
    int bx  = menu_btn_x_r();
    int bw  = MENU_BTN_W;
    int bh  = MENU_BTN_H;

    // Title (coloured block pattern — no font needed)
    draw_rect(&window, cx - 160, window.h/2 - 160, 320, 50, 0x1A1A2EFF);
    for(int i = 0; i < 14; i++){
        int col = (i % 2 == 0) ? 0xFF4444FF : 0xFFAA00FF;
        draw_rect(&window, cx - 140 + i*20, window.h/2 - 150, 16, 30, col);
    }

    // PvA button
    int pva_y = menu_pva_y_r();
    draw_rect(&window, bx, pva_y, bw, bh, 0x22AA44FF);
    draw_rect(&window, bx, pva_y,      bw, 2,  0xFFFFFFFF);
    draw_rect(&window, bx, pva_y+bh-2, bw, 2,  0xFFFFFFFF);
    draw_rect(&window, bx, pva_y,      2,  bh, 0xFFFFFFFF);
    draw_rect(&window, bx+bw-2, pva_y, 2,  bh, 0xFFFFFFFF);
    for(int i = 0; i < 8; i++)
        draw_rect(&window, cx - 60 + i*16, pva_y + 15, 12, 20, 0xEEFFEEFF);

    // PvP button
    int pvp_y = menu_pvp_y_r();
    draw_rect(&window, bx, pvp_y, bw, bh, 0x2244CCFF);
    draw_rect(&window, bx, pvp_y,      bw, 2,  0xFFFFFFFF);
    draw_rect(&window, bx, pvp_y+bh-2, bw, 2,  0xFFFFFFFF);
    draw_rect(&window, bx, pvp_y,      2,  bh, 0xFFFFFFFF);
    draw_rect(&window, bx+bw-2, pvp_y, 2,  bh, 0xFFFFFFFF);
    for(int i = 0; i < 8; i++)
        draw_rect(&window, cx - 60 + i*16, pvp_y + 15, 12, 20, 0xDDEEFFFF);

    // Hover highlights
    int mx = mouse.x, my = mouse.y;
    if(mx > bx && mx < bx + bw){
        if(my > pva_y && my < pva_y + bh)
            draw_rect(&window, bx, pva_y, bw, bh, 0xFFFFFF22);
        if(my > pvp_y && my < pvp_y + bh)
            draw_rect(&window, bx, pvp_y, bw, bh, 0xFFFFFF22);
    }

    // Controls hint
    int hx = cx - 180, hy = window.h/2 + 90;
    draw_rect(&window, cx - 200, hy - 10, 400, 40, 0x00000077);
    draw_rect(&window, hx,     hy, 20, 12, 0xFF4444FF); // WASD
    draw_rect(&window, hx+25,  hy, 20, 12, 0xFF8844FF); // SPACE
    draw_rect(&window, hx+50,  hy, 20, 12, 0xFFAA00FF); // TAB
    draw_rect(&window, hx+120, hy, 20, 12, 0x4466FFFF); // ARROWS
    draw_rect(&window, hx+145, hy, 20, 12, 0x44AAFFFF); // N
    draw_rect(&window, hx+170, hy, 20, 12, 0x22CCFFFF); // M
}


// -------------------------------------------------------
// IN-GAME CONTROLS HINT (bottom-left)
// -------------------------------------------------------
void draw_controls_hint(){
    int bx = 8, by = window.h - 52;
    draw_rect(&window, bx, by, 210, 44, 0x00000077);
    draw_rect(&window, bx+4,  by+6, 16, 10, 0xFF4444FF); // WASD
    draw_rect(&window, bx+24, by+6, 16, 10, 0xFF8844FF); // SPACE
    draw_rect(&window, bx+44, by+6, 16, 10, 0xFFAA00FF); // TAB
    if(game_mode == MODE_PVP){
        draw_rect(&window, bx+80,  by+6, 16, 10, 0x4466FFFF); // ARROWS
        draw_rect(&window, bx+100, by+6, 16, 10, 0x44AAFFFF); // N
        draw_rect(&window, bx+120, by+6, 16, 10, 0x22CCFFFF); // M
    }
}

#endif
