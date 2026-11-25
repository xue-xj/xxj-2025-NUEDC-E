#include "ti_msp_dl_config.h"
#include "step_motor.h"
#include "motion_math.h"
static volatile uint32_t remaining[2];
static GPTIMER_Regs *timer(uint8_t axis) {
    return axis == 1 ? DCC_100_PWM1_INST : DCC_100_PWM2_INST;
}
void StepMotor_Stop(uint8_t axis) {
    if (axis < 1 || axis > 2)
        return;
    DL_Timer_stopCounter(timer(axis));
    remaining[axis - 1] = 0;
    DL_Timer_setCCPOutputDisabled(timer(axis), DL_TIMER_CCP_DIS_OUT_LOW, DL_TIMER_CCP_DIS_OUT_LOW);
}
void StepMotor_Init(void) {
    /* 软件预设计数必须在使能后保持，不能被SDK默认的启动归零覆盖。 */
    DL_Timer_setCounterValueAfterEnable(DCC_100_PWM1_INST, DL_TIMER_COUNT_AFTER_EN_NO_CHANGE);
    DL_Timer_setCounterValueAfterEnable(DCC_100_PWM2_INST, DL_TIMER_COUNT_AFTER_EN_NO_CHANGE);
    DL_GPIO_setPins(STEP_MOTOR_RST1_PORT, STEP_MOTOR_RST1_PIN);
    DL_GPIO_setPins(STEP_MOTOR_SLP1_PORT, STEP_MOTOR_SLP1_PIN);
    DL_GPIO_setPins(STEP_MOTOR_DCY1_PORT, STEP_MOTOR_DCY1_PIN);
    DL_GPIO_setPins(STEP_MOTOR_RST2_PORT, STEP_MOTOR_RST2_PIN);
    DL_GPIO_setPins(STEP_MOTOR_SLP2_PORT, STEP_MOTOR_SLP2_PIN);
    DL_GPIO_setPins(STEP_MOTOR_DCY2_PORT, STEP_MOTOR_DCY2_PIN);
    StepMotor_Stop(1);
    StepMotor_Stop(2);
    NVIC_EnableIRQ(DCC_100_PWM1_INST_INT_IRQN);
    NVIC_EnableIRQ(DCC_100_PWM2_INST_INT_IRQN);
}
void StepMotor_SetSpeed(uint16_t speed, uint8_t axis) {
    uint32_t frequency = Motion_SpeedToFrequency(speed), period, clock;
    if (axis < 1 || axis > 2)
        return;
    if (!frequency)
        frequency = 1;
    clock = axis == 1 ? DCC_100_PWM1_INST_CLK_FREQ : DCC_100_PWM2_INST_CLK_FREQ;
    period = clock / frequency;
    if (axis == 1) {
        if (period > 65535)
            period = 65535;
        if (period < 800)
            period = 800;
    } else {
        if (period > 131071)
            period = 131071;
        if (period < 1600)
            period = 1600;
    }
    DL_Timer_setLoadValue(timer(axis), period);
    DL_Timer_setCaptureCompareValue(timer(axis), period / 2, DL_TIMER_CC_0_INDEX);
}
void StepMotor_Move(uint8_t direction, uint32_t steps, uint8_t axis) {
    uint32_t mask;
    if (axis < 1 || axis > 2 || direction > 1)
        return;
    mask = __get_PRIMASK();
    __disable_irq();
    StepMotor_Stop(axis);
    if (steps) {
        if (axis == 1) {
            if (direction)
                DL_GPIO_setPins(STEP_MOTOR_DIR1_PORT, STEP_MOTOR_DIR1_PIN);
            else
                DL_GPIO_clearPins(STEP_MOTOR_DIR1_PORT, STEP_MOTOR_DIR1_PIN);
        } else {
            if (direction)
                DL_GPIO_setPins(STEP_MOTOR_DIR2_PORT, STEP_MOTOR_DIR2_PIN);
            else
                DL_GPIO_clearPins(STEP_MOTOR_DIR2_PORT, STEP_MOTOR_DIR2_PIN);
        }
        /* 输出仍被ODIS压低：跨过CC0事件清除上一次中断动作可能留下的高电平锁存。 */
        DL_Timer_setTimerCount(
            timer(axis), DL_Timer_getCaptureCompareValue(timer(axis), DL_TIMER_CC_0_INDEX) - 1);
        DL_Timer_startCounter(timer(axis));
        delay_cycles(CPUCLK_FREQ / 1000000u);
        DL_Timer_stopCounter(timer(axis));
        remaining[axis - 1] = steps;
        DL_Timer_clearInterruptStatus(timer(axis), DL_TIMER_INTERRUPT_CC0_UP_EVENT);
        if (axis == 1)
            NVIC_ClearPendingIRQ(DCC_100_PWM1_INST_INT_IRQN);
        else
            NVIC_ClearPendingIRQ(DCC_100_PWM2_INST_INT_IRQN);
        /* 从LOAD起步，下一个ZERO产生首个上升沿；方向已有至少1us建立时间。 */
        DL_Timer_setTimerCount(timer(axis), DL_Timer_getLoadValue(timer(axis)));
        DL_Timer_setCCPOutputDisabled(timer(axis), DL_TIMER_CCP_DIS_OUT_SET_BY_OCTL,
                                      DL_TIMER_CCP_DIS_OUT_SET_BY_OCTL);
        DL_Timer_startCounter(timer(axis));
    }
    __set_PRIMASK(mask);
}
uint8_t StepMotor_Busy(void) {
    return (remaining[0] ? 1u : 0u) | (remaining[1] ? 2u : 0u);
}
uint32_t StepMotor_Remaining(uint8_t axis) {
    return axis == 1 ? remaining[0] : remaining[1];
}
static void pulse_event(uint8_t axis) {
    /* CC0_UP是当前PWM配置的下降沿，距离下次ZERO上升沿还有半个周期。 */
    if (DL_Timer_getPendingInterrupt(timer(axis)) == DL_TIMER_IIDX_CC0_UP) {
        if (remaining[axis - 1])
            remaining[axis - 1]--;
        if (!remaining[axis - 1])
            StepMotor_Stop(axis);
    }
}
void DCC_100_PWM1_INST_IRQHandler(void) {
    pulse_event(1);
}
void DCC_100_PWM2_INST_IRQHandler(void) {
    pulse_event(2);
}
