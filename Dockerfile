FROM ubuntu:22.04
ARG UID=1000
ARG GID=1000

RUN apt update && \
    apt upgrade -y && \
    apt install  -y --no-install-recommends \
        locales
RUN locale-gen en_US.UTF-8
RUN DEBIAN_FRONTEND="noninteractive" apt install  -y --no-install-recommends \
        sudo \
        gawk \
        wget \
        git \
        diffstat \
        unzip \
        texinfo \
        gcc \
        build-essential \
        chrpath \
        socat \
        cpio \
        python3 \
        python3-pip \
        python3-pexpect \
        xz-utils \
        debianutils \
        iputils-ping \
        python3-git \
        python3-jinja2 \
        python3-subunit \
        zstd \
        liblz4-tool \
        file \
        locales \
        libacl1 \
        make \
        inkscape \
        texlive-latex-extra \
        sphinx \
        python3-saneyaml \
        python3-sphinx-rtd-theme

RUN groupadd -g $GID builder
RUN useradd -m -u $UID -g $GID -s /bin/bash -G sudo builder
USER builder
WORKDIR /home/builder
