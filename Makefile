#
# Copyright (c) 2006-2011 Luc HONDAREYTE
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or
# without modification, are permitted provided that the
# following conditions are met:
# 1. Redistributions of source code must retain the above
#    copyright notice, this list of conditions and the
#    following disclaimer.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
# CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# $Id: Makefile,v 1.3 2011/04/08 12:43:06 luc Exp luc $
#

# /etc/avrdude.conf :
#programmer
#  id    = "usbasp-clone";
#  desc  = "Any usbasp clone with correct VID/PID";
#  type  = usbasp;
#  usbvid    = 0x16C0; # VOTI
#  usbpid    = 0x05DC; # Obdev's free shared PID
#  #usbvendor  = "VOTI";
#  #usbproduct = "";
#;
# /etc/udev/rules.d/60-UsbAsp.rules
# SUBSYSTEMS=="usb", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="05dc", GROUP="users", MODE="0666"
#

FIRMWARE	:=ServoVanne
MCU		:=atmega328
#HZ		:=10000000
HZ		:=16000000U
#
# Valeurs des fusibles
#LFUSE		:=0xce
#HFUSE		:=0xdf

#
# Vous ne devriez pas avoir besoin de modifier ce qui suit
CC		:=avr-gcc
OBJCOPY		:=avr-objcopy
CFLAGS		:=-Os -D F_CPU=$(HZ)
CFLAGS		+=-g -mmcu=$(MCU) -Wall -Wstrict-prototypes
HEADERS         := $(wildcard *.h)
SOURCES         := $(wildcard *.c)
OBJECTS         := $(patsubst %.c,%.o,$(SOURCES))

all: $(FIRMWARE).hex

coffee: all load verify wfuse mrproper
	@echo "All is nice. Have a good day."

$(FIRMWARE).hex: $(FIRMWARE).elf
	@printf "Generating $(FIRMWARE).hex..."
	@$(OBJCOPY) -R .eeprom -O ihex $(FIRMWARE).elf  \
		$(FIRMWARE).hex
	@echo "done."

$(FIRMWARE).elf: $(OBJECTS)
	@printf "Linking    $(FIRMWARE)..."
	@$(CC) $(CFLAGS) -o $(FIRMWARE).elf \
		-Wl,-Map,$(FIRMWARE).map $(OBJECTS)
	@echo "done."

.c.o:  $(HEADERS) $(SOURCES)
	@printf "Compiling  $<..."
	@$(CC) $(CFLAGS) -Os -c $< -o $@
	@echo "done."

asm: $(FIRMWARE).elf
	@printf "Generating assembler source file..."
	@avr-objdump -D -S $(FIRMWARE).elf > $(FIRMWARE).s
	@echo "done."

bin: $(FIRMWARE).elf
	@printf "Generating $(FIRMWARE).hex..."
	@$(OBJCOPY) -R .eeprom -O binary $(FIRMWARE).elf  \
		$(FIRMWARE).bin
	@echo "done."
#
# Configuration du programmateur ISP
LOADER=avrdude

ifeq ($(shell uname -s), FreeBSD)

#ISP=-c stk500v2 -P /dev/cuaU0		# Programmateur USB
#ISP	:=-c stk200 -P /dev/ppi0	# Programmateur CENTRONIX
endif

ifeq ($(shell uname -s), Darwin)
MODEM	:= $(shell ([-c /dev/tty*usbmodem* ] && ls /dev/tty*usbmodem*))
ISP	:=-c stk500v2 -P $(MODEM)
endif

ISP:=-c usbasp-clone -p m328p -P usb      
#ISP:=-c arduino -p m328p -P /dev/ttyACM0      
#ISP:=-c arduino -p m328p -P /dev/ttyS0      
#ISP:=-i10 -c ponyser -p m328p -P /dev/ttyS0      

LOADER	:=$(LOADER) $(ISP)
LOAD	:=$(LOADER) -i 5 -U flash:w:$(FIRMWARE).hex
DUMP	:=$(LOADER) -i 5 -U flash:r:$(FIRMWARE).hex:i
VERIFY	:=$(LOADER) -i 5 -U flash:v:$(FIRMWARE).hex
ERASE	:=$(LOADER) -i 5 -e
RFUSE	:=$(LOADER) -U lfuse:r:low.txt:b -U hfuse:r:high.txt:b
WFUSE	:=$(LOADER) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m

load:
	@printf "Loading firmware..."
	$(LOAD) > /dev/null 2>&1
	@echo "done."
dump:
	@printf "Reading $(MCU) device..."
	@$(DUMP) > /dev/null 2>&1
	@echo "done."
verify:
	@printf "Verify $(MCU) device..."
	@$(VERIFY) > /dev/null 2>&1
	@echo "done."
erase:
	@printf "Erasing $(MCU) device..."
	@$(ERASE) > /dev/null 2>&1
	@echo "done."
rfuse:
	@printf "Reading fuse..."
	@$(RFUSE) > /dev/null 2>&1
	cat high.txt
	cat low.txt
	@echo "done."
wfuse:
	@printf "Writing fuse..."
	@$(WFUSE) > /dev/null 2>&1
	@echo "done."
#
# Nettoyage, Archivage, etc.
clean :
	@printf "Cleaning source tree..."
	@rm -f *.map *.bin *~ *.elf *.gch *.hex *.vcd *.o \
		$(FIRMWARE) $(FIRMWARE).asm
	@echo "done."

mrproper : clean
	@rm -f *.hex

archive: mrproper
	@tar cvzf ../$(FIRMWARE).tgz ../$(FIRMWARE)
