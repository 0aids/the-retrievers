import itertools

from rich import print
from rich.panel import Panel

from lib import lora_send
from lib.lora import sent_commands

sequence_number = itertools.count()


def handle_command(cmd: int, args: int | None = None):
    data = cmd.to_bytes(byteorder="little")
    seq = next(sequence_number)
    data += seq.to_bytes(length=2, byteorder="little")

    sent_commands[seq] = cmd

    if args is not None:
        data += args.to_bytes(byteorder="little")

    print(
        Panel(
            f"[bold magenta]Command Data:[/bold magenta]\n{data}",
            title="Sending Command",
            border_style="green",
        )
    )

    lora_send(data)


# this is the plan rn
# Ground station sends request including number
# PSAT parsed command if it recieves it
#   PSAT extracts sequence number
#   PSAT checks if sequence number is more than last stored sequence number
#       if so: sends back an ACK and does not run code
#       if not: runs command or request and then sends ack
# after ground station sends command
# it does a block in the write loop that checks if most recent ack is bigger than most recent seq number in the queue
#   if it is: item is finally popped from the queue
#   if not:
#       item is popped from queue, ground station sends same request with same number, and then adds back to the queue
