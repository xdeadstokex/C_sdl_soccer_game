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
void move_footballer(struct footballer_data* f, double accel_x, double accel_y, double dt, double boost){
    update_clock(&f->clock);
    double real_dt = f->clock.delta;
    if(real_dt <= 0 || real_dt > 0.1) real_dt = dt;

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
	double maxv = PLAYER_MAXV * boost;
    if(spd > maxv){
        f->vx = f->vx / spd * maxv;
        f->vy = f->vy / spd * maxv;
    }

    f->x += f->vx * real_dt;
    f->y += f->vy * real_dt;

    // Field boundary bounce
    if(f->x < f->r)            { f->x = f->r;              f->vx =  fabs(f->vx)*0.3; }
    if(f->x > window.w - f->r) { f->x = window.w - f->r;   f->vx = -fabs(f->vx)*0.3; }
    if(f->y < f->r)            { f->y = f->r;              f->vy =  fabs(f->vy)*0.3; }
    if(f->y > window.h - f->r) { f->y = window.h - f->r;   f->vy = -fabs(f->vy)*0.3; }

    // add collider
    double goal_top = goals[0].y;
    double goal_bottom = goals[0].y + goals[0].h;
    //left goal
    if (f->x - f->r < goals[0].w) { 
        if (fabs(f->y - goal_top) < f->r || fabs(f->y - goal_bottom) < f->r) {
             if (f->y < goal_top || f->y > goal_bottom) {
             } else {
                 f->x = goals[0].w + f->r;
                 f->vx = 0;
             }
        }
    }
    //right goal
    if (f->x + f->r > window.w - goals[1].w) {
        if (fabs(f->y - goal_top) < f->r || fabs(f->y - goal_bottom) < f->r) {
             if (f->y >= goal_top && f->y <= goal_bottom) {
                 f->x = window.w - goals[1].w - f->r;
                 f->vx = 0;
             }
        }
    }
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
        //return;
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

    double gt = goals[0].y;          
    double gb = goals[0].y + goals[0].h; 
    double gw = goals[0].w;          


    if(ball.x - ball.r < gw) {
        if(ball.y < gt || ball.y > gb) {
            ball.x = gw + ball.r;
            ball.vx = fabs(ball.vx) * 0.7;
        } 
        else if (fabs(ball.y - gt) < ball.r || fabs(ball.y - gb) < ball.r) {
            ball.vx = fabs(ball.vx) * 0.8;
            ball.vy = -ball.vy * 0.8;    
        }
    }

    if(ball.x + ball.r > window.w - gw) {
        if(ball.y < gt || ball.y > gb) {
            ball.x = window.w - gw - ball.r;
            ball.vx = -fabs(ball.vx) * 0.7;
        }
        else if (fabs(ball.y - gt) < ball.r || fabs(ball.y - gb) < ball.r) {
            ball.vx = -fabs(ball.vx) * 0.8;
            ball.vy = -ball.vy * 0.8;
        }
    }
}


