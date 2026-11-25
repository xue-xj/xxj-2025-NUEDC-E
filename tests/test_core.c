#include "gimbal_protocol.h"
#include "gimbal_control.h"
#include "motion_math.h"
int main(void){
    GimbalParser p;GimbalCommand c;GimbalControl state;GimbalAction action;
    uint8_t packet[12]={0xaa,0xaa,1,0,50,51,0,0,20,20,0xff,0xff};unsigned i;
    GimbalParser_Init(&p);
    for(i=0;i<11;i++)if(GimbalParser_Feed(&p,packet[i],&c))return 1;
    if(!GimbalParser_Feed(&p,packet[11],&c)||c.x_steps!=50||c.y_steps!=20)return 2;
    GimbalControl_Init(&state);GimbalControl_Update(&state,0,0,0,0,&action);
    if(action.x_steps!=1600||state.mode!=GIMBAL_SCAN)return 3;
    GimbalControl_Update(&state,1,&c,0,3,&action);
    if(state.mode!=GIMBAL_TRACK||!action.stop_all||action.x_steps!=50)return 4;
    if(Motion_AngleToSteps(100)!=1||Motion_AngleToSteps(90000)!=1600)return 5;
    if(Motion_SpeedToFrequency(30)!=533)return 6;
    return 0;
}
