#include "ti_msp_dl_config.h"
#include "gimbal_app.h"
int main(void) {
    SYSCFG_DL_init();
    GimbalApp_Init();
    for (;;)
        GimbalApp_Poll();
}
