#pragma once

void* safe_alloc(int size);

// [min, max)
// return non-zero (true) if in range, zero (false) otherwise
int in_range(int value, int min, int max);