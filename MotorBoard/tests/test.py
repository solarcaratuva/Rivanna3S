import math
import time
import unittest

# Pedal 0 with regen enabled: firmware drives throttle to 0 and regen from the 50-count curve.
PEDAL = 0


def expected_regen(pedal: int) -> int:
    return int(79.159 * math.pow(50 - pedal, 0.3))


class MotorBoardTests(unittest.TestCase):
    def setUp(self):
        import config
        from gpioPins import reset_nucleo
        config.read_in_nucleo_pinmaps("MotorBoard")
        reset_nucleo()

    def test_pedal_and_regen_one_run(self):
        from CANBus import CANBus, clear_receive_queue
        from CANMessage import CanMessage
        from MotorInterfaceTest import MotorInterfaceTest
        bus = CANBus()
        self.addCleanup(bus.close)
        clear_receive_queue()
        session = MotorInterfaceTest()
        self.addCleanup(session.close)
        now = time.time()
        bus.sendMessage(CanMessage(
            "PedalStatus", 0x1F4, {"throttle_pedal": PEDAL, "brake_pedal": 0}, now))
        bus.sendMessage(CanMessage(
            "DashboardCommands", 0x12C, {"regen_en": 1}, now))
        want = expected_regen(PEDAL)
        frame = None
        throttle = regen = None
        deadline = time.time() + 2
        while time.time() < deadline:
            got = bus.getReceivedMessages("MotorCommands") or []
            if got:
                frame = got[-1]
            throttle, regen = session.read()
            if frame is not None and throttle is not None and regen is not None:
                break
            time.sleep(0.05)
        self.assertEqual(throttle, 0)
        self.assertEqual(regen, want)
        self.assertEqual(int(frame.sigDict["throttle"]), 0)
        self.assertEqual(int(frame.sigDict["regen_braking"]), want)
        self.assertEqual(int(frame.sigDict["regen_drive"]), 1)

    def test_unknown_pin_is_rejected(self):
        from gpioPins import PinValidate
        with self.assertRaises(ValueError):
            PinValidate("NOT_ON_SHEET", "DigitalInput")
