#ifndef IO_LOGIC_H
#define IO_LOGIC_H




//############  mouse section  ##################
struct signal_data{
int hold;
int hold_last;
int click;
int unclick;
};


struct mouse_data{
int x; int y;
int x_last; int y_last;
int dx; int dy;
int vx; int vy;
int vx_last; int vy_last;
int motion;

int click;

struct signal_data left;
struct signal_data middle;
struct signal_data right;
/*
int hold_left; int hold_left_last;
int click_left; int unclick_left;

int hold_right; int hold_right_last;
int click_right; int unclick_right;

int hold_middle; int hold_middle_last;
int click_middle; int unclick_middle;
*/
int scroll;
int scroll_v;

void* grab;
void (*gui_process)(void);
int new_grab; // to check whether new grab
};


void set_mouse(struct mouse_data* mouse, int x, int y){
mouse->x = x; mouse->y = y;
mouse->x_last = x; mouse->y_last = y;
mouse->dx = 0; mouse->dy = 0;
mouse->vx = 0; mouse->vy = 0;
mouse->vx_last = 0; mouse->vy_last = 0;
mouse->click = 0;

mouse->left.hold = 0; mouse->left.hold_last = 0;
mouse->left.click = 0; mouse->left.unclick = 0;

mouse->middle.hold = 0; mouse->middle.hold_last = 0;
mouse->middle.click = 0; mouse->middle.unclick = 0;

mouse->right.hold = 0; mouse->right.hold_last = 0;
mouse->right.click = 0; mouse->right.unclick = 0;
/*
mouse->hold_left = 0; mouse->hold_left_last = 0;
mouse->click_left = 0; mouse->unclick_left = 0;

mouse->hold_right = 0; mouse->hold_right_last = 0;
mouse->click_right = 0; mouse->unclick_right = 0;

mouse->hold_middle = 0; mouse->hold_middle_last = 0;
mouse->click_middle = 0; mouse->unclick_middle = 0;
*/

mouse->scroll = 0;
mouse->scroll_v = 0;

mouse->grab = NULL;
mouse->gui_process = NULL;
return;
}


void move_mouse(struct mouse_data* mouse, double x, double y){
mouse->x_last = mouse->x;
mouse->y_last = mouse->y;
mouse->x = x;
mouse->y = y;
mouse->dx = x - mouse->x_last;
mouse->dy = y - mouse->y_last;
return;
}


void update_signal(void* input, int signal){
struct signal_data* signal_info = (struct signal_data*) input;
signal_info->hold_last = signal_info->hold;
signal_info->hold = signal;

signal_info->click = signal && !signal_info->hold_last;
signal_info->unclick = !signal && signal_info->hold_last;
return;
}


#endif
