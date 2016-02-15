# Copyright (c) 2010- Authentec.  http://www.authentec.com

CC = gcc
LD = g++

C_OBJS_ALL += \
	sample.o \
	./patch/patch.o \
	./usb/crc.o \
	./usb/usbl.o \
	./usb/usblib.o \
	./pp/source/badlines_new.o \
	./pp/source/crypt.o \
	./pp/source/fingerdetect.o \
	./pp/source/ioreg.o \
	./pp/source/nvm.o \
	./pp/source/orientation.o \
	./pp/source/qualeng.o \
	./pp/source/query.o \
	./pp/source/reg_map.o \
	./pp/source/settings.o \
	./pp/source/tci.o \
	./pp/source/tci_ext.o \
	./pp/source/touchip.o \
	./pp/source/touchip1a.o \
	./pp/source/touchip1c.o \
	./pp/source/touchip2a.o \
	./pp/source/touchip2c.o

# Comment/uncomment to enable/disable debugging code
DEBUG = 0
ifeq ($(DEBUG),1)
	DEBFLAGS = -g -O -DDEBUG 
else
	DEBFLAGS = -O2 -DNDEBUG
endif

# Choose 32-bit or 64-bit environment
DO64BIT = 0
ifeq ($(DO64BIT),1)
	CFLAGS += -m64
	LFLAGS += -m64
else
	CFLAGS += -m32
	LFLAGS += -m32
endif

CFLAGS += -DLINUX -DVERSION_LINUX -DFORCE_STATIC -D_TDBG_LEVEL=0 -DUSE_MALLOC_FREE_MM
CFLAGS += $(DEBFLAGS) -fPIC -Wall
CFLAGS += -I./export -I./pp/include -I./usb

# need also thread support for interrupt handling
LFLAGS += -lpthread -lusb

PROG = sample

all: $(PROG)

$(PROG): $(C_OBJS_ALL) $(LIBS)
	@$(LD) $(LFLAGS) -o $@ $(C_OBJS_ALL) $(LIBS)

%.o: %.cpp
	@$(CC) -c $(CFLAGS) $< -o $@

clean: 
	@rm -f *.o
	@rm -f ./patch/*.o
	@rm -f ./usb/*.o
	@rm -f ./pp/source/*.o
	@rm -f $(PROG)
	@rm -f *~

rebuild: clean all

