# NumProcessors = `getconf NPROCESSORS_CONF`

.PHONY: all build configure clean test rebuild debug _debug

all: configure build

debug: _debug build


configure:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake

build:
	cmake --build build -j $(NumProcessors)

clean:
	rm -rf build
_debug:
	cmake -DDisableOPT=ON -S . -B build

