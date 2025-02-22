#include <cstdio>
#include <cstdlib>

// headers
#include "../core/globals.hpp"
#include "error.hpp"

void info(const char *message, const char *log)
{
    if (log)
        fprintf(stderr, "[INFO] %s: %s\n", message, log);
    else
        fprintf(stderr, "[INFO] %s\n", message);
}

void warn(const char *message, const char *log)
{
    if (log)
        fprintf(stderr, "[WARN] %s: %s\n", message, log);
    else
        fprintf(stderr, "[WARN] %s\n", message);
}

void critical(const char *message, const char *log)
{
    if (log)
        fprintf(stderr, "[CRITICAL] %s: %s\n", message, log);
    else
        fprintf(stderr, "[CRITICAL] %s\n", message);

    exit(EXIT_FAILURE);
}
