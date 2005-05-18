# LIBDISASM MAKEFILE
#

# ============================================================================
# DEFINES

VERSION = 0.21
DEST	= /usr/local	# install root
DEBUG	= -ggdb -DDEBUG

CC	= gcc
LD	= gcc
AR	= ar
RANLIB	= ranlib

RM	= rm -rf
CP	= cp -r
MKDIR	= mkdir -p
TAR	= tar

X86DIS	= x86dis/x86dis
TESTDIS	= test/testdis
QUIKDIS	= test/quikdis
LIBDIS  = libdisasm/libdisasm.so
LIBDISA = libdisasm/libdisasm.a
DOC	= doc/libdisasm.txt doc/README.TXT

# /usr/local/bin
BINDISTRIB_BIN 		= \
		$(X86DIS)
# /usr/local/lib
BINDISTRIB_LIB 		= \
		$(LIBDIS) $(LIBDISA)
# /usr/local/include/libdisasm
BINDISTRIB_INC		= \
		libdisasm/libdis.h
# /usr/local/man
BINDISTRIB_MAN		= \
		
# /usr/local/share/libdisasm: doc, sample src
BINDISTRIB_SHARE	= \
			
SRCDISTRIB		= \



# ============================================================================
# EXPORTS

export VERSION DEBUG
export CC LD AR RANLIB


# ============================================================================
# TARGETS

all: $(LIBDIS) $(X86DIS) $(TESTDIS) 
dummy: x86dis libdisasm test distrib src-tgz tgz rpm deb

# ------------------------------------------------------- LIBDIS
libdisasm: $(LIBDIS)
$(LIBDIS): 
	cd libdisasm && make

# ------------------------------------------------------- X86DIS
x86dis: $(X86DIS)

$(X86DIS): 
	cd x86dis && make

x86dis-clean:
	cd x86dis && make clean

# ------------------------------------------------------- TEST
test: $(TESTDIS) $(QUIKDIS)

$(TESTDIS):
	cd test && make

$(QUIKDIS):
	cd test && make

# ------------------------------------------------------- CLEAN
clean:
	cd libdisasm && make clean
	cd x86dis && make clean

# ------------------------------------------------------- INSTALL
install: $(LIBDIS) $(X86DIS)
	# install bindistrib files to INSTALL_LOC

# ------------------------------------------------------- DISTRIB
distrib: src-tgz tgz rpm deb
	@echo MAKE DISTRIB not yet supported 

src-tgz:

tgz: $(LIBDIS) $(X86DIS)
	@echo MAKE TGZ not yet supported 

rpm: $(LIBDIS) $(X86DIS)
	@echo MAKE RPM not yet supported 

deb: $(LIBDIS) $(X86DIS)
	@echo MAKE DEB not yet supported 
	mkdir -p distrib/deb/libdisasm/$(DEST)/lib
	mkdir -p distrib/deb/libdisasm/$(DEST)/bin
	mkdir -p distrib/deb/libdisasm/$(DEST)/man
	mkdir -p distrib/deb/libdisasm/$(DEST)/include/libdisasm
	mkdir -p distrib/deb/libdisasm/$(DEST)/share/libdisasm
