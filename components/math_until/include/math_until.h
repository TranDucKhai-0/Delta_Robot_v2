#pragma once

#include "type_data.h"


// Hàm bình phương
static inline float sqr(float x) {
    return x * x; // x^2
}

inline point_t Math_Linear_Interpolation(const point_t *p_point_current, const point_t *p_point_end, const float step);


