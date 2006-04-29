ifndef VERSION
VERSION 	= 	0.21
endif
LIBDIS_VERSION 	= 	$(VERSION)

LIBNAME	= libdisasm.so

ARCHIVE	= libdisasm.a

BIN_PKG	= libdisasm

DEV_PKG	= libdisasm-dev

INCLUDE = -I.

CFLAGS 	= $(INCLUDE) -O3 $(DEBUG) -Wall -Wno-strict-aliasing
LFLAGS 	= $(FLAGS) -fPIC -shared -Wl,-soname,$(LIBNAME)

ifndef CC
CC	= gcc
endif

# LD is pre-defined somehow?
#ifndef LD
LD	= gcc
#endif

ifndef AR
AR	= ar
endif

ifndef RANLIB
RANLIB	= ranlib
endif

ifndef TOP
TOP	= ..
endif

ifndef DISTRIB
DISTRIB	= $(TOP)/distrib
endif

DISTRIB_DEB	= $(DISTRIB)/deb/
DISTRIB_RPM	= $(DISTRIB)/rpm/
DISTRIB_TGZ	= $(DISTRIB)/tgz/
DISTRIB_SRC	= $(DISTRIB)/src/libdisasm-$(VERSION)/libdisasm


ifndef INSTALL
INSTALL = /usr/local
endif

INSTALL_LIB	= $(INSTALL)/lib
INSTALL_INC	= $(INSTALL)/include
INSTALL_SHARE	= $(INSTALL)/share/libdisasm

			#Removed: ia32_fixup.c
SRC		= 				\
			ia32_implicit.c		\
			ia32_insn.c		\
			ia32_invariant.c	\
			ia32_modrm.c		\
			ia32_opcode_tables.c	\
			ia32_operand.c		\
			ia32_reg.c		\
			ia32_settings.c		\
			x86_disasm.c		\
			x86_format.c		\
			x86_imm.c		\
			x86_insn.c		\
			x86_misc.c		\
			x86_operand_list.c

OBJ		=				\
			ia32_implicit.o		\
			ia32_insn.o		\
			ia32_invariant.o	\
			ia32_modrm.o		\
			ia32_opcode_tables.o	\
			ia32_operand.o		\
			ia32_reg.o		\
			ia32_settings.o		\
			x86_disasm.o		\
			x86_format.o		\
			x86_imm.o		\
			x86_insn.o		\
			x86_misc.o		\
			x86_operand_list.o

HEADERS		=				\
			ia32_implicit.h		\
			ia32_insn.h		\
			ia32_invariant.h	\
			ia32_modrm.h		\
			ia32_opcode_tables.h	\
			ia32_operand.h		\
			ia32_reg.h		\
			ia32_settings.h		\
			libdis.h		\
			qword.h			\
			version.h		\
			x86_imm.h		\
			x86_operand_list.h

# misc files to be distributed along with the source
MISC_SRC	=				\
			INTEL_BUGS		\
			Makefile		\
			NAMESPACE.TXT		\
			TODO			\
			ia32_opcode.dat
#			libdisasm.dsp		\
#			libdisasm.vcproj	\
#			libdisasm2.vcproj	\

# header files to be distributed with the 'dev' package
DEV_INC		=				\
			libdis.h		

# static library files to be distributed with the 'dev' package
DEV_LIB		=				\
			$(ARCHIVE)

DEV_SHARE	=				\
			ia32_opcode.dat

# shared library files to be distributed with the binary package
BIN_LIB		=				\
			$(LIBNAME)


all: $(ARCHIVE) $(LIBNAME)
dummy: dist deb-dist rpm-dist tgz-dist src-dist

.c.o: $(SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ARCHIVE): $(OBJ)
	# make .a
	$(AR) rc $(ARCHIVE) $(OBJ)
	$(RANLIB) $(ARCHIVE)

$(LIBNAME): $(OBJ)
	# make .so
	$(LD) $(LFLAGS) $(OBJ) -o $(LIBNAME)

clean:
	@$(RM) $(ARCHIVE) $(OBJ) $(LIBNAME)

install: $(LIBNAME) $(ARCHIVE)
	[ -d $(INSTALL_LIB) ] || mkdir -p $(INSTALL_LIB)
	cp $(BIN_LIB) $(DEV_LIB) $(INSTALL_LIB)
	[ -d $(INSTALL_INC) ] || mkdir -p $(INSTALL_INC)
	cp $(DEV_INC) $(INSTALL_LIB)
	[ -d $(INSTALL_SHARE) ] || mkdir -p $(INSTALL_SHARE)/data
	cp $(DEV_SHARE) $(INSTALL_SHARE)/data

uninstall: $(INSTALL_LIB)/$(LIBNAME)
	rm -f $(INSTALL_LIB)/$(BIN_LIB) $(INSTALL_LIB)/$(DEV_LIB)
	rm -f $(INSTALL_INC)/$(DEV_INC) 
	rm -f $(INSTALL_SHARE)/data/$(DEV_SHARE) 

dist: $(LIBNAME) $(ARCHIVE) deb-dist rpm-dist src-dist tgz-dist

deb-dist:
	# bin pkg libs
	[ -d $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_LIB) ] || \
	mkdir -p $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_LIB)
	cp $(BIN_LIB) $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_LIB)
	# dev pkg libs
	[ -d $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_LIB) ] || \
	mkdir -p $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_LIB)
	cp $(DEV_LIB) $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_LIB)
	# dev pkg headers
	[ -d $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_INC) ] || \
	mkdir -p $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_INC)
	cp $(DEV_INC) $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_INC)
	# dev shared files
	[ -d $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_SHARE)/data ] || \
	mkdir -p $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_SHARE)/data
	cp $(DEV_SHARE) $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_SHARE)/data

rpm-dist:
	# TODO :)

tgz-dist:
	# shared libs
	[ -d $(DISTRIB_TGZ)/$(INSTALL_LIB) ] || \
	mkdir -p $(DISTRIB_TGZ)/$(INSTALL_LIB)
	cp $(BIN_LIB) $(DISTRIB_TGZ)/$(INSTALL_LIB)
	# static libs
	[ -d $(DISTRIB_TGZ)/$(INSTALL_LIB) ] || \
	mkdir -p $(DISTRIB_TGZ)/$(INSTALL_LIB)
	cp $(DEV_LIB) $(DISTRIB_TGZ)/$(INSTALL_LIB)
	# headers
	[ -d $(DISTRIB_TGZ)/$(INSTALL_INC) ] || \
	mkdir -p $(DISTRIB_TGZ)/$(INSTALL_INC)
	cp $(DEV_INC) $(DISTRIB_TGZ)/$(INSTALL_INC)
	# shared files
	[ -d $(DISTRIB_TGZ)/$(INSTALL_SHARE)/data ] || \
	mkdir -p $(DISTRIB_TGZ)/$(INSTALL_SHARE)/data
	cp $(DEV_SHARE) $(DISTRIB_TGZ)/$(INSTALL_SHARE)/data

src-dist:
	mkdir -p $(DISTRIB_SRC)
	cp $(SRC) $(HEADERS) $(MISC_SRC) $(DISTRIB_SRC)
