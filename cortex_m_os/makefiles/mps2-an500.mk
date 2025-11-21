TOOL_CHAIN_DIR_0 = /home/ming/dev/toolchains/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi
TOOL_CHAIN_DIR_1 = $(TOOL_CHAIN_DIR_0)/lib/gcc/arm-none-eabi/14.3.1
TOOL_CHAIN_DIR_2 = $(TOOL_CHAIN_DIR_0)/arm-none-eabi
QEMU_DIR = /usr/local/bin
QEMU_ARM = $(QEMU_DIR)/qemu-system-arm
QEMU_AARCH64 = $(QEMU_DIR)/qemu-system-aarch64

# Run "qemu-system-arm -M help | grep M7" to find the boards that support Cortex-M7
# mps2 doc: https://www.qemu.org/docs/master/system/arm/mps2.html
QEMU_BOARD_NAME = mps2-an500

ARM_TOOLCHAIN_PREFIX=$(TOOL_CHAIN_DIR_0)/bin/arm-none-eabi-
AS = $(ARM_TOOLCHAIN_PREFIX)as
GCC = $(ARM_TOOLCHAIN_PREFIX)gcc
LD = $(ARM_TOOLCHAIN_PREFIX)ld
OBJCOPY = $(ARM_TOOLCHAIN_PREFIX)objcopy

RMDIR = rm -r -f
MKDIR = mkdir -p

CMSIS_MCU = cm7
QEMU_CPU = cortex-m7
DEVICE_FLAGS =


USART_MODEL = apb_uart


WORKSPACE = .
BUILD_DIR = $(WORKSPACE)/build
CMSIS_DIR = $(WORKSPACE)/cmsis
CMSIS_INCLUDE_DIR = $(CMSIS_DIR)/includes
RESET_DIR = $(WORKSPACE)/reset

PERIPHERAL_DIR = $(WORKSPACE)/peripherals
PERIPHERAL_USART_DIR = $(PERIPHERAL_DIR)/usart
PERIPHERAL_USART_MODEL_DIR = $(PERIPHERAL_USART_DIR)/$(USART_MODEL)

KERNEL_DIR = $(WORKSPACE)/kernel/$(CMSIS_MCU)
LD_SCRIPT_DIR = $(WORKSPACE)/linker
LD_SCRIPT = $(LD_SCRIPT_DIR)/$(QEMU_BOARD_NAME).ld


INCLUDES = \
    -I $(CMSIS_DIR) \
    -I $(RESET_DIR) \
    -I $(CMSIS_INCLUDE_DIR)/$(CMSIS_MCU) \
    -I $(PERIPHERAL_DIR)/include \
    -I $(TOOL_CHAIN_DIR_1)/include \
    -I $(TOOL_CHAIN_DIR_2)/include

LIBS = libs/libgcc.a
AS_FLAGS = -mcpu=$(QEMU_CPU)
GCC_FLAGS = -g -c -nostdinc $(INCLUDES) -Werror $(DEVICE_FLAGS) -mcpu=$(QEMU_CPU) -specs=nosys.specs
GCC_FLAGS_STARTUP = $(GCC_FLAGS) -D__STARTUP_CONFIG
LD_FLAGS = -T $(LD_SCRIPT) -nostdlib
OBJCOPY_FLAGS = -O binary

## start up mcu
START_UP = startup_$(CMSIS_MCU)_simple
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

OBJ_KERNEL_SYSTEMINIT = $(BUILD_DIR)/SystemInit.o
$(OBJ_KERNEL_SYSTEMINIT) : $(KERNEL_DIR)/SystemInit.c
	echo "build kernel SystemInit"
	$(GCC) $(GCC_FLAGS) $(KERNEL_DIR)/SystemInit.c -o $(OBJ_KERNEL_SYSTEMINIT)

## usart
OBJ_USART = $(BUILD_DIR)/usart.o
$(OBJ_USART) : $(PERIPHERAL_USART_MODEL_DIR)/impl.c
	echo "build usart driver"
	$(GCC) $(GCC_FLAGS) $(PERIPHERAL_USART_MODEL_DIR)/impl.c -o $(OBJ_USART)



OS_ELF = $(BUILD_DIR)/os.elf
OS_BIN = $(BUILD_DIR)/os.bin
OS_DPENDENCIES = $(OBJ_STARTUP_MCU) $(OBJ_KERNEL_MAIN) $(OBJ_KERNEL_SYSTEMINIT) $(OBJ_USART)
OS_IMG: $(OS_DPENDENCIES)
	$(LD) $(OS_DPENDENCIES) $(LD_FLAGS) -o $(OS_ELF)
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(OS_ELF) $(OS_BIN)



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
QEMU_CMD_DEBUG = $(QEMU_ARM) -s -S -M $(QEMU_BOARD_NAME) -cpu $(QEMU_CPU) -m 16 -serial telnet:127.0.0.1:1124,server -device loader,file=$(OS_BIN)
QEMU_CMD_RUN = $(QEMU_ARM) -s -M $(QEMU_BOARD_NAME) -cpu $(QEMU_CPU) -m 2048 -serial telnet:127.0.0.1:1124,server -device loader,file=$(OS_BIN)
# QEMU_CMD_DEBUG = $(QEMU_ARM) -s -S -M $(QEMU_BOARD_NAME) -cpu $(QEMU_CPU) -serial telnet:127.0.0.1:1124,server -kernel $(OS_BIN)
# QEMU_CMD_RUN = $(QEMU_ARM) -s -M $(QEMU_BOARD_NAME) -cpu $(QEMU_CPU) -serial telnet:127.0.0.1:1124,server -kernel $(OS_BIN)



CREATE_BLD_DIR:
	$(MKDIR) $(BUILD_DIR)
CLEAN :
	$(RMDIR) $(BUILD_DIR)
BUILD :  CREATE_BLD_DIR OS_IMG
REBUILD : CLEAN BUILD

DEBUG : BUILD
	$(QEMU_CMD_DEBUG)
RUN : BUILD
	$(QEMU_CMD_RUN)
