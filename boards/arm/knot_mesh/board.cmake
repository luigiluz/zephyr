#
# Code based on nrf52840_pca10059.
# Base commit: 3ae52624ffa129dfd1b0f5dc337afd1ddd6c4c1c
# Edited by KNoT contributors.
#

# Copyright (c) 2019 CESAR
#
# SPDX-License-Identifier: Apache-2.0

board_runner_args(nrfjprog "--nrf-family=NRF52")
board_runner_args(jlink "--device=nrf52" "--speed=4000")
board_runner_args(pyocd "--target=nrf52840")
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
