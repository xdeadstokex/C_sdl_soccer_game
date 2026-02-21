#ifndef SDL2_WRAPPER_H
#define SDL2_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>


/*
SDL2 WRAPPER - QUICK REFERENCE
===============================

STRUCTURES
----------
sound_data

img_data

struct cpu_window_data

struct internal_data (internal use)



INITIALIZATION
--------------
init_graphic_lib()
init_window(cpu_window_data* window, int w, int h, const char* name)


WINDOW MANAGEMENT
-----------------
update_screen(cpu_window_data* window)
clear_screen(cpu_window_data* window)
get_next_event(cpu_window_data* window)


TIME FUNCTIONS
--------------
get_time()
get_time_sec()
wait(unsigned int millisec)
wait_sec(double sec)
cap_fps(double fps)

SOUND FUNCTIONS
---------------
load_sound(sound_data* sound, const char* path)
play_sound(sound_data* sound)
play_sound_loop(sound_data* sound)
stop_sound(sound_data* sound)
set_sound_volume(sound_data* sound, int vol)
free_sound(sound_data* sound)


SHAPE DRAWING
-------------
draw_line(struct cpu_window_data* window, int x0, int y0, int x1, int y1, int color)
draw_rect(struct cpu_window_data* window, int x, int y, int w, int h, int color)
draw_rect_centered(struct cpu_window_data* window, int x, int y, int w, int h, int color)

IMAGE LOADING
-------------
load_img(cpu_window_data* window, img_data* img, const char* path)


IMAGE DRAWING - BASIC
---------------------
draw_img(cpu_window_data* window, img_data* img, int x, int y)
draw_img_part(cpu_window_data* window, img_data* img, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y)


IMAGE DRAWING - ROTATION
------------------------
draw_img_rotate(cpu_window_data* window, img_data* img, int x, int y, double angle)
draw_img_part_rotate(cpu_window_data* window, img_data* img, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, double angle)


IMAGE DRAWING - SCALING
-----------------------
draw_img_scaled(cpu_window_data* window, img_data* img, int x, int y, double scale_x, double scale_y)
draw_img_scaled_rotate(cpu_window_data* window, img_data* img, int x, int y, double scale_x, double scale_y, double angle)


IMAGE DRAWING - CENTERED
------------------------
draw_img_centered(cpu_window_data* window, img_data* img, int x, int y)
draw_img_rotate_centered(cpu_window_data* window, img_data* img, int x, int y, double angle)
draw_img_scaled_rotate_centered(cpu_window_data* window, img_data* img, int x, int y, int w, int h, double angle)


NOTES
-----
- All scale 
- All rotation angles are in radians
- Sound volume range: 0-128
- Sound types auto-detected: .mp3/.ogg/.flac = music, others = effects
- Renderer uses hardware acceleration
- color is hex 32 bit 0xFFAA11FF ( 0x (hex) FF (red) AA (green) 11 (blue) FF (alpha) )
*/


typedef struct {
void* data;  // Either Mix_Chunk* or Mix_Music*
int type;// 0 = sound effect, 1 = music
int channel; // Channel it's playing on (for sound effects)
} sound_data;


typedef struct {
int w;
int h;
SDL_Texture* data;  // Changed to texture
} img_data;


typedef struct {
double time;
double last_time;
double next_time;
double tagret_delta;
double delta;
int is_time;
} clock_data;


struct cpu_window_data{
int w; int h;
int size;

int mode_resizable;
int full_screen_signal_last;
int full_screen_mode;
int resize_signal;
int quit;

// key section
int key_signal;
int key_a; int key_b; int key_c;
int key_d; int key_e; int key_f;
int key_g; int key_h; int key_i;
int key_j; int key_k; int key_l;
int key_m; int key_n; int key_o;
int key_p; int key_q; int key_r;
int key_s; int key_t; int key_u;
int key_v; int key_w; int key_x;
int key_y; int key_z;

int key_0; int key_1; int key_2;
int key_3; int key_4; int key_5;
int key_6; int key_7; int key_8;
int key_9;

int key_arrow_up; int key_arrow_down;
int key_arrow_left; int key_arrow_right;

int key_space;
int key_enter;
int key_escape;
int key_tab;
int key_backspace;
int key_capslock;
int key_shift_l; int key_shift_r;
int key_ctrl_l; int key_ctrl_r;
int key_alt_l; int key_alt_r;

// mouse section
int mouse_motion;
int mouse_x;
int mouse_y;

int mouse_click;
int mouse_click_left;
int mouse_click_right;
int mouse_click_middle;

int mouse_scroll;
int mouse_scroll_v;

char name[1024];

void* data;
void* data_1;
void* data_2;
void* data_3;
void* data_4;
};


