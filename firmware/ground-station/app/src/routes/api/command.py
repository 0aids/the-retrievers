from fastapi import APIRouter, HTTPException
from pydantic import BaseModel

from core.commands import handle_command

command_router = APIRouter(
    tags=[
        "Command",
    ],
)


class Command(BaseModel):
    cmd: int
    args: dict = {}


@command_router.post("/api/command")
def send_command(command: Command):
    try:
        print("Handle Command...")
        handle_command(command.cmd, command.args)
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))

    return {"status": "ok"}
