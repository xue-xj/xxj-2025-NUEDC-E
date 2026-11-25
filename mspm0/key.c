#include "ti_msp_dl_config.h"
#include "key.h"
#include "gimbal_control.h"
#include "gimbal_config.h"
static volatile uint8_t mode_pending;
static uint32_t last_mode_at;
static uint8_t mode_seen;
void Key_Init(void) {
    NVIC_EnableIRQ(KEY_INT_IRQN);
}
int Key_TakeModePress(uint32_t now) {
    uint32_t mask = __get_PRIMASK();
    uint8_t pending;
    __disable_irq();
    pending = mode_pending;
    mode_pending = 0;
    __set_PRIMASK(mask);
    if (!pending)
        return 0;
    if (mode_seen && (uint32_t)(now - last_mode_at) < GIMBAL_MODE_DEBOUNCE_MS)
        return 0;
    mode_seen = 1;
    last_mode_at = now;
    return 1;
}
uint8_t Key_ReadDirections(void) {
    uint8_t keys = 0;
    if (DL_GPIO_readPins(KEY_KEY_1_PORT, KEY_KEY_1_PIN))
        keys |= GIMBAL_KEY_X_POS;
    if (DL_GPIO_readPins(KEY_KEY_4_PORT, KEY_KEY_4_PIN))
        keys |= GIMBAL_KEY_X_NEG;
    if (DL_GPIO_readPins(KEY_KEY_2_PORT, KEY_KEY_2_PIN))
        keys |= GIMBAL_KEY_Y_POS;
    if (DL_GPIO_readPins(KEY_KEY_3_PORT, KEY_KEY_3_PIN))
        keys |= GIMBAL_KEY_Y_NEG;
    return keys;
}
void GROUP1_IRQHandler(void) {
    if (DL_GPIO_getPendingInterrupt(KEY_Mode_change_PORT) == KEY_Mode_change_IIDX)
        mode_pending = 1;
}
