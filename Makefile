#############################################################################
#############################################################################
###***********************************************************************###
###* .oO0Oo.oO0Oo.oO0Oo.oO(:T:H:E: :B:A:S:T:A:R:D:)Oo.oO0Oo.oO0Oo.oO0Oo. *###
###*                                                                     *###
###*                                                                     *###
###*    Chief Makefile Architect : the grugq                             *###
###*    other people that did something:                                 *###
###*                                    mammon_ and ReZiDeNt and MO_K    *###
###*                                                                     *###
###*                                                                     *###
###***********************************************************************###
#############################################################################
#############################################################################



# ------------------------------------ Intialization -----------------
CC      = gcc
AR      = ar
TAR     = tar --exclude CVS -cf
ZIP     = gzip
RM      = /bin/rm -rf
#TERM    = -lcurses
TERM   = -ltermcap   # if you don't have ncurses
#NOBFD	= -DNO_LIBOPCODES	#flag to stop grug's bitching ;)
PROGRAM = bastard
VER     = 0.07

# ------------------------------------ Stuff in the ./src directory --
DEFINES  = -I./include -I/usr/local/include -DREADLINE_LIBRARY
#CFLAGS  = -Wall -g $(DEFINES)
CFLAGS   = -ggdb -rdynamic $(DEFINES) $(NO_BFD) -g
#CFLAGS   = -rdynamic  -O3 $(DEFINES)
LIBS     = -lm -ldl -lbfd -liberty -ltyphoon -lseer -lreadline $(TERM)
LDFLAGS  = -L./lib 

SRCDIR  = src
INCDIR  = include
OBJDIR  = src

SRCS    =       \
                ${SRCDIR}/bastard.c     		\
					 ${SRCDIR}/api_address.c		\
					 ${SRCDIR}/api_code.c			\
					 ${SRCDIR}/api_disasm.c			\
					 ${SRCDIR}/api_functions.c		\
					 ${SRCDIR}/api_importexport.c	\
					 ${SRCDIR}/api_loadsave.c		\
					 ${SRCDIR}/api_macros.c			\
					 ${SRCDIR}/api_namecomment.c	\
                ${SRCDIR}/api_search.c			\
					 ${SRCDIR}/api_section.c		\
					 ${SRCDIR}/api_strings.c		\
					 ${SRCDIR}/api_structconst.c	\
					 ${SRCDIR}/api_system.c			\
					 ${SRCDIR}/api_xref.c			\
                ${SRCDIR}/db.c          		\
                ${SRCDIR}/macro.c
HDRS    =       \
                ${INCDIR}/api.h           \
                ${INCDIR}/bastard.h       \
                ${INCDIR}/bdb.h           \
                ${INCDIR}/db.h            \
                ${INCDIR}/disasm.h        \
                ${INCDIR}/dislib.h        \
                ${INCDIR}/macro.h         \
                ${INCDIR}/seer.h          \
                ${INCDIR}/typhoon.h
OBJS    =       \
                ${OBJDIR}/bastard.o     		\
					 ${OBJDIR}/api_address.o		\
					 ${OBJDIR}/api_code.o			\
					 ${OBJDIR}/api_disasm.o			\
					 ${OBJDIR}/api_functions.o		\
					 ${OBJDIR}/api_importexport.o	\
					 ${OBJDIR}/api_loadsave.o		\
					 ${OBJDIR}/api_macros.o			\
					 ${OBJDIR}/api_namecomment.o	\
                ${OBJDIR}/api_search.o  		\
					 ${OBJDIR}/api_section.o		\
					 ${OBJDIR}/api_strings.o		\
					 ${OBJDIR}/api_structconst.o	\
					 ${OBJDIR}/api_system.o			\
					 ${OBJDIR}/api_xref.o			\
                ${OBJDIR}/db.o          		\
                ${OBJDIR}/macro.o

