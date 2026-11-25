#include "ti_msp_dl_config.h"
#include "gimbal_display.h"
uint8_t OLED_GRAM[144][8];
int main(void){
    unsigned i;GimbalDisplay_Init();
    *(volatile uint32_t *)&OLED_INST->MASTER.MSR=DL_I2C_CONTROLLER_STATUS_IDLE;
    for(i=0;i<300;i++){
        *(volatile uint32_t *)&OLED_INST->CPU_INT.RIS=DL_I2C_INTERRUPT_CONTROLLER_TX_DONE;
        GimbalDisplay_Poll(i);
    }
    if(!GimbalDisplay_Idle()||g_display_errors)return 1;
    GimbalDisplay_Begin();
    *(volatile uint32_t *)&OLED_INST->CPU_INT.RIS=0;
    GimbalDisplay_Poll(400);GimbalDisplay_Poll(421);
    if(g_display_errors!=1)return 2;
    return 0;
}
