"""几何与黑色比例过滤，选择满足条件的最小矩形。"""
from . import config
from .geometry import vector_angle_diff, rectangle_center


def find_rectangle(binary_image, width, height):
    selected = None
    selected_area = float("inf")
    for rect in binary_image.find_rects(threshold=config.FIND_RECT_THRESHOLD) or ():
        corners = rect.corners()
        area = rect.w() * rect.h()
        if len(corners) != 4 or area < config.MIN_RECT_AREA:
            continue
        errors = []
        for index in range(4):
            previous, point, following = corners[(index - 1) % 4], corners[index], corners[(index + 1) % 4]
            incoming = previous[0] - point[0], previous[1] - point[1]
            outgoing = following[0] - point[0], following[1] - point[1]
            errors.append(abs(vector_angle_diff(incoming, outgoing) - 90))
        if max(errors) > config.MAX_CORNER_ERROR or sum(errors) / 4 > config.MAX_AVERAGE_ERROR:
            continue
        center_x, center_y = rectangle_center(corners)
        left, right = max(0, center_x - rect.w() // 4), min(width - 1, center_x + rect.w() // 4)
        top, bottom = max(0, center_y - rect.h() // 4), min(height - 1, center_y + rect.h() // 4)
        black, total = 0, 0
        # 稀疏采样矩形中心区域，排除不满足黑色比例的轮廓。
        for y in range(top, bottom, config.PIXEL_SAMPLE_STEP):
            for x in range(left, right, config.PIXEL_SAMPLE_STEP):
                value = binary_image.get_pixel(x, y)
                if isinstance(value, tuple):
                    value = value[0]
                black += value == 0
                total += 1
        if total and black / total >= config.MIN_BLACK_RATIO and area < selected_area:
            selected, selected_area = corners, area
    return selected
