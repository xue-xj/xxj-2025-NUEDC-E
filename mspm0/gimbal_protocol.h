#ifndef GIMBAL_PROTOCOL_H
#define GIMBAL_PROTOCOL_H
#include <stdint.h>
#define GIMBAL_PACKET_SIZE 12u
typedef struct {
    uint8_t x_direction, y_direction;
    uint16_t x_steps, y_steps;
} GimbalCommand;
typedef struct {
    uint8_t bytes[GIMBAL_PACKET_SIZE];
    uint8_t length;
} GimbalParser;
void GimbalParser_Init(GimbalParser *parser);
/* 仅完整帧通过头尾、方向及双校验后才写out；步数字段始终表示脉冲数。 */
int GimbalParser_Feed(GimbalParser *parser, uint8_t byte, GimbalCommand *out);
#endif
