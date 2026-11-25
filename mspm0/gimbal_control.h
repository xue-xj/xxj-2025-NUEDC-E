#ifndef GIMBAL_CONTROL_H
#define GIMBAL_CONTROL_H
#include "gimbal_protocol.h"
typedef enum { GIMBAL_MANUAL = 0, GIMBAL_TRACK = 1, GIMBAL_SCAN = 2 } GimbalMode;
enum { GIMBAL_KEY_X_POS = 1, GIMBAL_KEY_X_NEG = 2, GIMBAL_KEY_Y_POS = 4, GIMBAL_KEY_Y_NEG = 8 };
typedef struct {
    GimbalMode mode;
    uint8_t scan_leg;
    uint32_t manual_at;
} GimbalControl;
typedef struct {
    uint8_t stop_all, x_direction, y_direction, update_axes;
    uint32_t x_steps, y_steps;
    uint16_t speed;
} GimbalAction;
void GimbalControl_Init(GimbalControl *state);
void GimbalControl_NextMode(GimbalControl *state);
/* busy位0=X、位1=Y；返回动作由硬件适配层执行，核心本身不操作寄存器。 */
void GimbalControl_Update(GimbalControl *state, uint32_t now, const GimbalCommand *command,
                          uint8_t keys, uint8_t busy, GimbalAction *action);
#endif
