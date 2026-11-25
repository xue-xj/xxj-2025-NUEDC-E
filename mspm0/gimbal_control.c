#include "gimbal_control.h"
#include "motion_math.h"
#include "gimbal_config.h"
#include <string.h>
void GimbalControl_Init(GimbalControl *state) {
    memset(state, 0, sizeof(*state));
    state->mode = GIMBAL_SCAN;
}
void GimbalControl_NextMode(GimbalControl *state) {
    state->mode = (GimbalMode)((state->mode + 1) % 3);
    state->scan_leg = 0;
}
void GimbalControl_Update(GimbalControl *state, uint32_t now, const GimbalCommand *command,
                          uint8_t keys, uint8_t busy, GimbalAction *action) {
    memset(action, 0, sizeof(*action));
    /* 先处理视觉结果，再判断扫描下一段，保证视觉指令可以立即抢占扫描。 */
    if (command && state->mode == GIMBAL_SCAN) {
        state->mode = GIMBAL_TRACK;
        action->stop_all = 1;
    }
    if (state->mode == GIMBAL_TRACK) {
        action->speed = GIMBAL_TRACK_SPEED;
        if (command) {
            action->update_axes = 3; /* 新帧同时替换两轴预算，零步轴也停止旧动作。 */
            action->x_direction = command->x_direction;
            action->y_direction = command->y_direction;
            action->x_steps = command->x_steps;
            action->y_steps = command->y_steps;
            if (!command->x_steps && !command->y_steps)
                action->stop_all = 1;
        }
    } else if (state->mode == GIMBAL_MANUAL) {
        action->speed = GIMBAL_MANUAL_SPEED;
        if ((uint32_t)(now - state->manual_at) < GIMBAL_MANUAL_INTERVAL_MS)
            return;
        state->manual_at = now;
        if (!!(keys & GIMBAL_KEY_X_POS) != !!(keys & GIMBAL_KEY_X_NEG)) {
            action->x_direction = !!(keys & GIMBAL_KEY_X_POS);
            action->x_steps = Motion_AngleToSteps(100);
        }
        if (!!(keys & GIMBAL_KEY_Y_POS) != !!(keys & GIMBAL_KEY_Y_NEG)) {
            action->y_direction = !!(keys & GIMBAL_KEY_Y_NEG);
            action->y_steps = Motion_AngleToSteps(100);
        }
    } else {
        action->speed = GIMBAL_SCAN_SPEED;
        if (busy)
            return;
        /* 闭合扫描路径，第四段回到起点，避免连续循环累积Y轴位移。 */
        switch (state->scan_leg) {
        case 0:
            action->x_direction = 1;
            action->x_steps = Motion_AngleToSteps(90000);
            break;
        case 1:
            action->y_direction = 0;
            action->y_steps = Motion_AngleToSteps(30000);
            break;
        case 2:
            action->x_direction = 0;
            action->x_steps = Motion_AngleToSteps(90000);
            break;
        default:
            action->y_direction = 1;
            action->y_steps = Motion_AngleToSteps(30000);
            break;
        }
        state->scan_leg = (state->scan_leg + 1) % 4;
    }
}
