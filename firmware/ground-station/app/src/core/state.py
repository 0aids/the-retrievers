import random
import threading
import time
from copy import deepcopy


class StateManager:
    def __init__(self):
        self._lock = threading.Lock()
        self._state = {
            "gps": {
                "latitude": None,
                "longitude": None,
                "speedKnots": None,
                "speedKph": None,
                "courseDeg": None,
                "hdop": None,
                "altitude": None,
                "geoidalSep": None,
                "day": None,
                "month": None,
                "year": None,
                "hours": None,
                "minutes": None,
                "seconds": None,
                "fixQuality": None,
                "satellitesTracked": None,
                "satsInView": None,
                "positionValid": None,
                "navValid": None,
                "fixInfoValid": None,
                "altitudeValid": None,
            },
            "fsm": {
                "state": None,
            },
            "radio": {"last_packet_time": None},
            "stats": {"packets_recieved": 0},
        }

    def update_gps(self, gps: dict):
        with self._lock:
            self._state["gps"].update(gps)
            self._state["radio"]["last_packet_time"] = time.time()
            self._state["stats"]["packets_recieved"] += 1

    def update_state(self, new_state: int):
        with self._lock:
            self._state["state"] = new_state

    def snapshot(self):
        with self._lock:
            snap = deepcopy(self._state)
            snap["gps"]["latitude"] = random.uniform(-36.854437, -36.848445)
            snap["gps"]["longitude"] = random.uniform(174.766667, 174.773276)
            return snap