struct internal_data{
SDL_Event* sdl_event;
SDL_Window* sdl_window;
SDL_Renderer* sdl_renderer;  // Added renderer
};


int init_graphic_lib(){
if(SDL_Init(SDL_INIT_VIDEO) < 0){
printf("SDL_Init Error: %s\n", SDL_GetError()); 
return 0; 
}


int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
if(!(IMG_Init(imgFlags) & imgFlags)){
printf("SDL_image init fail! Error: %s\n", IMG_GetError());
return 0;
}

if(SDL_Init(SDL_INIT_AUDIO) < 0){
printf("SDL_Init Audio: %s\n", SDL_GetError());
return 0;
}

if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
// Try fallback drivers
SDL_setenv("SDL_AUDIODRIVER", "pulseaudio", 1);
if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
SDL_setenv("SDL_AUDIODRIVER", "alsa", 1);
if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
printf("Mix_OpenAudio failed all drivers: %s\n", Mix_GetError());
return 0;
}
printf("Audio: Using dummy driver (no sound)\n");
}
}
}

Mix_AllocateChannels(16);

return 1;
}


int init_window(struct cpu_window_data* window, int w, int h, const char* window_name){
memset(window, 0x00, sizeof(struct cpu_window_data));

SDL_Window* sdl_window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
SDL_SetWindowResizable(sdl_window, 1);

SDL_Renderer* sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);

struct internal_data* internal = malloc(sizeof(struct internal_data));
internal->sdl_event = malloc(sizeof(SDL_Event));
internal->sdl_window = sdl_window;
internal->sdl_renderer = sdl_renderer;

window->w = w;
window->h = h;
window->size = w * h;
window->data = internal;

return 1;
}


void update_screen(struct cpu_window_data* window){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_RenderPresent(internal->sdl_renderer);
return;
}


void clear_screen(struct cpu_window_data* window){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_SetRenderDrawColor(internal->sdl_renderer, 0x00, 0x00, 0x00, 0x00);
SDL_RenderClear(internal->sdl_renderer);
return;
}


