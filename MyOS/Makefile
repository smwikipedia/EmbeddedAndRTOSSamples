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
    -I $(DEVICE_DIR)/vic

LIBS = libs/libgcc.a
AS_FLAGS = -mcpu=arm926ej-s
GCC_FLAGS = -mcpu=arm926ej-s -marm -g -c $(INCLUDES) 
LD_FLAGS = -T $(WORKSPACE)/myos.ld
OBJCOPY_FLAGS = -O binary

OBJ_BMP = \
	$(BUILD_DIR)/image.o
$(OBJ_BMP) : $(RESOURCE_DIR)/images/image.bmp
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm $(RESOURCE_DIR)/images/image.bmp $(BUILD_DIR)/image.o

OBJ_UART = $(BUILD_DIR)/uart.o
$(OBJ_UART) : $(DEVICE_DIR)/uart/pl011.c $(DEVICE_DIR)/uart/pl011.h
	$(GCC) $(GCC_FLAGS) $(DEVICE_DIR)/uart/pl011.c -o $(OBJ_UART)

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

OBJ_KERNEL = \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/display.o
$(OBJ_KERNEL) : \
	$(KERNEL_DIR)/kernel.c \
	$(KERNEL_DIR)/display.c
	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o
	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/display.c -o $(BUILD_DIR)/display.o

CLEAN :
	$(RMDIR) $(BUILD_DIR)
	$(MKDIR) $(BUILD_DIR)

MYOS: $(OBJ_RESET) $(OBJ_UART) $(OBJ_FONT) $(OBJ_KERNEL) $(OBJ_BMP)
	$(LD) $(OBJ_RESET) $(OBJ_UART) $(OBJ_FONT) $(OBJ_KERNEL) $(OBJ_BMP) $(WORKSPACE)/$(LIBS) $(LD_FLAGS) -o $(BUILD_DIR)/myos.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(BUILD_DIR)/myos.elf $(BUILD_DIR)/myos.bin

QEMU_CMD = qemu-system-arm -s -M versatilepb -cpu arm926 -serial telnet:localhost:1122,server -kernel

DEBUG: MYOS
	$(QEMU_CMD) $(BUILD_DIR)/myos.bin