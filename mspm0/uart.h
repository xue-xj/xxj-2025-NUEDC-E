#ifndef GIMBAL_UART_H
#define GIMBAL_UART_H
#include "gimbal_protocol.h"
void UART_InitReceiver(void);
int UART_TakeCommand(GimbalCommand *command);
extern volatile uint32_t g_uart_valid_frames;
#endif
