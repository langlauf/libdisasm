LIBDIS_VERSION 	= 	0.21-pre1
INSTALL_ROOT	=	/usr/local
LIBNAME	= libdisasm.so
ARCHIVE	= libdisasm.a
INCLUDE = -I.
FLAGS 	= $(INCLUDE)  -O3 -ggdb -Wall
LFLAGS 	= $(FLAGS) -fPIC -shared -Wl,-soname,$(LIBNAME)
RM	= rm -rf
CP	= cp -r
MKDIR	= mkdir -p
TAR	= tar
CC	= gcc
AR	= ar
RANLIB	= ranlib

SRC		= 	libdis.c        \
			bastard.c	\
			vm.c		\
			i386_invariant.c \
			i386.c

OBJ		=	libdis.o         \
			bastard.o	\
			vm.o		\
			i386_invariant.o \
			i386.o

SRC_DISTRIB	= 	$(SRC)			\
			libdisasm.dsp		\
			libdisasm.vcproj	\
			libdisasm2.vcproj	\
			bastard.h		\
			extension.h		\
			i386.h			\
			i386_opcode.h		\
			qword.h			\
			vm.h			\
			i386.opcode.map		\
			bin_from_dump.pl	\
			modify_opcode_map.pl	\
			modrm.pl

UTIL_SRC	=				\
			quikdis.c		\
			quikdis_old.c		\
			testdis.c		\
			x86dis.c

MAN1		=				\
			x86dis.1		\
	   
MAN3		=				\
			x86_disasm.3		\
			x86_format_insn.3	\
			x86_init.3		\

MAN7		=				\
			libdisasm.7		\

TXT		=				\
			LICENSE			\
			libdisasm.txt		\
			Perl-Disassembler-HOWTO.txt

DOC		=	$(MAN1) $(MAN3) $(MAN7)	$(TXT)

INC_DISTRIB	=				\
			libdis.h		

LIB_DISTRIB	=				\
			libdisasm.a		\
			libdisasm.so		

UTIL_DISTRIB	=				\
			x86dis			\
			x86disasm.pl

BIN_DISTRIB	=	$(LIB_DISTRIB)		\
			quikdis			\
			testdis			\
			x86dis
			
DISTRIB		= 	$(DOC) $(UTIL_SRC) $(INC_DISTRIB)	\
			Makefile				\
			x86disasm.pl

all: $(ARCHIVE) $(LIBNAME) quikdis quikdis_old testdis x86dis

.c.o: $(SRC)
	$(CC) $(FLAGS) -o $@ -c $<

$(ARCHIVE): $(OBJ)
	# make .a
	$(AR) rc $(ARCHIVE) $(OBJ)
	$(RANLIB) $(ARCHIVE)

$(LIBNAME): $(OBJ)
	# make .so
	$(CC) $(LFLAGS) $(OBJ) -o $(LIBNAME)

x86dis: $(ARCHIVE) $(LIBNAME)
	$(CC) $(FLAGS) x86dis.c $(ARCHIVE) -o x86dis

quikdis_old: $(ARCHIVE) $(LIBNAME)
	$(CC) $(FLAGS) quikdis_old.c $(ARCHIVE) -o quikdis_old

quikdis: $(ARCHIVE) $(LIBNAME)
	$(CC) $(FLAGS) quikdis.c $(ARCHIVE) -o quikdis

testdis: $(ARCHIVE) $(LIBNAME)
	$(CC) $(FLAGS) testdis.c $(ARCHIVE) -o testdis

clean:
	@$(RM) quikdis quikdis_old testdis $(ARCHIVE) $(OBJ) $(LIBNAME)

install: libdisasm-install utils-install man-install

man-install:
	@[ -d $(INSTALL_ROOT)/man ] || $(MKDIR) $(INSTALL_ROOT)/man
	@[ -d $(INSTALL_ROOT)/man/man1 ] || $(MKDIR) $(INSTALL_ROOT)/man/man1
	@[ -d $(INSTALL_ROOT)/man/man3 ] || $(MKDIR) $(INSTALL_ROOT)/man/man3
	@[ -d $(INSTALL_ROOT)/man/man7 ] || $(MKDIR) $(INSTALL_ROOT)/man/man7
	@$(CP) $(MAN1) $(INSTALL_ROOT)/man/man1
	@$(CP) $(MAN3) $(INSTALL_ROOT)/man/man3
	@$(CP) $(MAN7) $(INSTALL_ROOT)/man/man7
	@[ -d $(INSTALL_ROOT)/share/libdisasm/doc ] || \
		$(MKDIR) $(INSTALL_ROOT)/share/libdisasm/doc
	@$(CP) $(TXT) $(INSTALL_ROOT)/share/libdisasm/doc

libdisasm-install: $(ARCHIVE) $(LIBNAME)
	@[ -d $(INSTALL_ROOT)/lib ] || $(MKDIR) $(INSTALL_ROOT)/lib
	@$(CP) $(LIB_DISTRIB) $(INSTALL_ROOT)/lib
	@[ -d $(INSTALL_ROOT)/include ] || $(MKDIR) $(INSTALL_ROOT)/include
	@$(CP) $(INC_DISTRIB) $(INSTALL_ROOT)/include

utils-install: x86dis
	@[ -d $(INSTALL_ROOT)/bin ] || $(MKDIR) $(INSTALL_ROOT)/bin
	@$(CP) $(UTIL_DISTRIB) $(INSTALL_ROOT)/bin
	
tgz:
	@$(MKDIR) libdisasm_src-$(LIBDIS_VERSION)
	@$(CP) $(SRC_DISTRIB) $(DISTRIB) libdisasm_src-$(LIBDIS_VERSION)
	@$(TAR) -zcf libdisasm_src-$(LIBDIS_VERSION).tgz libdisasm_src-$(LIBDIS_VERSION)
	@$(RM) libdisasm_src-$(LIBDIS_VERSION)

	@$(MKDIR) libdisasm-$(LIBDIS_VERSION)
	@$(CP) $(BIN_DISTRIB) $(DISTRIB) libdisasm-$(LIBDIS_VERSION)
	@$(TAR) -zcf libdisasm-$(LIBDIS_VERSION).tgz libdisasm-$(LIBDIS_VERSION)
	@$(RM) libdisasm-$(LIBDIS_VERSION)
