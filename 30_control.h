#ifndef CONTROL_H
#define CONTROL_H
#include <stdio.h>
#include <stdlib.h>

#include "10_data.h"

#include "external_lib/sdl2_wrapper.h"
#include "external_lib/io_logic.h"


//###############################################
//####           CONTROL SECTION             ####
//###############################################
int control(){
int key_arrow_left = 0;
int key_arrow_right = 0;
int key_arrow_up = 0;

int mouse_motion = 0;
int mouse_click = 0;
int mouse_scroll = 0; // better be reset than unwanted scroll
int mouse_hold_left = mouse.left.hold;
int mouse_hold_right = mouse.right.hold;
int mouse_hold_middle = mouse.middle.hold;
int window_resize_signal = 0; // to make sure no last resize

for(int start = 0; get_next_event(&window);){

// for the purpose of smooth click
//(if a click event cant catch up yet - no event although mouse still hold,
//it should keep last state of click to avoid unwanted signal on off
if(!start){
mouse_hold_left = 0;
mouse_hold_right = 0;
mouse_hold_middle = 0;
start = 1;
}

if(window.quit){ return 0; }
if(window.key_q){ return 0; }


if(window.mouse_click){ mouse_click = 1; }
// to accurately track input and dont let a 0-1-0 signal accident
if(window.mouse_click_left){ mouse_hold_left = 1; }
if(window.mouse_click_right){ mouse_hold_right = 1; }
if(window.mouse_click_middle){ mouse_hold_middle = 1; }

if(window.mouse_motion){ mouse_motion = 1; }
if(window.mouse_scroll){ mouse_scroll = 1; }
if(window.resize_signal){ window_resize_signal = 1; }

if(window.key_arrow_left){ key_arrow_left = 1; }
if(window.key_arrow_right){ key_arrow_right = 1; }
if(window.key_arrow_up){ key_arrow_up = 1; }
}



/*
if(window_resize_signal){
printf("w%d h%d fbw%d fbh%d\n", window.w, window.h, window.fb_w, window.fb_h);
if(
window.fb_w <= window.w && window.fb_h <= window.h
&& !(window.fb_w == window.w && window.fb_h == window.h)
){
printf("resize\n");
refit_framebuffer(&window);
//resize_window_frame_buffer(&window, int w, int h);
//mem_set(window.framebuffer.data, 0xFF, window.framebuffer.size / 2);
fb0.data = window.fb0;
fb0.w = window.fb_w;
fb0.h = window.fb_h;
}

}
//printf("w%d h%d fbw%d fbh%d\n", window.w, window.h, window.fb_w, window.fb_h);
*/

mouse.motion = mouse_motion;
move_mouse(&mouse, window.mouse_x, window.mouse_y); // only need final coor
//int mouse_vx_last = mouse.vx_last;
//int mouse_vy_last = mouse.vy_last;
//mouse.vx_last = mouse.vx;
//mouse.vy_last = mouse.vy;
//mouse.vx = (mouse.dx / cpu_time_used);
//mouse.vy = (mouse.dy / cpu_time_used);
//printf("mdx%d mdy%d \n", mouse.dx, mouse.dy);
//printf("mvx%d mvy%d \n", mouse.vx, mouse.vy);
//printf("time%f fps%f \n", cpu_time_used , 1 / cpu_time_used);
// mouse click and mouse hold
mouse.click = mouse_click;
update_signal(&mouse.left, mouse_hold_left);
update_signal(&mouse.right, mouse_hold_right);
update_signal(&mouse.middle, mouse_hold_middle);
//printf("%d\n", mouse_hold_left);
//printf("hold %d hold_last %d click %d unclick %d\n", mouse.left.hold, mouse.left.hold_last, mouse.click_left, mouse.unclick_left);
//printf("hold %d hold_last %d click %d unclick %d\n", mouse.right.hold, mouse.right.hold_last, mouse.click_right, mouse.unclick_right);
mouse.scroll = mouse_scroll;
mouse.scroll_v = (mouse_scroll) ? window.mouse_scroll_v : 0;



return 1;
}
//###############################################

#endif
