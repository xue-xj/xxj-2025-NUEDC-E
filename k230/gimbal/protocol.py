"""12字节双轴帧：独立X/Y校验，步数为脉冲数而非角度。"""
def encode_xy(x_direction, x_steps, y_direction, y_steps):
    values = (x_direction, x_steps, y_direction, y_steps)
    if any(type(value) is not int for value in values):
        raise ValueError("directions and steps must be integers")
    if x_direction not in (0, 1, 2) or y_direction not in (0, 1, 2):
        raise ValueError("invalid direction")
    if not 0 <= x_steps <= 65535 or not 0 <= y_steps <= 65535:
        raise ValueError("steps out of range")
    if (x_direction == 2 or y_direction == 2) and (x_steps or y_steps):
        raise ValueError("direction 2 is reserved for zero-step packets")
    x_high, x_low = x_steps >> 8, x_steps & 255
    y_high, y_low = y_steps >> 8, y_steps & 255
    return bytes((0xAA, 0xAA, x_direction, x_high, x_low,
                  (x_direction + x_high + x_low) & 255,
                  y_direction, y_high, y_low,
                  (y_direction + y_high + y_low) & 255, 0xFF, 0xFF))


def send_packet(uart, packet):
    written = uart.write(packet)
    if written != len(packet):
        raise OSError("incomplete UART write")
