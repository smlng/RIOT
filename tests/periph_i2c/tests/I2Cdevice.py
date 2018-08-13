from periph_i2c_if import PeriphI2CIf

from robot.utils import (ConnectionCache, abspath, cmdline2list, console_decode,
                         is_list_like, is_truthy, NormalizedDict, py2to3,
                         secs_to_timestr, system_decode, system_encode,
                         timestr_to_secs, IRONPYTHON, JYTHON, WINDOWS)
from robot.version import get_version
from robot.api import logger

class I2Cdevice(PeriphI2CIf):

    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()
