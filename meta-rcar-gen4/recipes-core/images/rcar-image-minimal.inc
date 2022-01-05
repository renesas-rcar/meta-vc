IMAGE_INSTALL_append = " gdb gdbserver openssh openssh-sftp openssh-sftp-server "

IMAGE_INSTALL_append = " phytool "

# Networking tools
IMAGE_INSTALL_append = " \
    ethtool \
    bridge-utils \
    vlan \
    netcat \
    tcpdump \
    net-tools \
    iputils \
    iproute2 \
"

# Development tools
IMAGE_INSTALL_append = " \
    devdump \
    nano \
    git \
    subversion \
    python3 \
    python3-numpy \
    iftop \
    htop \
    strace \
    e2fsprogs-mke2fs \
    i2c-tools \
    util-linux \
    procps \
    pciutils \
    dosfstools \
    can-utils \
    mtd-utils \
"

