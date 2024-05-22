#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdio.h>
enum state {
    GAME_STATE_RUNNING,
    GAME_STATE_STOP
};

struct game_object {
    uint32_t fps;
    double width;
    double height;
    double x;
    double y;
    double dx;
    double dy;
};

struct game_state {
    enum state state;
    struct game_object paddle;    
    int fps;
    int msec_per_frame;
    int iio_fd;
};

int game_init();
int game_run();

#endif
