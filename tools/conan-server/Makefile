# Include .env file for the following variables:
#
#   USER
#   VOLUME
#   VOLUME_GID
#   VOLUME_GROUP
#
ifeq ($(wildcard .env),.env)
include .env
endif

USER := $(shell id -un)

.PHONY: help update add-group
help:
	@echo "Usage: make <target>"
	@echo "Available targets:"
	@echo "  update     update permissions of files"
	@echo "  add-group  add ${VOLUME_GROUP} for handling data"

update: ${VOLUME_DIR} server.conf
	chown -R :${VOLUME_GID} $^
	for file in $^; do                                 \
		if [ -d $$file ]; then                         \
			find $$file -type d -exec chmod 775 {} + ; \
			find $$file -type f -exec chmod 664 {} + ; \
		else                                           \
			chmod 664 "$$file" ;                       \
		fi;                                            \
	done
	chmod g+s $<

add-group:
	addgroup --gid ${VOLUME_GID} ${VOLUME_GROUP}
	adduser ${USER} ${VOLUME}
