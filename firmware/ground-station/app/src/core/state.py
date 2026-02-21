import time
import ctypes
import threading
from copy import deepcopy
from functools import wraps
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


def with_lock(writing: bool = True):
    def decorator(f):
        @wraps(f)
        def wrapper(self: "StateManager", *args, **kwds):
            with self._lock:
                if writing:
                    self._state["radio"]["lastPacketTime"] = time.time()
                    self._state["radio"]["packetsReceived"] += 1

                return f(self, *args, **kwds)

        return wrapper

    return decorator


class StateManager:
    def __init__(self):
        self._lock = threading.Lock()
        # I'm using camel case in C and TS so i'm using it here for consistency
        # im sorry python gods, i love snake case i really do
        self._state = {
            "preflightSuccess": None,
            "data": {
                "gps": {field[0]: None for field in GPSStruct._fields_},
                "ldrVoltage": None,
                "servoAngle": None,
                "batteryLevel": None,
                "temperature": None,
                "cameraOn": None,
                "pressure": None,
            },
            "fsm": {
                "currentState": None,
                "currentStateName": None,
                "prevState": None,
                "prevStateName": None,
            },
            "components": [],
            "radio": {
                "lastPacketTime": None,
                "packetsReceived": 0,
            },
        }

    @with_lock()
    def update_gps(self, gps: dict):
        self._state["data"]["gps"].update(gps)

    @with_lock()
    def update_state(self, new_state: int, prev_state: int):
        self._state["fsm"]["currentState"] = FSMState(new_state)
        self._state["fsm"]["currentStateName"] = FSMState(new_state).name

        self._state["fsm"]["prevState"] = FSMState(prev_state)
        self._state["fsm"]["prevStateName"] = FSMState(prev_state).name

    @with_lock(False)
    def snapshot(self):
        return deepcopy(self._state)


state_manager = StateManager()
