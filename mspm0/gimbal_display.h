#ifndef GIMBAL_DISPLAY_H
#define GIMBAL_DISPLAY_H
#include <stdint.h>
void GimbalDisplay_Init(void);
int GimbalDisplay_Idle(void);
void GimbalDisplay_Begin(void);
void GimbalDisplay_Poll(uint32_t now);
extern volatile uint32_t g_display_errors;
#endif
