# If enabled, the Hyperflash will be accessible from Linux as
# /dev/mtdblock*
ENABLE_HYPERFLASH_LINUX = "${@'1' if 'hyperflash-linux' in '${DISTRO_FEATURES}' else '0'}"
