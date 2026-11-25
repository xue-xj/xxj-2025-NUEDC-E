#ifndef MOTION_MATH_H
#define MOTION_MATH_H
#include <stdint.h>
uint32_t Motion_AngleToSteps(uint32_t millidegrees);
uint32_t Motion_SpeedToFrequency(uint16_t degrees_per_second);
#endif