// -------------------------------------------------------
// BALL ATTRACT — ball is sucked toward any nearby player
// When close enough it snaps to ownership
// -------------------------------------------------------
void check_ball_attract(double dt){
	// to allow ball to be taken away
    if(ball.owner >= 0){
	struct footballer_data* f = &footballers[ball.owner];
	double dx   = ball.x - f->x;
	double dy   = ball.y - f->y;
	// Angle from player to ball
	double ball_angle = atan2(dy, dx); // physics coords

	// Difference from player facing
	double diff = ball_angle - f->angle_rad;
	while(diff >  M_PI) diff -= 2*M_PI;
	while(diff < -M_PI) diff += 2*M_PI;

	// Only attract if ball is within 45 degrees in front
	if(fabs(diff) > M_PI / 4){
	ball.owner  = -1;
    f->has_ball = 0;
	}

	return;
	}
	
	
    for(int i = 0; i < NUM_FOOTBALLERS; i++){
        struct footballer_data* f = &footballers[i];
        double dx   = ball.x - f->x;
        double dy   = ball.y - f->y;
        double dist = sqrt(dx*dx + dy*dy);
        if(dist > ATTRACT_RADIUS || dist < 0.1) continue;


        // Angle from player to ball
        double ball_angle = atan2(dy, dx); // physics coords

        // Difference from player facing
        double diff = ball_angle - f->angle_rad;
        while(diff >  M_PI) diff -= 2*M_PI;
        while(diff < -M_PI) diff += 2*M_PI;

        // Only attract if ball is within 45 degrees in front
        if(fabs(diff) >M_PI / 4) continue;


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
    int max_iter = 5;
    for(int iter = 0; iter < max_iter; iter++){
        int any = 0;
        for(int i = 0; i < NUM_FOOTBALLERS; i++){
            struct footballer_data* f = &footballers[i];
            double dx   = ball.x - f->x;
            double dy   = ball.y - f->y;
            double dist = sqrt(dx*dx + dy*dy);
            double min_d = f->r + ball.r;
            if(dist >= min_d || dist < 0.01) continue;
            any = 1;
            double nx = dx / dist;
            double ny = dy / dist;
            ball.x = f->x + nx * (min_d + 1.0);
            ball.y = f->y + ny * (min_d + 1.0);
            double dot = ball.vx*nx + ball.vy*ny;
            if(dot < 0){
                ball.vx -= 2.0*dot*nx;
                ball.vy -= 2.0*dot*ny;
                ball.vx *= 0.75;
                ball.vy *= 0.75;
            }
        }
        if(!any) break; // no overlap, done early
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

	play_sound(&sfx_kick);
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
                  int boost_held,
                  int* team_active_ptr,
                  int next_idx,
                  double dt)
{
    if(tab_clicked){
        footballers[idx].is_active    = 0;
        *team_active_ptr              = next_idx;
        footballers[next_idx].is_active = 1;
        return;
    }
    struct footballer_data* f = &footballers[idx];

    // Rotate left/right
    double TURN_SPEED = 6.0; // radians/sec, tune
    if(left)  f->angle_rad += TURN_SPEED * dt;
    if(right) f->angle_rad -= TURN_SPEED * dt;

    // Move forward/backward along facing direction
    double ax = 0, ay = 0;
    double boost = boost_held ? 1.8 : 1.0; // tune

    if(up){
        ax += cos(f->angle_rad) * PLAYER_ACCEL * boost;
        ay += sin(f->angle_rad) * PLAYER_ACCEL * boost;
    }
    if(down){
        ax -= cos(f->angle_rad) * PLAYER_ACCEL * boost;
        ay -= sin(f->angle_rad) * PLAYER_ACCEL * boost;
    }

    move_footballer(f, ax, ay, dt, boost);

    if(f->is_goalie)
        clamp_goalie_zone(f, (f->team == 0) ? 0 : 1);

    kick_timer[idx] -= dt;
    if(kick_held && ball.owner == idx){
        kick_charge[idx] += KICK_CHARGE_RATE * dt;
        if(kick_charge[idx] > 1.0) kick_charge[idx] = 1.0;
    }
    if(kick_released && kick_timer[idx] <= 0){
        if(ball.owner == idx) kick_ball(idx, kick_charge[idx]);
        kick_timer[idx]  = KICK_COOLDOWN;
        kick_charge[idx] = 0;
    }
}


// -------------------------------------------------------
// AI
// -------------------------------------------------------
static double ai_kick_timer[NUM_FOOTBALLERS] = {0,0,0,0,0,0,0,0,0,0};

// ---- GOALIE POSSESSION STATE ----
// -1 = no goalie has ball; 0 = red goalie; 1 = blue goalie
static int    goalie_has_ball_team = -1;
static double goalie_hold_timer    = 0.0;  // countdown: goalie kicks when this hits 0

//hàm check đường ống từ ax, ay tới bx, by có bị đối thủ ở ex, ey chặn ko
static inline int is_pass_blocked(double ax, double ay, double bx, double by,
                                  double ex, double ey, double intercept_radius) {
    double abx = bx - ax;
    double aby = by - ay;
    double aex = ex - ax;
    double aey = ey - ay;

    double dot = aex * abx + aey * aby;
    double ab_len_sq = abx * abx + aby * aby;

    if (ab_len_sq == 0) return 0;

    double t = dot / ab_len_sq;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    double cx = ax + t * abx;
    double cy = ay + t * aby;

    double dx = ex - cx;
    double dy = ey - cy;

    return (sqrt(dx * dx + dy * dy) < intercept_radius);
}

void ai_footballer(int idx, double dt){
    struct footballer_data* f = &footballers[idx];

    // ----------------------------------------------------------------
    // GLOBAL POSSESSION TRACKING
    // ----------------------------------------------------------------
    static int team_possession = -1;
    if (ball.owner >= 0) {
        team_possession = footballers[ball.owner].team;

        if (footballers[ball.owner].is_goalie) {
            int goalie_team = footballers[ball.owner].team;
            if (goalie_has_ball_team != goalie_team) {
                goalie_has_ball_team = goalie_team;
                goalie_hold_timer = 1.5 + ((double)rand() / RAND_MAX) * 2.0;
            }
        } else {
            goalie_has_ball_team = -1;
        }
    } else {
        goalie_has_ball_team = -1;
    }

    // ----------------------------------------------------------------
    // GOAL REFERENCES
    // ----------------------------------------------------------------
    double enemy_gx, enemy_gy, own_gx, own_gy;
    if (f->team == 1) {
        enemy_gx = goals[0].x + goals[0].w/2.0; enemy_gy = goals[0].y + goals[0].h/2.0;
        own_gx   = goals[1].x + goals[1].w/2.0; own_gy   = goals[1].y + goals[1].h/2.0;
    } else {
        enemy_gx = goals[1].x + goals[1].w/2.0; enemy_gy = goals[1].y + goals[1].h/2.0;
        own_gx   = goals[0].x + goals[0].w/2.0; own_gy   = goals[0].y + goals[0].h/2.0;
    }

    // ----------------------------------------------------------------
    // GOALIE POSSESSION MODE — override all normal movement
    // ----------------------------------------------------------------
    int gk_possession_active = (goalie_has_ball_team >= 0);

    if (gk_possession_active && !f->is_goalie) {
        int gk_team  = goalie_has_ball_team;
        int gk_idx   = (gk_team == 0) ? RED_GOALIE_IDX : BLUE_GOALIE_IDX;
        double gk_x  = footballers[gk_idx].x;
        double gk_y  = footballers[gk_idx].y;

        double target_x, target_y;
        double attack_dir = (f->team == 0) ? 1.0 : -1.0;

        if (f->team != gk_team) {
            // ---- DEFENDING TEAM: press or drop back ----
            double my_dist = sqrt(pow(f->x - gk_x, 2) + pow(f->y - gk_y, 2));
            int press_rank = 0;
            int opp_start = (f->team == 0) ? 0 : 5;
            int opp_end   = (f->team == 0) ? 3 : 8;
            for (int i = opp_start; i <= opp_end; i++) {
                if (i == idx || footballers[i].is_goalie) continue;
                double od = sqrt(pow(footballers[i].x - gk_x, 2) + pow(footballers[i].y - gk_y, 2));
                if (od < my_dist || (od == my_dist && i < idx)) press_rank++;
            }

            if (press_rank == 0) {
                target_x = window.w / 2.0 - attack_dir * 150.0;
                target_y = gk_y;
            } else {
                target_x = own_gx + attack_dir * (window.w * 0.2);
                if (press_rank == 1)      target_y = 120.0;
                else if (press_rank == 2) target_y = window.h / 2.0;
                else                      target_y = window.h - 120.0;
            }

        } else {
            // ---- ATTACKING TEAM (has ball): spread out ----
            double my_dist = sqrt(pow(f->x - gk_x, 2) + pow(f->y - gk_y, 2));
            int recv_rank = 0;
            int own_start = (f->team == 0) ? 0 : 5;
            int own_end   = (f->team == 0) ? 3 : 8;
            for (int i = own_start; i <= own_end; i++) {
                if (i == idx || footballers[i].is_goalie) continue;
                double od = sqrt(pow(footballers[i].x - gk_x, 2) + pow(footballers[i].y - gk_y, 2));
                if (od < my_dist || (od == my_dist && i < idx)) recv_rank++;
            }

            if (recv_rank == 0 || recv_rank == 1) {
                target_x = own_gx + attack_dir * (window.w * 0.15);
                target_y = (recv_rank == 0) ? 80.0 : (window.h - 80.0);
            } else {
                target_x = own_gx + attack_dir * (window.w * 0.65);
                target_y = (recv_rank == 2) ? 100.0 : (window.h - 100.0);
            }

            double offside_line = enemy_gx - attack_dir * 80.0;
            if (f->team == 0 && target_x > offside_line) target_x = offside_line;
            if (f->team == 1 && target_x < offside_line) target_x = offside_line;
        }

        double dx = target_x - f->x, dy = target_y - f->y;
        double d  = sqrt(dx*dx + dy*dy);
        if (d > 8.0) {
            double spd = PLAYER_ACCEL * 1.05;
            f->angle_rad = atan2(dy, dx);
            move_footballer(f, (dx/d)*spd, (dy/d)*spd, dt, 1.0);
        } else {
            move_footballer(f, 0, 0, dt, 1.0);
        }
        return;
    }

    // ----------------------------------------------------------------
    // GOALIE — normal positioning + hold-then-clearance kick
    // ----------------------------------------------------------------
    double target_x, target_y;

    if (f->is_goalie) {

        if (ball.owner == idx) {
            goalie_hold_timer -= dt;

            if (goalie_hold_timer <= 0) {
                int best_target = -1;
                double best_score = -9999.0;
                int own_start = (f->team == 0) ? 0 : 5;
                int own_end   = (f->team == 0) ? 4 : 9;

                for (int i = own_start; i <= own_end; i++) {
                    if (footballers[i].is_goalie) continue;
                    struct footballer_data* recv = &footballers[i];

                    double min_enemy_dist = 9999.0;
                    for (int e = 0; e < NUM_FOOTBALLERS; e++) {
                        if (footballers[e].team == f->team) continue;
                        double ed = sqrt(pow(recv->x - footballers[e].x, 2) +
                                        pow(recv->y - footballers[e].y, 2));
                        if (ed < min_enemy_dist) min_enemy_dist = ed;
                    }

                    double dist_to_enemy_goal = sqrt(pow(enemy_gx - recv->x, 2) +
                                                     pow(enemy_gy - recv->y, 2));

                    double score = min_enemy_dist * 1.5
                                 + (window.w - dist_to_enemy_goal) * 0.5;

                    for (int e = 0; e < NUM_FOOTBALLERS; e++) {
                        if (footballers[e].team == f->team) continue;
                        if (is_pass_blocked(f->x, f->y, recv->x, recv->y,
                                            footballers[e].x, footballers[e].y,
                                            footballers[e].r + 28)) {
                            score -= 800.0;
                            break;
                        }
                    }

                    if (score > best_score) { best_score = score; best_target = i; }
                }

                if (best_target >= 0) {
                    struct footballer_data* recv = &footballers[best_target];
                    double pass_charge   = 0.65;
                    double pass_speed    = KICK_POWER_MIN + (KICK_POWER_MAX - KICK_POWER_MIN) * pass_charge;
                    double pass_dist     = sqrt(pow(recv->x - f->x, 2) + pow(recv->y - f->y, 2));
                    double time_to_reach = pass_dist / pass_speed;
                    double lead_x = recv->x + recv->vx * time_to_reach;
                    double lead_y = recv->y + recv->vy * time_to_reach;
                    f->angle_rad = atan2(lead_y - f->y, lead_x - f->x);
                    kick_ball(idx, pass_charge);
                    goalie_has_ball_team = -1;
                } else {
                    f->angle_rad = (f->team == 0) ? 0.0 : PH_PI;
                    kick_ball(idx, 1.0);
                    goalie_has_ball_team = -1;
                }
            }
            target_x = own_gx + ((f->team == 0) ?  (f->r + 4.0) : -(f->r + 4.0));
            target_y = own_gy;

        } else {
            double b_dx = ball.x - own_gx;
            double b_dy = ball.y - own_gy;
            double dist = sqrt(b_dx*b_dx + b_dy*b_dy);
            if (dist < 0.001) dist = 0.001;

            int lurking_enemy = 0;
            for (int i = 0; i < NUM_FOOTBALLERS; i++) {
                if (footballers[i].team != f->team && ball.owner != i) {
                    double edx = footballers[i].x - own_gx;
                    double edy = footballers[i].y - own_gy;
                    double edist = sqrt(edx*edx + edy*edy);
                    if (edist < goalie_zones[f->team].r * 1.5 && (b_dy * edy < 0)) {
                        lurking_enemy = 1; break;
                    }
                }
            }

            double THREAT = window.w * 0.4;
            if ((dist > THREAT) || !lurking_enemy) {
                int zi = (f->team == 0) ? 0 : 1;
                double r = goalie_zones[zi].r - f->r;
                target_x = own_gx + (b_dx / dist) * r;
                target_y = own_gy + (b_dy / dist) * r;
            } else {
                if (lurking_enemy && fabs(b_dx) > 70.0) {
                    double offset_x = (f->team == 0) ? (goals[f->team].h/2 + f->r) : -(goals[f->team].h/2 + f->r);
                    target_x = own_gx + offset_x;
                    target_y = own_gy + ((b_dy > 0) ? (goals[f->team].w/2) : -(goals[f->team].w/2));
                } else {
                    double aggro = 1.0 - (dist / THREAT);
                    if (aggro < 0.2) aggro = 0.2;
                    int zi = (f->team == 0) ? 0 : 1;
                    target_x = own_gx + (b_dx / dist) * goalie_zones[zi].r * aggro * 0.8;
                    target_y = own_gy + (b_dy / dist) * goalie_zones[zi].r * aggro * 0.8;
                }
            }

            int    zi   = (f->team == 0) ? 0 : 1;
            double tz_r = goalie_zones[zi].r - f->r;
            double tcx  = goalie_zones[zi].cx, tcy = goalie_zones[zi].cy;
            double tdx  = target_x - tcx,      tdy = target_y - tcy;
            double td   = sqrt(tdx*tdx + tdy*tdy);
            if (td > tz_r) { target_x = tcx + tdx/td*tz_r; target_y = tcy + tdy/td*tz_r; }
            double sl = f->r + 2.0;
            if (f->team == 0 && target_x < tcx + sl) target_x = tcx + sl;
            if (f->team == 1 && target_x > tcx - sl) target_x = tcx - sl;
        }

        {
            double dx = target_x - f->x, dy = target_y - f->y;
            double d  = sqrt(dx*dx + dy*dy);
            double ax = 0, ay = 0;
            if (d > 8.0) {
                ax = (dx/d)*PLAYER_ACCEL*1.05;
                ay = (dy/d)*PLAYER_ACCEL*1.05;
                f->angle_rad = atan2(ay, ax);
            }
            move_footballer(f, ax, ay, dt, 1.0);
        }
        clamp_goalie_zone(f, (f->team == 0) ? 0 : 1);
        return;
    }

    // ----------------------------------------------------------------
    // OUTFIELD — normal attack / defend
    // ----------------------------------------------------------------
    {
        int is_attacking = (team_possession == f->team);

        if (is_attacking) {
            if (ball.owner == idx) {
                // 1. Mặc định là dẫn bóng hướng thẳng về gôn địch
                target_x = enemy_gx;
                target_y = enemy_gy;
                
                //tìm kẻ địch gần nhất
                double closest_enemy_dist = 9999.0;
                struct footballer_data* closest_enemy = NULL;
                
                for (int i = 0; i < NUM_FOOTBALLERS; i++) {
                    if (footballers[i].team != f->team) {
                        double dist = sqrt(pow(footballers[i].x - f->x, 2) + pow(footballers[i].y - f->y, 2));
                        if (dist < closest_enemy_dist) {
                            closest_enemy_dist = dist;
                            closest_enemy = &footballers[i];
                        }
                    }
                }
                
                //né trường hợp dính nhau
                if (closest_enemy != NULL && closest_enemy_dist < 45.0) {
                    double gdx = enemy_gx - f->x;
                    double gdy = enemy_gy - f->y;
                    double g_len = sqrt(gdx*gdx + gdy*gdy);
                    if (g_len > 0.001) { gdx /= g_len; gdy /= g_len; }

                    double perp_x = -gdy;
                    double perp_y =  gdx;

                    double dot = perp_x * (closest_enemy->x - f->x) + perp_y * (closest_enemy->y - f->y);
                    if (dot > 0) { perp_x = -perp_x; perp_y = -perp_y; }

                    double blend_x = gdx * 0.5 + perp_x * 0.5;
                    double blend_y = gdy * 0.5 + perp_y * 0.5;
                    double b_len = sqrt(blend_x*blend_x + blend_y*blend_y);
                    if (b_len > 0.001) {
                        target_x = f->x + (blend_x / b_len) * 120.0;
                        target_y = f->y + (blend_y / b_len) * 120.0;
                    }
                }
            } 
            else{
                //tìm cầu thủ gần nhất
                double my_dist = sqrt(pow(f->x - ball.x, 2) + pow(f->y - ball.y, 2));
                int rank = 0;
                int start_idx = (f->team == 0) ? 0 : 5;
                int end_idx   = (f->team == 0) ? 3 : 8;
                for (int i = start_idx; i <= end_idx; i++) {
                    if (i == idx) continue;
                    double od = sqrt(pow(footballers[i].x - ball.x, 2) + pow(footballers[i].y - ball.y, 2));
                    if (od < my_dist || (od == my_dist && i < idx)) rank++;
                }

                //bóng gần mình
                if (ball.owner == -1 && rank == 0) {
                    target_x = ball.x + ball.vx * 0.3;
                    target_y = ball.y + ball.vy * 0.3;
                } 
                //trường hợp chạy chỗ
                else {
                    int my_index = (f->team == 0) ? idx : (idx - 5);
                    int ball_owner_index = -1;
                    if (ball.owner >= 0 && footballers[ball.owner].team == f->team)
                        ball_owner_index = (f->team == 0) ? ball.owner : (ball.owner - 5);

                    int relative_role = 0, role_counter = 0;
                    for (int i = 0; i < 4; i++) {
                        if (i == ball_owner_index) continue;
                        if (i == my_index) { relative_role = role_counter; break; }
                        role_counter++;
                    }

                    double attack_dir = (f->team == 0) ? 1.0 : -1.0;
                    double carrier_x  = (ball.owner >= 0) ? footballers[ball.owner].x : ball.x;

                    if (relative_role == 0) {
                        target_x = carrier_x + attack_dir * 220.0;
                        target_y = enemy_gy;
                    } else if (relative_role == 1) {
                        target_x = carrier_x + attack_dir * 160.0;
                        target_y = (ball.y <= window.h/2.0) ? (window.h - 90.0) : 90.0;
                    } else if (relative_role == 2) {
                        target_x = carrier_x - attack_dir * 60.0;
                        target_y = (ball.y <= window.h/2.0) ? 90.0 : (window.h - 90.0);
                    } else {
                        target_x = carrier_x - attack_dir * 180.0;
                        target_y = window.h / 2.0;
                    }

                    double offside_line = enemy_gx - attack_dir * 80.0;
                    if (f->team == 0 && target_x > offside_line) target_x = offside_line;
                    if (f->team == 1 && target_x < offside_line) target_x = offside_line;
                }
            }
        }
        else {
            // DEFEND
            double this_dist = sqrt(pow(f->x - ball.x, 2) + pow(f->y - ball.y, 2));
            int rank = 0;
            int start_idx = (f->team == 0) ? 0 : 5;
            int end_idx   = (f->team == 0) ? 4 : 9;

            for (int i = start_idx; i <= end_idx; i++) {
                if (i == idx) continue;
                double od = sqrt(pow(footballers[i].x - ball.x, 2) + pow(footballers[i].y - ball.y, 2));
                if (od < this_dist || (od == this_dist && i < idx)) rank++;
            }

            if (rank == 0) {
                double cvx = (ball.owner >= 0) ? footballers[ball.owner].vx : 0;
                double cvy = (ball.owner >= 0) ? footballers[ball.owner].vy : 0;
                target_x = ball.x + cvx * 0.25;
                target_y = ball.y + cvy * 0.25;
            } else {
                int mark_idx = (f->team == 0) ? (idx + 5) : (idx - 5);
                struct footballer_data* te = &footballers[mark_idx];
                target_x = te->x * 0.8 + own_gx * 0.2;
                target_y = te->y * 0.8 + own_gy * 0.2;
                if (mark_idx == ball.owner) {
                    target_x = own_gx + ((f->team == 0) ? 150.0 : -150.0);
                    target_y = own_gy + ((rank == 1) ? -80.0 : 80.0);
                }
            }
        }

        {
            double dx = target_x - f->x, dy = target_y - f->y;
            double d  = sqrt(dx*dx + dy*dy);
            double ax = 0, ay = 0;
            if (d > 8.0) {
                ax = (dx/d)*PLAYER_ACCEL*1.05;
                ay = (dy/d)*PLAYER_ACCEL*1.05;
                
                //xoay mặt nhìn bóng
                if (is_attacking && ball.owner != idx) {
                    f->angle_rad = atan2(ball.y - f->y, ball.x - f->x);
                } else {
                    f->angle_rad = atan2(ay, ax);
                }
            } else {
                if (is_attacking && ball.owner != idx) {
                    f->angle_rad = atan2(ball.y - f->y, ball.x - f->x);
                }
            }
            move_footballer(f, ax, ay, dt, 1.0);
        }

        //sút
        if (ball.owner == idx) {
            ai_kick_timer[idx] -= dt;

            double dist_to_goal = sqrt(pow(enemy_gx - f->x, 2) + pow(enemy_gy - f->y, 2));
            int can_shoot = (dist_to_goal < 200.0);

            if (can_shoot && ai_kick_timer[idx] <= 0) {
                double spread = ((double)rand()/RAND_MAX - 0.5) * 0.25;
                f->angle_rad = atan2(enemy_gy - f->y, enemy_gx - f->x) + spread;
                kick_ball(idx, 0.8 + ((double)rand()/RAND_MAX) * 0.2);
                ai_kick_timer[idx] = KICK_COOLDOWN + 0.2;
            } else if (ai_kick_timer[idx] <= 0) {
                //tìm người hoặc đập tường
                int best_teammate = -1;
                double best_score = -999.0;
                int best_is_wall_pass = 0; // 0: Chuyền thẳng, 1: Đập tường trên, 2: Đập tường dưới
                
                int start_idx = (f->team == 0) ? 0 : 5;
                int end_idx   = (f->team == 0) ? 3 : 8;

                for (int i = start_idx; i <= end_idx; i++) {
                    if (i == idx) continue;
                    struct footballer_data* recv = &footballers[i];
                    double dr = sqrt(pow(recv->x - f->x, 2) + pow(recv->y - f->y, 2));
                    if (dr < 140.0) continue;

                    double dtg   = sqrt(pow(enemy_gx - recv->x, 2) + pow(enemy_gy - recv->y, 2));
                    double score = 1000.0 - dtg;
                    if (dr > 500.0) score -= 500.0;

                    int is_blocked = 0;
                    for (int e = 0; e < NUM_FOOTBALLERS; e++) {
                        if (footballers[e].team != f->team) {
                            if (is_pass_blocked(f->x, f->y, recv->x, recv->y,
                                                footballers[e].x, footballers[e].y, footballers[e].r + 25)) {
                                is_blocked = 1; break;
                            }
                        }
                    }
                    
                    int wall_pass_type = 0;
                    if (is_blocked) {
                        //tính toán đập tường khi bị block chuyền thẳng
                        if (recv->y < window.h / 3.0) {
                            wall_pass_type = 1; 
                            score -= 200.0;
                        } else if (recv->y > window.h * 2.0 / 3.0) {
                            wall_pass_type = 2; 
                            score -= 200.0;
                        } else {
                            score -= 2000.0;
                        }
                    }

                    if (score > best_score) { 
                        best_score = score; 
                        best_teammate = i; 
                        best_is_wall_pass = wall_pass_type;
                    }
                }

                //truyền đón đầu
                if (best_teammate >= 0 && best_score > 0) {
                    struct footballer_data* recv = &footballers[best_teammate];
                    
                    double pass_charge   = 0.4;
                    if (best_is_wall_pass) pass_charge = 0.65; 

                    double pass_speed    = KICK_POWER_MIN + (KICK_POWER_MAX - KICK_POWER_MIN) * pass_charge;
                    double pass_dist     = sqrt(pow(recv->x - f->x, 2) + pow(recv->y - f->y, 2));
                    if (best_is_wall_pass) pass_dist *= 1.4;
                    
                    double ttr           = pass_dist / pass_speed;
                    
                    double lead_x = recv->x + recv->vx * ttr;
                    double lead_y = recv->y + recv->vy * ttr;
                    
                    double aim_x = lead_x;
                    double aim_y = lead_y;
                    
                    if (best_is_wall_pass == 1) {
                        aim_y = 0.0 - (lead_y / 0.7); 
                    } else if (best_is_wall_pass == 2) {
                        aim_y = window.h + ((window.h - lead_y) / 0.7);
                    }
                    
                    double wind_accel_x = wind.vx * 6.0; 
                    double wind_accel_y = wind.vy * 6.0;
                    double drift_x = 0.5 * wind_accel_x * ttr * ttr;
                    double drift_y = 0.5 * wind_accel_y * ttr * ttr;
                    
                    f->angle_rad = atan2(aim_y - drift_y - f->y, aim_x - drift_x - f->x);
                    kick_ball(idx, pass_charge);
                    ai_kick_timer[idx] = KICK_COOLDOWN + 0.5;
                }
            }
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
