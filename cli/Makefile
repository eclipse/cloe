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

.DEFAULT_GOAL := help
.PHONY: help
.SILENT: help
help:
	echo "Usage: make <target>"
	echo
	echo "Available targets:"
	echo "  install       to install cloe-launch locally"
	echo "  editable      to install cloe-launch locally in editable mode"
	echo "  export        to export cloe-launch-profile Conan python package"

.PHONY: install
install: export
	command -v ${PIP} >/dev/null 2>&1
	${PIP} install ${PIP_INSTALL_ARGS}

.PHONY: editable
editable: export
	command -v ${PIP} >/dev/null 2>&1
	${PIP} install -e ${PIP_INSTALL_ARGS}

.PHONY: export
export:
	conan export .
