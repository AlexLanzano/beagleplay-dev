#include "renderer.h"
#include "game.h"
#include "log.h"

int main(int argc, char **argv)
{
    int error;
    
    error = renderer_init("/dev/dri/card0");
    if (error) {
        log_error("Failed to init renderer\n");
        return error;
    }

    error = game_init();
    if (error) {
        log_error("Failed to init game\n");
        return error;
    }

    game_run();
}


