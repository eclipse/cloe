# Usage: make [options] <target>
#

VERSION := $(shell sed -rn "s/^ *\"version\": *\"(.*)\".*$$/\1/p" package.json)

DOCKER_NETWORK := \
	$(shell \
		if [ -n $$https_proxy ]; then \
			echo " --build-arg https_proxy=\"$$https_proxy\""; \
			echo " --build-arg http_proxy=\"$$http_proxy\""; \
			echo " --build-arg no_proxy=\"$$no_proxy\""; \
			echo " --network=host"; \
		fi \
	)

IMAGE_BASE      := cloe/cloe-ui-webserver
IMAGE_VERSION   := ${VERSION}
DOCKER_IMAGE := ${IMAGE_BASE}:${IMAGE_VERSION}
DOCKER_CONTEXT  := .

CONTAINER_NAME  := cloe-ui-webserver
override DOCKER_RUN_ARGS += -p=4000:4000
MOUNT_DIRECTORY := $$HOME/.local/share/cloe/replay_data
override DOCKER_RUN_ARGS += -v=${MOUNT_DIRECTORY}:/app/replay_data

.PHONY: help
.DEFAULT: help
help:
	@echo "Usage: make <target>"
	@echo
	@echo "Available targets:"
	@echo "  all        to build the cloe-ui-webserver docker image"
	@echo "  image      to build the cloe-ui-webserver docker image"
	@echo "  run        to run the cloe-ui-webserver docker image"
	@echo
	@echo "Configuration:"
	@echo "  IMAGE_BASE:      ${IMAGE_BASE}"
	@echo "  IMAGE_VERSION:   ${IMAGE_VERSION}"
	@echo "  DOCKER_IMAGE:    ${DOCKER_IMAGE}"
	@echo "  DOCKER_CONTEXT:  ${DOCKER_CONTEXT}"
	@echo "  MOUNT_DIRECTORY: ${MOUNT_DIRECTORY}"

.PHONY: all image run
all: image

image:
	docker build ${DOCKER_NETWORK} -t ${DOCKER_IMAGE} ${DOCKER_CONTEXT}

run:
	docker run --rm --name=${CONTAINER_NAME} ${DOCKER_RUN_ARGS} ${DOCKER_IMAGE}
