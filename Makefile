PREFIX = ~/vc4-toolchain/prefix/bin/vc4-elf-
CC = $(PREFIX)gcc
AS = $(CC)
LD = $(CC)
OBJCOPY = $(PREFIX)objcopy
GCCPARAMS = -c -nostdlib -Wno-multichar -fsingle-precision-constant -fno-leading-underscore -Wdouble-promotion -D__VIDEOCORE4__
ASPARAMS = -c -nostdlib -D__VIDEOCORE4__
LDPARAMS = -nostdlib -nostartfiles -Wl,--build-id=none

LINKER = linker.ld

SRCS = $(shell find . -type f -name '*.c')
OBJS = $(patsubst ./%.c,build/obj/%.o,$(SRCS))

ASM_SRCS = $(wildcard *.S)
ASM_OBJS = $(patsubst %.S,build/obj/asm_%.o,$(ASM_SRCS))

all: build/start.elf

build/obj/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -c -o $@ $<

build/obj/asm_%.o: %.S
	mkdir -p $(@D)
	$(AS) $(ASPARAMS) -o $@ $<

build/start.elf: $(LINKER) $(ASM_OBJS) $(OBJS)
	$(LD) $(LDPARAMS) -T $(LINKER) -o $@ $(ASM_OBJS) $(OBJS)

clean:
	rm -rf build/vc.* build/obj/*

.PHONY: all clean
