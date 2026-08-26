<img align="right" src="https://raw.githubusercontent.com/electux/minihil/master/docs/minihil_logo.png" width="25%">

**minihil** is development device which can be used for **[HWIL](https://en.wikipedia.org/wiki/Hardware-in-the-loop_simulation)**.

The README is used to introduce the tool and provide instructions on
how to install the tool, any machine dependencies it may have and any
other information that should be provided before the tool is installed.

[![minihil C++ Checker](https://github.com/electux/minihil/actions/workflows/minihil_cc_checker.yml/badge.svg)](https://github.com/electux/minihil/actions/workflows/minihil_cc_checker.yml) [![minihil Build Checker (SIL)](https://github.com/electux/minihil/actions/workflows/minihil_build_checker.yml/badge.svg)](https://github.com/electux/minihil/actions/workflows/minihil_build_checker.yml) [![GitHub issues open](https://img.shields.io/github/issues/electux/minihil.svg)](https://github.com/electux/minihil/issues) [![GitHub contributors](https://img.shields.io/github/contributors/electux/minihil.svg)](https://github.com/electux/minihil/graphs/contributors)

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Installation](#installation)
- [Dependencies](#dependencies)
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

### JSON-RPC 2.0 API Specification

You can send JSON-RPC text frames (terminated by `\n`) to port `9000`.

#### `set_relay`
Energize or de-energize a relay channel (1 to 8):
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "set_relay", "params": {"relay_id": 3, "state": true}, "id": 1}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"relay_id": 3, "state": true, "success": true}, "id": 1}
  ```

#### `get_relays`
Query states of all 8 relay channels:
- **Request**:
  ```json
  {"jsonrpc": "2.0", "method": "get_relays", "id": 2}
  ```
- **Response**:
  ```json
  {"jsonrpc": "2.0", "result": {"1": false, "2": false, "3": true, "4": false, "5": false, "6": false, "7": false, "8": false}, "id": 2}
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

<img src="https://raw.githubusercontent.com/electux/minihil/master/docs/foundations.png" alt="Raspberry Pi & GNOME Foundations" width="681" height="100">