#ifndef PROCESS_HELPER_H
#define PROCESS_HELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "10_data.h"

#define PH_PI               3.14159265358979

#define PLAYER_ACCEL        900.0   // px/s^2
#define PLAYER_MAXV         50.0   // px/s
#define PLAYER_FRICTION     6.0     // drag: v *= (1 - friction*dt)
#define BALL_FRICTION       1.4
#define KICK_POWER_MIN      300.0
#define KICK_POWER_MAX      900.0
#define KICK_CHARGE_RATE    1.8     // fills 0->1 in ~0.55s
#define KICK_COOLDOWN       0.35
#define ATTRACT_RADIUS      38.0    // ball sucked in within this px
#define ATTRACT_FORCE       1200.0  // px/s^2 attraction

#define WIND_BALL_SCALE     0.1   // wind acceleration on ball per frame
#define WIND_PLAYER_SCALE   0.025   // wind nudge on players
#define WIND_MAX            70.0    // max target wind speed px/s


// -------------------------------------------------------
// WIND
// -------------------------------------------------------
void update_wind(double dt){
    // Smooth lerp toward target
    double lerp = 1.0 - pow(0.01, dt);
    wind.vx += (wind.target_vx - wind.vx) * lerp;
    wind.vy += (wind.target_vy - wind.vy) * lerp;
    wind.change_timer -= dt;
    if(wind.change_timer <= 0){
        double angle = ((double)rand() / RAND_MAX) * 2.0 * PH_PI;
        double speed = ((double)rand() / RAND_MAX) * WIND_MAX;
        wind.target_vx    = cos(angle) * speed;
        wind.target_vy    = sin(angle) * speed;
        wind.change_timer = 6.0 + ((double)rand() / RAND_MAX) * 8.0;
    }
}

void spawn_dust(double dt){
    double ws = sqrt(wind.vx*wind.vx + wind.vy*wind.vy);
    if(ws < 8.0) return;
    double spawn_rate = ws / WIND_MAX * 20.0;
    static double accum = 0;
    accum += spawn_rate * dt;
    while(accum >= 1.0){
        accum -= 1.0;
        for(int i = 0; i < MAX_DUST; i++){
            if(dust[i].active) continue;
            double nx = -wind.vx / (ws + 0.001);
            double ny = -wind.vy / (ws + 0.001);
            if(fabs(nx) > fabs(ny)){
                dust[i].x = (nx > 0) ? 0.0 : (double)window.w;
                dust[i].y = (double)rand() / RAND_MAX * window.h;
            } else {
                dust[i].y = (ny > 0) ? 0.0 : (double)window.h;
                dust[i].x = (double)rand() / RAND_MAX * window.w;
            }
            dust[i].life   = 1.0;
            dust[i].active = 1;
            break;
        }
    }
}

void update_dust(double dt){
    double ws    = sqrt(wind.vx*wind.vx + wind.vy*wind.vy);
    double decay = 0.5 + ws / WIND_MAX * 0.8;
    for(int i = 0; i < MAX_DUST; i++){
        if(!dust[i].active) continue;
        dust[i].x    += wind.vx * dt;
        dust[i].y    -= wind.vy * dt;  // screen y is flipped vs physics y
        dust[i].life -= decay * dt;
        if(dust[i].life <= 0 ||
           dust[i].x < -10 || dust[i].x > window.w + 10 ||
           dust[i].y < -10 || dust[i].y > window.h + 10){
            dust[i].active = 0;
        }
    }
}


