#
# Utilities
#
RMDIR = rm -r -f
MKDIR = mkdir -p
CLANG_FORMAT = clang-format-15

#
# Board config
#
# Run "qemu-system-arm -M help | grep M7" to find the boards that support Cortex-M7
# mps2 doc: https://www.qemu.org/docs/master/system/arm/mps2.html
QEMU_BOARD_NAME = mps2-an500
QEMU_CPU = cortex-m7
# https://stackoverflow.com/a/53720371/264052
QEMU_CPU_ATTRIBUTES = +nofp
CMSIS_MCU = cm7
DEVICE_FLAGS =

#
# Peripherals, including on-chip peripherals and off-chip/external peripherals
#
# This is the UART model name mentioned in the DDI0479C_cortex_m_system_design_kit_r1p0_trm.pdf.
UART_MODEL = cmsdk_apb_uart


#
# Build config
#
WORKSPACE = .
BOARDS_DIR = $(WORKSPACE)/boards
BOARD_DIR = $(BOARDS_DIR)/$(QEMU_BOARD_NAME)
BUILD_DIR = $(WORKSPACE)/build
HAL_DIR = $(WORKSPACE)/hal
CMSIS_DIR = $(HAL_DIR)/cmsis
DRIVERS_DIR = $(WORKSPACE)/drivers
# RESET_DIR = $(WORKSPACE)/reset


#
#  Toolchain
#
TOOL_CHAIN_DIR_0 = /home/ming/dev/toolchains/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi
TOOL_CHAIN_DIR_1 = $(TOOL_CHAIN_DIR_0)/lib/gcc/arm-none-eabi/14.3.1
TOOL_CHAIN_DIR_2 = $(TOOL_CHAIN_DIR_0)/arm-none-eabi

QEMU_DIR = /usr/local/bin
QEMU_ARM = $(QEMU_DIR)/qemu-system-arm
QEMU_AARCH64 = $(QEMU_DIR)/qemu-system-aarch64

ARM_TOOLCHAIN_PREFIX=$(TOOL_CHAIN_DIR_0)/bin/arm-none-eabi-
AS = $(ARM_TOOLCHAIN_PREFIX)as
CC = $(ARM_TOOLCHAIN_PREFIX)gcc
LD = $(ARM_TOOLCHAIN_PREFIX)ld
OBJCOPY = $(ARM_TOOLCHAIN_PREFIX)objcopy

LIBS = libs/libgcc.a
AS_FLAGS = -mcpu=$(QEMU_CPU)$(QEMU_CPU_ATTRIBUTES) -g
GCC_FLAGS = -g -c -nostdinc $(INCLUDES) -Werror $(DEVICE_FLAGS) -mcpu=$(QEMU_CPU)$(QEMU_CPU_ATTRIBUTES) -specs=nosys.specs -O0
GCC_FLAGS_STARTUP = $(GCC_FLAGS) -D__STARTUP_CONFIG
LD_FLAGS = -T $(LD_SCRIPT) -nostdlib
OBJCOPY_FLAGS = -O binary


INCLUDES = \
    -I $(BOARDS_DIR)/include \
    -I $(BOARD_DIR)/include \
    -I $(CMSIS_DIR)/include/ \
    -I $(DRIVERS_DIR)/include \
    -I $(TOOL_CHAIN_DIR_1)/include \
    -I $(TOOL_CHAIN_DIR_2)/include


LD_SCRIPT = $(BOARD_DIR)/$(QEMU_BOARD_NAME).ld


#
# board
#

# start up mcu
START_UP = startup
# the suffix must be "S" rather than "s" if the file contains preprocessor directives.
# ref: https://stackoverflow.com/a/51110745/264052
SRC_START_UP = $(BOARD_DIR)/core/$(START_UP).S
OBJ_START_UP = $(BUILD_DIR)/$(START_UP).o
$(OBJ_START_UP) : $(SRC_START_UP)
	echo "build startup assembly"
	# if the startup.S contains preprocessor directive like #include.
	# we have to use gcc rather than as directly.
	# $(AS) $(AS_FLAGS) $(SRC_START_UP) -o $(OBJ_START_UP)
	$(CC) $(GCC_FLAGS_STARTUP) $(SRC_START_UP) -o $(OBJ_START_UP)

# system init
SRC_SYSTEM_INIT = $(BOARD_DIR)/core/SystemInit.c
OBJ_SYSTEMINIT = $(BUILD_DIR)/SystemInit.o
$(OBJ_SYSTEMINIT) : $(SRC_SYSTEM_INIT)
	echo "build kernel SystemInit"
	$(CC) $(GCC_FLAGS) $(SRC_SYSTEM_INIT) -o $(OBJ_SYSTEMINIT)

# kernel
KERNEL_DIR = $(WORKSPACE)/kernel/
SRC_KERNEL = $(KERNEL_DIR)/main.c
OBJ_KERNEL = $(BUILD_DIR)/kernel.o
$(OBJ_KERNEL) : $(SRC_KERNEL)
	echo "build kernel main"
	$(CC) $(GCC_FLAGS) $(SRC_KERNEL) -o $(OBJ_KERNEL)


# drivers
SRC_UART_DRIVER = $(DRIVERS_DIR)/uart/$(UART_MODEL)/$(UART_MODEL).c
OBJ_UART_DRIVER = $(BUILD_DIR)/$(UART_MODEL).o
$(OBJ_UART_DRIVER) : $(SRC_UART_DRIVER)
	echo "build uart driver"
	$(CC) $(GCC_FLAGS) $(SRC_UART_DRIVER) -o $(OBJ_UART_DRIVER)


# peripherals
SRC_UARTS = $(BOARD_DIR)/peripherals/uarts.c
OBJ_UARTS = $(BUILD_DIR)/uarts.o
$(OBJ_UARTS) : $(SRC_UARTS)
	echo "build board uarts"
	$(CC) $(GCC_FLAGS) $(SRC_UARTS) -o $(OBJ_UARTS)

#
# OS image
#
OS_ELF = $(BUILD_DIR)/os.elf
OS_BIN = $(BUILD_DIR)/os.bin
OS_DPENDENCIES = $(OBJ_START_UP) $(OBJ_SYSTEMINIT) $(OBJ_KERNEL) $(OBJ_UART_DRIVER) $(OBJ_UARTS)
OS_IMG: $(OS_DPENDENCIES)
	$(LD) $(OS_DPENDENCIES) $(LD_FLAGS) -o $(OS_ELF)
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(OS_ELF) $(OS_BIN)




#
# QEMU run
#
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


#
# major targets
#
CREATE_BLD_DIR:
	$(MKDIR) $(BUILD_DIR)
CLEAN :
	$(RMDIR) $(BUILD_DIR)
BUILD :  CREATE_BLD_DIR OS_IMG
REBUILD : CLEAN BUILD

DEBUG : REBUILD
	$(QEMU_CMD_DEBUG)
RUN : REBUILD
	$(QEMU_CMD_RUN)

FORMAT:
	@find . -name "*.c" | xargs $(CLANG_FORMAT) -i --style=file:.clang-format
