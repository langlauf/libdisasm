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

LIBPKG_DOC	= README.TXT 
DEVPKG_DOC	= libdisasm.txt
SIG_DOC		= libsignatures.txt
GRAPH_DOC	= libgraph.txt
MAGIC_DOC	= libMAGIC.txt

all: 

dummy: install dist deb-dist rpm-dist tgz-dist src-dist

install:
	[ -d $(INSTALL_DOC) ] || mkdir -p $(INSTALL_DOC)
	cp $(LIBPKG_DOC) $(DEVPKG_DOC) $(INSTALL_DOC)
	#cp $(SIG_DOC) $(INSTALL_DOC)
	#cp $(MAGIC_DOC) $(INSTALL_DOC)


dist: deb-dist rpm-dist src-dist tgz-dist

deb-dist:
	# bin pkg manuals
	[ -d $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_DOC) ] || \
	mkdir -p $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_DOC)
	cp $(LIBPKG_DOC) $(DISTRIB_DEB)$(BIN_PKG)$(INSTALL_DOC)
	# dev pkg manuals
	[ -d $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_DOC) ] || \
	mkdir -p $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_DOC)
	cp $(DEVPKG_DOC) $(DISTRIB_DEB)$(DEV_PKG)$(INSTALL_DOC)

rpm-dist:
	# TODO :)

tgz-dist:
	[ -d $(DISTRIB_TGZ)$(INSTALL_DOC) ] || \
	mkdir -p $(DISTRIB_TGZ)$(INSTALL_DOC)
	cp $(LIBPKG_DOC) $(DEVPKG_DOC) $(DISTRIB_TGZ)$(INSTALL_DOC)

src-dist:
	mkdir -p $(DISTRIB_SRC)/doc
	cp Makefile $(LIBPKG_DOC) $(DEVPKG_DOC) $(DISTRIB_SRC)/doc
