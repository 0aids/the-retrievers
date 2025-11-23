# IDK Why make doesn't support spaces for indentation.
.PHONY: configure build flash
CC := clang
TARGET := esp32

all: configure build



configure:
	IDF_TOOLCHAIN=$(CC) idf.py set-target $(TARGET)

build:
	IDF_TOOLCHAIN=$(CC) idf.py build

flash: build
	IDF_TOOLCHAIN=$(CC) idf.py flash monitor
