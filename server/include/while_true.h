#pragma once

#include "signal.h"

extern volatile sig_atomic_t while_true;

#define WHILE_TRUE() while (while_true)