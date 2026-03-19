#!/bin/bash
export ZEPHYR_SDK_INSTALL_DIR=$PWD/zephyr-sdk
west flash -d build/$1