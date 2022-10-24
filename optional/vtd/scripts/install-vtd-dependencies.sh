#!/bin/bash
set -e

if [[ $(id -u) != "0" ]]; then
  echo "This script requires to be run as root. Restarting with sudo -E"
  exec sudo -E $0 $@
fi

if tty -s; then
    echo "Install VTD dependencies (uses root privileges)? [Y/n]"
    read -r -n1 answer
    if [[ ${answer,,} == 'n' ]]; then
        exit 1
    fi
fi

apt-get update
apt-get install --no-install-recommends -y \
    freeglut3 \
    libaa1 \
    libasn1-8-heimdal \
    libasound2 \
    libasyncns0 \
    libatk1.0-0 \
    libaudio2 \
    libcaca0 \
    libcairo2 \
    libdatrie1 \
    libdv4 \
    libdvdnav4 \
    libdvdread4 \
    libegl1-mesa \
    libflac8 \
    libfontconfig1 \
    libfreetype6 \
    libgdk-pixbuf2.0-0 \
    libgl1-mesa-glx \
    libglib2.0-0 \
    libglib2.0-dev \
    libglu1-mesa \
    libgpm2 \
    libgraphite2-3 \
    libgsm1 \
    libgssapi3-heimdal \
    libgtk2.0-0 \
    libharfbuzz0b \
    libhcrypto4-heimdal \
    libheimbase1-heimdal \
    libheimntlm0-heimdal \
    libhx509-5-heimdal \
    libice6 \
    libjack-jackd2-0 \
    libjpeg-turbo8 \
    libkrb5-26-heimdal \
    libldap-2.4-2 \
    libldb1 \
    liblzo2-2 \
    libmp3lame0 \
    libntdb1 \
    libogg0 \
    liborc-0.4-0 \
    libpango-1.0-0 \
    libpangocairo-1.0-0 \
    libpangoft2-1.0-0 \
    libpixman-1-0 \
    libpulse0 \
    libqt4-network \
    libqt4-qt3support \
    libqt4-script \
    libqt4-sql \
    libqt4-svg \
    libqt4-xml \
    libqt4-xmlpatterns \
    libqtcore4 \
    libqtgui4 \
    libroken18-heimdal \
    libsasl2-2 \
    libsm6 \
    libsmbclient \
    libsndfile1 \
    libspeex1 \
    libtalloc2 \
    libtdb1 \
    libtevent0 \
    libthai0 \
    libtheora0 \
    libusb-0.1-4 \
    libvorbis0a \
    libvorbisenc2 \
    libwbclient0 \
    libwind0-heimdal \
    libwrap0 \
    libx11-6 \
    libxau6 \
    libxcb1 \
    libxcb-render0 \
    libxcb-shm0 \
    libxcomposite1 \
    libxcursor1 \
    libxdamage1 \
    libxdmcp6 \
    libxext6 \
    libxfixes3 \
    libxi6 \
    libxinerama1 \
    libxrandr2 \
    libxrender1 \
    libxss1 \
    libxt6 \
    libxv1 \
    libxvidcore4 \
    libxvmc1 \
    libxxf86dga1 \
    libxxf86vm1 \
    samba-libs \
    netcat \
    tcsh \
    xterm
