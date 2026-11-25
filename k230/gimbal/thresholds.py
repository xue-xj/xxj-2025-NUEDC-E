"""触摸编辑使用独立副本；应用后仅本次运行有效。"""
class ThresholdRange:
    def __init__(self, low, high):
        if not 0 <= low <= high <= 255:
            raise ValueError("invalid grayscale threshold")
        self.low, self.high = low, high

    def adjust(self, minimum, delta):
        if minimum:
            self.low = max(0, min(self.high, self.low + delta))
        else:
            self.high = min(255, max(self.low, self.high + delta))

    def pair(self):
        return self.low, self.high
