#ifndef CONTROL_H
#define CONTROL_H
#include <stdio.h>
#include <stdlib.h>

#include "10_data.h"
#include "external_lib/sdl2_wrapper.h"
#include "external_lib/io_logic.h"

int control(){
    int mouse_motion      = 0;
    int mouse_click       = 0;
    int mouse_hold_left   = mouse.left.hold;
    int mouse_hold_right  = mouse.right.hold;
    int mouse_hold_middle = mouse.middle.hold;

    for(int start = 0; get_next_event(&window);){
        if(!start){
            mouse_hold_left   = 0;
            mouse_hold_right  = 0;
            mouse_hold_middle = 0;
            start = 1;
        }
        if(window.quit)   { return 0; }
        if(window.key_q)  { return 0; }
        if(window.mouse_click)        { mouse_click       = 1; }
        if(window.mouse_click_left)   { mouse_hold_left   = 1; }
        if(window.mouse_click_right)  { mouse_hold_right  = 1; }
        if(window.mouse_click_middle) { mouse_hold_middle = 1; }
        if(window.mouse_motion)       { mouse_motion      = 1; }
    }

    mouse.motion = mouse_motion;
    move_mouse(&mouse, window.mouse_x, window.mouse_y);
    mouse.click = mouse_click;
    update_signal(&mouse.left,   mouse_hold_left);
    update_signal(&mouse.right,  mouse_hold_right);
    update_signal(&mouse.middle, mouse_hold_middle);

    return 1;
}

#endif
