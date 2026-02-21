#ifndef RENDER_H
#define RENDER_H
#include <stdio.h>
#include <stdlib.h>

#include "10_data.h"
#include "51_render_helper.h"

void render(){
    clear_screen(&window);

    if(game_mode == MODE_MENU){
        draw_field();
        draw_menu();
    } else {
        draw_field();
        draw_goals();
        draw_goalie_zones();
        draw_dust();
        for(int i = 0; i < NUM_FOOTBALLERS; i++)
            draw_footballer(&footballers[i]);
        draw_ball();
        draw_wind_indicator();
        draw_score();
        draw_back_button();
        draw_controls_hint();
    }

    update_screen(&window);
    cap_fps(60);
}

#endif
