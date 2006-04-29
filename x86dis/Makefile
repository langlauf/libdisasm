ifndef VERSION
VERSION 	= 	0.21
endif

X86DIS_VERSION 	= 	$(VERSION)

PROGRAM = x86dis
LIBDIS  = ../libdisasm/libdisasm.so

ifndef CC
CC	= gcc
endif

LD	= gcc

INCLUDE = -I../libdisasm
CFLAGS 	= $(INCLUDE)  -O3 $(DEBUG) -Wall -Wno-strict-aliasing \
	  -DX86DIS_VERSION=$(X86DIS_VERSION) -DPACKAGE_VERSION=$(X86DIS_VERSION)
LFLAGS 	= -L../libdisasm -ldisasm

SRC		= 	x86dis.c

OBJ		=	x86dis.o

ifndef TOP
TOP	= ..
endif

ifndef DISTRIB
DISTRIB	= $(TOP)/distrib
endif

DISTRIB_DEB	= $(DISTRIB)/deb/
DISTRIB_RPM	= $(DISTRIB)/rpm/
DISTRIB_TGZ	= $(DISTRIB)/tgz/
DISTRIB_SRC	= $(DISTRIB)/src/libdisasm-$(VERSION)/x86dis

BIN_PKG		= x86dis


ifndef INSTALL
INSTALL = /usr/local
endif

INSTALL_BIN	= $(INSTALL)/bin


all: $(PROGRAM) 
dummy: dist deb-dist rpm-dist tgz-dist src-dist

.c.o: $(SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIBDIS):
	cd .. && make libdisasm

$(PROGRAM): $(OBJ) $(LIBDIS)
	$(LD) $(LFLAGS) -o $@ $< 

clean:
	rm $(OBJ) $(PROGRAM)

install: $(PROGRAM)
	[ -d $(INSTALL_BIN) ] || mkdir $(INSTALL_BIN)
	cp $(PROGRAM) $(INSTALL_BIN)/$(PROGRAM)

uninstall: $(INSTALL_BIN)/$(PROGRAM)
	rm $(INSTALL_BIN)/$(PROGRAM)

dist: deb-dist rpm-dist src-dist tgz-dist

deb-dist:
	[ -d $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_BIN) ] || \
	mkdir -p $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_BIN)
	cp $(PROGRAM) $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_BIN)

rpm-dist:
	# TODO :)

tgz-dist:
	[ -d $(DISTRIB_TGZ)/$(INSTALL_BIN) ] || \
	mkdir -p $(DISTRIB_TGZ)/$(INSTALL_BIN)
	cp $(PROGRAM) $(DISTRIB_TGZ)/$(INSTALL_BIN)

src-dist:
	mkdir -p $(DISTRIB_SRC)
	cp Makefile $(SRC) $(DISTRIB_SRC)
