import ctypes
from queue import Queue
from enum import IntEnum, auto
from os.path import join, dirname

from rich import print
from rich.panel import Panel
from rich.pretty import Pretty

from core.state import state_manager
from core.types import GPSStruct, FSMState, FSMComponent, ComponentData, PacketType


LIBRARY_PATH = join(dirname(__file__), "libLoraParser.so")


lib_lora = ctypes.CDLL(LIBRARY_PATH)
sending_queue: Queue[bytes] = Queue()


@ctypes.CFUNCTYPE(
    None,
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_uint16,
    ctypes.c_int16,
    ctypes.c_int8,
)
def lora_receive_callback(payload, size, _rssi, _snr):
    if not size:
        return

    recieved_data = ctypes.string_at(payload, size)
    packet_type = PacketType(recieved_data[0])
    data = recieved_data[1:]

    print(
        Panel(
            f"[bold cyan]Received Packet from PSAT[/bold cyan]\n\n"
            f"[bold yellow]Packet Type:[/bold yellow] [green]{packet_type.name}[/green]\n"
            f"[bold yellow]Size:[/bold yellow] [green]{size} bytes[/green]\n\n"
            f"[bold magenta]Data:[/bold magenta]\n{data}",
            title="LoRa RX",
            border_style="green",
        )
    )

    if packet_type == PacketType.loraFsm_packetType_stateData:
        new_state = FSMState(data[0])
        prev_state = FSMState(data[1])
        state_manager.update_state(new_state, prev_state)
        print(
            Panel(
                f"[bold green]{prev_state.name}[/bold green] → "
                f"[bold cyan]{new_state.name}[/bold cyan]",
                title="FSM State Change",
                border_style="cyan",
            )
        )

    elif packet_type == PacketType.loraFsm_packetType_ping:
        print(
            Panel(
                "Received [bold yellow]PING[/bold yellow], Sending [bold yellow]PONG[/bold yellow]",
                title="LoRa",
                border_style="yellow",
            )
        )
        lora_send(PacketType.loraFsm_packetType_pong.to_bytes())

    elif packet_type == PacketType.loraFsm_packetType_gpsData:
        gps = GPSStruct.from_buffer_copy(data)
        state_manager.update_gps(
            dict((field, getattr(gps, field)) for field, *_ in gps._fields_)
        )

    elif packet_type == PacketType.loraFsm_packetType_preflightData:
        print(
            Panel(
                f"[bold magenta]Raw Preflight Data:[/bold magenta]\n{data}",
                title="Preflight RX",
                border_style="magenta",
            )
        )

        data_uint16 = ctypes.c_uint16.from_buffer(bytearray(data)).value
        data_bits = list(map(int, bin(data_uint16)[2:]))

        results = []
        for component_id, component_result in enumerate(data_bits):
            try:
                component = FSMComponent(component_id)
                results.append(
                    ComponentData(component, component.name, bool(component_result))
                )
            except Exception:
                break

        print(
            Panel(
                Pretty(results),
                title="Preflight Results",
                border_style="blue",
            )
        )
        state_manager.set_preflight_results(results)


@ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint16)
def lora_send_callback(payload, size):
    if not size:
        return

    data = ctypes.string_at(payload, size)
    print(
        Panel(
            f"[bold cyan]Created packet and added to queue\n\n"
            f"[bold cyan]Size:[/bold cyan] {size} bytes\n\n"
            f"[bold magenta]Data:[/bold magenta]\n{data}",
            title="Lora Tx",
            border_style="blue",
        )
    )

    sending_queue.put(data)


def lora_init():
    lib_lora.lora_init()
    lib_lora.loraImpl_setSendCallback(lora_send_callback)
    lib_lora.lora_setCallbacks(
        None,
        lora_receive_callback,
        None,
        None,
        None,
    )


def get_rx_done_callback():
    ptr = ctypes.c_void_p.in_dll(lib_lora, "loraImpl_onRxDoneCallback").value
    if ptr is None:
        raise RuntimeError("loraImpl_onRxDoneCallback not found")
    return ctypes.CFUNCTYPE(ctypes.c_void_p)(ptr)


def lora_send(packet):
    buffer = (ctypes.c_uint8 * len(packet)).from_buffer_copy(packet)
    lib_lora.lora_send(
        buffer,
        ctypes.c_uint16(len(packet)),
    )
