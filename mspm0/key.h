#ifndef GIMBAL_KEY_H
#define GIMBAL_KEY_H
#include <stdint.h>
void Key_Init(void);
uint8_t Key_ReadDirections(void);
int Key_TakeModePress(uint32_t now);
#endif
