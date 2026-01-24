all:
	@gcc -c -fPIC \
		../../../../psat-station/components/loraFsm/packets.c \
		-I../../../../platform-agnostic/shared/ \
		-I../../../../psat-station/components/loraFsm/include/ \
		-I../../../../psat-station/components/helpers/include \
		../../../../psat-station/components/helpers/helpers.c \
		loraImpl.c \
		../../../../psat-station/components/loraFsm/packets.c \
		../../../../platform-agnostic/shared/shared_lora.c -I \
		../../../../platform-agnostic/required-impls utilsImpl.c \
		-I../../../../platform-agnostic/shared \
		-DBalls -std=c23
	@gcc -shared -Wl  -o libLoraParser.so *.o

