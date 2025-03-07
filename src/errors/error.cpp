#include <cstdio>
#include <cstdlib>

// headers
#include "error.hpp"

void error(const char *message, const char *log) {
    if (log)
        fprintf(stderr, "[ERROR] %s: %s\n", message, log);
    else
        fprintf(stderr, "[ERROR] %s\n", message);
}

void debug(const char *message, const char *log) {
    if (log)
        fprintf(stderr, "[DEBUG] %s: %s\n", message, log);
    else
        fprintf(stderr, "[DEBUG] %s\n", message);
}

void info(const char *message, const char *log) {
    if (log)
        fprintf(stderr, "[INFO] %s: %s\n", message, log);
    else
        fprintf(stderr, "[INFO] %s\n", message);
}

void okay(const char *message, const char *log) {
    if (log)
        fprintf(stderr, "[OK] %s: %s\n", message, log);
    else
        fprintf(stderr, "[OK] %s\n", message);
}

void warn(const char *message, const char *log) {
    if (log)
        fprintf(stderr, "[WARN] %s: %s\n", message, log);
    else
        fprintf(stderr, "[WARN] %s\n", message);
}

void critical(const char *message, const char *log) {
    if (log)
        fprintf(stderr, "[CRITICAL] %s: %s\n", message, log);
    else
        fprintf(stderr, "[CRITICAL] %s\n", message);

    exit(EXIT_FAILURE);
}
