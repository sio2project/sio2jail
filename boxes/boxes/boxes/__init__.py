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
