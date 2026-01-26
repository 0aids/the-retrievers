from lib import lora_send


def handle_command(cmd: str, args: dict):
    lora_send(b"\x05")
