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

# Set default root password to 'minihil' for SSH login
inherit extrausers
EXTRA_USERS_PARAMS = "usermod -p '\$6\$Zgq4tqK8KIuS.KdH\$fD6cUaHSJJCO2DnAd0AyLAevMpimJRgUSxsw4PV30uahLVzpW2ueGvSXxAOmhkoGw6s/soOHR27rv3v2x.Fg8/' root;"

# Set rootfs partition size to 1GB (1048576 KB)
IMAGE_ROOTFS_SIZE = "1048576"
