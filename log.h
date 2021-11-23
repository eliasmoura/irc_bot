#ifndef LOG_H
#define LOG_H
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#define log_error(msg, ...) fprintf(stderr, "%s:%u: ERROR: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_warn(msg, ...)  fprintf(stderr, "%s:%u: WARNNING: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(msg, ...)  fprintf(stderr, "INFO: " msg "\n", ##__VA_ARGS__)
#endif // LOG_H
