from lib import lora_send
from core.serial_io import FOOTER


def handle_command(cmd: str, args: dict):
    lora_send(b"\x05" + FOOTER)
