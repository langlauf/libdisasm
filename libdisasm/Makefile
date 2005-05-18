ifndef VERSION
VERSION 	= 	0.21
endif
LIBDIS_VERSION 	= 	$(VERSION)


LIBNAME	= libdisasm.so

ARCHIVE	= libdisasm.a

INCLUDE = -I.

CFLAGS 	= $(INCLUDE) -O3 $(DEBUG) -Wall -Wno-strict-aliasing
LFLAGS 	= $(FLAGS) -fPIC -shared -Wl,-soname,$(LIBNAME)

ifndef CC
CC	= gcc
endif

ifndef LD
LD	= gcc
endif

ifndef AR
AR	= ar
endif

ifndef RANLIB
RANLIB	= ranlib
endif

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

all: $(ARCHIVE) $(LIBNAME)

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
