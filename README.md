# JZ-F407VET6 Zephyr
Zephyr RTOS on the JZ-F407VET6 development board (STM32F407).

I based this repository on:

* [JZ-F407VET6](https://github.com/art103/JZ-F407VET6)
* [zenoh-pico-zephyr](https://github.com/entire/zenoh-pico-zephyr)
* [zephyr_driver_propyradio](https://github.com/everedero/zephyr_driver_propyradio)

And of course, the samples from Zephyr project!.

## Overview

* Zephyr v4.3.0
* Hardware: STM32F407VET6 (Cortex-M4)

## Prerequisites (Ubuntu and similars)
### System Dependencies 
Install the essential build tools and libraries required by the Zephyr environment.
```bash
sudo apt update
sudo apt install --no-install-recommends git cmake ninja-build gperf
ccache dfu-util device-tree-compiler wget
python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file
make gcc libsdl2-dev libmagic1
```

### Zephyr SDK
The SDK contains the toolchains needed to cross-compile.
```bash
west sdk install --install-dir $PWD/zephyr-sdk -t arm-zephyr-eabi
```

Note: If your board is not recognized by the programmer (ST-Link/J-Link), ensure your udev rules are up to date:

```bash
sudo wget https://raw.githubusercontent.com/openocd-org/openocd/master/contrib/60-openocd.rules -P /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

## Environment Setup
I used west, Zephyr's meta-tool, within a Python virtual environment to keep my system clean.

### Initialize Virtual Environment

```bash
python3 -m venv .venv
source .venv/bin/activate
```

**From now on, all other terminals must initialize the Python environment.**

### Install West and project requirements

```bash
pip install west
pip install -r zephyr/scripts/requirements.txt
```

## Building and Flashing
This repository is structured with a custom board root. Follow these steps to build the application.

### Build some application

Change \<APP_FOLDER\> for the correct one.

```bash
./build.sh <APP_FOLDER>
```
### Flash the Hardware

Connect your programmer and run:

```bash
./flash.sh <APP_FOLDER>
```

## Troubleshooting

* West Not Found: Ensure your virtual environment is active (source .venv/bin/activate).

* Flash Errors: Check if your board is powered and the ST-Link is correctly wired to the SWD pins.