int get_next_event(struct cpu_window_data* window){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Event* event = internal->sdl_event;
if(!SDL_PollEvent(event)){ return 0; }

// Reset per-frame flags
window->resize_signal = 0;
window->mouse_motion = 0;
window->mouse_click = 0;
window->mouse_scroll = 0;

window->quit = (event->type == SDL_QUIT);

if(event->type == SDL_WINDOWEVENT){
if(event->window.event == SDL_WINDOWEVENT_RESIZED){
window->w = event->window.data1;
window->h = event->window.data2;
window->size = window->w * window->h;
window->resize_signal = 1;
}
}

// Mouse motion
if(event->type == SDL_MOUSEMOTION){
window->mouse_motion = 1;
window->mouse_x = event->motion.x;
window->mouse_y = event->motion.y;
}

// Mouse button down
if(event->type == SDL_MOUSEBUTTONDOWN){
window->mouse_click = 1;
if(event->button.button == SDL_BUTTON_LEFT) window->mouse_click_left = 1;
if(event->button.button == SDL_BUTTON_RIGHT) window->mouse_click_right = 1;
if(event->button.button == SDL_BUTTON_MIDDLE) window->mouse_click_middle = 1;
window->mouse_x = event->button.x;
window->mouse_y = event->button.y;
}

// Mouse button up
if(event->type == SDL_MOUSEBUTTONUP){
if(event->button.button == SDL_BUTTON_LEFT) window->mouse_click_left = 0;
if(event->button.button == SDL_BUTTON_RIGHT) window->mouse_click_right = 0;
if(event->button.button == SDL_BUTTON_MIDDLE) window->mouse_click_middle = 0;
window->mouse_x = event->button.x;
window->mouse_y = event->button.y;
}

// Mouse wheel
if(event->type == SDL_MOUSEWHEEL){
window->mouse_scroll = 1;
window->mouse_scroll_v = event->wheel.y; // positive = up, negative = down
}

// Keyboard
int key_signal = -1;
if(event->type == SDL_KEYDOWN) key_signal = 1;
else if(event->type == SDL_KEYUP) key_signal = 0;
if(key_signal >= 0){
switch(event->key.keysym.sym){
case SDLK_a: window->key_a = key_signal; break;
case SDLK_b: window->key_b = key_signal; break;
case SDLK_c: window->key_c = key_signal; break;
case SDLK_d: window->key_d = key_signal; break;
case SDLK_e: window->key_e = key_signal; break;
case SDLK_f: window->key_f = key_signal; break;
case SDLK_g: window->key_g = key_signal; break;
case SDLK_h: window->key_h = key_signal; break;
case SDLK_i: window->key_i = key_signal; break;
case SDLK_j: window->key_j = key_signal; break;
case SDLK_k: window->key_k = key_signal; break;
case SDLK_l: window->key_l = key_signal; break;
case SDLK_m: window->key_m = key_signal; break;
case SDLK_n: window->key_n = key_signal; break;
case SDLK_o: window->key_o = key_signal; break;
case SDLK_p: window->key_p = key_signal; break;
case SDLK_q: window->key_q = key_signal; break;
case SDLK_r: window->key_r = key_signal; break;
case SDLK_s: window->key_s = key_signal; break;
case SDLK_t: window->key_t = key_signal; break;
case SDLK_u: window->key_u = key_signal; break;
case SDLK_v: window->key_v = key_signal; break;
case SDLK_w: window->key_w = key_signal; break;
case SDLK_x: window->key_x = key_signal; break;
case SDLK_y: window->key_y = key_signal; break;
case SDLK_z: window->key_z = key_signal; break;

case SDLK_0: window->key_0 = key_signal; break;
case SDLK_1: window->key_1 = key_signal; break;
case SDLK_2: window->key_2 = key_signal; break;
case SDLK_3: window->key_3 = key_signal; break;
case SDLK_4: window->key_4 = key_signal; break;
case SDLK_5: window->key_5 = key_signal; break;
case SDLK_6: window->key_6 = key_signal; break;
case SDLK_7: window->key_7 = key_signal; break;
case SDLK_8: window->key_8 = key_signal; break;
case SDLK_9: window->key_9 = key_signal; break;

case SDLK_UP: window->key_arrow_up = key_signal; break;
case SDLK_DOWN: window->key_arrow_down = key_signal; break;
case SDLK_LEFT: window->key_arrow_left = key_signal; break;
case SDLK_RIGHT: window->key_arrow_right = key_signal; break;

case SDLK_SPACE: window->key_space = key_signal; break;
case SDLK_RETURN: window->key_enter = key_signal; break;
case SDLK_ESCAPE: window->key_escape = key_signal; break;
case SDLK_TAB: window->key_tab = key_signal; break;
case SDLK_BACKSPACE: window->key_backspace = key_signal; break;
case SDLK_CAPSLOCK: window->key_capslock = key_signal; break;
case SDLK_LSHIFT: window->key_shift_l = key_signal; break;
case SDLK_RSHIFT: window->key_shift_r = key_signal; break;
case SDLK_LCTRL: window->key_ctrl_l = key_signal; break;
case SDLK_RCTRL: window->key_ctrl_r = key_signal; break;
case SDLK_LALT: window->key_alt_l = key_signal; break;
case SDLK_RALT: window->key_alt_r = key_signal; break;
}
return 1;
}

return 1;
}

/*
double get_time(){
return SDL_GetTicks();
}

double get_time_sec(){
return (double)(SDL_GetTicks()) / 1000;
}
*/

static inline double get_time_sec(){
static Uint64 freq = 0;
if(freq == 0){ freq = SDL_GetPerformanceFrequency(); }
return (double)SDL_GetPerformanceCounter() / freq;
}


static inline double get_time(){
return get_time_sec() * 1000.0;  // milliseconds
}


void wait(unsigned int millisec){
SDL_Delay(millisec);
return;
}

void wait_sec(double sec){
SDL_Delay(sec * 1000);
return;
}

void reset_clock(clock_data* clock, double tagret_delta){
clock->time = 0;
clock->last_time = 0;
clock->next_time = 0;
clock->tagret_delta = 0;
clock->delta = 0;
clock->is_time = 0;
return;
}


void update_clock(clock_data* clock){
clock->is_time = 0;
double last_time = clock->time;
clock->time = get_time_sec();

if(clock->next_time == 0){
clock->next_time = clock->time + clock->tagret_delta;
}

if(clock->time >= clock->next_time){
clock->is_time = 1;
clock->next_time += clock->tagret_delta - (clock->next_time - clock->time);
}

clock->delta = clock->time - clock->last_time;
clock->last_time = last_time;
return;
}

