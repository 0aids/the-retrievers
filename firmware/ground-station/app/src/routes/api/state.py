import random
from fastapi import APIRouter

from core.state import state_manager
from core.types import ComponentData

state_router = APIRouter(
    tags=[
        "State",
    ],
)

# @state_router.get("/api/state")
# def get_state():
#     return state_manager.snapshot()


# ALL THIS CODE BELOW IS THE VERSION FOR TESTSING
counter = 0
lat: float | None = None
long: float | None = None


def random_coords():
    global lat, long, counter

    counter += 1

    if counter > 20:
        lat = -36.852646
        long = 174.770197

    if counter > 100:
        return

    if lat and long:
        lat += random.uniform(-0.0001, 0.0001)
        long += random.uniform(-0.0001, 0.0001)


@state_router.get("/api/state")
def get_state():
    random_coords()
    s = state_manager.snapshot()
    if counter > 10:
        state_manager.update_state(2, 1)
    else:
        state_manager.update_state(1, 0)

    s.data.gps.positionValid = counter > 10
    s.data.gps.latitude = lat
    s.data.gps.longitude = long

    s.components = [
        ComponentData(id=1, name="Battery", status=True),
        ComponentData(id=2, name="LDR", status=True),
        ComponentData(id=3, name="Servo", status=True),
        ComponentData(id=4, name="Timers", status=True),
        ComponentData(id=5, name="GPS", status=True),
        ComponentData(id=6, name="Pressure", status=False),
        ComponentData(id=7, name="Camera", status=True),
        ComponentData(id=8, name="Buzzers", status=True),
    ]
    return s
