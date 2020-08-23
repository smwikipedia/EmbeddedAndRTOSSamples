AS = arm-none-eabi-as
GCC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

RMDIR = rm -r
MKDIR = mkdir

FONT_NAME = font12x16

WORKSPACE = .
BUILD_DIR = ./build
KERNEL_DIR = $(WORKSPACE)/kernel
RESOURCE_DIR = $(WORKSPACE)/resource
DEVICE_DIR = $(WORKSPACE)/devices

MYOS_BIN = myos.bin
MYOS_ELF = myos.elf


INCLUDES = \
	-I $(KERNEL_DIR)/includes \
    -I $(RESOURCE_DIR)/fonts \
    -I $(DEVICE_DIR)/board \
    -I $(DEVICE_DIR)/lcd \
    -I $(DEVICE_DIR)/uart \
    -I $(DEVICE_DIR)/vic \
	-I $(DEVICE_DIR)/timer \
	-I $(DEVICE_DIR)/kbd

LIBS = libs/libgcc.a
AS_FLAGS = -mcpu=arm926ej-s
GCC_FLAGS = -mcpu=arm926ej-s -marm -g -c -nodefaultlibs -nostdlib -nostartfiles -fno-builtin -nostdinc $(INCLUDES) -Werror
LD_FLAGS = -T $(WORKSPACE)/myos.ld -nostdlib
OBJCOPY_FLAGS = -O binary

OBJ_BMP = \
	$(BUILD_DIR)/image.o
$(OBJ_BMP) : $(RESOURCE_DIR)/images/image.bmp
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm $(RESOURCE_DIR)/images/image.bmp $(BUILD_DIR)/image.o

OBJ_UART = $(BUILD_DIR)/uart.o
$(OBJ_UART) : $(DEVICE_DIR)/uart/pl011.c $(DEVICE_DIR)/uart/pl011.h
	$(GCC) $(GCC_FLAGS) $(DEVICE_DIR)/uart/pl011.c -o $(OBJ_UART)

OBJ_TIMER = $(BUILD_DIR)/timer.o
$(OBJ_TIMER) : $(DEVICE_DIR)/timer/sp804.c $(DEVICE_DIR)/timer/sp804.h
	$(GCC) $(GCC_FLAGS) $(DEVICE_DIR)/timer/sp804.c -o $(OBJ_TIMER)

OBJ_KBD = $(BUILD_DIR)/kbd.o
$(OBJ_KBD) : $(DEVICE_DIR)/kbd/pl050.c $(DEVICE_DIR)/kbd/pl050.h
	$(GCC) $(GCC_FLAGS) $(DEVICE_DIR)/kbd/pl050.c -o $(OBJ_KBD)

OBJ_LCD = $(BUILD_DIR)/lcd.o
$(OBJ_LCD) : $(DEVICE_DIR)/lcd/pl110.c $(DEVICE_DIR)/lcd/pl110.h
	$(GCC) $(GCC_FLAGS) $(DEVICE_DIR)/lcd/pl110.c -o $(OBJ_LCD)

OBJ_FONT = \
	$(BUILD_DIR)/$(FONT_NAME).S.o \
	$(BUILD_DIR)/$(FONT_NAME).c.o
$(OBJ_FONT) : \
	$(RESOURCE_DIR)/fonts/$(FONT_NAME).S \
	$(RESOURCE_DIR)/fonts/$(FONT_NAME).c \
	$(RESOURCE_DIR)/fonts/$(FONT_NAME).h
	$(AS) $(AS_FLAGS) $(RESOURCE_DIR)/fonts/$(FONT_NAME).S -o $(BUILD_DIR)/$(FONT_NAME).S.o
	$(GCC) $(GCC_FLAGS) $(RESOURCE_DIR)/fonts/$(FONT_NAME).c -o $(BUILD_DIR)/$(FONT_NAME).c.o

OBJ_RESET = $(BUILD_DIR)/reset.o
$(OBJ_RESET) : $(KERNEL_DIR)/reset.S
	$(AS) $(AS_FLAGS) $(KERNEL_DIR)/reset.S -o $(BUILD_DIR)/reset.o

OBJ_BOARD = $(BUILD_DIR)/versatilepb.o
$(OBJ_BOARD) : \
	$(DEVICE_DIR)/board/versatilepb.c \
	$(DEVICE_DIR)/board/versatilepb.h
	$(GCC) $(GCC_FLAGS) $(DEVICE_DIR)/board/versatilepb.c -o $(OBJ_BOARD)

OBJ_DISPLAY = \
	$(BUILD_DIR)/display.o
$(OBJ_DISPLAY) : \
	$(KERNEL_DIR)/display.c
	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/display.c -o $(BUILD_DIR)/display.o

