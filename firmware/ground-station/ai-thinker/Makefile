# NumProcessors = `getconf NPROCESSORS_CONF`

.PHONY: all build clean configure

all: configure build

configure:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake

build:
	cmake --build build -j $(NumProcessors)

clean:
	rm -rf build

