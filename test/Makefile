ifndef VERSION
VERSION 	= 	0.21
endif
LIBDIS_VERSION 	= 	$(VERSION)


INCLUDE = -I../libdisasm
CFLAGS 	= $(INCLUDE)  -O3 $(DEBUG) -Wall -Wno-strict-aliasing \
	  -DLIBDIS_VERSION=$(LIBDIS_VERSION)

LFLAGS 	= -L../libdisasm -ldisasm

ifndef CC
CC	= gcc
endif

ifndef LD
LD	= gcc
endif

LIBDIS			=	../libdisasm/libdisasm.so

TESTDIS			=	testdis
TESTDIS_SRC		= 	testdis.c
TESTDIS_OBJ		=	testdis.o

QUIKDIS			=	quikdis
QUIKDIS_SRC		= 	quikdis.c
QUIKDIS_OBJ		=	quikdis.o


all: $(TESTDIS) $(QUIKDIS)

.c.o: $(TESTDIS_SRC) $(QUIKDIS_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TESTDIS): $(TESTDIS_OBJ) $(LIBDIS)
	$(LD) $(LFLAGS) -o $@ $< 

$(QUIKDIS): $(QUIKDIS_OBJ) $(LIBDIS)
	$(LD) $(LFLAGS) -o $@ $<

$(LIBDIS):
	cd .. && make libdisasm

clean:
	rm $(TESTDIS_OBJ) $(TESTDIS) $(QUIKDIS_OBJ) $(QUIKDIS)
