import math
import time
import unittest

# Same samples and 0.82 / 3.3 thresholds as the Rivanna3 throttle case.
SAMPLES = (0.1, 0.5, 0.75)
LOW = 0.82
HIGH = 3.3


def throttle_counts(fraction: float) -> int:
    voltage = fraction * 3.3
    if voltage <= LOW:
        return 0
    if voltage >= HIGH:
        return 256
    return math.floor((voltage - LOW) / (HIGH - LOW) * 256)


class BottomDistTests(unittest.TestCase):
    def setUp(self):
        import config
        from gpioPins import reset_nucleo
        config.read_in_nucleo_pinmaps("BottomDist")
        reset_nucleo()

    def test_throttle_wiper_publishes_pedal_status(self):
        from CANBus import CANBus, clear_receive_queue
        from gpioPins import AnalogOutput
        bus = CANBus()
        self.addCleanup(bus.close)
        clear_receive_queue()
        wiper = AnalogOutput("THROTTLE_WIPER")
        for fraction in SAMPLES:
            wiper.write(float(fraction))
            time.sleep(0.3)
            got = None
            deadline = time.time() + 1
            while time.time() < deadline:
                messages = bus.getReceivedMessages("PedalStatus") or []
                if messages:
                    got = messages[-1]
                    break
                time.sleep(0.05)
            self.assertIsNotNone(got, f"no PedalStatus at duty {fraction}")
            self.assertEqual(int(got.sigDict["throttle_pedal"]), throttle_counts(fraction))

    def test_unknown_pin_is_rejected(self):
        from gpioPins import PinValidate
        with self.assertRaises(ValueError):
            PinValidate("NOT_ON_SHEET", "DigitalInput")
