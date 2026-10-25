import time
import unittest


def _edges(read, timeout: float) -> int:
    previous = read()
    count = 0
    start = time.time()
    while time.time() - start < timeout:
        current = read()
        if current != previous:
            count += 1
            previous = current
        if count >= 2:
            return count
        time.sleep(0.05)
    return count


class TopDistTests(unittest.TestCase):
    def setUp(self):
        import config
        from gpioPins import reset_nucleo
        config.read_in_nucleo_pinmaps("TopDist")
        reset_nucleo()

    def _bus(self):
        from CANBus import CANBus, clear_receive_queue
        bus = CANBus()
        self.addCleanup(bus.close)
        clear_receive_queue()
        return bus

    def test_dashboard_left_turn_flashes_pa1(self):
        from CANMessage import CanMessage
        from gpioPins import DigitalInput
        bus = self._bus()
        bus.sendMessage(CanMessage(
            "DashboardCommands", 0x12C, {"left_turn_signal": 1}, time.time()))
        lamp = DigitalInput("LEFT_TURN_EN")
        self.assertGreaterEqual(_edges(lamp.read, 5), 2, "PA1 left turn did not flash")

    def test_bps_fault_flashes_pc2(self):
        from CANMessage import CanMessage
        from gpioPins import DigitalInput
        bus = self._bus()
        bus.sendMessage(CanMessage(
            "BpsError", 0x3AA, {"internal_cell_communication_fault": 1}, time.time()))
        strobe = DigitalInput("BMS_STROBE_EN")
        self.assertGreaterEqual(_edges(strobe.read, 5), 2, "PC2 BMS strobe did not flash")

    def test_unknown_pin_is_rejected(self):
        from gpioPins import PinValidate
        with self.assertRaises(ValueError):
            PinValidate("NOT_ON_SHEET", "DigitalInput")
