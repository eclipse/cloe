# Usage: make [options] <target>
#

CLOE_ROOT := ..
CLOE_VERSION := $(shell sed -rn "s/^ *\"version\": *\"(.*)\".*$$/\1/p" package.json)

HAS_GIT := $(shell [ -d ${CLOE_ROOT}/.git ] && echo "true")
ifeq (${HAS_GIT},true)
	GIT_COMMIT_HASH  := $(shell git log -1 --format=%h)
	GIT_COMMIT_DIRTY := $(shell git diff --quiet || echo "-dirty")
	GIT_DESCRIBE     := ${GIT_COMMIT_HASH}${GIT_COMMIT_DIRTY}
else
	GIT_DESCRIBE     := "unknown"
endif

DOCKER_NETWORK := \
	$(shell \
		if [ -n $$https_proxy ]; then \
			echo " --build-arg https_proxy=\"$$https_proxy\""; \
			echo " --build-arg http_proxy=\"$$http_proxy\""; \
			echo " --build-arg no_proxy=\"$$no_proxy\""; \
			echo " --network=host"; \
		fi \
	)

IMAGE_BASE      := cloe/cloe-ui
IMAGE_VERSION   := ${CLOE_VERSION}${CLOE_RELEASE}
DOCKER_IMAGE    := ${IMAGE_BASE}:${IMAGE_VERSION}
DOCKER_RUN_ARGS := -p=5000:5000
DOCKER_CONTEXT  := .

CONTAINER_NAME := cloe-ui

.DEFAULT_GOAL : help
.PHONY: help
.SILENT: help
help:
	echo "Usage: make <target>"
	echo
	echo "Available targets:"
	echo "  all        to build the cloe-ui docker image"
	echo "  image      to build the cloe-ui docker image"
	echo "  run        to run the cloe-ui docker image"
	echo
	echo "Configuration:"
	echo "  IMAGE_BASE:      ${IMAGE_BASE}"
	echo "  IMAGE_VERSION:   ${IMAGE_VERSION}"
	echo "  DOCKER_IMAGE:    ${DOCKER_IMAGE}"
	echo "  DOCKER_CONTEXT:  ${DOCKER_CONTEXT}"

.PHONY: all
all: image

.PHONY: image
image:
	docker build ${DOCKER_NETWORK} -t ${DOCKER_IMAGE} ${DOCKER_CONTEXT}
	docker tag ${DOCKER_IMAGE} ${DOCKER_IMAGE}

.PHONY: run
run:
	docker run --rm --name=${CONTAINER_NAME} ${DOCKER_RUN_ARGS} ${DOCKER_IMAGE}
