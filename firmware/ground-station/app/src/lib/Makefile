all:
	@gcc -c -fPIC \
		-I../../../../platform-agnostic/shared/ \
		-I../../../../platform-agnostic/required-impls \
		-I../../../../psat-station/components/loraFsm/include/ \
		../../../../platform-agnostic/shared/shared_lora.c \
		loraImpl.c \
		utilsImpl.c \
		-std=c23
	@gcc -shared -Wl  -o libLoraParser.so *.o

