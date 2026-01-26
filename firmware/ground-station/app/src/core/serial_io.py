import time
import ctypes
from queue import Empty
from typing import Final
from threading import Thread, Event

import serial
from rich import print
from serial.tools import list_ports
from serial.serialutil import SerialException

from lib import sending_queue, lib_lora, lora_init, get_rx_done_callback


DEBUG_HEADER: Final = 0x11
FOOTER: Final = b"\xaa\xaa\xaa\xaa"
FOOTER_SEND: Final = b"\x61\x61\x61\x61"
BAUD_RATE: Final = 19200
SERIAL_RETRY_DELAY: Final = 5
SERIAL_READ_TIMEOUT: Final = None


stop_event = Event()


def find_serial_port():
    for port in list_ports.comports():
        if "usbserial" in port.device.lower():
            return port.device
    return None


def serial_main(rx_done_cb, stop_event: Event):
    while True:
        while not stop_event.is_set():
            port = find_serial_port()
            if not port:
                print("waiting for serial device...")
                time.sleep(SERIAL_RETRY_DELAY)
                continue

            try:
                with serial.Serial(
                    port,
                    BAUD_RATE,
                    timeout=SERIAL_READ_TIMEOUT,
                ) as ser:
                    print(f"opened serial port {port}")

                    Thread(
                        target=_read_loop,
                        args=(ser, rx_done_cb, stop_event),
                        daemon=True,
                    ).start()

                    Thread(
                        target=_write_loop,
                        args=(ser, stop_event),
                        daemon=True,
                    ).start()

                    stop_event.wait()

            except SerialException as e:
                print(f"[red]Serial error:[/red] {e}")
                time.sleep(SERIAL_RETRY_DELAY)
        stop_event.clear()


def start_serial(rx_done_cb, stop_event: Event):
    Thread(
        target=serial_main,
        args=(rx_done_cb, stop_event),
        daemon=True,
    ).start()


def _read_loop(ser, rx_done_cb, stop_event):
    try:
        while not stop_event.is_set():
            packet = ser.read_until(FOOTER)
            if not packet or not packet.endswith(FOOTER):
                continue

            if packet[0] == DEBUG_HEADER:
                print(
                    "[DEBUG]",
                    packet[1:-4].decode("utf-8", errors="replace"),
                    end="",
                )
                continue

            payload = packet[:-4]
            buf = (ctypes.c_uint8 * len(payload)).from_buffer_copy(payload)

            rx_done_cb(
                buf,
                ctypes.c_uint16(len(payload)),
                ctypes.c_int16(0),
                ctypes.c_int8(0),
            )
    except Exception as e:
        print(f"serial reading error {e}")
        stop_event.set()


def _write_loop(ser, stop_event):
    try:
        while not stop_event.is_set():
            try:
                data = sending_queue.get(timeout=0.1)
                ser.write(data + FOOTER_SEND)
            except Empty:
                pass
    except Exception as e:
        print(f"[red]Serial write error:[/red] {e}")
        stop_event.set()


def _irq_loop(stop_event):
    while not stop_event.is_set():
        lib_lora.lora_irqProcess()


def start_irq_loop(stop_event: Event):
    Thread(
        target=_irq_loop,
        args=(stop_event,),
        daemon=True,
    ).start()


def start_lora():
    lora_init()

    rx_done_cb = get_rx_done_callback()
    start_serial(rx_done_cb, stop_event)
    start_irq_loop(stop_event)

    print("[green]started serial tasks[/green]")


def stop_lora():
    stop_event.set()
    print("[yellow]stopped serial tasks[/yellow]")
