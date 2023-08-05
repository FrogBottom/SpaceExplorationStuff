#pragma once

// C standard library stuff
#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GMath.h"

// Integer typedefs
typedef int32_t s32;
typedef int64_t s64;

// To get array length. Doesn't work for empty arrays, or anything that has been cast to a pointer.
#define ARRAYCOUNT(x) (sizeof(x) / sizeof(x[0]))