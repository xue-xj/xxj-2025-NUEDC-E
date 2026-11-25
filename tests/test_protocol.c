#include "gimbal_protocol.h"
#include <string.h>
int main(void){
    GimbalParser parser;GimbalCommand out={1,1,999,999};unsigned i;
    uint8_t frame[12]={0xaa,0xaa,1,0,50,51,0,0,20,20,0xff,0xff};
    GimbalParser_Init(&parser);
    for(i=0;i<11;i++)if(GimbalParser_Feed(&parser,frame[i],&out))return 1;
    if(out.x_steps!=999)return 2;
    if(!GimbalParser_Feed(&parser,frame[11],&out)||out.x_steps!=50)return 3;
    frame[5]++;for(i=0;i<12;i++)if(GimbalParser_Feed(&parser,frame[i],&out))return 4;
    if(out.x_steps!=50)return 5;
    frame[5]--;GimbalParser_Feed(&parser,0x12,&out);GimbalParser_Feed(&parser,0xaa,&out);
    for(i=0;i<12;i++)if(GimbalParser_Feed(&parser,frame[i],&out)&&i!=11)return 6;
    if(out.y_steps!=20)return 7;
    frame[2]=3;frame[5]=53;
    for(i=0;i<12;i++)if(GimbalParser_Feed(&parser,frame[i],&out))return 8;
    return 0;
}
