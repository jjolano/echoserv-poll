# Makefile designed for OpenPS3FTP v3 by jjolano
# Simple Makefile for c/c++ code

ifeq ($(strip $(PSL1GHT)),)
$(error "PSL1GHT is not installed. Please install PSL1GHT.")
endif

# Basic configuration
TARGET		:= echoserv-poll

TITLE		:= echoserv-poll
APPID		:= ECHO00001
CONTENTID	:= UP0001-$(APPID)_00-0000000000000000

ICON0		:= $(PS3DEV)/bin/ICON0.PNG
SFOXML		:= ./sfo.xml

LIBS		:= -lrt -llv2 -lnet -lio -lsysmodule -lsysutil

CFLAGS		:= -D__PS3__

# Compile tools
MAKE_SELF	:= make_self_npdrm
MAKE_SFO	:= sfo.py
MAKE_PKG	:= pkg.py

# Files
CFILES		:= $(shell find * -name "*.c"   -print)
CPPFILES	:= $(shell find * -name "*.cpp" -print)

OBJS		:= ${CFILES:.c=.o} ${CPPFILES:.cpp=.o}

INCLUDES	:= -I$(PSL1GHT)/ppu/include -I$(PS3DEV)/portlibs/ppu/include
LIBDIRS		:= -L$(PSL1GHT)/ppu/lib -L$(PS3DEV)/portlibs/ppu/lib

# Rules
.PHONY: all clean

all: $(TARGET).elf

clean:
	rm -f $(OBJS) $(TARGET).elf

%.o: %.c
	@echo "$< -> $@"
	@ppu-gcc -Wall -O3 -std=gnu99 $(INCLUDES) -c $< -o $@ $(CFLAGS)

%.o: %.cpp
	@echo "$< -> $@"
	@ppu-g++ -Wall -O3 $(INCLUDES) -c $< -o $@ $(CFLAGS)

$(TARGET).elf: $(OBJS)
	@echo "$(OBJS) -> $@"
	@ppu-gcc $(OBJS) $(LIBDIRS) $(LIBS) -o $@

pkg: $(TARGET).elf
	@echo $(TARGET).pkg
	@rm -Rf ./temp
	@mkdir -p ./temp
	@mkdir -p ./temp/USRDIR
	@cp $(ICON0) ./temp
	@touch ./temp/USRDIR/firstrun
	@$(MAKE_SELF) $(TARGET).elf ./temp/USRDIR/EBOOT.BIN $(CONTENTID) >> /dev/null
	@$(MAKE_SFO) --title "$(TITLE)" --appid "$(APPID)" -f $(SFOXML) ./temp/PARAM.SFO
	@$(MAKE_PKG) --contentid $(CONTENTID) ./temp/ $(TARGET).pkg >> /dev/null
	@rm -Rf ./temp

