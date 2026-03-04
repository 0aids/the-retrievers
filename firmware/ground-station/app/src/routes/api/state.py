import random
from fastapi import APIRouter

from core.state import state_manager

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

    s.data.gps.positionValid = counter > 10
    s.data.gps.latitude = lat
    s.data.gps.longitude = long

    return s
