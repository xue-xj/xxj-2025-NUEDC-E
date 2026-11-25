#include "gimbal_control.h"
int main(void){
    GimbalControl state;GimbalAction action;GimbalCommand command={1,0,0,20};
    unsigned i;GimbalControl_Init(&state);
    for(i=0;i<4;i++){
        GimbalControl_Update(&state,i,0,0,0,&action);
        if(i%2==0&&action.x_steps!=1600)return 1;
        if(i%2==1&&action.y_steps!=533)return 2;
    }
    if(state.scan_leg!=0)return 3;
    GimbalControl_Update(&state,5,0,0,3,&action);if(action.x_steps||action.y_steps)return 4;
    GimbalControl_Update(&state,6,&command,0,3,&action);
    if(state.mode!=GIMBAL_TRACK||!action.stop_all||action.x_steps||action.y_steps!=20)return 5;
    if(action.update_axes!=3)return 6;
    GimbalControl_NextMode(&state); /* TRACK -> SCAN */
    GimbalControl_NextMode(&state); /* SCAN -> MANUAL */
    GimbalControl_Update(&state,20,0,GIMBAL_KEY_Y_POS,0,&action);
    if(action.y_direction!=0||action.y_steps!=1)return 7;
    GimbalControl_Update(&state,21,0,GIMBAL_KEY_X_POS,0,&action);if(action.x_steps)return 8;
    return 0;
}
