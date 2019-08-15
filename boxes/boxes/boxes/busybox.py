from typing import List

from . import base


class BusyBox(base.Sio2JailBox):
    def get_packages(self) -> List[str]:
        return ['busybox']
