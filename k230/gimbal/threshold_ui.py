"""800x480触摸阈值编辑，摄像头和显示对象由应用注入。"""
import time
import os
from .thresholds import ThresholdRange


def edit_threshold(sensor, touch, display, image_module, channel, current):
    settings = ThresholdRange(*current)
    canvas = image_module.Image(800, 480, image_module.RGB565)
    while True:
        os.exitpoint()
        low, high = settings.pair()
        canvas.draw_rectangle(0, 0, 800, 480, color=(255, 255, 255), fill=True)
        canvas.draw_string_advanced(280, 20, 40, "阈值调节", color=(0, 0, 0))
        canvas.draw_string_advanced(220, 70, 30, "Min: {} Max: {}".format(low, high), color=(0, 0, 0))
        for x in (40, 640):
            for y, label in ((150, "-"), (280, "+")):
                canvas.draw_rectangle(x, y, 120, 110, color=(200, 200, 200), fill=True)
                canvas.draw_string_advanced(x + 25, y + 30, 50, label, color=(0, 0, 0))
        snapshot = sensor.snapshot(chn=channel)
        if snapshot:
            preview = snapshot.to_grayscale(copy=True).binary([settings.pair()])
            canvas.draw_image(preview, 240, 150)
        for x, label in ((100, "返回"), (540, "应用")):
            canvas.draw_rectangle(x, 400, 160, 80, color=(200, 200, 200), fill=True)
            canvas.draw_string_advanced(x + 40, 425, 35, label, color=(0, 0, 0))
        display.show_image(canvas, 0, 0)
        points = touch.read()
        if points:
            x, y = points[0].x, points[0].y
            if 100 <= x <= 260 and 400 <= y <= 480:
                return current
            if 540 <= x <= 700 and 400 <= y <= 480:
                return settings.pair()
            # 调节按钮与底部按钮不重叠，且始终保证Min<=Max。
            if 40 <= x <= 160 and 150 <= y <= 260:
                settings.adjust(True, -1)
            elif 40 <= x <= 160 and 280 <= y <= 390:
                settings.adjust(True, 1)
            elif 640 <= x <= 760 and 150 <= y <= 260:
                settings.adjust(False, -1)
            elif 640 <= x <= 760 and 280 <= y <= 390:
                settings.adjust(False, 1)
            time.sleep_ms(200)
        time.sleep_ms(20)
