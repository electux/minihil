minihil
---------

**minihil** is development device which can be used for HWIL.

The docs introduce the tool and provide instructions on how to install, build, and use the MiniHIL software.

.. toctree::
   :hidden:

   self

|GitHub issues| |Documentation Status| |GitHub contributors|

.. |GitHub issues| image:: https://img.shields.io/github/issues/electux/minihil.svg
   :target: https://github.com/electux/minihil/issues

.. |GitHub contributors| image:: https://img.shields.io/github/contributors/electux/minihil.svg
   :target: https://github.com/electux/minihil/graphs/contributors

.. |Documentation Status| image:: https://readthedocs.org/projects/minihil/badge/?version=latest
   :target: https://minihil.readthedocs.io/projects/minihil/en/latest/?badge=latest

About minihild
--------------

The core of the software is ``minihild`` — a POSIX C++ daemon running as a server on the Raspberry Pi target. It listens for incoming connections on TCP port ``9000`` and parses **JSON-RPC 2.0** commands. It controls the **Waveshare Relay Board (B)** (an 8-channel relay board) via standard Linux character device GPIO controls (``libgpiod`` v2).

It is designed with a decoupled architecture to allow easy expansion (e.g. adding WebSockets) and supports **Software-in-the-Loop (SIL) simulation** so that you can compile and test the server and its API directly on your local developer PC without Raspberry Pi hardware.

SIL (Software-in-the-Loop) Host Build
-------------------------------------

To compile and run the daemon locally on your developer PC in SIL mock mode:

Prerequisites
~~~~~~~~~~~~~
Install the required JSON header libraries:

.. code-block:: bash

   sudo apt-get install nlohmann-json3-dev

Compile and Run
~~~~~~~~~~~~~~~
.. code-block:: bash

   # Configure and compile using CMake
   cmake -B sw/minihil/build -S sw/minihil
   cmake --build sw/minihil/build

   # Start the mock daemon
   ./sw/minihil/build/minihild

The daemon will boot in SIL mode and print:
``[SilRelayController] Software-in-the-Loop simulation initialized.``

Yocto Image Build (Raspberry Pi Target)
---------------------------------------

MiniHIL packages ``minihild`` into a custom Yocto Linux image (``minihil-image``) using ``meta-raspberrypi`` and Poky.

Prerequisites
~~~~~~~~~~~~~
Ensure your build host has all required packages for Yocto Scarthgap (refer to ``sw/rpi-base-platform/README.md`` for the list of packages).

Compile target image
~~~~~~~~~~~~~~~~~~~~
.. code-block:: bash

   # 1. Initialize environment (sources Poky and sets up configs/layers)
   source sw/setup-env.sh

   # 2. Trigger the bitbake build
   bitbake minihil-image

This compiles the C++ application, bundles the systemd daemon config (``minihil.service``) to start automatically on boot, and outputs a flashable image.

JSON-RPC 2.0 API Specification
------------------------------

You can send JSON-RPC text frames (terminated by ``\n``) to port ``9000``.

set_relay
~~~~~~~~~
Energize or de-energize a relay channel (1 to 8):

* **Request**:

  .. code-block:: json

     {"jsonrpc": "2.0", "method": "set_relay", "params": {"relay_id": 3, "state": true}, "id": 1}

* **Response**:

  .. code-block:: json

     {"jsonrpc": "2.0", "result": {"relay_id": 3, "state": true, "success": true}, "id": 1}

get_relays
~~~~~~~~~~
Query states of all 8 relay channels:

* **Request**:

  .. code-block:: json

     {"jsonrpc": "2.0", "method": "get_relays", "id": 2}

* **Response**:

  .. code-block:: json

     {"jsonrpc": "2.0", "result": {"1": false, "2": false, "3": true, "4": false, "5": false, "6": false, "7": false, "8": false}, "id": 2}

Copyright and licence
----------------------

|License: GPL v3| |License: Apache 2.0|

.. |License: GPL v3| image:: https://img.shields.io/badge/License-GPLv3-blue.svg
   :target: https://www.gnu.org/licenses/gpl-3.0

.. |License: Apache 2.0| image:: https://img.shields.io/badge/License-Apache%202.0-blue.svg
   :target: https://opensource.org/licenses/Apache-2.0

Copyright (C) 2020 - 2026 by `electux.github.io/minihil <https://electux.github.io/minihil>`_

.. image:: https://raw.githubusercontent.com/electux/minihil/master/docs/foundations.png
   :alt: Raspberry Pi & GNOME Foundations
   :width: 681px
   :height: 100px

