import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class TestReportedTimes(unittest.TestCase):
    SEC_PROGRAM_PATH = os.path.join(TEST_BIN_PATH, '1-sec-prog')
    SEC_PROGRAM_TH_PATH = os.path.join(TEST_BIN_PATH, '1-sec-prog-th')

    def setUp(self):
        self.sio2jail = SIO2Jail()

    def test_1_sec_program(self):
        result = self.sio2jail.run(self.SEC_PROGRAM_PATH)
        self.assertAlmostEqual(result.time, 1.0)

    def test_1_sec_program_threads_1(self):
        result = self.sio2jail.run(
            [self.SEC_PROGRAM_TH_PATH, 'flat', 1],
            memory='1G',
            extra_options=['-t', 1])
        self.assertAlmostEqual(result.time, 1.0)

    def test_1_sec_program_threads_5_flat(self):
        result = self.sio2jail.run(
            [self.SEC_PROGRAM_TH_PATH, 'flat', 5],
            memory='1G',
            extra_options=['-t', 5])
        self.assertAlmostEqual(result.time, 1.0)

    def test_1_sec_program_threads_5_deep(self):
        result = self.sio2jail.run(
            [self.SEC_PROGRAM_TH_PATH, 'deep', 5],
            memory='1G',
            extra_options=['-t', 5])
        self.assertAlmostEqual(result.time, 1.0)
