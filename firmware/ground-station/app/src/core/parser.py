import time
import ctypes
from collections import deque
from typing import Deque
from threading import Thread, Event, Lock
from os.path import join, dirname

import serial
from serial.serialutil import SerialException
from serial.tools import list_ports
from rich import print

from core.state import state_manager, FSMState, GPSStruct

BAUD_RATE = 19200
DEBUG_HEADER = 0x11
FOOTER = b"\xaa\xaa\xaa\xaa"
LIBRARY_PATH = join(dirname(__file__), "../lib/libLoraParser.so")
PACKET_FSM = 6
PACKET_GPS = 4
SERIAL_RETRY_DELAY = 5
SERIAL_READ_TIMEOUT = None

lib = ctypes.CDLL(LIBRARY_PATH)


class Queue:
    def __init__(self) -> None:
        self._lock = Lock()
        self._queue: Deque[bytes] = deque()

    def push(self, data: bytes) -> None:
        with self._lock:
            self._queue.append(data)

    def pop(self) -> bytes:
        with self._lock:
            return self._queue.popleft()

    def is_empty(self) -> bool:
        return len(self._queue) == 0


queue = Queue()


def find_serial_port():
    for port in list_ports.comports():
        if "usbserial" in port.device.lower():
            return port.device
    return None


@ctypes.CFUNCTYPE(
    None, ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint16, ctypes.c_int16, ctypes.c_int8
)
def _lora_recieve_callback(payload, size, _rss, _snr):
    data = ctypes.string_at(payload, size)
    print(f"[bold blue]Data of size {size} recieved: {data}")

    if not size:
        return

    packet_type = data[0]

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


def send_data(packet):
    buffer = (ctypes.c_uint8 * len(packet)).from_buffer_copy(packet)
    lib.lora_send(
        buffer,
        ctypes.c_uint16(len(packet)),
    )


@ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint16)
def _lora_send_callback(payload, size):
    data = ctypes.string_at(payload, size)
    print(f"[bold blue]Data of size {size} recieved: {data}")

    if not size:
        return

    queue.push(data)


def lora_init():
    lib.lora_init()

    lib.loraImpl_setSendCallback(_lora_send_callback)

    lib.lora_setCallbacks(
        None,
        _lora_recieve_callback,
        None,
        None,
        None,
    )


def _read_serial_loop(ser: serial.Serial, rx_done_cb, stop_event: Event):
    try:
        while not stop_event.is_set():
            packet = ser.read_until(FOOTER)
            if not packet:
                continue

            if packet[0] == DEBUG_HEADER:
                print(
                    "[DEBUG]",
                    packet[1:-4].decode("utf-8", errors="replace"),
                    end="",
                )
                continue

            if not packet.endswith(FOOTER):
                continue

            packet = packet[:-4]
            buffer = (ctypes.c_uint8 * len(packet)).from_buffer_copy(packet)

            rx_done_cb(
                buffer,
                ctypes.c_uint16(len(packet)),
                ctypes.c_int16(0),
                ctypes.c_int8(0),
            )
    except Exception:
        stop_event.set()

    print("Exiting Read Loop")


def _write_serial_loop(ser: serial.Serial, stop_event: Event):
    try:
        while not stop_event.is_set():
            if not queue.is_empty():
                data = queue.pop()
                print(f"Sending data of size {len(data)} over serial")
                ser.write(data)

    except Exception:
        stop_event.set()

    print("Exiting Write Loop")


def serial_loop(rx_done_cb):
    while True:
        port = find_serial_port()
        if not port:
            print("waiting for a serial device...")
            time.sleep(SERIAL_RETRY_DELAY)
            continue

        print(f"opening port {port}...")
        stop_event = Event()

        try:
            with serial.Serial(
                port,
                BAUD_RATE,
                timeout=SERIAL_READ_TIMEOUT,
            ) as ser:
                read_thread = Thread(
                    target=_write_serial_loop,
                    args=(ser, stop_event),
                )
                write_thread = Thread(
                    target=_read_serial_loop,
                    args=(ser, rx_done_cb, stop_event),
                )
                print(f"port {port} opened")

                read_thread.start()
                write_thread.start()

                while not stop_event.is_set():
                    pass

        except SerialException as e:
            print(f"[red]Serial error:[/red] {e}, retrying in {SERIAL_RETRY_DELAY}..")
            time.sleep(SERIAL_RETRY_DELAY)

        except OSError as e:
            print(f"os error: {e}, retrying in {SERIAL_RETRY_DELAY}...")
            time.sleep(SERIAL_RETRY_DELAY)

        stop_event.set()


def _irq_loop():
    while True:
        lib.lora_irqProcess()


def start_serial_tasks():
    lora_init()

    if (
        rxDoneCallback_ptr := ctypes.c_void_p.in_dll(
            lib, "loraImpl_onRxDoneCallback"
        ).value
    ) is None:
        print("loraImpl_onRxDoneCallback not found")
        return

    rxDoneCallback = ctypes.CFUNCTYPE(ctypes.c_void_p)(rxDoneCallback_ptr)

    Thread(target=serial_loop, args=(rxDoneCallback,)).start()
    Thread(target=_irq_loop).start()


if __name__ == "__main__":
    start_serial_tasks()
