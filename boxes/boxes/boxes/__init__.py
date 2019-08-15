from . import minimal
from . import busybox
from . import python


BOXES = {
    'minimal': minimal.Minimal,
    'busybox': busybox.BusyBox,
    'python': python.Python,
}

DEFAULT_BOXES = [
    'minimal', 'busybox', 'python-2.7', 'python-3.7'
]