// -------------------------------------------------------
// GOALIE ZONE CLAMP
// Constrains a goalie inside their half-circle penalty area.
// zone_idx 0 = red (zone extends right from goal mouth)
// zone_idx 1 = blue (zone extends left from goal mouth)
// -------------------------------------------------------
void clamp_goalie_zone(struct footballer_data* f, int zone_idx){
    struct goalie_zone_data* z = &goalie_zones[zone_idx];
    double dx   = f->x - z->cx;
    double dy   = f->y - z->cy;
    double dist = sqrt(dx*dx + dy*dy);
    double max_r = z->r - f->r;

    // Keep inside circle
    if(dist > max_r && dist > 0.001){
        double nx = dx / dist;
        double ny = dy / dist;
        f->x = z->cx + nx * max_r;
        f->y = z->cy + ny * max_r;
        // Kill outward velocity component
        double dot = f->vx*nx + f->vy*ny;
        if(dot > 0){ f->vx -= dot*nx; f->vy -= dot*ny; }
    }

    // Keep on field side of goal mouth (don't go behind the line)
    if(z->field_side == 1 && f->x < z->cx){
        f->x = z->cx + f->r;
        if(f->vx < 0) f->vx = 0;
    }
    if(z->field_side == -1 && f->x > z->cx){
        f->x = z->cx - f->r;
        if(f->vx > 0) f->vx = 0;
    }
}


// -------------------------------------------------------
// FOOTBALLER MOVEMENT (velocity-style)
// accel_x/y already scaled to PLAYER_ACCEL magnitude
// -------------------------------------------------------
void move_footballer(struct footballer_data* f, double accel_x, double accel_y, double dt){
    update_clock(&f->clock);
    double real_dt = f->clock.delta;
    if(real_dt <= 0 || real_dt > 0.1) real_dt = dt;

    // Facing angle follows input direction
    if(fabs(accel_x) > 0.1 || fabs(accel_y) > 0.1)
        f->angle_rad = atan2(accel_y, accel_x);

    // Wind nudge on player
    double wx = wind.vx * WIND_PLAYER_SCALE;
    double wy = wind.vy * WIND_PLAYER_SCALE;

    f->vx += (accel_x + wx) * real_dt;
    f->vy += (accel_y + wy) * real_dt;

    // Drag
    double drag = 1.0 - PLAYER_FRICTION * real_dt;
    if(drag < 0) drag = 0;
    f->vx *= drag;
    f->vy *= drag;

    // Speed cap
    double spd = sqrt(f->vx*f->vx + f->vy*f->vy);
    if(spd > PLAYER_MAXV){
        f->vx = f->vx / spd * PLAYER_MAXV;
        f->vy = f->vy / spd * PLAYER_MAXV;
    }

    f->x += f->vx * real_dt;
    f->y += f->vy * real_dt;

    // Field boundary bounce
    if(f->x < f->r)            { f->x = f->r;              f->vx =  fabs(f->vx)*0.3; }
    if(f->x > window.w - f->r) { f->x = window.w - f->r;   f->vx = -fabs(f->vx)*0.3; }
    if(f->y < f->r)            { f->y = f->r;              f->vy =  fabs(f->vy)*0.3; }
    if(f->y > window.h - f->r) { f->y = window.h - f->r;   f->vy = -fabs(f->vy)*0.3; }
}


// -------------------------------------------------------
// PLAYER-PLAYER COLLISION (circle vs circle, elastic)
// -------------------------------------------------------
void resolve_player_collisions(){
    for(int i = 0; i < NUM_FOOTBALLERS; i++){
        for(int j = i+1; j < NUM_FOOTBALLERS; j++){
            double dx       = footballers[j].x - footballers[i].x;
            double dy       = footballers[j].y - footballers[i].y;
            double dist     = sqrt(dx*dx + dy*dy);
            double min_dist = footballers[i].r + footballers[j].r;
            if(dist >= min_dist || dist < 0.001) continue;

            double nx      = dx / dist;
            double ny      = dy / dist;
            double overlap = min_dist - dist;

            // Push apart equally
            footballers[i].x -= nx * overlap * 0.5;
            footballers[i].y -= ny * overlap * 0.5;
            footballers[j].x += nx * overlap * 0.5;
            footballers[j].y += ny * overlap * 0.5;

            // Velocity exchange along collision normal (elastic, restitution 0.4)
            double vi = footballers[i].vx*nx + footballers[i].vy*ny;
            double vj = footballers[j].vx*nx + footballers[j].vy*ny;
            if(vi - vj > 0){
                double impulse = (1.0 + 0.4) * (vi - vj) / 2.0;
                footballers[i].vx -= impulse * nx;
                footballers[i].vy -= impulse * ny;
                footballers[j].vx += impulse * nx;
                footballers[j].vy += impulse * ny;
            }
        }
    }
    // Re-clamp goalies in case a collision pushed them out of their zone
    clamp_goalie_zone(&footballers[RED_GOALIE_IDX],  0);
    clamp_goalie_zone(&footballers[BLUE_GOALIE_IDX], 1);
}


