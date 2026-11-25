#include "motion_math.h"
#include "gimbal_config.h"
uint32_t Motion_AngleToSteps(uint32_t millidegrees) {
    /* 使用整数单位避免把50步误解释为50度；毫度输入采用向下取整。 */
    return (uint32_t)((uint64_t)millidegrees * GIMBAL_STEP_MILLIDEG_DENOMINATOR /
                      GIMBAL_STEP_MILLIDEG_NUMERATOR);
}
uint32_t Motion_SpeedToFrequency(uint16_t degrees_per_second) {
    return (uint32_t)degrees_per_second * 1000u * GIMBAL_STEP_MILLIDEG_DENOMINATOR /
           GIMBAL_STEP_MILLIDEG_NUMERATOR;
}
