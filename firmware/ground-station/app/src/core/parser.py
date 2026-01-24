import ctypes
from threading import Thread
from os.path import join, dirname

import serial

BAUD_RATE = 115200
DEBUG_HEADER = 0x11
FOOTER = b"\xaa\xaa\xaa\xaa"
LIBRARY_PATH = join(dirname(__file__), "../lib/libLoraParser.so")

lib = ctypes.CDLL(LIBRARY_PATH)


@ctypes.CFUNCTYPE(
    None, ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint16, ctypes.c_int16, ctypes.c_int8
)
def _lora_recieve_callback(payload, size, _rss, _snr):
    data = ctypes.string_at(payload, size)
    print(f"Data Recieved: {data}")


def lora_init():
    lib.lora_init()

    lib.lora_setCallbacks(
        None,
        _lora_recieve_callback,
        None,
        None,
        None,
    )


def _read_serial_loop(rxDoneCallback):
    with serial.Serial("/dev/cu.usbserial-120", BAUD_RATE, timeout=100) as ser:
        while True:
            data_read = ser.read_until(FOOTER)
            if not data_read:
                continue

            if data_read[0] == DEBUG_HEADER:
                print("[DEBUG]", data_read[1:-4].decode("utf8"), end="")
                continue

            data_read = data_read[:-4]  # remove the footer
            buffer = (ctypes.c_uint8 * len(data_read)).from_buffer_copy(data_read)

            rxDoneCallback(
                buffer,
                ctypes.c_uint16(len(data_read)),
                ctypes.c_int16(0),
                ctypes.c_int8(0),
            )


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

    Thread(target=_read_serial_loop, args=(rxDoneCallback,)).start()
    Thread(target=_irq_loop).start()


if __name__ == "__main__":
    start_serial_tasks()
