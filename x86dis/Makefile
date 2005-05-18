ifndef VERSION
VERSION 	= 	0.21
endif

X86DIS_VERSION 	= 	$(VERSION)

PROGRAM = x86dis
LIBDIS  = ../libdisasm/libdisasm.so

INCLUDE = -I../libdisasm
CFLAGS 	= $(INCLUDE)  -O3 $(DEBUG) -Wall -Wno-strict-aliasing \
	  -DX86DIS_VERSION=$(X86DIS_VERSION)
LFLAGS 	= -L../libdisasm -ldisasm

ifndef CC
CC	= gcc
endif

ifndef LD
LD	= gcc
endif

SRC		= 	x86dis.c

OBJ		=	x86dis.o


all: $(PROGRAM) 

.c.o: $(SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIBDIS):
	cd .. && make libdisasm

$(PROGRAM): $(OBJ) $(LIBDIS)
	$(LD) $(LFLAGS) -o $@ $< 

clean:
	rm $(OBJ) $(PROGRAM)
