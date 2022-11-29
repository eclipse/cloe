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

.PHONY: install
install: conan-profile
	command -v ${PIP} >/dev/null 2>&1
	# Work-around pip confused by pyproject.toml
	-mv pyproject.toml pyproject.toml.bak
	${PIP} install ${PIP_INSTALL_ARGS} . || ( \
		mv pyproject.toml.bak pyproject.toml; \
		exit 1; \
	)
	mv pyproject.toml.bak pyproject.toml

.PHONY: editable
editable: conan-profile
	command -v ${PIP} >/dev/null 2>&1
	# Work-around pip confused by pyproject.toml
	-mv pyproject.toml pyproject.toml.bak
	${PIP} install -e ${PIP_INSTALL_ARGS} . || ( \
		mv pyproject.toml.bak pyproject.toml; \
		exit 1; \
	)
	mv pyproject.toml.bak pyproject.toml

.PHONY: conan-profile
conan-profile:
	conan export .
