import ctypes
from queue import Queue
from enum import IntEnum, auto
from os.path import join, dirname

from rich import print

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
def lora_receive_callback(payload, size, rssi, snr):
    if not size:
        return

    recieved_data = ctypes.string_at(payload, size)
    packet_type = PacketType(recieved_data[0])
    data = recieved_data[1:]

    print(f"RSSI: {rssi}, SNR: {snr}")

    print(
        f"[bold blue]Data of type: {packet_type.name} (size: {size}) recieved: {data}"
    )

    if packet_type == PacketType.loraFsm_packetType_stateData:
        new_state = FSMState(data[0])
        prev_state = FSMState(data[1])
        state_manager.update_state(new_state, prev_state)
        print(f"[green]FSM changed state to: {new_state.name}")

    elif packet_type == PacketType.loraFsm_packetType_ping:
        print("[green]pong em back fr")
        lora_send(PacketType.loraFsm_packetType_pong.to_bytes())

    elif packet_type == PacketType.loraFsm_packetType_gpsData:
        gps = GPSStruct.from_buffer_copy(data)
        state_manager.update_gps(
            dict((field, getattr(gps, field)) for field, *_ in gps._fields_)
        )

    elif packet_type == PacketType.loraFsm_packetType_preflightData:
        print("Preflight Data:", data)

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

        print(results)
        state_manager.set_preflight_results(results)


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
