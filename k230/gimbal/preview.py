"""本地识别画面叠加，不参与目标选择与控制计算。"""
from . import config
from .geometry import rectangle_center


def annotate(frame, corners, previous, confirmed, fps):
    if corners is not None and confirmed:
        if not previous:
            for index in range(4):
                start, end = corners[index], corners[(index + 1) % 4]
                frame.draw_line(start[0], start[1], end[0], end[1], color=(0, 255, 0), thickness=2)
        x, y = rectangle_center(corners)
        frame.draw_circle(x, y, 2, color=(255, 255, 0), thickness=1)
    x, y = config.TARGET_POINT
    frame.draw_line(x - 5, y, x + 5, y, color=(255, 0, 0), thickness=1)
    frame.draw_line(x, y - 5, x, y + 5, color=(255, 0, 0), thickness=1)
    frame.draw_string_advanced(10, 10, 15, "fps: {:.1f}".format(fps), color=(255, 0, 0))
    frame.draw_string_advanced(10, 30, 15, "status: {}".format("Tracking" if confirmed else "Lost"),
                               color=(0, 255, 0) if confirmed else (255, 0, 0))