double cap_fps(double fps){
static double next_frame_time_sec = 0;
double target_frame_time = 1.0 / fps;
double current_time = get_time_sec();

if (next_frame_time_sec == 0){
next_frame_time_sec = current_time;
}


double sleep_time = next_frame_time_sec - current_time;
if (sleep_time > 0){
wait_sec(sleep_time);
/*
for(;;){
double time = get_time_sec();
if(next_frame_time_sec - time <= 0){
next_frame_time_sec = get_time_sec();
break;
}
}
*/
next_frame_time_sec = get_time_sec();
}

next_frame_time_sec += target_frame_time;  // Preserve cadence


return 1.0 / ((double)(get_time_sec()) - current_time);
}



// Load sound (auto-detects type from extension)
void load_sound(sound_data* sound, const char* path){
sound->data = NULL;
sound->channel = -1;

// Get extension
const char* ext = strrchr(path, '.');
if(ext == NULL) return;

// .mp3, .ogg, .flac = music, everything else = sound effect
if(strcmp(ext, ".mp3") == 0 || strcmp(ext, ".ogg") == 0 || strcmp(ext, ".flac") == 0){
sound->type = 1;
sound->data = Mix_LoadMUS(path);
if(sound->data == NULL) printf("Mix_LoadMUS: %s\n", Mix_GetError());
}
else{
sound->type = 0;
sound->data = Mix_LoadWAV(path);
if(sound->data == NULL) printf("Mix_LoadWAV: %s\n", Mix_GetError());
}
return;
}


// Play sound
void play_sound(sound_data* sound){
if(sound->data == NULL) return;

if(sound->type == 0){ sound->channel = Mix_PlayChannel(-1, (Mix_Chunk*)sound->data, 0); }
else{ Mix_PlayMusic((Mix_Music*)sound->data, 0); }
return;
}


// Play sound looped
void play_sound_loop(sound_data* sound){
if(sound->data == NULL) return;

if(sound->type == 0){ sound->channel = Mix_PlayChannel(-1, (Mix_Chunk*)sound->data, -1); }
else{ Mix_PlayMusic((Mix_Music*)sound->data, -1); }
return;
}


// Stop sound
void stop_sound(sound_data* sound){
if(sound->data == NULL) return;

if(sound->type == 0){
if(sound->channel >= 0) Mix_HaltChannel(sound->channel);
}
else{ Mix_HaltMusic(); }
return;
}


// Set volume (0-128)
void set_sound_volume(sound_data* sound, int vol){
if(sound->data == NULL) return;

if(sound->type == 0){ Mix_VolumeChunk((Mix_Chunk*)sound->data, vol); }
else{ Mix_VolumeMusic(vol); }
return;
}


void free_sound(sound_data* sound){
if(sound->data == NULL) return;

if(sound->type == 0){ Mix_FreeChunk((Mix_Chunk*)sound->data); }
else{ Mix_FreeMusic((Mix_Music*)sound->data); }
sound->data = NULL;
return;
}









void load_img(struct cpu_window_data* window, img_data* img, const char* path){
struct internal_data* internal = (struct internal_data*) window->data;

SDL_Surface* temp = IMG_Load(path);
if(temp == NULL){
printf("IMG_Load: %s\n", IMG_GetError());
img->data = NULL;
return;
}

img->w = temp->w;
img->h = temp->h;
img->data = SDL_CreateTextureFromSurface(internal->sdl_renderer, temp);
SDL_FreeSurface(temp);

return;
}


static inline double get_physic_y_cor(double window_h, double y){
return window_h - y - 1; // -1 bcs sdl mouse pos base on pixel cor
}


void draw_line(struct cpu_window_data* window, int x0, int y0, int x1, int y1, int color){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_SetRenderDrawColor(internal->sdl_renderer, color >> 24, color >> 16, color >> 8, color);
SDL_RenderDrawLine(internal->sdl_renderer, x0, y0, x1, y1);
return;
}



void draw_rect(struct cpu_window_data* window, int x, int y, int w, int h, int color){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_SetRenderDrawColor(internal->sdl_renderer, color >> 24, color >> 16, color >> 8, color);
SDL_RenderFillRect(internal->sdl_renderer, &(SDL_Rect){x, y, w, h});
return;
}

