#!/bin/bash
export ZEPHYR_SDK_INSTALL_DIR=$PWD/zephyr-sdk
west build -p always -b JZ_F407VET6 -d build/$1 -s app_jz407/$1 -- -DBOARD_ROOT=$PWD