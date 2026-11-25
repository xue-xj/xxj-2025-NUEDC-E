import unittest
from gimbal.protocol import encode_xy, send_packet
from gimbal.pi import IncrementalPI
from gimbal.geometry import rectangle_center
from gimbal.tracking import VisionTracker
from gimbal.thresholds import ThresholdRange
from gimbal.detection import find_rectangle

RECT = ((100, 80), (200, 80), (200, 160), (100, 160))


class VisionTests(unittest.TestCase):
    def test_packet_has_independent_axis_checksums(self):
        self.assertEqual(encode_xy(1, 50, 0, 20), bytes.fromhex('AA AA 01 00 32 33 00 00 14 14 FF FF'))
        self.assertEqual(encode_xy(2, 0, 2, 0), bytes.fromhex('AA AA 02 00 00 02 02 00 00 02 FF FF'))

    def test_reject_invalid_command(self):
        for values in [(2, 1, 0, 0), (0, -1, 1, 0), (0, 65536, 1, 0), (3, 0, 0, 0)]:
            with self.assertRaises(ValueError):
                encode_xy(*values)

    def test_short_serial_write_is_not_success(self):
        class Port:
            def write(self, packet): return 2
        with self.assertRaises(OSError):
            send_packet(Port(), encode_xy(0, 1, 1, 2))

    def test_incremental_pi_and_rectangle_center(self):
        pi = IncrementalPI(.5, .1, 156)
        self.assertAlmostEqual(pi.calculate(150), 3.6)
        self.assertAlmostEqual(pi.calculate(150), .6)
        self.assertEqual(rectangle_center(RECT), (150, 120))

    def test_two_frames_confirm_and_five_missing_frames_lose(self):
        tracker = VisionTracker()
        tracker.button_pressed()
        self.assertIsNone(tracker.update(RECT)[2])
        self.assertIsNotNone(tracker.update(RECT)[2])
        for _ in range(4):
            self.assertIsNotNone(tracker.update(None)[2])
        self.assertIsNone(tracker.update(None)[2])
        self.assertFalse(tracker.confirmed)

    def test_detection_requires_consecutive_frames(self):
        tracker = VisionTracker()
        tracker.update(RECT); tracker.update(None); tracker.update(RECT)
        self.assertFalse(tracker.confirmed)

    def test_packets_limited_to_50_steps_and_opposite_axis_directions(self):
        tracker = VisionTracker(); tracker.button_pressed()
        far = ((0, 0), (10, 0), (10, 10), (0, 10))
        tracker.update(far); packet = tracker.update(far)[2]
        self.assertEqual((packet[2], packet[4], packet[6], packet[8]), (1, 50, 0, 50))

    def test_threshold_bounds_do_not_cross(self):
        settings = ThresholdRange(0, 96)
        settings.adjust(True, 300); self.assertEqual(settings.pair(), (96, 96))
        settings.adjust(False, -300); self.assertEqual(settings.pair(), (96, 96))

    def test_geometry_and_black_ratio_filter(self):
        class Rect:
            def corners(self): return RECT
            def w(self): return 100
            def h(self): return 80
        class Image:
            def find_rects(self, threshold): return [Rect()]
            def get_pixel(self, x, y): return 0
        self.assertEqual(find_rectangle(Image(), 320, 240), RECT)
        class White(Image):
            def get_pixel(self, x, y): return 255
        self.assertIsNone(find_rectangle(White(), 320, 240))


if __name__ == '__main__':
    unittest.main()
