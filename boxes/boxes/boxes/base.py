import logging
import pathlib
from typing import List, Optional

import boxes.utils.cmd


class Box:
    def __init__(self, logger: logging.Logger, version: Optional[str]):
        self._logger = logger
        self._version = version

    def get_packages(self) -> List[str]:
        return []

    def get_version(self) -> str:
        if not self._version:
            raise Exception("Version not specified, but needed")
        return self._version

    def finalize_box(self, cmd: boxes.utils.cmd.CommandRunner, path: pathlib.Path) -> None:
        pass


class Sio2JailBox(Box):
    def finalize_box(self, cmd: boxes.utils.cmd.CommandRunner, path: pathlib.Path) -> None:
        super(Sio2JailBox, self).finalize_box(cmd, path)
        cmd.run('mkdir', '-pv', str(path / 'proc'))
        cmd.run('touch', str(path / 'exe'))
        cmd.run('chmod', '+wx', str(path / 'exe'))
