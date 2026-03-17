# SPDX-License-Identifier: Apache-2.0

board_runner_args(stm32cubeprogrammer "--port=swd" "--reset-mode=hw")
board_runner_args(openocd 
    "--target-handle=target" 
    "--config=interface/stlink.cfg" 
    "--config=target/stm32f4x.cfg"
    "-c" "transport select hla_swd"
    "-c" "reset_config srst_only srst_nogate connect_assert_srst"
)
board_runner_args(jlink "--device=STM32F407VE" "--speed=4000" "--reset-after-load")


include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/blackmagicprobe.board.cmake)
