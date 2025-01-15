#CROSS_COMPILE		= arm-linux-androideabi-
CROSS_COMPILE		?= 
AS					=   $(CROSS_COMPILE)as
AR					=   $(CROSS_COMPILE)ar
CC					=   $(CROSS_COMPILE)gcc
CPP					=   $(CC) -E
LD					=   $(CROSS_COMPILE)ld
NM					=   $(CROSS_COMPILE)nm
OBJCOPY				=   $(CROSS_COMPILE)objcopy
OBJDUMP				=   $(CROSS_COMPILE)objdump
RANLIB				=   $(CROSS_COMPILE)ranlib
READELF				=   $(CROSS_COMPILE)readelf
SIZE				=   $(CROSS_COMPILE)size
STRINGS				=   $(CROSS_COMPILE)strings
STRIP				=   $(CROSS_COMPILE)strip

#CFLAGS				= -Wall  
CFLAGS				= -fPIC
LDFLAGS				= -static
OBJS1				=   
LIBS1				=	
FINTEK_LIB_SRC			= $(wildcard fintek_api/*.c) $(wildcard fintek_api/*.h)
DEMO_PROG			= demo_uart.o demo_i2c_protocol.o demo_gpio.o demo_wdt.o demo_i2c.o demo_list_device.o demo_id.o demo_spi.o demo_raspberry.o demo_caseopen.o demo_pwm.o demo_fan.o demo_temperature.o
DEMO_PROG_EXE			= $(DEMO_PROG:.o=)
GIT_SHA					= `git rev-parse --short HEAD`

all: $(DEMO_PROG_EXE)
#all: $(DEMO_PROG_EXE) demo_gpio_int
#	$(CC) $(CFLAGS) -o $(OBJS1) $(OBJS1).c $(LIBS1) $(LDFLAGS)
#	$(CC) $(CFLAGS) -o $(OBJS2) $(OBJS2).c $(LIBS2) $(LDFLAGS)
#	$(CC) $(CFLAGS) -o $(OBJS3) $(OBJS3).c 
#	$(CC) $(CFLAGS) -o $(OBJS4) $(OBJS4).c 
#	$(STRIP) $(OBJS1)
#	$(STRIP) $(OBJS2)

demo_gpio_int: demo_gpio_int.o libfintek_api.a
	$(CC) $(CFLAGS) -static -o demo_gpio_int demo_gpio_int.c libfintek_api.a -lpthread

release: all
	tar zcf fintek_demo_release-$(GIT_SHA)-`uname -m`.tar.gz *.c *.h *.a Makefile

$(DEMO_PROG_EXE): libfintek_api.a common.o

libfintek_api.a: $(FINTEK_LIB_SRC)
	make -C fintek_api GIT_TAG=$(GIT_TAG)

common.o: libfintek_api.a

clean:
	-@make clean -C fintek_api
	-@rm *.o *.gz $(DEMO_PROG) $(DEMO_PROG_EXE) 2> /dev/null > /dev/null || true

