TOOL_CHAIN_DIR_0 = /home/ming/dev/toolchains/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi
TOOL_CHAIN_DIR_1 = $(TOOL_CHAIN_DIR_0)/lib/gcc/arm-none-eabi/14.2.1
TOOL_CHAIN_DIR_2 = $(TOOL_CHAIN_DIR_0)/arm-none-eabi
QEMU_DIR = /usr/local/bin
QEMU_ARM = $(QEMU_DIR)/qemu-system-arm
QEMU_AARCH64 = $(QEMU_DIR)/qemu-system-aarch64

# Run "qemu-system-arm -M help | grep M4" to find the boards that support Cortex-M4
QEMU_BOARD_NAME = ast1030-evb

ARM_TOOLCHAIN_PREFIX=$(TOOL_CHAIN_DIR_0)/bin/arm-none-eabi-
AS = $(ARM_TOOLCHAIN_PREFIX)as
GCC = $(ARM_TOOLCHAIN_PREFIX)gcc
LD = $(ARM_TOOLCHAIN_PREFIX)ld
OBJCOPY = $(ARM_TOOLCHAIN_PREFIX)objcopy

RMDIR = rm -r -f
MKDIR = mkdir -p

MCU = m4
DEVICE_FLAGS =
WORKSPACE = .
BUILD_DIR = $(WORKSPACE)/build
CMSIS_DIR = $(WORKSPACE)/cmsis
RESET_DIR = $(WORKSPACE)/reset
PERIPHERAL_DIR = $(WORKSPACE)/peripheral
KERNEL_DIR = $(WORKSPACE)/kernel
LD_SCRIPT_DIR = $(WORKSPACE)/linker
LD_SCRIPT = $(LD_SCRIPT_DIR)/$(QEMU_BOARD_NAME).ld


INCLUDES = \
	-I $(CMSIS_DIR) \
    -I $(RESET_DIR) \
    -I $(PERIPHERAL_DIR) \
	-I $(TOOL_CHAIN_DIR_1)/include \
	-I $(TOOL_CHAIN_DIR_2)/include

LIBS = libs/libgcc.a
AS_FLAGS = -mcpu=cortex-m4
GCC_FLAGS = -g -c -nostdinc $(INCLUDES) -Werror $(DEVICE_FLAGS) -mcpu=cortex-m4 -specs=nosys.specs -masm-syntax-unified
GCC_FLAGS_STARTUP = $(GCC_FLAGS) -D__STARTUP_CONFIG
LD_FLAGS = -T $(LD_SCRIPT) -nostdlib
OBJCOPY_FLAGS = -O binary

## start up mcu
START_UP = startup_$(MCU)_simple
# the suffix must be "S" rather than "s" if the file contains preprocessor directives.
# ref: https://stackoverflow.com/a/51110745/264052
START_UP_SRC_SUFFIX = S
START_UP_SRC = $(START_UP).$(START_UP_SRC_SUFFIX)
OBJ_STARTUP_MCU = $(BUILD_DIR)/$(START_UP).o
$(OBJ_STARTUP_MCU) : $(RESET_DIR)/$(START_UP_SRC)
	echo "build startup assembly"
	# if the startup.S contains preprocessor directive like #include.
	# we have to use gcc rather than as directly.
	# $(AS) $(AS_FLAGS) $(RESET_DIR)/$(START_UP_SRC) -o $(OBJ_STARTUP_MCU)
	$(GCC) $(GCC_FLAGS_STARTUP) $(RESET_DIR)/$(START_UP_SRC) -o $(OBJ_STARTUP_MCU)

## kernel
OBJ_KERNEL_MAIN = $(BUILD_DIR)/main.o
$(OBJ_KERNEL_MAIN) : $(KERNEL_DIR)/main.c
	echo "build kernel main"
	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/main.c -o $(OBJ_KERNEL_MAIN)



KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL: $(OBJ_STARTUP_MCU) $(OBJ_KERNEL_MAIN)
	$(LD) $(OBJ_STARTUP_MCU) $(OBJ_KERNEL_MAIN) $(LD_FLAGS) -o $(KERNEL_ELF)
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(KERNEL_ELF) $(KERNEL_BIN)



# "-kernel" is the linux kernel loader
# "-device loader,file=" is the generic loader, which is good for a completely bare-metal image which includes
# the exception vector table and want to have it start in the same way the hardware would out of reset
# Though KC Wang's Embedded and RTOS book use the -kernel, I think it is not a good choice for bare metal.
# refs:
# https://stackoverflow.com/questions/79616268/where-does-qemu-load-the-kernel-image-with-the-kernel-option
# https://stackoverflow.com/questions/58420670/qemu-bios-vs-kernel-vs-device-loader-file
# 
# If you see "Cannot load specified image... exceeds maximum image size" error, add the "-m" to specify the memory. Some boards need it.
# 
QEMU_CMD_DEBUG = $(QEMU_ARM) -s -S -M $(QEMU_BOARD_NAME) -cpu cortex-m4 -m 2048 -serial telnet:127.0.0.1:1124,server -device loader,file=$(KERNEL_BIN)
QEMU_CMD_RUN = $(QEMU_ARM) -s -M $(QEMU_BOARD_NAME) -cpu cortex-m4 -m 2048 -serial telnet:127.0.0.1:1124,server -device loader,file=$(KERNEL_BIN)
# QEMU_CMD_DEBUG = $(QEMU_ARM) -s -S -M $(QEMU_BOARD_NAME) -cpu cortex-m4 -serial telnet:127.0.0.1:1124,server -kernel $(KERNEL_BIN)
# QEMU_CMD_RUN = $(QEMU_ARM) -s -M $(QEMU_BOARD_NAME) -cpu cortex-m4 -serial telnet:127.0.0.1:1124,server -kernel $(KERNEL_BIN)



CREATE_BLD_DIR:
	$(MKDIR) $(BUILD_DIR)
CLEAN :
	$(RMDIR) $(BUILD_DIR)
BUILD :  CREATE_BLD_DIR KERNEL
REBUILD : CLEAN BUILD

DEBUG : BUILD
	$(QEMU_CMD_DEBUG)
RUN : BUILD
	$(QEMU_CMD_RUN)
