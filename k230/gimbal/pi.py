"""按帧更新的增量式PI；没有微分项，不把它描述成完整三项PID。"""
class IncrementalPI:
    def __init__(self, kp, ki, target):
        self.kp = kp
        self.ki = ki
        self.target = target
        self.error = 0
        self.previous_error = 0

    def calculate(self, value):
        self.error = self.target - value
        delta = self.kp * (self.error - self.previous_error) + self.ki * self.error
        self.previous_error = self.error
        return delta
