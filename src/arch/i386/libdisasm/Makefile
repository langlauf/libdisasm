LIBNAME	= libdisasm.so
ARCHIVE	= libdisasm.a
INCLUDE 	= -I.
FLAGS 	= $(INCLUDE)  -O3 -ggdb
LFLAGS 	= $(FLAGS) -fPIC -shared -Wl,-soname,$(LIBNAME)
RM		= rm
CC		= gcc
AR		= ar
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

all: libdis quikdis quikdis_old testdis

.c.o: $(SRC)
	$(CC) $(FLAGS) -o $@ -c $<

libdis: $(OBJ)
	# make .a
	$(AR) rc $(ARCHIVE) $(OBJ)
	$(RANLIB) $(ARCHIVE)
	# make .so
	$(CC) $(LFLAGS) $(OBJ) -o $(LIBNAME)

quikdis_old: libdis
	$(CC) $(FLAGS) quikdis_old.c $(ARCHIVE) -o quikdis_old
	
quikdis: libdis
	$(CC) $(FLAGS) quikdis.c $(ARCHIVE) -o quikdis

testdis: libdis
	$(CC) $(FLAGS) testdis.c $(ARCHIVE) -o testdis

clean:
	@$(RM) quikdis testdis  $(LIBNAME)
