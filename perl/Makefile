ifndef VERSION
VERSION 	= 	0.21
endif
LIBDIS_VERSION 	= 	$(VERSION)

BIN_PKG	= libdisasm

DEV_PKG	= libdisasm-dev

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

INSTALL_SHARE	= $(INSTALL)/share/libdisasm
INSTALL_DOC	= $(INSTALL_SHARE)/doc

DEVPKG_DOC	= Perl-Disassembler-HOWTO.txt
DEVPKG_PERL	= bin_from_dump.pl \
		  build_opcode_map.pl \
		  modify_opcode_map.pl \
		  x86disasm.pl


all: 

dummy: install dist deb-dist rpm-dist tgz-dist src-dist

install:
	[ -d $(INSTALL_DOC) ] || mkdir -p $(INSTALL_DOC)
	cp $(DEVPKG_DOC) $(INSTALL_DOC)
	[ -d $(INSTALL_SHARE)/perl ] || mkdir -p $(INSTALL_SHARE)/perl
	cp $(DEVPKG_PERL) $(INSTALL_SHARE)/perl


dist: deb-dist rpm-dist src-dist tgz-dist

deb-dist:
	# dev pkg manuals
	[ -d $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_DOC) ] || \
	mkdir -p $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_DOC)
	cp $(DEVPKG_DOC) $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_DOC)
	# dev pkg scripts
	[ -d $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_SHARE)/perl ] || \
	mkdir -p $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_SHARE)/perl
	cp $(DEVPKG_PERL) $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_SHARE)/perl

rpm-dist:
	# TODO :)

tgz-dist:
	[ -d $(DISTRIB_TGZ)$(INSTALL_DOC) ] || \
	mkdir -p $(DISTRIB_TGZ)$(INSTALL_DOC)
	cp $(DEVPKG_DOC) $(DISTRIB_TGZ)$(INSTALL_DOC)
	[ -d $(DISTRIB_TGZ)$(INSTALL_SHARE)/perl ] || \
	mkdir -p $(DISTRIB_TGZ)$(INSTALL_SHARE)/perl
	cp $(DEVPKG_PERL) $(DISTRIB_TGZ)$(INSTALL_SHARE)/perl

src-dist:
	mkdir -p $(DISTRIB_SRC)/perl
	cp Makefile $(DEVPKG_PERL) $(DEVPKG_DOC) $(DISTRIB_SRC)/perl
