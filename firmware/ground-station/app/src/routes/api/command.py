from fastapi import APIRouter, HTTPException
from pydantic import BaseModel

from core.commands import handle_command

command_router = APIRouter(
    tags=[
        "Command",
    ],
)


class Command(BaseModel):
    cmd: str
    args: dict = {}


@command_router.post("/api/command")
def send_command(command: Command):
    try:
        handle_command(command.cmd, command.args)
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))

    return {"status": "ok"}
