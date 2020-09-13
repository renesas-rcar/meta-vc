require recipes-graphics/images/core-image-renesas-base.inc
require recipes-core/images/core-image-minimal.bb

COMPATIBLE_MACHINE = "h3vc"

IMAGE_INSTALL_append = " usbutils "
IMAGE_INSTALL_append = " gdb gdbserver openssh openssh-sftp openssh-sftp-server"

IMAGE_INSTALL_append += " sllin phytool inic-flasher wl18xx-fw"

# RSwitch 1 releated tools
IMAGE_INSTALL_append += " vc-build-info rswitch1tool rswitch1-init snott rlatency ptp4lrswitch gptp-master rexecd vc-addon-script flash-fpga"

# Wifi Support
IMAGE_INSTALL_append += " ti-bt-firmware wl18xx-calibrator wlconf wl18xx-target-scripts iw wireless-tools wpa-supplicant hostapd"

# Networking tools
IMAGE_INSTALL_append += " ethtool bridge-utils vlan netcat tcpdump net-tools"

# Development tools
IMAGE_INSTALL_append += " devdump nano git subversion python3 python-numpy python3-numpy iftop htop strace e2fsprogs-mke2fs i2c-tools util-linux procps pciutils dosfstools"

# Flash tool
PREFERRED_VERSION_flashrom = "1.2"
IMAGE_INSTALL_append += " flashrom"

# Install modules into RFS
CORE_IMAGE_EXTRA_INSTALL += " kernel-modules"

# Install kernel and DTB
IMAGE_INSTALL_append += " kernel-image kernel-devicetree"

