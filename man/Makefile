ifndef VERSION
VERSION 	= 	0.21
endif
LIBDIS_VERSION 	= 	$(VERSION)

BIN_PKG	= libdisasm

DEV_PKG	= libdisasm-dev

X86_PKG = x86dis

ifndef TOP
TOP	= ..
endif

ifndef DISTRIB
DISTRIB	= $(TOP)/distrib
endif

DISTRIB_DEB	= $(DISTRIB)/deb/
DISTRIB_RPM	= $(DISTRIB)/rpm/
DISTRIB_TGZ	= $(DISTRIB)/tgz
DISTRIB_SRC	= $(DISTRIB)/src/libdisasm-$(VERSION)


ifndef INSTALL
INSTALL = /usr/local
endif

INSTALL_MAN	= $(INSTALL)/man

MAN1		= x86dis.1
MAN3		= x86_disasm.3 \
		  x86_format_insn.3 \
		  x86_init.3
MAN7		= libdisasm.7

all: 

dummy: install dist deb-dist rpm-dist tgz-dist src-dist

install:
	# section 1 manuals
	[ -d $(INSTALL_MAN)/man1 ] || \
	mkdir -p $(INSTALL_MAN)/man1
	cp $(MAN1) $(INSTALL_MAN)/man1
	# section 3 manuals
	[ -d $(INSTALL_MAN)/man3 ] || \
	mkdir -p $(INSTALL_MAN)/man3
	cp $(MAN3) $(INSTALL_MAN)/man3
	# section 7 manuals
	[ -d $(INSTALL_MAN)/man7 ] || \
	mkdir -p $(INSTALL_MAN)/man7
	cp $(MAN7) $(INSTALL_MAN)/man7


dist: deb-dist rpm-dist src-dist tgz-dist

deb-dist:
	# bin pkg manuals
	[ -d $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_MAN)/man7 ] || \
	mkdir -p $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_MAN)/man7
	cp $(MAN7) $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_MAN)/man7
	# dev pkg manuals
	[ -d $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_MAN)/man3 ] || \
	mkdir -p $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_MAN)/man3
	cp $(MAN3) $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_MAN)/man3
	# x86dis manuals
	[ -d $(DISTRIB_DEB)$(X86_PKG)$(INSTALL_MAN)/man1 ] || \
	mkdir -p $(DISTRIB_DEB)$(X86_PKG)$(INSTALL_MAN)/man1
	cp $(MAN1) $(DISTRIB_DEB)$(X86_PKG)$(INSTALL_MAN)/man1

rpm-dist:
	# TODO :)

tgz-dist:
	# section 1 manuals
	[ -d $(DISTRIB_TGZ)$(INSTALL_MAN)/man1 ] || \
	mkdir -p $(DISTRIB_TGZ)$(INSTALL_MAN)/man1
	cp $(MAN1) $(DISTRIB_TGZ)$(INSTALL_MAN)/man1
	# section 3 manuals
	[ -d $(DISTRIB_TGZ)$(INSTALL_MAN)/man3 ] || \
	mkdir -p $(DISTRIB_TGZ)$(INSTALL_MAN)/man3
	cp $(MAN3) $(DISTRIB_TGZ)$(INSTALL_MAN)/man3
	# section 7 manuals
	[ -d $(DISTRIB_TGZ)$(INSTALL_MAN)/man7 ] || \
	mkdir -p $(DISTRIB_TGZ)$(INSTALL_MAN)/man7
	cp $(MAN7) $(DISTRIB_TGZ)$(INSTALL_MAN)/man7

src-dist:
	mkdir -p $(DISTRIB_SRC)/man
	cp Makefile $(MAN1) $(MAN3) $(MAN7) $(DISTRIB_SRC)/man