# ------------------------------------ Stuff in the ./arch directory --
A_CFLAGS = -fPIC -I./include -I./arch -I/usr/local/include -O3
A_SOFLAGS = -shared -lc -Wl,-soname,
A_ARFLAGS = -L./lib 
ARCHDIR = arch

A_SRCS  =       \
	             ${ARCHDIR}/i386.c
A_HDRS  =       \
	             ${ARCHDIR}/i386.h       \
	             ${ARCHDIR}/i386.opcode.map
A_OBJS  =       \
	             ${ARCHDIR}/i386.o
A_SOS   =       \
	             ${ARCHDIR}/libi386.so
A_ARS   =       \
	             ${ARCHDIR}/libi386.a

# ------------------------------------ Tarfile collections -----------
DOCS       = doc .license README.* TODO
XTRA       = src/seer src/typhoon
REQ_FILES  = scripts utils lib formats Makefile bdb.dbd $(SRCDIR)/bdb.ddl
BAST_SRC   = $(SRCS) $(HDRS) $(A_SRCS) $(A_HDRS) $(REQ_FILES) $(DOCS) $(XTRA)
BAST_BIN   = $(A_SOS) $(PROGRAM) $(REQ_FILES) $(DOCS)
ARCH_SRC   = $(A_SRCS) $(A_HDRS) doc/libi386.txt ${INCDIR}/dislib.h Makefile
ARCH_BIN   = $(A_SOS) $(A_ARS) doc/libi386.txt ${INCDIR}/dislib.h

# ------------------------------------ Actual Makefile Rulesets ------
all:            $(PROGRAM)

seer: libseer.a
typhoon: libtyphoon.a
arch: ${ARCHDIR}/libi386.so

libseer.a:
	@echo "   Compiling SeerC embedded interpreter..."
	@cd ./src/seer; make install
	@cp ./src/seer/bin/seerc ./utils

libtyphoon.a:
	@echo "   Compiling Typhoon embedded relational database..."
	@cd ./src/typhoon; [ -f src/Makefile ] || ./configure; make install

$(A_OBJS):  $(A_SRCS)
	$(CC) $(A_CFLAGS) -o $@ -c $<

$(A_SOS): $(A_OBJS)
	$(CC) $(A_SOFLAGS)$@ -o $@ $<

$(A_ARS): $(A_OBJS)
	$(AR) rcs $@ $< 

arch: $(A_SOS) $(A_ARS)

.c.o: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

$(PROGRAM): libseer.a libtyphoon.a $(OBJS) $(A_SOS) $(A_ARS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
	@ [ -f /etc/debian_version ]  || echo "Real Bastards use Debian!"

release: $(PROGRAM)
	strip $(PROGRAM)

clean:
	$(RM) $(PROGRAM) $(OBJS) ${SRCDIR}/*~ *core
	$(RM) $(A_OBJS) $(A_SOS) $(A_ARS)

distclean: clean
	cd src/typhoon; make distclean
	cd src/seer; make clean
	$(RM) doc/man/catl/*

tgz:
	$(TAR) $(PROGRAM)-$(VER).tar $(BAST_SRC)
	$(ZIP) $(PROGRAM)-$(VER).tar
	$(TAR) $(PROGRAM)_bin-$(VER).tar $(BAST_BIN)
	$(ZIP) $(PROGRAM)_bin-$(VER).tar
	$(TAR) libdisasm-$(VER).tar $(ARCH_SRC)
	$(ZIP) libdisasm-$(VER).tar
	$(TAR) libdisasm_bin-$(VER).tar $(ARCH_BIN)
	$(ZIP) libdisasm_bin-$(VER).tar
	mv $(PROGRAM)-$(VER).tar.gz $(PROGRAM)-$(VER).tgz
	mv $(PROGRAM)_bin-$(VER).tar.gz $(PROGRAM)_bin-$(VER).tgz
	mv libdisasm-$(VER).tar.gz libdisasm-$(VER).tgz
	mv libdisasm_bin-$(VER).tar.gz libdisasm_bin-$(VER).tgz
