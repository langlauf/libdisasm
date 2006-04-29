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

TOP	= $(PWD)
DISTRIB = $(TOP)/distrib
INSTALL = /usr/local
INSTALL_BIN	= $(INSTALL)/bin
INSTALL_LIB	= $(INSTALL)/lib
INSTALL_INC	= $(INSTALL)/include
INSTALL_MAN	= $(INSTALL)/man
INSTALL_SHARE	= $(INSTALL)/share/libdisasm
export TOP DISTRIB INSTALL
export INSTALL_BIN INSTALL_LIB INSTALL_INC INSTALL_MAN INSTALL_SHARE



# ============================================================================
# EXPORTS

export VERSION DEBUG
export CC LD AR RANLIB


# ============================================================================
# TARGETS

all: $(LIBDIS) $(X86DIS) $(TESTDIS) swig-modules
dummy: x86dis libdisasm test dist swig-modules install clean

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

# ------------------------------------------------------- SWIG
swig-modules: $(LIBDIS)
	#cd swig && make 

# ------------------------------------------------------- CLEAN
clean:
	cd libdisasm && make clean
	cd x86dis && make clean
	cd test && make clean
	cd swig && make clean

# ------------------------------------------------------- INSTALL
install: $(LIBDIS) $(X86DIS) 
#swig-install
	# install bindistrib files to INSTALL_LOC
	cd libdisasm && make install
	cd x86dis && make install
	cd man && make install
	cd doc && make install
	cd perl && make install
	cd swig && make install

uninstall:
	cd libdisasm && make uninstall
	cd x86dis && make uninstall
	cd man && make uninstall
	cd doc && make uninstall
	cd perl && make uninstall
	cd swig && make uninstall


# ------------------------------------------------------- DISTRIB
dist: $(LIBDIS) $(X86DIS)
	cd libdisasm && make dist
	cd x86dis && make dist
	cd man && make dist
	cd doc && make dist
	cd perl && make dist
	cd test && make dist
	# handle swig modules
	cd swig && make dist || true
	# copy Makefile and License
	cp Makefile distrib/src/libdisasm-$(VERSION)/Makefile
	cp LICENSE distrib/src/libdisasm-$(VERSION)/LICENSE
	cp LICENSE distrib/tgz/usr/local/share/libdisasm/doc/LICENSE
	cp LICENSE distrib/deb/libdisasm/usr/local/share/libdisasm/doc/LICENSE
	cp LICENSE \
	   distrib/deb/libdisasm-dev/usr/local/share/libdisasm/doc/LICENSE
	#cp LICENSE distrib/rpm/libdisasm/usr/local/share/libdisasm/doc/LICENSE
	cd distrib && make dist
