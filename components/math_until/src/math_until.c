#include "math_until.h"


inline point_t Math_Linear_Interpolation(const point_t *p_point_current, const point_t *p_point_end, const float step) {
    point_t point_target;
    point_target.x = p_point_current->x + (p_point_end->x - p_point_current->x) * step;
    point_target.y = p_point_current->y + (p_point_end->y - p_point_current->y) * step;
    point_target.z = p_point_current->z + (p_point_end->z - p_point_current->z) * step;

    return point_target;
}