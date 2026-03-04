import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class TestFakeTime(unittest.TestCase):
    CLOCK_GETTIME_PATH = os.path.join(TEST_BIN_PATH, 'time-clock-gettime')
    RDTSC_PATH = os.path.join(TEST_BIN_PATH, 'time-rdtsc')
    RDTSCP_PATH = os.path.join(TEST_BIN_PATH, 'time-rdtscp')
    RDTSC_TWICE_PATH = os.path.join(TEST_BIN_PATH, 'time-rdtsc-twice')
    NOTIME_PATH = os.path.join(TEST_BIN_PATH, 'time-notime')

    def setUp(self):
        self.sio2jail = SIO2Jail()

    def test_clock_gettime_allowed_by_default(self):
        result = self.sio2jail.run(self.CLOCK_GETTIME_PATH)
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('OK' in line for line in result.stdout))

    def test_clock_gettime_fake_random(self):
        result = self.sio2jail.run(
            self.CLOCK_GETTIME_PATH,
            extra_options=['--fake-time', 'random'])
        # vDSO clock_gettime uses RDTSC internally, which is now emulated
        # with random values. The program gets a garbage timestamp
        # or EPERM if it falls back to a real syscall.
        self.assertEqual(result.supervisor_return_code, 0)

    def test_clock_gettime_fake_zero(self):
        result = self.sio2jail.run(
            self.CLOCK_GETTIME_PATH,
            extra_options=['--fake-time', 'zero'])
        self.assertEqual(result.supervisor_return_code, 0)

    def test_rdtsc_emulated_random(self):
        result = self.sio2jail.run(
            self.RDTSC_PATH,
            extra_options=['--fake-time', 'random'])
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('TSC' in line for line in result.stdout))

    def test_rdtsc_emulated_zero(self):
        result = self.sio2jail.run(
            self.RDTSC_PATH,
            extra_options=['--fake-time', 'zero'])
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('TSC' in line for line in result.stdout))

    def test_rdtscp_emulated_random(self):
        result = self.sio2jail.run(
            self.RDTSCP_PATH,
            extra_options=['--fake-time', 'random'])
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('TSC' in line for line in result.stdout))

    def test_rdtscp_emulated_zero(self):
        result = self.sio2jail.run(
            self.RDTSCP_PATH,
            extra_options=['--fake-time', 'zero'])
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('TSC' in line for line in result.stdout))

    def test_rdtsc_returns_random_values(self):
        result = self.sio2jail.run(
            self.RDTSC_TWICE_PATH,
            extra_options=['--fake-time', 'random'])
        # Real RDTSC is monotonically increasing with tiny diffs (~20-100 cycles).
        # Random emulation produces unrelated values: b < a or huge diff.
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('RANDOM' in line for line in result.stdout),
                        "Emulated RDTSC should return random (non-monotonic) values")

    def test_rdtsc_returns_zero(self):
        result = self.sio2jail.run(
            self.RDTSC_TWICE_PATH,
            extra_options=['--fake-time', 'zero'])
        # Both RDTSC calls return 0, so values are the same
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('MONOTONIC 0 0' in line for line in result.stdout),
                        "Zero mode RDTSC should return 0")

    def test_notime_program_unaffected_random(self):
        result = self.sio2jail.run(
            self.NOTIME_PATH,
            extra_options=['--fake-time', 'random'])
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('OK' in line for line in result.stdout))

    def test_notime_program_unaffected_zero(self):
        result = self.sio2jail.run(
            self.NOTIME_PATH,
            extra_options=['--fake-time', 'zero'])
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertTrue(any('OK' in line for line in result.stdout))
