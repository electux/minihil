<img align="right" src="https://raw.githubusercontent.com/electux/minihil/master/docs/minihil_logo.png" width="25%">

**minihil** is development device which can be used for **[HWIL](https://en.wikipedia.org/wiki/Hardware-in-the-loop_simulation)**.

The README is used to introduce the tool and provide instructions on
how to install the tool, any machine dependencies it may have and any
other information that should be provided before the tool is installed.

[![minihil C++ Checker](https://github.com/electux/minihil/actions/workflows/minihil_cc_checker.yml/badge.svg)](https://github.com/electux/minihil/actions/workflows/minihil_cc_checker.yml) [![minihil Build Checker (SIL)](https://github.com/electux/minihil/actions/workflows/minihil_build_checker.yml/badge.svg)](https://github.com/electux/minihil/actions/workflows/minihil_build_checker.yml) [![minihil TOC](https://github.com/electux/minihil/actions/workflows/minihil_toc.yml/badge.svg)](https://github.com/electux/minihil/actions/workflows/minihil_toc.yml)
[![minihildesk C++ Checker](https://github.com/electux/minihil/actions/workflows/minihildesk_cc_checker.yml/badge.svg)](https://github.com/electux/minihil/actions/workflows/minihildesk_cc_checker.yml) [![minihildesk Build Checker (SIL)](https://github.com/electux/minihil/actions/workflows/minihildesk_build_checker.yml/badge.svg)](https://github.com/electux/minihil/actions/workflows/minihildesk_build_checker.yml)
[![GitHub issues open](https://img.shields.io/github/issues/electux/minihil.svg)](https://github.com/electux/minihil/issues) [![GitHub contributors](https://img.shields.io/github/contributors/electux/minihil.svg)](https://github.com/electux/minihil/graphs/contributors)

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [About minihild](#about-minihild)
- [SIL (Software-in-the-Loop) Host Build](#sil-software-in-the-loop-host-build)
  - [Prerequisites](#prerequisites)
  - [Compile and Run](#compile-and-run)
- [minihildesk GUI Client](#minihildesk-gui-client)
- [Yocto Image Build (Raspberry Pi Target)](#yocto-image-build-raspberry-pi-target)
  - [Prerequisites](#prerequisites-1)
  - [Compile target image](#compile-target-image)
- [Flashing the Image to SD Card](#flashing-the-image-to-sd-card)
  - [Option A: Using `bmaptool` (Recommended - Fast & Direct)](#option-a-using-bmaptool-recommended---fast--direct)
  - [Option B: Using `dd` (Piped Decompression)](#option-b-using-dd-piped-decompression)
- [Running on Raspberry Pi 3B+](#running-on-raspberry-pi-3b)
- [JSON-RPC 2.0 API Specification](#json-rpc-20-api-specification)
  - [`set_relay`](#set_relay)
  - [`get_relays`](#get_relays)
- [Docs](#docs)
- [Copyright and licence](#copyright-and-licence)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

### About minihild

The core of the software is `minihild` — a POSIX C++ daemon running as a server on the Raspberry Pi target. It listens for incoming connections on TCP port `9000` and parses **JSON-RPC 2.0** commands. It controls the **Waveshare Relay Board (B)** (an 8-channel relay board) via standard Linux character device GPIO controls (`libgpiod` v2).

It is designed with a decoupled architecture to allow easy expansion (e.g. adding WebSockets) and supports **Software-in-the-Loop (SIL) simulation** so that you can compile and test the server and its API directly on your local developer PC without Raspberry Pi hardware.

---

### SIL (Software-in-the-Loop) Host Build

To compile and run the daemon locally on your developer PC in SIL mock mode:

#### Prerequisites
Install the required JSON header libraries:
```bash
sudo apt-get install nlohmann-json3-dev
```

#### Compile and Run
```bash
# Configure and compile using CMake
cmake -B sw/minihil/build -S sw/minihil
cmake --build sw/minihil/build

# Start the mock daemon
./sw/minihil/build/minihild
```
The daemon will boot in SIL mode and print:
`[SilRelayController] Software-in-the-Loop simulation initialized.`

---

### minihildesk GUI Client

**minihildesk** is a premium GTKmm-based C++ desktop GUI client designed to connect to the `minihild` server (either running on a Raspberry Pi target or locally in SIL mock mode). 

It features:
- A connection bar supporting standard TCP connection, **SSL/TLS secure connection**, and **Mutual TLS (mTLS) client verification**.
- A grid of 8 Relay Control Cards displaying status via glowing LED indicators and active green borders.
- Multi-mode relay control per channel: **Toggle** (manual switch), **Timer** (seconds spin-input), **Pulse** (milliseconds spin-input up to 100,000 ms), and **Blink** (ON/OFF ms inputs and cycle count).
- Automatic hardware safety auto-off logic: switching modes on an active channel immediately turns the relay OFF on the server before entering the new mode.
- A monospace green log terminal at the bottom showcasing outgoing and incoming JSON-RPC traffic.

#### Prerequisites
Install the GTKmm-4.0 development headers, OpenSSL, and JSON library:
```bash
sudo apt-get install libgtkmm-4.0-dev libssl-dev nlohmann-json3-dev
```

#### Compile and Run
```bash
# Configure and compile using CMake
cmake -B sw/minihildesk/build -S sw/minihildesk
cmake --build sw/minihildesk/build

# Start the desktop GUI app
./sw/minihildesk/build/minihildesk
```

---

### Yocto Image Build (Raspberry Pi Target)

MiniHIL packages `minihild` into a custom Yocto Linux image (`minihil-image`) using `meta-raspberrypi` and Poky.

#### Prerequisites
Ensure your build host has all required packages for Yocto Scarthgap (refer to [rpi-base-platform/README.md](file:///data/dev/raspberry/minihil/github/minihil/sw/rpi-base-platform/README.md) for the list of packages).

#### Compile target image
```bash
# 1. Initialize environment (sources Poky and sets up configs/layers)
source sw/setup-env.sh

# 2. Trigger the bitbake build
bitbake minihil-image
```
This compiles the C++ application, bundles the systemd daemon config (`minihil.service`) to start automatically on boot, and outputs a flashable image.

---

### Flashing the Image to SD Card

The Yocto build generates a flashable `.wic.bz2` image in the deployment directory:
`sw/build-minihil/tmp/deploy/images/raspberrypi3-64/minihil-image-raspberrypi3-64.rootfs.wic.bz2`

Identify your SD card's device name (e.g. `/dev/sdX` or `/dev/mmcblkX`) using `lsblk` or `dmesg`.

> [!WARNING]
> Double-check the target device name before flashing! Writing to the wrong disk can destroy data on your host system.

Before flashing, ensure that all partitions on the target SD card are unmounted (otherwise you will get a "Device or resource busy" error):
```bash
sudo umount /dev/sdX* 2>/dev/null || true
```

#### Option A: Using `bmaptool` (Recommended - Fast & Direct)
`bmaptool` natively supports compressed images and will automatically decompress the Yocto symlink on-the-fly.
```bash
# Flash directly (replace /dev/sdX with your SD card device)
sudo bmaptool copy sw/build-minihil/tmp/deploy/images/raspberrypi3-64/minihil-image-raspberrypi3-64.rootfs.wic.bz2 /dev/sdX
```

#### Option B: Using `dd` (Piped Decompression)
To avoid modifying the Yocto symlink files and save host disk space, decompress the image on-the-fly and pipe it directly to `dd`:
```bash
# Decompress on-the-fly and flash (replace /dev/sdX with your SD card device)
bzcat sw/build-minihil/tmp/deploy/images/raspberrypi3-64/minihil-image-raspberrypi3-64.rootfs.wic.bz2 | sudo dd of=/dev/sdX bs=4M status=progress conv=fsync
```

---

### Running on Raspberry Pi 3B+

1. **Boot the board**: Insert the flashed SD card into your Raspberry Pi 3B+ and power it on.
2. **Access the console**:
   - **Via SSH**: Connect as `root` using the default password `root`:
     ```bash
     ssh root@<rpi-ip-address>
     ```
   - **Via Serial UART**: Connect a USB-to-UART adapter to the Raspberry Pi GPIO header (TX on pin 8, RX on pin 10, GND on pin 6). Access the serial interface using `picocom` or `minicom` (enabled with `115200` baud rate by `ENABLE_UART = "1"` in `local.conf`):
     ```bash
     picocom -b 115200 /dev/ttyUSB0
     ```
3. **Verify the Daemon**: Check that the `minihil` server starts automatically on boot:
   ```bash
   systemctl status minihil
   ```
4. **Test the JSON-RPC interface locally**: Send a command to port 9000 to query the relay states:
   ```bash
   echo '{"jsonrpc": "2.0", "method": "get_relays", "id": 1}' | nc localhost 9000
   ```

---

### JSON-RPC 2.0 API Specification

You can send JSON-RPC text frames (terminated by `\n`) to port `9000`.

#### `set_relay`
Energize or de-energize a relay channel manually (1 to 8):
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "set_relay", "params": {"relay_id": 3, "state": true}, "id": 1}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"relay_id": 3, "state": true, "success": true}, "id": 1}
  ```

#### `get_relays`
Query immediate physical states of all 8 relay channels:
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "get_relays", "id": 2}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"1": false, "2": false, "3": true, "4": false, "5": false, "6": false, "7": false, "8": false}, "id": 2}
  ```

#### `start_timer`
Keep a relay active for a specific duration in seconds:
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "start_timer", "params": {"relay_id": 2, "seconds": 10}, "id": 3}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"relay_id": 2, "seconds": 10, "success": true}, "id": 3}
  ```

#### `start_pulse`
Generate a single momentary pulse in milliseconds (up to 100,000 ms):
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "start_pulse", "params": {"relay_id": 3, "duration_ms": 5000}, "id": 4}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"relay_id": 3, "duration_ms": 5000, "success": true}, "id": 4}
  ```

#### `start_blink`
Repeatedly cycle relay state ON and OFF:
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "start_blink", "params": {"relay_id": 4, "on_ms": 1000, "off_ms": 1000, "count": 5}, "id": 5}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"relay_id": 4, "on_ms": 1000, "off_ms": 1000, "count": 5, "success": true}, "id": 5}
  ```

#### `get_relay_status`
Query detailed diagnostic status and remaining time for a single channel:
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "get_relay_status", "params": {"relay_id": 2}, "id": 6}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"relay_id": 2, "status": "Channel 2: ON (Timer, rem: 8s)"}, "id": 6}
  ```

### Docs

[![Documentation Status](https://readthedocs.org/projects/minihil/badge/?version=latest)](https://minihil.readthedocs.io/projects/minihil/en/latest/?badge=latest)

More documentation and info at
* [https://minihil.readthedocs.io/en/latest/](https://minihil.readthedocs.io/en/latest/)
* [https://www.gnome.org](https://www.gnome.org/)

### Copyright and licence

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Copyright (C) 2020 - 2026 by [electux.github.io/minihil](https://electux.github.io/minihil)

**minihil** is free software; you can redistribute it and/or modify it.

Lets help and support Raspberry PI && GNOME.

<img src="https://raw.githubusercontent.com/electux/minihil/master/docs/foundations.png" alt="Foundations">