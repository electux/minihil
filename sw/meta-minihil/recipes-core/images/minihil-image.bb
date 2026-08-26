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
    ${CORE_IMAGE_EXTRA_INSTALL} \
"