// -------------------------------------------------------
// BALL PHYSICS
// -------------------------------------------------------
void update_ball_dt(double dt){
    if(ball.owner >= 0){
        // Ball rides in front of owner along their facing direction
        struct footballer_data* own = &footballers[ball.owner];
        double carry_dist = own->r + ball.r + 2.0;
        ball.x  = own->x + cos(own->angle_rad) * carry_dist;
        ball.y  = own->y + sin(own->angle_rad) * carry_dist;
        ball.vx = own->vx;
        ball.vy = own->vy;
        return;
    }

    // Wind accelerates free ball
    ball.vx += wind.vx * WIND_BALL_SCALE * dt * 60.0;
    ball.vy += wind.vy * WIND_BALL_SCALE * dt * 60.0;

    // Drag
    double drag = 1.0 - BALL_FRICTION * dt;
    if(drag < 0) drag = 0;
    ball.vx *= drag;
    ball.vy *= drag;

    ball.x += ball.vx * dt;
    ball.y += ball.vy * dt;

    // Top / bottom wall bounce
    if(ball.y - ball.r < 0)          { ball.y = ball.r;            ball.vy =  fabs(ball.vy)*0.7; }
    if(ball.y + ball.r > window.h)   { ball.y = window.h - ball.r; ball.vy = -fabs(ball.vy)*0.7; }

    // Left / right wall bounce — only outside goal opening
    double gt0 = goals[0].y, gb0 = goals[0].y + goals[0].h;
    double gt1 = goals[1].y, gb1 = goals[1].y + goals[1].h;
    if(ball.x - ball.r < 0){
        if(ball.y < gt0 || ball.y > gb0){
            ball.x = ball.r; ball.vx = fabs(ball.vx)*0.7;
        }
    }
    if(ball.x + ball.r > window.w){
        if(ball.y < gt1 || ball.y > gb1){
            ball.x = window.w - ball.r; ball.vx = -fabs(ball.vx)*0.7;
        }
    }
}


// -------------------------------------------------------
// BALL ATTRACT — ball is sucked toward any nearby player
// When close enough it snaps to ownership
// -------------------------------------------------------
void check_ball_attract(double dt){
    if(ball.owner >= 0) return;
    for(int i = 0; i < NUM_FOOTBALLERS; i++){
        struct footballer_data* f = &footballers[i];
        double dx   = ball.x - f->x;
        double dy   = ball.y - f->y;
        double dist = sqrt(dx*dx + dy*dy);
        if(dist > ATTRACT_RADIUS || dist < 0.1) continue;

        // Pull ball toward player
        ball.vx += (-dx / dist) * ATTRACT_FORCE * dt;
        ball.vy += (-dy / dist) * ATTRACT_FORCE * dt;

        // Snap to ownership on contact
        if(dist < f->r + ball.r + 4.0){
            ball.owner = i;
            f->has_ball = 1;
            for(int j = 0; j < NUM_FOOTBALLERS; j++)
                if(j != i) footballers[j].has_ball = 0;
            return;
        }
    }
}


