# STM32F103 Project
SRC = $(wildcard *.c)
SRC += syscall_mini.c
SRC += stm32f10x_gpio.c stm32f10x_rcc.c
SRC += stm32f10x_fsmc.c
# SRC += stm32f10x_tim.c

# build options
# debug build?
DEBUG ?= 0
# build for debug in ram?
RAMBUILD ?= 0
# Build path
BUILD_DIR = $(CURDIR)/build
CONFIG_DIR = $(CURDIR)/config

# CPU
TARGET = STM32F103VET6
CPU = STM32F10X_HD
PORTS = -mcpu=cortex-m3 -mlittle-endian -mthumb
# PORTS += -msoft-float
STARTUP := $(CONFIG_DIR)/startup_stm32f10x_hd.s
LDS_RAM = $(CONFIG_DIR)/STM32F103VE_ram.ld
LDS_FLASH = $(CONFIG_DIR)/STM32F103VE_flash.ld

ifeq ($(RAMBUILD), 1)
	LDS = $(LDS_RAM)
else
	LDS = $(LDS_FLASH)
endif


# Project defines
DEF =
DEF += $(CPU)
DEF += USE_FULL_ASSERT
DEF += USE_STDPERIPH_DRIVER
DEF += STDOUT_USART=1
DEF += STDIN_USART=1
DEF += STDERR_USART=1


ifeq ($(RAMBUILD), 1)
	DEF += VECT_TAB_SRAM
endif
ifeq ($(DEBUG), 1)
	DEF += DEBUG _DEBUG
else
	DEF += RELEASE
endif


# Library
ST_BASE = $(LIB_EXTERNAL)/c/stm32/STM32F10x_StdPeriph_Lib_V3.5.0/
ST_LIB = $(ST_BASE)Libraries/

# Incs
CPATH :=
CPATH += $(CURDIR)
CPATH += $(ST_LIB)CMSIS/Include/
CPATH += $(ST_LIB)CMSIS/CM3/CoreSupport/
CPATH += $(ST_LIB)CMSIS/CM3/DeviceSupport/ST/STM32F10x/
CPATH += $(ST_LIB)STM32F10x_StdPeriph_Driver/inc/

# Libs
VPATH := $(CURDIR)
VPATH += $(ST_LIB)CMSIS/CM3/CoreSupport/
VPATH += $(ST_LIB)CMSIS/CM3/DeviceSupport/ST/STM32F10x/
VPATH += $(ST_LIB)STM32F10x_StdPeriph_Driver/src/


CPATH += $(LIB_MACROBULL)/c/stm32/inc/
VPATH += $(LIB_MACROBULL)/c/stm32/src/

# Toolchain
CC = arm-none-eabi-gcc
# LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
GDB = arm-none-eabi-gdb

# Generate CFLAGS
_EMPTY :=
_SPACE := $(_EMPTY) $(_EMPTY)
CFLAGS := $(subst $(_SPACE),$(_SPACE)-I,$(CPATH)) -lc -lm \
	 -specs=nano.specs -lnosys -std=c99 -Wall $(PORTS) \
	 $(subst $(_SPACE),$(_SPACE)-D,$(DEF))
OBJ=$(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))

ifeq ($(DEBUG), 1)
	CFLAGS += -g -gdwarf-2 -Og
else
	CFLAGS += -Ofast
endif


#########################################
# Targets

all: build

build: obj_std

res.c: pre-resource

pre-resource:
# 	echo '#include <stdint.h>' > res.c
# 	echo '#include <stdint.h>' > res.h
	./cvt_font.py
	./cvt_image.py

debug: obj_std
# debug: clean obj_std
	arm-none-eabi-objdump -S $(BUILD_DIR)/out_std.elf  > $(BUILD_DIR)/asm.s
	ps -ea | grep openocd || openocd -f $(CONFIG_DIR)/stm32-debug.cfg &
	-killall $(GDB)
	yakuake-session -e $(GDB) -- --command=$(CONFIG_DIR)/startup.gdb

install: obj_std
# 	$(OBJCOPY) -O binary $(BUILD_DIR)/out_std.elf $(BUILD_DIR)/out_std.bin
	$(OBJCOPY) -O ihex $(BUILD_DIR)/out_std.elf $(BUILD_DIR)/out_std.hex
# 	st-flash write out.bin 0x08000000
	-killall openocd
	openocd -f $(CONFIG_DIR)/stm32-flash.cfg

external: obj_std
	$(OBJCOPY) -O ihex $(BUILD_DIR)/out_std.elf $(BUILD_DIR)/out_std.hex
	-killall openocd
	ps -ea | grep openocd || openocd -f $(CONFIG_DIR)/stm32-external.cfg &
	telnet localhost 4444

obj_std: $(OBJ)
	$(CC) -o $(BUILD_DIR)/out_std.elf $(CFLAGS) $(OBJ) $(STARTUP) -T$(LDS_FLASH)

obj_ram: $(OBJ)
	$(CC) -o $(BUILD_DIR)/out_ram.elf $(CFLAGS) $(OBJ) $(STARTUP) -T$(LDS_RAM)

$(BUILD_DIR)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
# 	-rm *.asm *.map *.o *.elf *.bin *.hex
	rm -rf $(BUILD_DIR)/*
	# For KDevelop
# 	echo ${CPATH} | sed "s/ /\\n/g" > .kdev_include_paths
# 	echo ${VPATH} | sed "s/ /\\n/g" >> .kdev_include_paths
	# For QtCreator
	echo ${CPATH} | sed "s/ /\\n/g" > $(wildcard *.includes)
	echo ${VPATH} | sed "s/ /\\n/g" >> $(wildcard *.includes)
	echo -n "#define " > $(wildcard *.config)
	echo ${DEF} | sed "s/ /\\n#define /g" >> $(wildcard *.config)
