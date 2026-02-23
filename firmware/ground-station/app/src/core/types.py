import ctypes
from enum import IntEnum, auto
from typing import Optional, List
from dataclasses import dataclass, field


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


@dataclass
class GPSData:
    latitude: Optional[float] = None
    longitude: Optional[float] = None
    speedKnots: Optional[float] = None
    speedKph: Optional[float] = None
    courseDeg: Optional[float] = None
    hdop: Optional[float] = None
    altitude: Optional[float] = None
    geoidalSep: Optional[float] = None
    day: Optional[int] = None
    month: Optional[int] = None
    year: Optional[int] = None
    hours: Optional[int] = None
    minutes: Optional[int] = None
    seconds: Optional[int] = None
    fixQuality: Optional[int] = None
    satellitesTracked: Optional[int] = None
    satsInView: Optional[int] = None
    positionValid: Optional[bool] = None
    navValid: Optional[bool] = None
    fixInfoValid: Optional[bool] = None
    altitudeValid: Optional[bool] = None


@dataclass
class FSMData:
    currentState: Optional[FSMState | int] = None
    currentStateName: Optional[str] = None
    prevState: Optional[FSMState | int] = None
    prevStateName: Optional[str] = None


@dataclass
class RadioData:
    lastPacketTime: Optional[float] = None
    packetsReceived: int = 0


@dataclass
class ComponentData:
    id: int
    name: str
    status: bool


@dataclass
class DataSection:
    gps: GPSData = field(default_factory=GPSData)
    ldrVoltage: Optional[float] = None
    servoAngle: Optional[float] = None
    batteryLevel: Optional[float] = None
    temperature: Optional[float] = None
    cameraOn: Optional[bool] = None
    pressure: Optional[float] = None


@dataclass
class FullState:
    preflightSuccess: Optional[bool] = None
    data: DataSection = field(default_factory=DataSection)
    fsm: FSMData = field(default_factory=FSMData)
    components: List[ComponentData] = field(default_factory=list)
    radio: RadioData = field(default_factory=RadioData)