void draw_rect_centered(struct cpu_window_data* window, int x, int y, int w, int h, int color){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_SetRenderDrawColor(internal->sdl_renderer, color >> 24, color >> 16, color >> 8, color);
SDL_RenderFillRect(internal->sdl_renderer, &(SDL_Rect){x - w / 2, y - h / 2, w, h});
return;
}


void draw_rect_bottom_left(struct cpu_window_data* window, int x, int y, int w, int h, int color){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_SetRenderDrawColor(internal->sdl_renderer, color >> 24, color >> 16, color >> 8, color);
SDL_RenderFillRect(internal->sdl_renderer, &(SDL_Rect){x, get_physic_y_cor(window->h, y + h), w, h});
return;
}



void draw_img(struct cpu_window_data* window, img_data* img, int x, int y){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect dst = {x, y, img->w, img->h};
SDL_RenderCopy(internal->sdl_renderer, img->data, NULL, &dst);
return;
}



// Draw part of image (sprite sheet support)
void draw_img_part(struct cpu_window_data* window, img_data* img, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect src = {src_x, src_y, src_w, src_h};
SDL_Rect dst = {dst_x, dst_y, src_w, src_h};
SDL_RenderCopy(internal->sdl_renderer, img->data, &src, &dst);
return;
}

// Draw part with rotation (angle in radians)
void draw_img_part_rotate(struct cpu_window_data* window, img_data* img, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, double angle){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect src = {src_x, src_y, src_w, src_h};
SDL_Rect dst = {dst_x, dst_y, src_w, src_h};
SDL_RenderCopyEx(internal->sdl_renderer, img->data, &src, &dst, angle * 180.0 / M_PI, NULL, SDL_FLIP_NONE);
return;
}


// Original rotate functions also in radians
void draw_img_rotate(struct cpu_window_data* window, img_data* img, int x, int y, double angle){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect dst = {x, y, img->w, img->h};
SDL_RenderCopyEx(internal->sdl_renderer, img->data, NULL, &dst, angle * 180.0 / M_PI, NULL, SDL_FLIP_NONE);
return;
}


// Draw scaled
void draw_img_scaled(struct cpu_window_data* window, img_data* img, int x, int y, double scale_x, double scale_y){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect dst = {x, y, scale_x * img->w, scale_y * img->h};
SDL_RenderCopy(internal->sdl_renderer, img->data, NULL, &dst);
return;
}

// Draw scaled with rotation (angle in radians)
void draw_img_scaled_rotate(struct cpu_window_data* window, img_data* img, int x, int y, double scale_x, double scale_y, double angle){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect dst = {x, y, scale_x * img->w, scale_y * img->h};
SDL_RenderCopyEx(internal->sdl_renderer, img->data, NULL, &dst, angle * 180.0 / M_PI, NULL, SDL_FLIP_NONE);
return;
}


// Draw centered (x,y is center point)
void draw_img_centered(struct cpu_window_data* window, img_data* img, int x, int y){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect dst = {x - img->w/2, y - img->h/2, img->w, img->h};
SDL_RenderCopy(internal->sdl_renderer, img->data, NULL, &dst);
return;
}

// Draw centered with rotation (rotates around center, angle in radians)
void draw_img_rotate_centered(struct cpu_window_data* window, img_data* img, int x, int y, double angle){
struct internal_data* internal = (struct internal_data*) window->data;
SDL_Rect dst = {x - img->w/2, y - img->h/2, img->w, img->h};
SDL_Point center = {img->w/2, img->h/2};
SDL_RenderCopyEx(internal->sdl_renderer, img->data, NULL, &dst, angle * 180.0 / M_PI, &center, SDL_FLIP_NONE);
return;
}

// Draw centered and scaled with rotation (angle in radians)
void draw_img_scaled_rotate_centered(struct cpu_window_data* window, img_data* img, int x, int y, double scale_x, double scale_y, double angle){
struct internal_data* internal = (struct internal_data*) window->data;
double scaled_w = scale_x * img->w;
double scaled_h = scale_y * img->h;
SDL_Rect dst = {x - scaled_w / 2, y - scaled_h / 2, scaled_w, scaled_h};
SDL_Point center = {scaled_w / 2, scaled_h / 2};
SDL_RenderCopyEx(internal->sdl_renderer, img->data, NULL, &dst, angle * 180.0 / M_PI, &center, SDL_FLIP_NONE);
return;
}

#endif