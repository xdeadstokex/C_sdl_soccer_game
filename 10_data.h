#ifndef DATA_H
#define DATA_H

#include "external_lib/sdl2_wrapper.h"
#include "external_lib/io_logic.h"

//###############################################
//####             SYSTEM DATA               ####
//###############################################
struct cpu_window_data window;
struct mouse_data mouse;

//###############################################
//####              APP DATA                 ####
//###############################################

struct footballer_data {
    double x, y;
    double r;           // radius (circle)
    double vx, vy;
    double angle_rad;   // facing direction (physics: 0=right, PI/2=up)
    int has_ball;
    int team;           // 0=red, 1=blue
    int is_goalie;
    int is_active;      // 1 = currently human-selected (white outline)
    clock_data clock;
};

struct ball_data {
    double x, y;
    double r;
    double vx, vy;
    int owner;          // -1 = free, else footballer index
};

struct goal_data {
    double x, y;
    double w, h;
};

// Half-circle constraint zone for goalies.
// cx,cy = center at goal mouth edge on field side.
// r     = radius.
// field_side: +1 = zone extends right (red), -1 = extends left (blue).
struct goalie_zone_data {
    double cx, cy;
    double r;
    int field_side;
};

struct wind_data {
    double vx, vy;
    double target_vx, target_vy;
    double change_timer;
};

#define MAX_DUST 80
struct dust_particle {
    double x, y;    // screen coords
    double life;    // 1.0 -> 0.0
    int active;
};

struct game_data {
    int score_red;
    int score_blue;
    int goal_cooldown;  // frames to freeze after goal
};

// Game mode
typedef enum { MODE_MENU = 0, MODE_PVA = 1, MODE_PVP = 2 } game_mode_t;
game_mode_t game_mode;

// Layout:
//   Red  team: 0,1,2 = field players   |  3 = goalie
//   Blue team: 4,5,6 = field players   |  7 = goalie
#define NUM_FOOTBALLERS  8
#define RED_GOALIE_IDX   3
#define BLUE_GOALIE_IDX  7

struct footballer_data   footballers[NUM_FOOTBALLERS];
struct ball_data         ball;
struct goal_data         goals[2];           // [0]=left  [1]=right
struct goalie_zone_data  goalie_zones[2];    // [0]=red   [1]=blue
struct game_data         game;
struct wind_data         wind;
struct dust_particle     dust[MAX_DUST];

// Currently human-selected index per team.
// Red  cycles 0->1->2->0  (TAB)
// Blue cycles 4->5->6->4  (M)
int team0_active;
int team1_active;

double kick_charge[NUM_FOOTBALLERS];  // 0..1, charge builds while holding kick key
double kick_timer[NUM_FOOTBALLERS];   // cooldown seconds remaining



// Menu button screen coords — single source of truth used by both process and render
#define MENU_BTN_CX      600   // will be overwritten to window.w/2 at runtime but fine as default
#define MENU_BTN_W       260
#define MENU_BTN_H       50
// PvA button top-left y in screen coords (set relative to window.h at runtime)
// These are computed in render and process using the same formula:
//   pva_y = window.h / 2 - 80
//   pvp_y = window.h / 2 + 10

// Back button (top-left during play)
#define BACK_BTN_X  8
#define BACK_BTN_Y  8
#define BACK_BTN_W  60
#define BACK_BTN_H  26

#endif
