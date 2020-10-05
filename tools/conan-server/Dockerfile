FROM python:alpine

ARG VOLUME_GID
ARG VOLUME_GROUP
RUN addgroup --gid ${VOLUME_GID} ${VOLUME_GROUP}

ARG CONAN_HOME
ARG CONAN_USER=conan
RUN adduser -S -h ${CONAN_HOME} -G ${VOLUME_GROUP} -s /bin/sh ${CONAN_USER}

ARG CONAN_VERSION
RUN pip install --no-cache-dir "conan==${CONAN_VERSION}"

COPY ./entrypoint.sh /entrypoint.sh

USER conan
EXPOSE 9300

CMD ["/bin/sh", "entrypoint.sh"]
