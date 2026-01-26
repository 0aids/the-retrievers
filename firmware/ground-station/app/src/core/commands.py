from lib import lora_send


def handle_command(cmd: int, args: dict):
    print("Bytes:", cmd.to_bytes())
    lora_send(cmd.to_bytes())
