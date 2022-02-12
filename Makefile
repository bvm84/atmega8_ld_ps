# Задаем имя проекта, в результате получатся файлы test.hex test.bin
# http://ccrma.stanford.edu/planetccrma/man/man1/avr-gcc.1.html
F_CPU = 2000000
MCU = atmega8
TARG = main
CC = C:/code/avr-gcc-11.1.0-x64-windows/bin/avr-gcc.exe
SIZE = C:/code/avr-gcc-11.1.0-x64-windows/bin/avr-size.exe
OBJCOPY = C:/code/avr-gcc-11.1.0-x64-windows/bin/avr-objcopy.exe
OBJDUMP = C:/code/avr-gcc-11.1.0-x64-windows/bin/avr-objdump.exe
CSTANDARD = -std=gnu99
SRC_DIR := src
OBJ_DIR := output
EXTRAINCDIRS = include
#DUDE = C:/code/avr-gcc-11.1.0-x64-windows/avrdude64/avrdude.exe
DUDE = C:/code/avr-gcc-11.1.0-x64-windows/avrdude-v6.3.1.1-windows/avrdude.exe
PROG = usbasp
PORT = usb
#PROG = ft232r_brendel
#PORT = ft0
#RATE = 57200
#https://www.engbedded.com/fusecalc/
# Задаем из каких файлов собирать проект, можно указать несколько файлов
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
 
# Флаги компилятора, при помощи F_CPU определяем частоту на которой будет работать контроллер,
# CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Wall -g -Os -lm -mcall-prologues
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -g -Os -lm -Wall -Wno-multichar -Wno-switch-outside-range -ffunction-sections -fdata-sections
# Pass options to assembly, generate listing files for debug
# CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJ_DIR)/%.lst)
# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
# CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)
#Pass options to linker
CFLAGS += -Wl,--gc-sections
# CFLAGS += -Wl,-u,vfprintf -lprintf_min
# CFLAGS += -finput-charset=utf-8 -fexec-charset=iso-8859-15
#Preprocessor iptions
CPPFLAGS := $(patsubst %,-I%,$(EXTRAINCDIRS)) -MMD -MP

test:
	@echo $(OBJS)

all: gccversion $(TARG) size
 
$(TARG): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(OBJ_DIR)/$@.elf $(OBJS)
	$(OBJCOPY) -O binary -R .eeprom -R .nwram  $(OBJ_DIR)/$@.elf $(OBJ_DIR)/$@.bin
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $(OBJ_DIR)/$@.elf $(OBJ_DIR)/$@.hex
	$(OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 $(OBJ_DIR)/$@.elf $(OBJ_DIR)/$@.eep
	$(OBJDUMP) -h -S $(OBJ_DIR)/$@.elf > $(OBJ_DIR)/$@.lst

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*
	
gccversion: 
	@$(CC) --version

HEXSIZE = $(SIZE) $(OBJ_DIR)/$(TARG).hex
EEPSIZE = $(SIZE) $(OBJ_DIR)/$(TARG).eep
size:
	@$(HEXSIZE)
	@$(EEPSIZE)

flash:
#	$(DUDE) -c $(PROG) -B0.666 -p $(MCU) -U flash:w:$(OBJ_DIR)/$(TARG).hex -U eeprom:w:$(OBJ_DIR)/$(TARG).eep -U lfuse:w:0xe1:m -U hfuse:w:0xd9:m
	$(DUDE) -c $(PROG) -P $(PORT) -p $(MCU) -U flash:w:$(OBJ_DIR)/$(TARG).hex -U eeprom:w:$(OBJ_DIR)/$(TARG).eep -U lfuse:w:0xc2:m -U hfuse:w:0xd9:m
#   $(DUDE) -c $(PROG) -P $(PORT) -B0.666 -p $(MCU) -U flash:w:$(OBJ_DIR)/$(TARG).hex -U eeprom:w:$(OBJ_DIR)/$(TARG).eep -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m
#	$(DUDE) -c $(PROG) -P $(PORT) -B0.666 -p $(MCU) -U flash:w:$(OBJ_DIR)/$(TARG).hex -U eeprom:w:$(OBJ_DIR)/$(TARG).eep -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m
#avrdude -c ft232r -B0.666 -p atmega8 - -U flash:w:$(OBJ_DIR)/$(TARG).hex -U eeprom:w:$(OBJ_DIR)/$(TARG).eep -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m

.PHONY: all test clean flash