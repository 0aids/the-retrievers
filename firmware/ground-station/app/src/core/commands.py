from lib import lora_send

from rich import print
from rich.panel import Panel


def handle_command(cmd: int, args: int | None = None):
    data = cmd.to_bytes()

    if args is not None:
        data += args.to_bytes()

    print(
        Panel(
            f"[bold magenta]Command Data:[/bold magenta]\n{data}",
            title="Sending Command",
            border_style="green",
        )
    )

    lora_send(data)
