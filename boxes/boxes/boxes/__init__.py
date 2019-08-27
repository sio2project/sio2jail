from . import minimal
from . import busybox
from . import python
from . import java


BOXES = {
    'minimal': minimal.Minimal,
    'busybox': busybox.BusyBox,
    'python': python.Python,
    'java': java.JavaOpenJDK,
}

DEFAULT_BOXES = [
    'minimal', 'busybox', 'python-2', 'python-3', 'java-11'
]
