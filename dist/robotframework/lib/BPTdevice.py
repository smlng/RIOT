from if_lib import bpt_if

from robot.utils import (ConnectionCache, abspath, cmdline2list, console_decode,
                         is_list_like, is_truthy, NormalizedDict, py2to3,
                         secs_to_timestr, system_decode, system_encode,
                         timestr_to_secs, IRONPYTHON, JYTHON, WINDOWS)
from robot.version import get_version
from robot.api import logger

import time

class BPTdevice(bpt_if.BptIf):

    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()

    def reset_dut(self):
        self.set_sys_cr(1)
        self.execute_changes()
        time.sleep(0.01)
        self.set_sys_cr(0)
        self.execute_changes()
        time.sleep(2)
