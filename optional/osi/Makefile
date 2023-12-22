PROJECT_ROOT := ../..
include ${PROJECT_ROOT}/Makefile.package

# -------------------------------------------------

ALL_VENDOR := $(wildcard vendor/*)
.PHONY: ${ALL_VENDOR}

WITHOUT_VENDOR :=
UNSELECT_VENDOR := ${WITHOUT_VENDOR}
WITH_VENDOR :=
SELECT_VENDOR := $(call uniq, $(filter-out ${UNSELECT_VENDOR}, ${ALL_VENDOR}) ${WITH_VENDOR})

vendor/open-simulation-interface-3.3.1: vendor/protoc

REGEX_TARGET := 's/(-vendor|-select)?-each//'
${SELECT_VENDOR}:
	${MAKE} -C $@ $(shell echo ${MAKECMDGOALS} | sed -re ${REGEX_TARGET})

# Usage: $(call _make_target_rule, TARGET-NAME, MAKE-TARGET, HELP-DESCRIPTION, MAKE-ARGUMENTS)
define _make_target_rule
${1}:
	$(call print_header, "Proceeding to $(call unquote, ${3})")
	${MAKE} ${SUBMAKEFLAGS} PROJECT_ROOT=$(realpath ${PROJECT_ROOT}) ${4} ${2}
endef

# Usage: $(call _make_target_rules, TARGET-NAME, HELP-DESCRIPTION, HELP-CATEGORY, PACKAGE-DIRS)
define _make_target_rules
help::
	$(call print_help_target, ${1}, ${2}, ${3})
$(call _make_target_rule,${1},${1}-each,${2})
${1}-each: ${4}
endef

# Usage: $(call make_vendor_target, TARGET-NAME, HELP-DESCRIPTION, HELP-CATEGORY)
define make_vendor_target
$(eval $(call _make_target_rules,${1},${2},${3},${SELECT_VENDOR}))
endef

help::
	$(call print_help_section, "Available vendor targets")

$(call make_vendor_target, export-vendor, "export all vendor packages", "[conan-cache]")
$(call make_vendor_target, package-vendor, "create all vendor packages", "[conan-cache]")
$(call make_vendor_target, download-vendor, "download or build vendor packages", "[conan-cache]")
