#ifndef RENDERER_H
#define RENDERER_H
#include <stdint.h>
int renderer_init(const char *path);
int renderer_draw_rect(double x, double y, double w, double h, uint32_t color);
int renderer_update();

#endif
