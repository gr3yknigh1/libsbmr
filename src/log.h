#ifndef BREAKOUT_LOG_H_
#define BREAKOUT_LOG_H_

#include <stdio.h>

#define LOG_INFO(...) fprintf(stdout, "I: " __VA_ARGS__)
#define LOG_DEBUG(...) fprintf(stdout, "D: " __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, "E: " __VA_ARGS__)

#endif // BREAKOUT_LOG_H_
