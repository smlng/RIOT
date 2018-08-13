import serial

from robot.utils import (ConnectionCache, abspath, cmdline2list, console_decode,
                         is_list_like, is_truthy, NormalizedDict, py2to3,
                         secs_to_timestr, system_decode, system_encode,
                         timestr_to_secs, IRONPYTHON, JYTHON, WINDOWS)
from robot.version import get_version
from robot.api import logger


class RiotConnector(object):

    def __init__(self):
        self._devices = ConnectionCache('No Device Connected.')
        self._results = {}
        self._config = {}
        self._metainfo = {}

    def __del__(self):
        self.disconnect_all()

    def __exit__(self):
        self.disconnect_all()

    def connect_serial(self, port, baudrate, timeout=5):
        config = { 'port' : port, 'baudrate' : baudrate, 'timeout' : timeout }
        device = serial.Serial(port=port, baudrate=int(baudrate), timeout=int(timeout), dsrdtr=0, rtscts=0)
        logger.info(u'Connecting  device with port={}, baudrate={}, timeout={}'.format(system_decode(port), baudrate, timeout))
        self._current = self._devices.register(device)
        self._config[self._current] = config
        return self._current

    def disconnect(self, handle=None):
        if handle:
            self._devices[handle].close()
        else:
            self._devices[self._current].close()

    def disconnect_all(self):
        for device in self._devices:
            device.close()

    def is_connected(self, handle=None):
        if handle:
            return self._devices[handle].is_open
        else:
            return self._devices[self._current].is_open

    def should_be_connected(self, handle=None, error_message='device is not connected.'):
        if not self.is_connected(handle):
            raise AssertionError(error_message)

    def should_be_disconnected(self, handle=None, error_message='device is connected.'):
        if self.is_connected(handle):
            raise AssertionError(error_message)

    def set_active(self, handle=None):
        if handle:
            self._current = handle

    def set_metainfo(self, handle=None, params=None):
        if handle:
            self._metainfo[handle] = params
        else:
            self._metainfo[self._current] = params

    def get_metainfo(self, handle=None):
        if handle:
            return self._metainfo[handle]
        else:
            return self._metainfo[self._current]

    def get_config(self, handle=None):
        if handle:
            return self._config[handle]
        else:
            return self._config[self._current]

    def run_command(self, handle=None, cmd=None, *arguments):
        device = self._devices[self._current]
        if handle:
            device = self._devices[handle]
        if not cmd:
            return ''
        command = ' '.join(str(x) for x in [cmd] + list(arguments))
        logger.info(u'Device {} running command: {}'.format(system_decode(device.name), system_decode(command)))
        device.write(system_encode('{}\n'.format(command)))
        response = 'error'
        result = list()
        while (response != ''):
            try:
                response = system_decode(device.readline())
            except:
                logger.error(u'Failed to decode string!')
            else:
                result.append(response.strip())

        return '\n'.join(result) if len(result) > 0 else ''
