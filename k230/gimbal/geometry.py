"""与摄像头SDK无关的矩形几何运算。"""
import math


def vector_angle_diff(first, second):
    dot = first[0] * second[0] + first[1] * second[1]
    determinant = first[0] * second[1] - first[1] * second[0]
    return abs(math.atan2(determinant, dot) * 180 / math.pi)


def line_intersection(first, second):
    (x1, y1), (x2, y2) = first
    (x3, y3), (x4, y4) = second
    a1, b1 = y2 - y1, x1 - x2
    a2, b2 = y4 - y3, x3 - x4
    c1, c2 = a1 * x1 + b1 * y1, a2 * x3 + b2 * y3
    determinant = a1 * b2 - a2 * b1
    if determinant == 0:
        return ((x1 + x3) / 2, (y1 + y3) / 2)
    return ((b2 * c1 - b1 * c2) / determinant,
            (a1 * c2 - a2 * c1) / determinant)


def rectangle_center(corners):
    center = line_intersection((corners[0], corners[2]), (corners[1], corners[3]))
    return int(center[0]), int(center[1])
