# NumProcessors = `getconf NPROCESSORS_CONF`

.PHONY: all build configure_standalone configure_combo_mode clean test rebuild debug _debug

combo_mode: configure_combo_mode build

standalone_mode: configure_standalone build

debug: _debug build


configure_standalone:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake -Dd_psatStandaloneMode

configure_combo_mode: # Basically the uart forwarding firmware
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake

build:
	cmake --build build -j $(NumProcessors)

clean:
	rm -rf build
_debug:
	cmake -DDisableOPT=ON -S . -B build

