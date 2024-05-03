#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define log(format, ...) printf(format, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define log_info(format, ...) log("INFO: (%s) " format __VA_OPT__(,) __VA_ARGS__)
#define log_error(format, ...) log("INFO: (%s) " format __VA_OPT__(,) __VA_ARGS__)
#define log_debug(format, ...) log("INFO: (%s) " format __VA_OPT__(,) __VA_ARGS__)

#endif
