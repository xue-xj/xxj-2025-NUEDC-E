#include "gimbal_protocol.h"
#include <string.h>
void GimbalParser_Init(GimbalParser *parser) {
    memset(parser, 0, sizeof(*parser));
}
int GimbalParser_Feed(GimbalParser *parser, uint8_t byte, GimbalCommand *out) {
    uint8_t *frame = parser->bytes;
    GimbalCommand candidate;
    if (parser->length == GIMBAL_PACKET_SIZE) {
        memmove(frame, frame + 1, GIMBAL_PACKET_SIZE - 1);
        parser->length--;
    }
    frame[parser->length++] = byte;
    if (parser->length < GIMBAL_PACKET_SIZE)
        return 0;
    /* 滑动窗口重同步：错误帧中的AA也可能是下一帧开头，不丢弃整段输入。 */
    if (frame[0] != 0xaa || frame[1] != 0xaa || frame[10] != 0xff || frame[11] != 0xff ||
        (uint8_t)(frame[2] + frame[3] + frame[4]) != frame[5] ||
        (uint8_t)(frame[6] + frame[7] + frame[8]) != frame[9])
        return 0;
    candidate.x_direction = frame[2];
    candidate.y_direction = frame[6];
    candidate.x_steps = (uint16_t)((frame[3] << 8) | frame[4]);
    candidate.y_steps = (uint16_t)((frame[7] << 8) | frame[8]);
    /* 方向2仅用于零步数控制包，不允许带步数驱动电机。 */
    if (candidate.x_direction > 2 || candidate.y_direction > 2 ||
        ((candidate.x_direction == 2 || candidate.y_direction == 2) &&
         (candidate.x_steps || candidate.y_steps)))
        return 0;
    *out = candidate;
    parser->length = 0;
    return 1;
}
