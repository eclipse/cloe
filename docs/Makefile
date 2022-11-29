# Makefile
#
# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS :=
SPHINXBUILD := $(shell command -v sphinx-build 2> /dev/null)
ifndef SPHINXBUILD
SPHINXBUILD := python3 -m sphinx.cmd.build
endif
SPHINXPROJ := Cloe
SOURCEDIR := .
BUILDDIR := ../build/sphinx

.DEFAULT_GOAL := help
.PHONY: help
.SILENT: help
help:
	$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	echo
	echo "Available serving targets:"
	echo "  serve          to serve the documentation on 0.0.0.0:8000"
	echo "  livehtml       to autobuild and serve the documentation on localhost:8000"

.PHONY: serve
serve:
	cd ${BUILDDIR}/html && python3 -m http.server

.PHONY: livehtml
livehtml:
	sphinx-autobuild --host 0.0.0.0 -b html . ${BUILDDIR}/html

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%:
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
