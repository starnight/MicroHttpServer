# Name of the binaries.
PROJ_NAME=FreeRTOS-STM32F4-UART

######################################################################
#                         SETUP SOURCES                              #
######################################################################


# This is the directory containing the firmware package,
# the unzipped folder downloaded from here:
# http://www.st.com/web/en/catalog/tools/PF257904
STM_DIR=../../STM32F4xx_DSP_StdPeriph_Lib_V1.8.0

# This is where the source files are located,
# which are not in the current directory
# (the sources of the standard peripheral library, which we use)
# see also "info:/make/Selective Search" in Konqueror
STM_SRC = $(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src

# This is the directory containing FreeRTOS
FREERTOS_DIR=../../FreeRTOSv10.0.1/FreeRTOS

# These are where the FreeRTOS files located
FREERTOS_SRC=$(FREERTOS_DIR)/Source
FREERTOS_PORT=$(FREERTOS_SRC)/portable
FREERTOS_PORT_PLAT=$(FREERTOS_PORT)/GCC/ARM_CM4F

# Tell make to look in that folder if it cannot find a source
# in the current directory
vpath %.c $(STM_SRC)

# My source file
SRCS   = src/main.c
SRCS  += src/gpio.c
SRCS  += src/usart.c
SRCS  += src/socket.c
SRCS  += src/select.c
SRCS  += src/mac_esp8266.c
SRCS  += lib/server.c
SRCS  += lib/middleware.c
SRCS  += src/app.c

# Contains initialisation code and must be compiled into
# our project. This file is in the current directory and
# was writen by ST.
SRCS  += src/system_stm32f4xx.c

# These source files implement the functions we use.
# make finds them by searching the vpath defined above.
SRCS  += stm32f4xx_rcc.c 
SRCS  += stm32f4xx_gpio.c
SRCS  += stm32f4xx_usart.c

# Startup file written by ST
# The assembly code in this file is the first one to be
# executed. Normally you do not change this file.
SRCS += $(STM_DIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f40_41xxx.s

# The source files of FreeRTOS
SRCS +=$(FREERTOS_SRC)/*.c
SRCS +=$(FREERTOS_PORT)/MemMang/heap_1.c
SRCS +=$(FREERTOS_PORT_PLAT)/port.c

# The header files we use are located here
#INC_DIRS  = $(STM_DIR)/Utilities/STM32F4-Discovery
INC_DIRS  = $(STM_DIR)/Libraries/CMSIS/Include
INC_DIRS += $(STM_DIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Include
INC_DIRS += $(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/inc
INC_DIRS += lib
INC_DIRS += include

# The header files of FreeRTOS
INC_DIRS += $(FREERTOS_SRC)/include
INC_DIRS += $(FREERTOS_PORT_PLAT)

# in case we have to many sources and don't want 
# to compile all sources every time
# OBJS = $(SRCS:.c=.o)

######################################################################
#                         SETUP TOOLS                                #
######################################################################


# This is the path to the toolchain
# (we don't put our toolchain on $PATH to keep the system clean)
#TOOLS_DIR = /opt/gcc-arm-embedded/gcc-arm-none-eabi-4_7-2013q1/bin

# The tool we use
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
GDB     = arm-none-eabi-gdb

## Preprocessor options

# directories to be searched for header files
INCLUDE = $(addprefix -I,$(INC_DIRS))

# #defines needed when working with the STM library
DEFS    = -DUSE_STDPERIPH_DRIVER -DSTM32F40_41xxx
# if you use the following option, you must implement the function 
#    assert_failed(uint8_t* file, uint32_t line)
# because it is conditionally used in the library
# DEFS   += -DUSE_FULL_ASSERT
DEFS += -DMAX_HTTP_CLIENT=5

## Compiler options
#CFLAGS  = -ggdb
# please do not optimize anything because we are debugging
CFLAGS += -Os
CFLAGS += -Wextra -Warray-bounds
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

## Linker options
# tell ld which linker file to use
# (this file is in the current directory)
LFLAGS  = -Tstm32_flash.ld -specs=nosys.specs #--specs=rdimon.specs


######################################################################
#                         SETUP TARGETS                              #
######################################################################

.PHONY: $(PROJ_NAME)
$(PROJ_NAME): $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(INCLUDE) $(DEFS) $(CFLAGS) $(LFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf   $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin

# Flash the STM32F4
flash: 
	st-flash write $(PROJ_NAME).bin 0x8000000

.PHONY: debug
debug:
# before you start gdb, you must start st-util
	$(GDB) $(PROJ_NAME).elf
