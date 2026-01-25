import ctypes
from queue import Queue
from os.path import join, dirname

from rich import print

from core.state import state_manager, FSMState, GPSStruct

LIBRARY_PATH = join(dirname(__file__), "libLoraParser.so")

PACKET_FSM = 6
PACKET_GPS = 4

lib_lora = ctypes.CDLL(LIBRARY_PATH)
sending_queue: Queue[bytes] = Queue()


@ctypes.CFUNCTYPE(
    None,
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_uint16,
    ctypes.c_int16,
    ctypes.c_int8,
)
def lora_receive_callback(payload, size, _rss, _snr):
    if not size:
        return

    data = ctypes.string_at(payload, size)
    packet_type = data[0]

    print(f"[bold blue]Data of size {size} recieved: {data}")

    # the 50 is till aidan reflashes firmware
    if packet_type == PACKET_FSM and size < 50:
        state = FSMState(data[1])
        state_manager.update_state(state)
        print(f"[green]FSM changed state to: {state.name}")

    elif packet_type == PACKET_GPS or size > 50:
        gps = GPSStruct.from_buffer_copy(data[1:])
        state_manager.update_gps(
            {
                "latitude": gps.latitude,
                "longitude": gps.longitude,
            }
        )


@ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint16)
def lora_send_callback(payload, size):
    if not size:
        return

    data = ctypes.string_at(payload, size)
    print(f"[bold blue]Data of size {size} recieved: {data}")

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
