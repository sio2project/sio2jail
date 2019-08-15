from typing import List

from . import base


class Python(base.Sio2JailBox):
    def get_packages(self) -> List[str]:
        return ['python' + self.get_version()]
