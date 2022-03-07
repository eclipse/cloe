# Makefile
#
# Minimal makefile for Sphinx documentation
#
PIPX := $(shell command -v pipx 2>/dev/null)
PIPX_INSTALL_ARGS :=

PIP := $(or \
	$(shell command -v pipx 2>/dev/null), \
	$(shell command -v pip3 2>/dev/null), \
	$(shell command -v pip 2>/dev/null) \
)
ifndef PIPX
PIP_INSTALL_ARGS := --upgrade --user
else
PIP_INSTALL_ARGS := --force ${PIPX_INSTALL_ARGS}
endif

# You can set these variables from the command line.
SPHINXOPTS :=
SPHINXBUILD := $(shell command -v sphinx-build 2> /dev/null)
ifndef SPHINXBUILD
SPHINXBUILD := python3 -m sphinx.cmd.build
endif
SPHINXPROJ := Cloe
SOURCEDIR := .
BUILDDIR := ../build/sphinx

.PHONY: help
.DEFAULT: help
.SILENT: help

# Put it first so that "make" without argument is like "make help".
help:
	$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O) \
	|| echo "\nYou may try:\n  $$ make install-sphinx-deps"
	echo
	echo "Available serving targets:"
	echo "  serve          to serve the documentation on 0.0.0.0:8000"
	echo "  livehtml       to autobuild and serve the documentation on localhost:8000"
	echo
	echo "Available setup targets:"
	echo "  install-sphinx-deps    to install dependencies for sphinx"

.PHONY: install-sphinx-deps serve livehtml

SPHINXDEPS := \
	sphinx_rtd_theme \
	sphinxcontrib-runcmd \
	sphinxcontrib-plantuml \
	sphinxcontrib.asciinema \
	jsx-lexer
install-sphinx-deps:
	${PIP} install ${PIP_INSTALL_ARGS} sphinx
	[ -n "${PIPX}" ] && ${PIP} inject sphinx ${SPHINXDEPS} \
	|| ${PIP} install ${PIP_INSTALL_ARGS} ${SPHINXDEPS}
	${PIP} install ${PIP_INSTALL_ARGS} sphinx-autobuild
	[ -n "${PIPX}" ] && ${PIP} inject sphinx-autobuild ${SPHINXDEPS}|| exit 0

serve:
	cd ${BUILDDIR}/html && python3 -m http.server

livehtml:
	sphinx-autobuild --host 0.0.0.0 -b html . ${BUILDDIR}/html

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%:
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
