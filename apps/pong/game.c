#include "game.h"
#include <time.h>
#include <unistd.h>
#include "renderer.h"

static struct game_state g_game_state = {0};

int game_init()
{
    int error;
    
    // Place paddle at center bottom of the screen
    g_game_state.paddle.width = 0.10;
    g_game_state.paddle.height = 0.05;
    g_game_state.paddle.x = 0.5;
    g_game_state.paddle.y = 1.0 - g_game_state.paddle.height;
    g_game_state.paddle.dx = 0.02;

    g_game_state.fps = 60;
    g_game_state.msec_per_frame = 1000 / g_game_state.fps;
    return 0;
}

static int game_update_objects()
{
    struct game_object *paddle = &g_game_state.paddle; 
    // clear current paddle position
    renderer_draw_rect(paddle->x, paddle->y, paddle->width, paddle->height, 0xffffffff);

    paddle->x += paddle->dx;
    paddle->y += paddle->dy;
    return 0;
}

static int game_check_collision()
{
    struct game_object *paddle = &g_game_state.paddle;

    if (paddle->x + paddle->width/2 >= 1.0) {
        paddle->dx = -0.02;
        paddle->x = 1.0 - paddle->width/2;
    }

    if (paddle->x - paddle->width/2 <= 0.0) {
        paddle->dx = 0.02;
        paddle->x = paddle->width/2;
    }

    return 0;
}

static int game_draw_objects()
{
    struct game_object *paddle = &g_game_state.paddle;
    renderer_draw_rect(paddle->x, paddle->y, paddle->width, paddle->height, 0);
    return 0;
}

int game_run()
{
    int error;

    g_game_state.state = GAME_STATE_RUNNING;
    renderer_draw_rect(0.5, 0.5, 1.0, 1.0, 0xffffffff);    
    renderer_update();
    while (g_game_state.state != GAME_STATE_STOP) {
        int start_time = clock() * 1000 / CLOCKS_PER_SEC;
        
        game_update_objects();
        game_check_collision();
        game_draw_objects();
        renderer_update();
        int end_time = clock() * 1000 / CLOCKS_PER_SEC;
        int time_elapsed = end_time - start_time;
        if (time_elapsed < g_game_state.msec_per_frame) {
            usleep(g_game_state.msec_per_frame - time_elapsed); 
        }
        
    }
    return 0;
}
