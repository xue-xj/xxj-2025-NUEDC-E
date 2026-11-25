#include "ti_msp_dl_config.h"
#include "gimbal_display.h"
#include <string.h>
extern uint8_t OLED_GRAM[144][8];
static const uint8_t init_commands[] = {0xae, 0x00, 0x10, 0x40, 0x81, 0xcf, 0xa1, 0xc8, 0xa6, 0xa8,
                                        0x3f, 0xd3, 0x00, 0xd5, 0x80, 0xd9, 0xf1, 0xda, 0x12, 0xdb,
                                        0x40, 0x20, 0x02, 0x8d, 0x14, 0xa4, 0xa6, 0xaf};
static uint8_t initializing, init_index, active, page, column, command_index, in_flight, disabled;
static uint32_t started_at;
volatile uint32_t g_display_errors;
void GimbalDisplay_Init(void) {
    memset(OLED_GRAM, 0, sizeof(uint8_t) * 144 * 8);
    initializing = 1;
    active = 1;
}
int GimbalDisplay_Idle(void) {
    return !active && !disabled;
}
void GimbalDisplay_Begin(void) {
    if (GimbalDisplay_Idle()) {
        active = 1;
        page = 0;
        column = 0;
        command_index = 0;
    }
}
static void transfer(uint8_t *bytes, uint8_t count, uint32_t now) {
    DL_I2C_clearInterruptStatus(OLED_INST, DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);
    DL_I2C_fillControllerTXFIFO(OLED_INST, bytes, count);
    DL_I2C_startControllerTransfer(OLED_INST, 0x3c, DL_I2C_CONTROLLER_DIRECTION_TX, count);
    in_flight = 1;
    started_at = now;
}
void GimbalDisplay_Poll(uint32_t now) {
    uint8_t bytes[8], count = 0;
    uint32_t status;
    if (!active || disabled)
        return;
    status = DL_I2C_getControllerStatus(OLED_INST);
    if (in_flight) {
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) || (uint32_t)(now - started_at) > 20u) {
            disabled = 1;
            active = 0;
            g_display_errors++;
            return;
        }
        if (!DL_I2C_getRawInterruptStatus(OLED_INST, DL_I2C_INTERRUPT_CONTROLLER_TX_DONE))
            return;
        in_flight = 0;
    }
    if (!(status & DL_I2C_CONTROLLER_STATUS_IDLE))
        return;
    /* 每次轮询最多启动一笔FIFO可容纳的事务，不等待I2C硬件完成。 */
    if (initializing) {
        if (init_index < sizeof(init_commands)) {
            bytes[0] = 0;
            bytes[1] = init_commands[init_index++];
            transfer(bytes, 2, now);
            return;
        }
        initializing = 0;
        page = 0;
        column = 0;
        command_index = 0;
    }
    if (page >= 8) {
        active = 0;
        return;
    }
    if (command_index < 3) {
        static const uint8_t commands[] = {0, 0x00, 0x10};
        bytes[0] = 0;
        bytes[1] = command_index == 0 ? (uint8_t)(0xb0 + page) : commands[command_index];
        command_index++;
        transfer(bytes, 2, now);
        return;
    }
    bytes[count++] = 0x40;
    while (count < 8 && column < 128)
        bytes[count++] = OLED_GRAM[column++][page];
    transfer(bytes, count, now);
    if (column == 128) {
        page++;
        column = 0;
        command_index = 0;
    }
}
