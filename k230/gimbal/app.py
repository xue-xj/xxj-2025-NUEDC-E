"""K230硬件生命周期与主循环；硬件SDK只在run内导入，便于主机测试。"""
import time
import os
from . import config
from .tracking import VisionTracker
from .detection import find_rectangle
from .protocol import send_packet
from .preview import annotate
from .threshold_ui import edit_threshold


def run():
    from media.sensor import Sensor, CAM_CHN_ID_0
    from media.display import Display
    from media.media import MediaManager
    from machine import FPIOA, Pin, UART, TOUCH
    import image

    sensor, uart, touch = None, None, None
    display_started, media_started = False, False
    try:
        fpioa = FPIOA()
        fpioa.set_function(config.BUTTON_PIN, FPIOA.GPIO53)
        fpioa.set_function(config.UART_TX_PIN, FPIOA.UART2_TXD)
        fpioa.set_function(config.UART_RX_PIN, FPIOA.UART2_RXD)
        button = Pin(config.BUTTON_PIN, Pin.IN, Pin.PULL_DOWN)
        uart = UART(UART.UART2, config.UART_BAUD)
        touch = TOUCH(0)
        sensor = Sensor()
        sensor.reset()
        sensor.set_framesize(Sensor.QVGA)
        sensor.set_pixformat(Sensor.RGB565)
        time.sleep(1)
        Display.init(Display.ST7701, width=config.DISPLAY_WIDTH, height=config.DISPLAY_HEIGHT, to_ide=True)
        display_started = True
        MediaManager.init()
        media_started = True
        sensor.run()
        clock, tracker = time.clock(), VisionTracker()
        black, touch_frames = config.BLACK_THRESHOLD, 0
        while True:
            os.exitpoint()
            if button.value():
                while button.value():
                    os.exitpoint()
                    time.sleep_ms(5)
                packet = tracker.button_pressed()
                if packet is not None:
                    send_packet(uart, packet)
                time.sleep_ms(20)
            points = touch.read()
            if points:
                touch_frames += 1
                if touch_frames > 20:
                    black = edit_threshold(sensor, touch, Display, image, CAM_CHN_ID_0, black)
                    touch_frames = 0
            else:
                touch_frames = max(0, touch_frames - 1)
            clock.tick()
            frame = sensor.snapshot(chn=CAM_CHN_ID_0)
            binary = frame.to_grayscale(copy=True).binary([black])
            binary.dilate(2)
            corners = find_rectangle(binary, frame.width(), frame.height())
            selected, previous, packet = tracker.update(corners)
            if packet is not None:
                send_packet(uart, packet)
            annotate(frame, selected, previous, tracker.confirmed, clock.fps())
            frame.compressed_for_ide()
            Display.show_image(frame, x=(config.DISPLAY_WIDTH-config.FRAME_WIDTH)//2,
                               y=(config.DISPLAY_HEIGHT-config.FRAME_HEIGHT)//2)
    finally:
        # 分别释放已初始化资源，某一步失败也继续清理其他模块。
        operations = []
        if sensor is not None:
            operations.append(sensor.stop)
        if display_started:
            operations.append(Display.deinit)
        if uart is not None:
            operations.append(uart.deinit)
        if touch is not None:
            operations.append(touch.deinit)
        if media_started:
            operations.append(MediaManager.deinit)
        for close in operations:
            try:
                close()
            except Exception as error:
                print("cleanup:", error)