// -------------------------------------------------------
// BALL REFLECT — free ball bounces off player circles
// -------------------------------------------------------
void check_ball_reflect(){
    if(ball.owner >= 0) return;
    for(int i = 0; i < NUM_FOOTBALLERS; i++){
        struct footballer_data* f = &footballers[i];
        double dx   = ball.x - f->x;
        double dy   = ball.y - f->y;
        double dist = sqrt(dx*dx + dy*dy);
        double min_d = f->r + ball.r;
        if(dist >= min_d || dist < 0.01) continue;

        double nx = dx / dist;
        double ny = dy / dist;
        // Push ball outside player
        ball.x = f->x + nx * (min_d + 1.0);
        ball.y = f->y + ny * (min_d + 1.0);
        // Reflect velocity, slightly dampened
        double dot = ball.vx*nx + ball.vy*ny;
        if(dot < 0){
            ball.vx -= 2.0*dot*nx;
            ball.vy -= 2.0*dot*ny;
            ball.vx *= 0.75;
            ball.vy *= 0.75;
        }
    }
}


// -------------------------------------------------------
// KICK — release ball along facing direction
// charge 0..1 maps to KICK_POWER_MIN..KICK_POWER_MAX
// -------------------------------------------------------
void kick_ball(int idx, double charge){
    if(ball.owner != idx) return;
    struct footballer_data* f = &footballers[idx];
    double power = KICK_POWER_MIN + (KICK_POWER_MAX - KICK_POWER_MIN) * charge;
    ball.owner  = -1;
    f->has_ball = 0;
    ball.vx = cos(f->angle_rad) * power;
    ball.vy = sin(f->angle_rad) * power;
    // Eject ball clear of player to avoid instant re-attract
    ball.x  = f->x + cos(f->angle_rad) * (f->r + ball.r + 6.0);
    ball.y  = f->y + sin(f->angle_rad) * (f->r + ball.r + 6.0);
    kick_timer[idx]  = KICK_COOLDOWN;
    kick_charge[idx] = 0;
}


// -------------------------------------------------------
// HUMAN INPUT
// idx         = footballer index to control
// next_idx    = index to switch to on tab
// -------------------------------------------------------
void handle_human(int idx,
                  int left, int right, int up, int down,
                  int kick_held, int kick_released,
                  int tab_clicked,
                  int* team_active_ptr,
                  int next_idx,
                  double dt)
{
    // Tab/M: cycle to next teammate
    if(tab_clicked){
        footballers[idx].is_active    = 0;
        *team_active_ptr              = next_idx;
        footballers[next_idx].is_active = 1;
        return;
    }

    struct footballer_data* f = &footballers[idx];

    // Acceleration from keys
    double ax = 0, ay = 0;
    if(left)  ax -= PLAYER_ACCEL;
    if(right) ax += PLAYER_ACCEL;
    if(up)    ay += PLAYER_ACCEL;
    if(down)  ay -= PLAYER_ACCEL;

    // Normalize diagonal so it doesn't go faster
    double alen = sqrt(ax*ax + ay*ay);
    if(alen > PLAYER_ACCEL + 0.1){
        ax = ax / alen * PLAYER_ACCEL;
        ay = ay / alen * PLAYER_ACCEL;
    }

    move_footballer(f, ax, ay, dt);

    // Goalie zone applies even when human-controlled
    if(f->is_goalie)
        clamp_goalie_zone(f, (f->team == 0) ? 0 : 1);

    // Kick charge while holding key with ball
    kick_timer[idx] -= dt;
    if(kick_held && ball.owner == idx){
        kick_charge[idx] += KICK_CHARGE_RATE * dt;
        if(kick_charge[idx] > 1.0) kick_charge[idx] = 1.0;
    }
    // Kick fires on key release
    if(kick_released && kick_timer[idx] <= 0){
        if(ball.owner == idx) kick_ball(idx, kick_charge[idx]);
        kick_timer[idx]  = KICK_COOLDOWN;
        kick_charge[idx] = 0;
    }
}


// -------------------------------------------------------
// AI
// -------------------------------------------------------
static double ai_kick_timer[NUM_FOOTBALLERS] = {0,0,0,0,0,0,0,0};

