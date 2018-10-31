import os
import unittest

from base.supervisor import SIO2Jail
from base.paths import *


class MemoryLimitExceededMeta(type):
    def __new__(mcs, name, bases, members):
        def create_mle_test(program):
            def test(self):
                self._run_memory_test(program, 16 * self.MB, expect_mle=True)
                self._run_memory_test(program, 64 * self.MB, expect_mle=True)
            return test

        test_cases = [
                leak_type + '_' + arch
                for leak_type in ['tiny', 'huge', 'dive']
                for arch in ['32', '64']]

        for test_case in test_cases:
            members['test_memory_leak_' + test_case] = \
                    create_mle_test('leak-' + test_case)

        return type.__new__(mcs, name, bases, members)


class TestMemoryLimit(unittest.TestCase):
    __metaclass__ = MemoryLimitExceededMeta

    MB = 1 * 1024
    GB = 1 * 1024 * MB

    def setUp(self):
        self.sio2jail = SIO2Jail()

    def _run_memory_test(self, program, memory_limit, memory_delta=1024,
            expected_memory=None, expect_mle=False):
        program = os.path.join(TEST_BIN_PATH, program)
        result = self.sio2jail.run(program, memory=memory_limit)

        self.assertEqual(result.supervisor_return_code, 0)
        if expect_mle:
            self.assertGreater(result.memory, memory_limit)
            self.assertEqual('MLE', result.status)
        else:
            self.assertAlmostEqual(result.memory, expected_memory,
                    delta=memory_delta)
            self.assertEqual('ok', result.message)

    def test_memory_result(self):
        self._run_memory_test('1-sec-prog', None, 1024, False)
