#include "MathUtil.h"

#include <cmath>

#define EPSILON 0.0000001

float remap(float value, float in_min, float in_max, float out_min, float out_max) {
    if (fabs(in_min - in_max) < EPSILON){
        return out_min;
    } else {
        return ((value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min);
    }
}

#undef EPSILON
