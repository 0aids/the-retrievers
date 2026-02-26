import time

import threading
from copy import deepcopy
from functools import wraps

from core.types import FullState, FSMState, ComponentData, FSMComponent


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

        self._state.components = [
            ComponentData(
                id=comp.value,
                name=comp.name,
                enabled=False,
                inited=False,
                task=False,
                error=False,
                preflightSuccess=False,
            )
            for comp in FSMComponent
            if comp != FSMComponent.psatFSM_component__COUNT
        ]

    @with_lock()
    def update_gps(self, gps: dict):
        for key, value in gps.items():
            if hasattr(self._state.data.gps, key):
                setattr(self._state.data.gps, key, value)

    @with_lock()
    def update_component_runtime(self, enabled, init, task, error):
        for c in self._state.components:
            bit = 1 << c.id

            c.enabled = bool(enabled & bit)
            c.inited = bool(init & bit)
            c.task = bool(task & bit)
            c.error = bool(error & bit)

    @with_lock()
    def update_preflight(self, mask: int):
        for c in self._state.components:
            bit = 1 << c.id
            c.preflightSuccess = bool(mask & bit)

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
    def set_preflight_results(self, data: list[ComponentData]):
        self._state.components = data

    @with_lock(False)
    def snapshot(self):
        return deepcopy(self._state)


state_manager = StateManager()
