# NumProcessors = `getconf NPROCESSORS_CONF`

.PHONY: all build clean configure

all: configure build

configure:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake

build:
	cmake --build build -j $(NumProcessors)
	echo "Making binary to build/main.bin"
	arm-none-eabi-objcopy -O binary build/main build/main.bin

clean:
	rm -rf build

