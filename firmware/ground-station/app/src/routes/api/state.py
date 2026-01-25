from fastapi import APIRouter

from core.state import state_manager

state_router = APIRouter(
    tags=[
        "State",
    ],
)


@state_router.get("/api/state")
def get_state():
    return state_manager.snapshot()
