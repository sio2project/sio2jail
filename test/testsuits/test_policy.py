import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class TestPolicy(unittest.TestCase):
    EVIL_PROGRAM_PATH = os.path.join(TEST_BIN_PATH, '1-sec-evil')

    def setUp(self):
        self.sio2jail = SIO2Jail()

    def test_1_sec_program(self):
        result = self.sio2jail.run(self.EVIL_PROGRAM_PATH)
        self.assertIn('forbidden syscall', result.message)
        self.assertEqual('RV', result.status)