OBJ_STRING = \
	$(BUILD_DIR)/string.o
$(OBJ_STRING) : \
	$(KERNEL_DIR)/string.c
	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/string.c -o $(BUILD_DIR)/string.o

OBJ_KERNEL = \
	$(BUILD_DIR)/kernel.o
#	$(BUILD_DIR)/display.o \
#	$(BUILD_DIR)/string.o
$(OBJ_KERNEL) : \
	$(KERNEL_DIR)/kernel.c
#	$(KERNEL_DIR)/display.c \
#	$(KERNEL_DIR)/string.c
	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o
#	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/display.c -o $(BUILD_DIR)/display.o
#	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/string.c -o $(BUILD_DIR)/string.o

CLEAN :
	$(RMDIR) $(BUILD_DIR)
	$(MKDIR) $(BUILD_DIR)

MYOS: $(OBJ_RESET) $(OBJ_BOARD) $(OBJ_UART) $(OBJ_TIMER) $(OBJ_KBD) $(OBJ_LCD) $(OBJ_FONT) $(OBJ_KERNEL) $(OBJ_STRING) $(OBJ_DISPLAY) $(OBJ_BMP)
	$(LD) $(OBJ_RESET) $(OBJ_BOARD) $(OBJ_UART) $(OBJ_TIMER) $(OBJ_KBD) $(OBJ_LCD) $(OBJ_FONT) $(OBJ_KERNEL) $(OBJ_STRING) $(OBJ_DISPLAY) $(OBJ_BMP) $(WORKSPACE)/$(LIBS) $(LD_FLAGS) -o $(BUILD_DIR)/myos.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(BUILD_DIR)/myos.elf $(BUILD_DIR)/myos.bin

KERNEL_BIN = $(BUILD_DIR)/myos.bin
QEMU_CMD = qemu-system-arm -s -M versatilepb -cpu arm926 -serial telnet:127.0.0.1:1122,server -kernel $(KERNEL_BIN)

DEBUG: MYOS
	$(QEMU_CMD)


# C5_2 start - Simple demo of multi-tasking
C5_2_BLD_DIR = $(BUILD_DIR)/C5_2
C5_2_SRC_DIR = $(KERNEL_DIR)/C5_2

CREATE_BLD_DIR_C_5_2:
	mkdir -p $(C5_2_BLD_DIR)

OBJ_RESET_C5_2 = $(C5_2_BLD_DIR)/reset.o
$(OBJ_RESET_C5_2) : CREATE_BLD_DIR_C_5_2 $(C5_2_SRC_DIR)/reset.S
	$(AS) $(AS_FLAGS) $(C5_2_SRC_DIR)/reset.S -o $(C5_2_BLD_DIR)/reset.o

OBJ_KERNEL_C5_2 = $(C5_2_BLD_DIR)/kernel.o
$(OBJ_KERNEL_C5_2) : CREATE_BLD_DIR_C_5_2 $(C5_2_SRC_DIR)/kernel.c
	$(GCC) $(GCC_FLAGS) $(C5_2_SRC_DIR)/kernel.c -o $(C5_2_BLD_DIR)/kernel.o

MYOS_C5_2: $(OBJ_RESET_C5_2) $(OBJ_BOARD) $(OBJ_UART) $(OBJ_TIMER) $(OBJ_KBD) $(OBJ_LCD) $(OBJ_FONT) $(OBJ_KERNEL_C5_2) $(OBJ_STRING) $(OBJ_DISPLAY)
	$(LD) $(OBJ_RESET_C5_2) $(OBJ_BOARD) $(OBJ_UART) $(OBJ_TIMER) $(OBJ_KBD) $(OBJ_LCD) $(OBJ_FONT) $(OBJ_KERNEL_C5_2) $(OBJ_STRING) $(OBJ_DISPLAY) $(WORKSPACE)/$(LIBS) $(LD_FLAGS) -o $(BUILD_DIR)/MYOS_C5_2.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(BUILD_DIR)/MYOS_C5_2.elf $(BUILD_DIR)/MYOS_C5_2.bin

KERNEL_BIN_C5_2= $(BUILD_DIR)/MYOS_C5_2.bin
QEMU_CMD_C5_2 = qemu-system-arm -s -M versatilepb -cpu arm926 -serial telnet:127.0.0.1:1122,server -kernel $(KERNEL_BIN_C5_2)

DEBUG_C5_2: MYOS_C5_2
	$(QEMU_CMD_C5_2)

# C5_2 end


# C5_3 start - Dynamic process creation
C5_3_BLD_DIR = $(BUILD_DIR)/C5_3
C5_3_SRC_DIR = $(KERNEL_DIR)/C5_3

CREATE_BLD_DIR_C_5_3:
	mkdir -p $(C5_3_BLD_DIR)

# C5_3 end