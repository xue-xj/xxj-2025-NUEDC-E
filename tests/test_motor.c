#include "ti_msp_dl_config.h"
#include "step_motor.h"
void DCC_100_PWM1_INST_IRQHandler(void);
static void falling_edge(void){
    *(volatile uint32_t *)&DCC_100_PWM1_INST->CPU_INT.IIDX=DL_TIMER_IIDX_CC0_UP;
    DCC_100_PWM1_INST_IRQHandler();
}
int main(void){
    DL_Timer_setCounterValueAfterEnable(DCC_100_PWM1_INST,DL_TIMER_COUNT_AFTER_EN_ZERO);
    DL_Timer_setCounterValueAfterEnable(DCC_100_PWM2_INST,DL_TIMER_COUNT_AFTER_EN_ZERO);
    StepMotor_Init();
    if(DL_Timer_getCounterValueAfterEnable(DCC_100_PWM1_INST)!=DL_TIMER_COUNT_AFTER_EN_NO_CHANGE||
       DL_Timer_getCounterValueAfterEnable(DCC_100_PWM2_INST)!=DL_TIMER_COUNT_AFTER_EN_NO_CHANGE)return 7;
    StepMotor_SetSpeed(30,1);StepMotor_Move(1,3,1);
    if(StepMotor_Remaining(1)!=3)return 1;
    falling_edge();if(StepMotor_Remaining(1)!=2)return 2;
    falling_edge();if(StepMotor_Remaining(1)!=1)return 3;
    falling_edge();if(StepMotor_Remaining(1)!=0||StepMotor_Busy())return 4;
    if(!(DCC_100_PWM1_INST->COMMONREGS.ODIS&GPTIMER_ODIS_C0CCP0_MASK))return 5;
    StepMotor_Move(0,2,1);StepMotor_Move(0,0,1);
    if(StepMotor_Remaining(1))return 6;
    return 0;
}
