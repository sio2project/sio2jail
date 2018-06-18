import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class TestReportedTimes(unittest.TestCase):
    SEC_PROGRAM_PATH = os.path.join(TEST_BIN_PATH, '1-sec-prog')

    def setUp(self):
        self.sio2jail = SIO2Jail()

    def test_1_sec_program(self):
        result = self.sio2jail.run(self.SEC_PROGRAM_PATH)
        self.assertAlmostEqual(result.time, 1.0)
