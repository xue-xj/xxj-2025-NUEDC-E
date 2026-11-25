#ifndef GIMBAL_STEP_MOTOR_H
#define GIMBAL_STEP_MOTOR_H
#include <stdint.h>
void StepMotor_Init(void);
void StepMotor_SetSpeed(uint16_t speed, uint8_t axis);
void StepMotor_Move(uint8_t direction, uint32_t steps, uint8_t axis);
void StepMotor_Stop(uint8_t axis);
uint8_t StepMotor_Busy(void);
uint32_t StepMotor_Remaining(uint8_t axis);
/* axis=1为X(TIMA1/PA15)，axis=2为Y(TIMG12/PA14)，完整接线见docs/wiring.md。 */
#endif
