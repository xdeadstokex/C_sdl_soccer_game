#ifndef RENDER_H
#define RENDER_H
#include <stdio.h>
#include <stdlib.h>

#include "10_data.h"
#include "51_render_helper.h"


//###############################################
//####      main render func                 ####
//###############################################
void render(){
clear_screen(&window);
draw_rect_centered(&window, mouse.x, mouse.y, 10, 10, 0xFF0000FF);

update_screen(&window);
return;
}
//###############################################




#endif
