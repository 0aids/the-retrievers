import ctypes
import threading
import time
from copy import deepcopy
from enum import IntEnum, auto


class FSMState(IntEnum):
    psatFSM_state_start = 0
    psatFSM_state_prelaunch = auto()
    psatFSM_state_ascent = auto()
    psatFSM_state_deployPending = auto()
    psatFSM_state_deployed = auto()
    psatFSM_state_descent = auto()
    psatFSM_state_landing = auto()
    psatFSM_state_recovery = auto()
    psatFSM_state_lowPower = auto()
    psatFSM_state_error = auto()
    psatFSM_state__COUNT = auto()


class GPSStruct(ctypes.Structure):
    _fields_ = [
        ("latitude", ctypes.c_float),
        ("longitude", ctypes.c_float),
        ("speedKnots", ctypes.c_float),
        ("speedKph", ctypes.c_float),
        ("courseDeg", ctypes.c_float),
        ("hdop", ctypes.c_float),
        ("altitude", ctypes.c_float),
        ("geoidalSep", ctypes.c_float),
        ("day", ctypes.c_uint32),
        ("month", ctypes.c_uint32),
        ("year", ctypes.c_uint32),
        ("hours", ctypes.c_uint32),
        ("minutes", ctypes.c_uint32),
        ("seconds", ctypes.c_uint32),
        ("fixQuality", ctypes.c_uint32),
        ("satellitesTracked", ctypes.c_uint32),
        ("satsInView", ctypes.c_uint32),
        ("positionValid", ctypes.c_bool),
        ("navValid", ctypes.c_bool),
        ("fixInfoValid", ctypes.c_bool),
        ("altitudeValid", ctypes.c_bool),
    ]


class StateManager:
    def __init__(self):
        self._lock = threading.Lock()
        self._state = {
            "gps": {field[0]: None for field in GPSStruct._fields_},
            "fsm": {"state": None, "name": None},
            "radio": {"last_packet_time": None},
            "stats": {"packets_received": 0},
        }

    def update_gps(self, gps: dict):
        with self._lock:
            self._state["gps"].update(gps)
            self._state["radio"]["last_packet_time"] = time.time()
            self._state["stats"]["packets_received"] += 1

    def update_state(self, new_state: int | FSMState):
        with self._lock:
            self._state["fsm"]["state"] = FSMState(new_state)
            self._state["fsm"]["name"] = FSMState(new_state).name

    def snapshot(self):
        with self._lock:
            return deepcopy(self._state)


state_manager = StateManager()
