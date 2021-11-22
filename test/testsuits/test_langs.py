import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class TestLanguages(unittest.TestCase):
    C_PROGRAM_PATH = os.path.join(TEST_BIN_PATH, 'sum_c')
    CXX_PROGRAM_PATH = os.path.join(TEST_BIN_PATH, 'sum_cxx')
    PYTHON2_PROGRAM_PATH = os.path.join(
        SOURCE_PATH, './test/src/sum_python2.py')
    PYTHON3_PROGRAM_PATH = os.path.join(
        SOURCE_PATH, './test/src/sum_python3.py')

    PYTHON3_9_PROGRAM_PATH = os.path.join(
        SOURCE_PATH, './test/src/sum_python3.9.py')

    URANDOM_MOCKED_FILE = os.path.join(SOURCE_PATH, './test/src/zero')

    def setUp(self):
        self.sio2jail = SIO2Jail()

    def perform(self, *args, **kwargs):
        kwargs.update({'stdin': '18 24'})
        result = self.sio2jail.run(*args, **kwargs)
        self.assertEqual(result.return_code, 0)
        self.assertEqual(result.supervisor_return_code, 0)
        self.assertGreater(len(result.stdout), 0)
        self.assertEqual(result.stdout[0], '42')

    def test_c(self):
        self.perform(self.C_PROGRAM_PATH)

    def test_cxx(self):
        self.perform(self.CXX_PROGRAM_PATH)

    def test_python2(self):
        self.perform(self.PYTHON2_PROGRAM_PATH, box='python2')

    def test_python3_5(self):
        self.perform(self.PYTHON3_PROGRAM_PATH, box='python3',
                     extra_options=['-b', self.URANDOM_MOCKED_FILE + ':/dev/urandom:ro,dev'])

    def test_python3_9(self):
        self.perform(self.PYTHON3_9_PROGRAM_PATH, box='python3_9',
                     extra_options=['--memory-limit', "100M"])
