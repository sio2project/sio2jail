import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class TestThreadsLimit(unittest.TestCase):
    SEC_PROGRAM_TH_PATH = os.path.join(TEST_BIN_PATH, '1-sec-prog-th')

    def setUp(self):
        self.sio2jail = SIO2Jail()

    @unittest.expectedFailure
    def test_threads_limit_exceeded_flat_race_condition(self):
        result = self.sio2jail.run(
            [self.SEC_PROGRAM_TH_PATH, 'flat', 16],
            memory='1G',
            extra_options=['-t', 15])
        self.assertEqual(result.message, 'threads limit exceeded')

    @unittest.expectedFailure
    def test_threads_limit_exceeded_deep_race_condition(self):
        result = self.sio2jail.run(
            [self.SEC_PROGRAM_TH_PATH, 'deep', 16],
            memory='1G',
            extra_options=['-t', 15])
        self.assertEqual(result.message, 'threads limit exceeded')
