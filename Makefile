#############################################################################
#############################################################################
###***********************************************************************###
###* .oO0Oo.oO0Oo.oO0Oo.oO(:T:H:E: :B:A:S:T:A:R:D:)Oo.oO0Oo.oO0Oo.oO0Oo. *###
###*                                                                     *###
###*                                                                     *###
###*    Chief Makefile Architect : the grugq                             *###
###*    other people that did something:                                 *###
###*                                    mammon_ and ReZiDeNt             *###
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
TERM    = -lcurses
#TERM   = -ltermcap   # if you don't have ncurses
PROGRAM = bastard
VER     = 0.04

# ------------------------------------ Stuff in the ./src directory --
DEFINES  = -I./include -I/usr/local/include -DREADLINE_LIBRARY
#CFLAGS  = -Wall -g $(DEFINES)
CFLAGS   = -rdynamic  -O3 $(DEFINES)
LIBS     = -lm -ldl -ltyphoon -lseer -lreadline $(TERM)
LDFLAGS  = -L./lib

SRCDIR  = src
INCDIR  = include
OBJDIR  = src

SRCS    =       \
                ${SRCDIR}/bastard.c     \
                ${SRCDIR}/api.c         \
                ${SRCDIR}/db.c          \
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
                ${OBJDIR}/bastard.o     \
                ${OBJDIR}/api.o         \
                ${OBJDIR}/db.o          \
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
REQ_FILES  = $(DOCS) utils lib formats Makefile bdb.dbd $(SRCDIR)/bdb.ddl
BAST_SRC   = $(SRCS) $(HDRS) $(A_SRCS) $(A_HDRS) $(REQ_FILES) 
BAST_BIN   = $(A_SOS) $(PROGRAM) $(REQ_FILES)
ARCH_SRC   = $(A_SRCS) $(A_HDRS) doc/libi386.txt ${INCDIR}/dislib.h Makefile
ARCH_BIN   = $(A_SOS) $(A_ARS) doc/libi386.txt ${INCDIR}/dislib.h

# ------------------------------------ Actual Makefile Rulesets ------
all:            $(PROGRAM)

$(A_OBJS):  $(A_SRCS)
	$(CC) $(A_CFLAGS) -o $@ -c $<

$(A_SOS): $(A_OBJS)
	$(CC) $(A_SOFLAGS)$@ -o $@ $<

$(A_ARS): $(A_OBJS)
	$(AR) rcs $@ $< 

arch: $(A_SOS) $(A_ARS)

.c.o: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

$(PROGRAM): $(OBJS) $(A_SOS) $(A_ARS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
	strip $(PROGRAM)

clean:
	$(RM) $(PROGRAM) $(OBJS) ${SRCDIR}/*~ *core
	$(RM) $(A_OBJS) $(A_SOS) $(A_ARS)

tgz:
	$(TAR) $(PROGRAM)-$(VER).tar $(BAST_SRC)
	$(ZIP) $(PROGRAM)-$(VER).tar
	$(TAR) $(PROGRAM)_bin-$(VER).tar $(BAST_BIN)
	$(ZIP) $(PROGRAM)_bin-$(VER).tar
	$(TAR) libdisasm-$(VER).tar $(ARCH_SRC)
	$(ZIP) libdisasm-$(VER).tar
	$(TAR) libdisasm_bin-$(VER).tar $(ARCH_BIN)
	$(ZIP) libdisasm_bin-$(VER).tar