void ai_footballer(int idx, double dt){
    struct footballer_data* f = &footballers[idx];

    // Goal references
    double enemy_gx, enemy_gy, own_gx, own_gy;
    if(f->team == 1){ // blue attacks left goal
        enemy_gx = goals[0].x + goals[0].w/2.0; enemy_gy = goals[0].y + goals[0].h/2.0;
        own_gx   = goals[1].x + goals[1].w/2.0; own_gy   = goals[1].y + goals[1].h/2.0;
    } else {           // red attacks right goal
        enemy_gx = goals[1].x + goals[1].w/2.0; enemy_gy = goals[1].y + goals[1].h/2.0;
        own_gx   = goals[0].x + goals[0].w/2.0; own_gy   = goals[0].y + goals[0].h/2.0;
    }

    double target_x, target_y;

    if(f->is_goalie){
        // Patrol along own goal mouth, tracking ball vertically
        double offset = (f->team == 0) ? 40.0 : -40.0;
        target_x = own_gx + offset;
        target_y = ball.y;
        // Clamp target to inside zone so AI doesn't overshoot boundary
        int     zi   = (f->team == 0) ? 0 : 1;
        double  tz_r = goalie_zones[zi].r - f->r;
        double  tcx  = goalie_zones[zi].cx;
        double  tcy  = goalie_zones[zi].cy;
        double  tdx  = target_x - tcx;
        double  tdy  = target_y - tcy;
        double  td   = sqrt(tdx*tdx + tdy*tdy);
        if(td > tz_r){ target_x = tcx + tdx/td*tz_r; target_y = tcy + tdy/td*tz_r; }
    } else {
        // Role within team: red uses idx 0,1,2 ; blue uses idx-4 = 0,1,2
        int role = (f->team == 0) ? idx : idx - 4;
        if(ball.owner >= 0 && footballers[ball.owner].team == f->team){
            // Teammate has ball: make a support run
            double spread = (role == 0) ? -80.0 : 80.0;
            target_x = (ball.x + enemy_gx) * 0.5;
            target_y = ball.y + spread;
        } else {
            // Chase ball with slight spread to avoid stacking
            double spread = (role == 0) ? -40.0 : (role == 1) ? 40.0 : 0.0;
            target_x = ball.x;
            target_y = ball.y + spread;
        }
        // Defensive mid (role 2) stays back in own half
        if(role == 2){
            double mid = window.w * 0.5;
            if(f->team == 0 && target_x > mid) target_x = mid;
            if(f->team == 1 && target_x < mid) target_x = mid;
        }
    }

    double dx   = target_x - f->x;
    double dy   = target_y - f->y;
    double dist = sqrt(dx*dx + dy*dy);
    double ax = 0, ay = 0;
    if(dist > 8.0){
        double spd = PLAYER_ACCEL * 1.05;
        ax = (dx / dist) * spd;
        ay = (dy / dist) * spd;
    }
    move_footballer(f, ax, ay, dt);

    // Goalie clamp for AI too
    if(f->is_goalie)
        clamp_goalie_zone(f, (f->team == 0) ? 0 : 1);

    // Kick when ball owned: aim at enemy goal and fire
    if(ball.owner == idx){
        f->angle_rad = atan2(enemy_gy - f->y, enemy_gx - f->x);
        ai_kick_timer[idx] -= dt;
        if(ai_kick_timer[idx] <= 0){
            kick_ball(idx, 0.6 + ((double)rand()/RAND_MAX)*0.4);
            ai_kick_timer[idx] = KICK_COOLDOWN + 0.05;
        }
    }
}


// -------------------------------------------------------
// GOAL CHECK
// Returns 1 if red scores (ball in left goal),
//         2 if blue scores (ball in right goal),
//         0 otherwise.
// -------------------------------------------------------
int check_goal(){
    if(ball.owner >= 0) return 0;
    // Left goal (goals[0]) — red scores
    if(ball.x - ball.r < goals[0].x + goals[0].w &&
       ball.y > goals[0].y && ball.y < goals[0].y + goals[0].h)
        return 1;
    // Right goal (goals[1]) — blue scores
    if(ball.x + ball.r > goals[1].x &&
       ball.y > goals[1].y && ball.y < goals[1].y + goals[1].h)
        return 2;
    return 0;
}

#endif
