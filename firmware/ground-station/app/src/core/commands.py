from lib import lora_send


def handle_command(cmd: int, args: int | None = None):
    data = cmd.to_bytes()

    if args:
        data += args.to_bytes()

    print(f"Sending Data: {data}")
    lora_send(data)
