FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

PATCHTOOL = "git"

SRC_URI_append_h3vc = " \
    file://0064-H3VC-Add-support-for-Renenas-VC-board-series.patch \
    file://0065-H3VC3-Add-I2C5-for-CPLD-access.patch \
    file://0066-H3VC-I2C-Added-Cetitec-driver.patch \
    file://0067-H3VC-I2C-Updated-Makefile.patch \
    file://0068-H3VC-I2C-Updated-KConfig.patch \
    file://0069-H3VC-Fixed-pinmux-for-I2C5.patch \
"
