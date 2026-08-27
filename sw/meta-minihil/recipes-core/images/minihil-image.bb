SUMMARY = "Custom MiniHIL Linux Image"
DESCRIPTION = "Production image for MiniHIL platform including BSP support and custom tools."
LICENSE = "MIT"

inherit core-image

# Add core-image-base features to ensure standard BSP capabilities (Wi-Fi, Bluetooth, drivers, serial UART console)
IMAGE_FEATURES += "ssh-server-dropbear"

IMAGE_INSTALL:append = " \
    packagegroup-core-boot \
    bash \
    grep \
    iproute2 \
    curl \
    minihil \
    libgpiod-tools \
    minihil-wifi-config \
    ${CORE_IMAGE_EXTRA_INSTALL} \
"

# Set default root password to 'root' for SSH login
inherit extrausers
EXTRA_USERS_PARAMS = "usermod -p '\$6\$PPKCFZH0Umomf26n\$HNEXabvmmTxiTTElkkKqupvxItpGIbJ9vQoyljK7dCb3XochlJwz5WRQykybUfH.fojSZKnYPLHPZ2aU75AYU0' root;"

# Set rootfs partition size to 1GB (1048576 KB)
IMAGE_ROOTFS_SIZE = "1048576"
