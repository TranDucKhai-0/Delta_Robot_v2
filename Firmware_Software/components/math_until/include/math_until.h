#pragma once

#include "type_data.h"


// Hàm bình phương
static inline float sqr(float x) {
    return x * x; // x^2
}

float Math_Calculate_Distance(const point_t *p1, const point_t *p2);

point_t Math_Linear_Interpolation(const point_t *p_start, const point_t *p_end, const float t) ;

void Math_Low_Pass_Filter(point_t *p_current, const point_t *p_target);

point_t Math_Quadratic_Bezier_Interpolation(const point_t *p_A, const point_t *p_C, const point_t *p_B, const float t);

point_t Math_Move_Towards(const point_t *p_corner, const point_t *p_target, float radius);

float Math_Calculate_Bezier_Length(const point_t *p_A, const point_t *p_C, const point_t *p_B);

// point_t Math_Get_Parabolic_Arc_Point(point_t *point_current, point_t *point_end,const float height,const float t);

