import contextlib
import fcntl
import logging
import os
import pathlib
import select
import subprocess
from typing import Iterable, List, Iterator, Optional


class LinePollReader:
    def __init__(self):
        self._poll = select.poll()
        self._fd_buffers = {}
        self._fd_line_cb = {}

    def add(self, fd, line_cb):
        if hasattr(fd, 'fileno'):
            fd = fd.fileno()
        self._fd_buffers[fd] = ""
        self._fd_line_cb[fd] = line_cb
        self._poll.register(fd)

    def poll(self):
        for fd, _ in self._poll.poll():
            self._fd_buffers[fd] += os.read(fd, 4096).decode('utf-8')
            while '\n' in self._fd_buffers[fd]:
                line, self._fd_buffers[fd] = self._fd_buffers[fd].split('\n', 1)
                self._fd_line_cb[fd](line)

    def finish(self):
        for fd in self._fd_buffers:
            while True:
                buff = os.read(fd, 4096).decode('utf-8')
                if not buff:
                    break
                self._fd_buffers[fd] += buff
                while '\n' in self._fd_buffers[fd]:
                    line, self._fd_buffers[fd] = self._fd_buffers[fd].split('\n', 1)
                    self._fd_line_cb[fd](line)
            if self._fd_buffers[fd]:
                self._fd_line_cb[fd](self._fd_buffers[fd])


class CommandRunner:
    def __init__(self, logger: logging.Logger):
        self._logger = logger
        self._sudo = False
        self._chroot = None

    @contextlib.contextmanager
    def sudo(self) -> Iterator[None]:
        prev_sudo = self._sudo
        self._sudo = True
        try:
            yield
        finally:
            self._sudo = prev_sudo

    @contextlib.contextmanager
    def chroot(self, path: pathlib.Path) -> Iterator[None]:
        with self.sudo():
            self.run('mount', '-t', 'proc', 'none', str(path / 'proc'))
        prev_chroot = self._chroot
        self._chroot = path
        try:
            yield
        finally:
            self._chroot = prev_chroot
            with self.sudo():
                self.run('umount', '-R', str(path / 'proc'))

    def _make_cmd(self, prog: str, args: Iterable[str]) -> List[str]:
        cmd = [prog] + list(args)
        if self._chroot:
            cmd = ['chroot', str(self._chroot)] + cmd
        if self._sudo or self._chroot:
            cmd = ['sudo', '--'] + cmd
        return cmd

    def _set_nonblock(self, fds) -> None:
        for fd in fds:
            flags = fcntl.fcntl(fd, fcntl.F_GETFL)
            fcntl.fcntl(fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)

    def run(
        self,
        prog,
        *args,
        return_stdout=False,
        cwd: Optional[pathlib.Path] = None
    ) -> Iterator[str]:
        assert cwd is None or not self._chroot

        cmd = self._make_cmd(prog, args)
        self._logger.info("Executing: %s", ' '.join(cmd))

        process = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=str(cwd) if cwd else None
        )

        process.stdin.close()
        self._set_nonblock([process.stdout, process.stderr])

        def log(line):
            self._logger.info("{}: {}".format(prog, line))
        stdout = []

        def gather(line):
            log(line)
            stdout.append(line)

        poll = LinePollReader()
        poll.add(process.stdout, gather if return_stdout else log)
        poll.add(process.stderr, log)

        while process.poll() is None:
            poll.poll()
        poll.finish()

        if process.poll() == 0:
            self._logger.info("Exited with %s", process.poll())
        else:
            self._logger.error("Exited with %s: %s", process.poll(), cmd)
            raise Exception("Command failed with code {}: {}".format(process.poll(), cmd))

        if return_stdout:
            return stdout
