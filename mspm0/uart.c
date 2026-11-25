#include "ti_msp_dl_config.h"
#include "uart.h"
static GimbalParser parser;
static volatile GimbalCommand pending;
static volatile uint8_t ready;
volatile uint32_t g_uart_valid_frames;
void UART_InitReceiver(void) {
    GimbalParser_Init(&parser);
    ready = 0;
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}
int UART_TakeCommand(GimbalCommand *command) {
    uint32_t mask = __get_PRIMASK();
    int available;
    /* 临界区仅复制一条完整命令，避免主循环拿到混合的X/Y半帧。 */
    __disable_irq();
    available = ready;
    if (available) {
        command->x_direction = pending.x_direction;
        command->y_direction = pending.y_direction;
        command->x_steps = pending.x_steps;
        command->y_steps = pending.y_steps;
        ready = 0;
    }
    __set_PRIMASK(mask);
    return available;
}
void UART_0_INST_IRQHandler(void) {
    GimbalCommand complete;
    if (DL_UART_getPendingInterrupt(UART_0_INST) == DL_UART_IIDX_RX) {
        uint8_t byte = (uint8_t)DL_UART_receiveData(UART_0_INST);
        if (GimbalParser_Feed(&parser, byte, &complete)) {
            pending.x_direction = complete.x_direction;
            pending.y_direction = complete.y_direction;
            pending.x_steps = complete.x_steps;
            pending.y_steps = complete.y_steps;
            ready = 1;
            g_uart_valid_frames++;
        }
    }
}
