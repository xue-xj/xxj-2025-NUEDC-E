#include "ti_msp_dl_config.h"
#include "gimbal_app.h"
#include "gimbal_control.h"
#include "gimbal_config.h"
#include "uart.h"
#include "key.h"
#include "step_motor.h"
#include "oled.h"
#include "gimbal_display.h"
#include "delay.h"
#include <string.h>
extern uint8_t OLED_GRAM[144][8];
static volatile uint32_t milliseconds;
static GimbalControl controller;
static GimbalCommand latest_command;
void SysTick_Handler(void) {
    milliseconds++;
}
static void show_state(void) {
    static const char *names[] = {"Manual", "Tracking", "Scanning"};
    memset(OLED_GRAM, 0, sizeof(uint8_t) * 144 * 8);
    OLED_ShowString(0, 0, (u8 *)names[controller.mode], 16);
    OLED_ShowString(0, 16, (u8 *)"X DIR:", 16);
    OLED_ShowNum(64, 16, latest_command.x_direction, 1, 16);
    OLED_ShowString(0, 32, (u8 *)"Y DIR:", 16);
    OLED_ShowNum(64, 32, latest_command.y_direction, 1, 16);
    OLED_ShowNum(80, 16, StepMotor_Remaining(1), 5, 16);
    OLED_ShowNum(80, 32, StepMotor_Remaining(2), 5, 16);
    GimbalDisplay_Begin();
}
void GimbalApp_Init(void) {
    GimbalDisplay_Init();
    StepMotor_Init();
    delay_ms(2);
    UART_InitReceiver();
    Key_Init();
    GimbalControl_Init(&controller);
    SysTick_Config(CPUCLK_FREQ / 1000u);
}
void GimbalApp_Poll(void) {
    static uint32_t displayed_at;
    static uint16_t last_speed;
    uint32_t now = milliseconds;
    GimbalAction action;
    GimbalCommand incoming;
    const GimbalCommand *command = 0;
    if (Key_TakeModePress(now)) {
        GimbalControl_NextMode(&controller);
        StepMotor_Stop(1);
        StepMotor_Stop(2);
    }
    if (UART_TakeCommand(&incoming)) {
        latest_command = incoming;
        command = &incoming;
    }
    GimbalControl_Update(&controller, now, command, Key_ReadDirections(), StepMotor_Busy(),
                         &action);
    if (action.stop_all) {
        StepMotor_Stop(1);
        StepMotor_Stop(2);
    }
    if (action.speed && action.speed != last_speed) {
        StepMotor_SetSpeed(action.speed, 1);
        StepMotor_SetSpeed(action.speed, 2);
        last_speed = action.speed;
    }
    if (action.x_steps || (action.update_axes & 1))
        StepMotor_Move(action.x_direction, action.x_steps, 1);
    if (action.y_steps || (action.update_axes & 2))
        StepMotor_Move(action.y_direction, action.y_steps, 2);
    GimbalDisplay_Poll(now);
    if (GimbalDisplay_Idle() && (uint32_t)(now - displayed_at) >= GIMBAL_DISPLAY_INTERVAL_MS) {
        displayed_at = now;
        show_state();
    }
}
