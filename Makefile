# ------------------------------------------------------------------------------------------- #
# Board

# ST NUCLEO F401RE board
#BOARD		:= NUCLEO_F401RE
#FPU		:= hard
#HSE_VALUE	:= 0

# STM32F4-Discovery board
#BOARD		:= STM32F4_DISCOVERY
#FPU		:= hard
#HSE_VALUE	:= 8000000

# Made by mySelf STM32F103C8 board
BOARD		:= STM32F1_MINIMAL
FPU			:= 
HSE_VALUE	:= 8000000

# ------------------------------------------------------------------------------------------- #
# MCU

# ------------------------------------------------------ #
ifeq ($(BOARD),STM32F1_MINIMAL)
CPU			:= STM32F1
MCPU		:= -mcpu=cortex-m3 -mfloat-abi=soft
LDSCRIPT	:= -T "stm32f103x8.ld"
OOCD_IF		?= cmsis_dap
# ------------------------------------------------------ #
else ifeq ($(BOARD),NUCLEO_F401RE)
CPU			:= STM32F4
MCPU		:= -mcpu=cortex-m4 -mfloat-abi=$(FPU)
ifeq ($(FPU),hard)
MCPU		+= -mfpu=fpv4-sp-d16
endif
LDSCRIPT	+= -T "stm32f401xe.ld"
OOCD_IF		?= v2.1
# ------------------------------------------------------ #
else ifeq ($(BOARD),STM32F4_DISCOVERY)
CPU			:= STM32F4
MCPU		:= -mcpu=cortex-m4 -mfloat-abi=$(FPU)
ifeq ($(FPU),hard)
MCPU		+= -mfpu=fpv4-sp-d16
endif
LDSCRIPT	+= -T "stm32f407xg.ld"
OOCD_IF		?= v2
# ------------------------------------------------------ #

endif

cpu			:= $(shell echo $(CPU) | tr A-Z a-z)

# ------------------------------------------------------------------------------------------- #
# Programm name
PROG_NAME	:= stm32_cmsis_dap

# where built files will be stored
TARGET_DIR	:= build
TARGET		:= $(PROG_NAME).elf

# Libopencm3 directory
OPENCM3_DIR	:= libopencm3

# CMSIS-DAP reference implementation
OBJS		+= $(patsubst %.c, %.o, $(shell find -L cmsis_dap -type f -name "*.c"))

# Main programm sources
OBJS		+= $(patsubst %.c, %.o, $(shell find -L src -type f -name "*.c"))

# Optimization / debug flags
OPT			:=  -O3 #-Og -g3

# Common C and Linker flags
FLAGS		:= $(MCPU) $(OPT)
FLAGS		+= -mthumb -fmessage-length=0 -fsigned-char 
FLAGS		+= -ffunction-sections -fdata-sections -ffreestanding 
FLAGS		+= -fno-common -fno-move-loop-invariants -Wall -Wextra

# C compiler flags
CFLAGS		:= $(FLAGS) -D$(CPU) -DHSE_VALUE=$(HSE_VALUE) -D$(BOARD) #-DUSE_DEBUG

# Linker flags
LDFLAGS		:= $(FLAGS)
LDFLAGS		+= --static -nostartfiles -Wl,--gc-sections
LDFLAGS 	+= -L "ldscripts/" $(LDSCRIPT) -Wl,-Map,"$(TARGET_DIR)/$(PROG_NAME).map"
LDFLAGS		+= --specs=nano.specs


# Include directories
INCLUDE		:= $(OPENCM3_DIR)/include cmsis_dap src

# External libraries
LIBS		:= -L$(OPENCM3_DIR)/lib -lopencm3_$(cpu) -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

# Cross-compile tools
PREFIX		:= arm-none-eabi-
CC			:= $(PREFIX)gcc
LD			:= $(PREFIX)gcc


all: $(TARGET)	
	# Get disassembled
	$(PREFIX)objdump -d --wide $(TARGET_DIR)/$(TARGET) > $(TARGET_DIR)/$(PROG_NAME).asm
	# ===========================================================================
	# Memory usage
	# ===========================================================================
	@$(PREFIX)size -A -x $(TARGET_DIR)/$(TARGET)

# Get binary copy
bin: $(TARGET)
	$(PREFIX)objcopy -O binary $(TARGET_DIR)/$(TARGET) $(TARGET_DIR)/$(PROG_NAME).bin
	
# Upload firmware to MCU	
pgm: $(TARGET)
	openocd -f $(OOCD_IF).cfg -c "source [find target/$(cpu)x.cfg]" -c "program $(TARGET_DIR)/$(TARGET) verify reset exit"
	
# Start OpenOCD server	
debug: $(TARGET)
	openocd -f $(OOCD_IF).cfg -c "source [find target/$(cpu)x.cfg]"
		
	
$(TARGET): $(OBJS)
	@$(LD) $(LDFLAGS) -o $(TARGET_DIR)/$@ $^ $(LIBS)

%.o : %.c
	@$(CC) $(CFLAGS) $(addprefix -I, $(INCLUDE)) -o $@ -c $<
		
clean:
	@echo "Cleaning..."
	@rm -rf $(OBJS)
	@rm -rf $(TARGET_DIR)/$(TARGET)
	@rm -rf $(TARGET_DIR)/$(PROG_NAME).map
	@rm -rf $(TARGET_DIR)/$(PROG_NAME).bin
	@rm -rf $(TARGET_DIR)/$(PROG_NAME).asm	
	
.PHONY:
	all clean pgm debug
