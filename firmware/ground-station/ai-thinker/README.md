# Ai-thinker ground station firmware
It will just essentially forward all data directly to the computer via uart,
and a specialised c program on the laptop will actually parse everything.
And vice versa, any packets it receives over uart will be sent to lora immediately.

The testing will be able to send data to the lora, and we'll double check everything is correct
from there.

# Setup the environment
0. Required packages: \
    **THE PACKAGE NAMES MIGHT DIFFER**\
    IE For ubuntu `gcc-arm-embedded` is `gcc-arm-none-eabi`
    ```
    gcc-arm-embedded
    python313
    screen
    cmake
    make
    ```

    OR if you have nix (ur the goat if you do)
    ```bash
    nix develop
    ```

1. Clone the repo and initialize the submodules.
    ```bash
    git clone --depth=1 https://github.com/0aids/aithinker-lora-buildscript.git
    cd aithinker-lora-buildscript
    git submodule update --init
    ```

2. Setup the python venv environment, and install required packages `python3` or `python` or `py`
    ```bash
    python -m venv venv
    source venv/bin/activate
    python -m pip install configparser pyserial
    ```

3. Give yourself permissions to access the `/dev/ttyUSB*` by adding yourself to dialout group
    ```bash
    sudo usermod -aG dialout $(whoami)
    ```

4. Resign in to your account (IE wsl terminal type `exit` and then `wsl`)

# Building and flashing
1. Enter the python venv
    ```bash
    source venv/bin/activate
    ```

2. Build the binary
    ```bash
    make
    ```

3. On the lora module, press and HOLD the IO2 button, then press and release the RST button, then release IO2.

4. Flash the lora module (change the usb0 to be whatever the lora module is connected to).
    ```bash
    python tremo_loader.py -p /dev/ttyUSB0 -b 921600 flash 0x08000000 build/main.bin
    ```
