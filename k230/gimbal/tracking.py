"""帧确认、短时遮挡保持与方向／步数计算，不导入板端SDK。"""
from . import config
from .geometry import rectangle_center
from .pi import IncrementalPI
from .protocol import encode_xy


class VisionTracker:
    def __init__(self):
        self.confirmed = False
        self.detect_count = 0
        self.lost_count = 0
        self.previous_corners = None
        self.enabled = False
        self.x_pi = IncrementalPI(config.PI_KP, config.PI_KI, config.TARGET_POINT[0])
        self.y_pi = IncrementalPI(config.PI_KP, config.PI_KI, config.TARGET_POINT[1])

    def button_pressed(self):
        if not self.enabled:
            self.enabled = True
            return None
        # 运行中按键发一次零步控制包，随后继续跟踪；不假设存在回零传感器。
        return encode_xy(2, 0, 2, 0)

    def update(self, corners):
        using_previous = corners is None and self.previous_corners is not None
        selected = self.previous_corners if using_previous else corners
        if corners is not None:
            if not self.confirmed:
                self.detect_count += 1
                if self.detect_count >= config.DETECT_FRAMES:
                    self.confirmed, self.detect_count = True, 0
            else:
                self.lost_count = 0
        elif self.confirmed:
            self.lost_count += 1
            if self.lost_count >= config.LOST_FRAMES:
                self.confirmed, self.lost_count = False, 0
                self.previous_corners = None
                selected = None
        else:
            self.detect_count = 0
        packet = None
        if selected is not None and self.confirmed:
            if not using_previous:
                self.previous_corners = selected
            center_x, center_y = rectangle_center(selected)
            if self.enabled:
                x_delta = self.x_pi.calculate(center_x)
                y_delta = self.y_pi.calculate(center_y)
                x_steps = min(abs(int(x_delta)), config.MAX_STEPS_PER_FRAME)
                y_steps = min(abs(int(y_delta)), config.MAX_STEPS_PER_FRAME)
                # X与Y安装方向不同，方向编码不能简单共用同一个符号映射。
                packet = encode_xy(1 if x_delta >= 0 else 0, x_steps,
                                   0 if y_delta >= 0 else 1, y_steps)
        return selected, using_previous, packet
