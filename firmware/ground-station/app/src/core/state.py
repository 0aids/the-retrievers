import time

import threading
from copy import deepcopy
from functools import wraps

from core.types import FullState, FSMState, ComponentData


def with_lock(writing: bool = True):
    def decorator(f):
        @wraps(f)
        def wrapper(self: "StateManager", *args, **kwds):
            with self._lock:
                if writing:
                    self._state.radio.lastPacketTime = time.time()
                    self._state.radio.packetsReceived += 1

                return f(self, *args, **kwds)

        return wrapper

    return decorator


class StateManager:
    def __init__(self):
        self._lock = threading.Lock()
        self._state = FullState()

    @with_lock()
    def update_gps(self, gps: dict):
        for key, value in gps.items():
            if hasattr(self._state.data.gps, key):
                setattr(self._state.data.gps, key, value)

    @with_lock()
    def update_state(self, new_state: int, prev_state: int):
        try:
            self._state.fsm.currentState = FSMState(new_state)
            self._state.fsm.currentStateName = FSMState(new_state).name
        except Exception:
            self._state.fsm.currentState = int(new_state)
            self._state.fsm.currentStateName = f"state_{int(new_state)}"

        try:
            self._state.fsm.prevState = FSMState(prev_state)
            self._state.fsm.prevStateName = FSMState(prev_state).name
        except Exception:
            self._state.fsm.prevState = int(prev_state)
            self._state.fsm.prevStateName = f"state_{int(prev_state)}"

    @with_lock()
    def set_component_status(self, component_id: int, status: bool):
        for c in self._state.components:
            if c.id == component_id:
                c.status = status
                break

    @with_lock()
    def set_preflight_results(self, data: list[ComponentData]):
        self._state.components = data

    @with_lock(False)
    def snapshot(self):
        return deepcopy(self._state)


state_manager = StateManager()
