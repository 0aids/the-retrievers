from fastapi import APIRouter

from core import state

state_router = APIRouter(
    tags=[
        "State",
    ],
)


@state_router.get("/api/state")
def get_state():
    return state.snapshot()
