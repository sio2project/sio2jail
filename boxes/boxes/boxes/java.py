
from typing import List

from . import base


class JavaOpenJDK(base.Sio2JailBox):
    def get_packages(self) -> List[str]:
        if self.get_version() == '11':
            return ['openjdk-11-jre-headless']
        elif self.get_version() == '8':
            return ['openjdk-8-jre-headless']
        raise NotImplementedError(
            "Java using openjdk for java version {}".format(self.get_version()))
