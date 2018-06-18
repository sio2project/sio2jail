import os
import subprocess

from .paths import *


class Supervisor(object):
    SUPERVISOR_BIN = None

    class Result(object):
        def __init__(self):
            self.supervisor_return_code = None
            self.return_code = None
            self.message = None
            self.memory = None
            self.time = None

    def run(self, program, stdin=None, extra_options=None):
        if extra_options is None:
            extra_options = []

        options = extra_options + [program]

        process = subprocess.Popen([self.SUPERVISOR_BIN] + options,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE)
        (stdout, stderr) = process.communicate(stdin)

        if process.poll() is None:
            process.kill()
            process.poll()

        result = self.Result()
        self.parse_results(result, stdout, stderr)
        result.stdout = map(lambda s: s.strip(), stdout.split('\n'))
        result.stderr = map(lambda s: s.strip(), stderr.split('\n')[:-3])
        result.supervisor_return_code = process.returncode
        return result

    def parse_results(self, result, stdout, stderr):
        raise NotImplementedError()


class SIO2Jail(Supervisor):
    SUPERVISOR_BIN = os.path.join(BIN_PATH, 'sio2jail')
    MINIMAL_BOX_PATH = os.path.join(BOXES_PATH, './minimal/')

    def run(self, program, box=None, memory=None, stdin=None, extra_options=None):
        if extra_options is None:
            extra_options = []
        if box is None:
            box = self.MINIMAL_BOX_PATH
        else:
            box = os.path.join(BOXES_PATH, box)

        extra_options = ['-b', box + ':/:ro'] + extra_options

        if memory is not None:
            extra_options.extend(['-m', str(memory)])

        return super(SIO2Jail, self).run(program, stdin, extra_options)

    def parse_results(self, result, stdout, stderr):
        lines = [s.strip() for s in stderr.split('\n') if len(s.strip()) > 0]
        result.message = lines[-1]
        result.return_code = int(lines[-2].split()[1])
        result.memory = int(lines[-2].split()[4])
        result.time = int(lines[-2].split()[2]) / 1000.0

class Oitimetool(Supervisor):
    SUPERVISOR_BIN = os.path.join(OITIMETOOL_BIN_PATH, 'oitimetool')

    def parse_results(self, result, stdout, stderr):
        lines = [s.strip() for s in stderr.split('\n') if len(s.strip()) > 0]
        result.time = float(lines[-1].strip('s'))
