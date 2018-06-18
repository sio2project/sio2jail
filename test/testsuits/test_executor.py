import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class TestExecutor(unittest.TestCase):
    STDERR_PROGRAM_PATH = os.path.join(TEST_BIN_PATH, 'stderr-write')
    LOOP_PROGRAM_PATH = os.path.join(TEST_BIN_PATH, 'infinite-loop')

    def setUp(self):
        self.sio2jail = SIO2Jail()

    def test_stderr_write(self):
        result = self.sio2jail.run(self.STDERR_PROGRAM_PATH)
        self.assertEqual(result.stdout, ['stdout'])
        self.assertFalse(result.stderr)

    def test_infinite_loop(self):
        options = ["--instruction-count-limit", "1g"]
        result = self.sio2jail.run(
                self.LOOP_PROGRAM_PATH, extra_options=options)
        self.assertAlmostEqual(result.time, 0.5)
