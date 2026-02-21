from fastapi import APIRouter, HTTPException
from pydantic import BaseModel

from core.commands import handle_command
from lib import PacketType

command_router = APIRouter(
    tags=[
        "Command",
    ],
)


class Command(BaseModel):
    cmd: PacketType
    args: int | None = None


@command_router.post("/api/command")
def send_command(command: Command):
    try:
        print(f"Handle Command ({command.cmd.name})...")
        handle_command(command.cmd, command.args)
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))

    return {"status": "ok"}
