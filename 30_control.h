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
    int key_signal        = 0;  // any key event this frame (mirrors mouse.click)

    // Seed holds from last frame
    int mouse_hold_left   = mouse.left.hold;
    int mouse_hold_right  = mouse.right.hold;
    int mouse_hold_middle = mouse.middle.hold;

    int k_a=kb.key_a.hold, k_b=kb.key_b.hold, k_c=kb.key_c.hold;
    int k_d=kb.key_d.hold, k_e=kb.key_e.hold, k_f=kb.key_f.hold;
    int k_g=kb.key_g.hold, k_h=kb.key_h.hold, k_i=kb.key_i.hold;
    int k_j=kb.key_j.hold, k_k=kb.key_k.hold, k_l=kb.key_l.hold;
    int k_m=kb.key_m.hold, k_n=kb.key_n.hold, k_o=kb.key_o.hold;
    int k_p=kb.key_p.hold, k_q=kb.key_q.hold, k_r=kb.key_r.hold;
    int k_s=kb.key_s.hold, k_t=kb.key_t.hold, k_u=kb.key_u.hold;
    int k_v=kb.key_v.hold, k_w=kb.key_w.hold, k_x=kb.key_x.hold;
    int k_y=kb.key_y.hold, k_z=kb.key_z.hold;
    int k_0=kb.key_0.hold, k_1=kb.key_1.hold, k_2=kb.key_2.hold;
    int k_3=kb.key_3.hold, k_4=kb.key_4.hold, k_5=kb.key_5.hold;
    int k_6=kb.key_6.hold, k_7=kb.key_7.hold, k_8=kb.key_8.hold;
    int k_9=kb.key_9.hold;
    int k_up=kb.key_arrow_up.hold,     k_down=kb.key_arrow_down.hold;
    int k_left=kb.key_arrow_left.hold, k_right=kb.key_arrow_right.hold;
    int k_space=kb.key_space.hold,     k_enter=kb.key_enter.hold;
    int k_escape=kb.key_escape.hold,   k_tab=kb.key_tab.hold;
    int k_backspace=kb.key_backspace.hold, k_capslock=kb.key_capslock.hold;
    int k_shift_l=kb.key_shift_l.hold, k_shift_r=kb.key_shift_r.hold;
    int k_ctrl_l=kb.key_ctrl_l.hold,   k_ctrl_r=kb.key_ctrl_r.hold;
    int k_alt_l=kb.key_alt_left.hold,  k_alt_r=kb.key_alt_right.hold;

    for(int start = 0; get_next_event(&window);){

        // First event: reset all holds to 0 (smooth-click pattern)
        if(!start){
            mouse_hold_left=0; mouse_hold_right=0; mouse_hold_middle=0;
            k_a=0;k_b=0;k_c=0;k_d=0;k_e=0;k_f=0;k_g=0;k_h=0;k_i=0;k_j=0;
            k_k=0;k_l=0;k_m=0;k_n=0;k_o=0;k_p=0;k_q=0;k_r=0;k_s=0;k_t=0;
            k_u=0;k_v=0;k_w=0;k_x=0;k_y=0;k_z=0;
            k_0=0;k_1=0;k_2=0;k_3=0;k_4=0;k_5=0;k_6=0;k_7=0;k_8=0;k_9=0;
            k_up=0;k_down=0;k_left=0;k_right=0;
            k_space=0;k_enter=0;k_escape=0;k_tab=0;
            k_backspace=0;k_capslock=0;
            k_shift_l=0;k_shift_r=0;k_ctrl_l=0;k_ctrl_r=0;
            k_alt_l=0;k_alt_r=0;
            start = 1;
        }

        if(window.quit) { return 0; }

        if(window.mouse_click)        { mouse_click     = 1; }
        if(window.mouse_click_left)   { mouse_hold_left   = 1; }
        if(window.mouse_click_right)  { mouse_hold_right  = 1; }
        if(window.mouse_click_middle) { mouse_hold_middle = 1; }
        if(window.mouse_motion)       { mouse_motion      = 1; }

        if(window.key_a){ k_a=1; key_signal=1; }
        if(window.key_b){ k_b=1; key_signal=1; }
        if(window.key_c){ k_c=1; key_signal=1; }
        if(window.key_d){ k_d=1; key_signal=1; }
        if(window.key_e){ k_e=1; key_signal=1; }
        if(window.key_f){ k_f=1; key_signal=1; }
        if(window.key_g){ k_g=1; key_signal=1; }
        if(window.key_h){ k_h=1; key_signal=1; }
        if(window.key_i){ k_i=1; key_signal=1; }
        if(window.key_j){ k_j=1; key_signal=1; }
        if(window.key_k){ k_k=1; key_signal=1; }
        if(window.key_l){ k_l=1; key_signal=1; }
        if(window.key_m){ k_m=1; key_signal=1; }
        if(window.key_n){ k_n=1; key_signal=1; }
        if(window.key_o){ k_o=1; key_signal=1; }
        if(window.key_p){ k_p=1; key_signal=1; }
        if(window.key_q){ k_q=1; key_signal=1; }
        if(window.key_r){ k_r=1; key_signal=1; }
        if(window.key_s){ k_s=1; key_signal=1; }
        if(window.key_t){ k_t=1; key_signal=1; }
        if(window.key_u){ k_u=1; key_signal=1; }
        if(window.key_v){ k_v=1; key_signal=1; }
        if(window.key_w){ k_w=1; key_signal=1; }
        if(window.key_x){ k_x=1; key_signal=1; }
        if(window.key_y){ k_y=1; key_signal=1; }
        if(window.key_z){ k_z=1; key_signal=1; }

        if(window.key_0){ k_0=1; key_signal=1; }
        if(window.key_1){ k_1=1; key_signal=1; }
        if(window.key_2){ k_2=1; key_signal=1; }
        if(window.key_3){ k_3=1; key_signal=1; }
        if(window.key_4){ k_4=1; key_signal=1; }
        if(window.key_5){ k_5=1; key_signal=1; }
        if(window.key_6){ k_6=1; key_signal=1; }
        if(window.key_7){ k_7=1; key_signal=1; }
        if(window.key_8){ k_8=1; key_signal=1; }
        if(window.key_9){ k_9=1; key_signal=1; }

        if(window.key_arrow_up)   { k_up=1;    key_signal=1; }
        if(window.key_arrow_down) { k_down=1;  key_signal=1; }
        if(window.key_arrow_left) { k_left=1;  key_signal=1; }
        if(window.key_arrow_right){ k_right=1; key_signal=1; }

        if(window.key_space)    { k_space=1;     key_signal=1; }
        if(window.key_enter)    { k_enter=1;     key_signal=1; }
        if(window.key_escape)   { k_escape=1;    key_signal=1; }
        if(window.key_tab)      { k_tab=1;       key_signal=1; }
        if(window.key_backspace){ k_backspace=1; key_signal=1; }
        if(window.key_capslock) { k_capslock=1;  key_signal=1; }
        if(window.key_shift_l)  { k_shift_l=1;  key_signal=1; }
        if(window.key_shift_r)  { k_shift_r=1;  key_signal=1; }
        if(window.key_ctrl_l)   { k_ctrl_l=1;   key_signal=1; }
        if(window.key_ctrl_r)   { k_ctrl_r=1;   key_signal=1; }
        if(window.key_alt_l)    { k_alt_l=1;    key_signal=1; }
        if(window.key_alt_r)    { k_alt_r=1;    key_signal=1; }
    }

    // Mouse
    mouse.motion = mouse_motion;
    move_mouse(&mouse, window.mouse_x, window.mouse_y);
    mouse.click = mouse_click;
    update_signal(&mouse.left,   mouse_hold_left);
    update_signal(&mouse.right,  mouse_hold_right);
    update_signal(&mouse.middle, mouse_hold_middle);

    // Keyboard
    kb.key_signal = key_signal;
    update_signal(&kb.key_a, k_a); update_signal(&kb.key_b, k_b);
    update_signal(&kb.key_c, k_c); update_signal(&kb.key_d, k_d);
    update_signal(&kb.key_e, k_e); update_signal(&kb.key_f, k_f);
    update_signal(&kb.key_g, k_g); update_signal(&kb.key_h, k_h);
    update_signal(&kb.key_i, k_i); update_signal(&kb.key_j, k_j);
    update_signal(&kb.key_k, k_k); update_signal(&kb.key_l, k_l);
    update_signal(&kb.key_m, k_m); update_signal(&kb.key_n, k_n);
    update_signal(&kb.key_o, k_o); update_signal(&kb.key_p, k_p);
    update_signal(&kb.key_q, k_q); update_signal(&kb.key_r, k_r);
    update_signal(&kb.key_s, k_s); update_signal(&kb.key_t, k_t);
    update_signal(&kb.key_u, k_u); update_signal(&kb.key_v, k_v);
    update_signal(&kb.key_w, k_w); update_signal(&kb.key_x, k_x);
    update_signal(&kb.key_y, k_y); update_signal(&kb.key_z, k_z);

    update_signal(&kb.key_0, k_0); update_signal(&kb.key_1, k_1);
    update_signal(&kb.key_2, k_2); update_signal(&kb.key_3, k_3);
    update_signal(&kb.key_4, k_4); update_signal(&kb.key_5, k_5);
    update_signal(&kb.key_6, k_6); update_signal(&kb.key_7, k_7);
    update_signal(&kb.key_8, k_8); update_signal(&kb.key_9, k_9);

    update_signal(&kb.key_arrow_up,    k_up);
    update_signal(&kb.key_arrow_down,  k_down);
    update_signal(&kb.key_arrow_left,  k_left);
    update_signal(&kb.key_arrow_right, k_right);

    update_signal(&kb.key_space,     k_space);
    update_signal(&kb.key_enter,     k_enter);
    update_signal(&kb.key_escape,    k_escape);
    update_signal(&kb.key_tab,       k_tab);
    update_signal(&kb.key_backspace, k_backspace);
    update_signal(&kb.key_capslock,  k_capslock);
    update_signal(&kb.key_shift_l,   k_shift_l);
    update_signal(&kb.key_shift_r,   k_shift_r);
    update_signal(&kb.key_ctrl_l,    k_ctrl_l);
    update_signal(&kb.key_ctrl_r,    k_ctrl_r);
    update_signal(&kb.key_alt_left,  k_alt_l);
    update_signal(&kb.key_alt_right, k_alt_r);

    if(kb.key_q.hold) return 0;

    return 1;
}

#endif