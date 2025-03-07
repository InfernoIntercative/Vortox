#pragma once

#include "../core/globals.hpp"

void error(const char *message, const char *log = nullptr);
void debug(const char *message, const char *log = nullptr);
void okay(const char *message, const char *log = nullptr);
void info(const char *message, const char *log = nullptr);
void warn(const char *message, const char *log = nullptr);
void critical(const char *message, const char *log = nullptr);

