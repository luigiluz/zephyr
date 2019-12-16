/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <flash.h>
#include <device.h>
#include <stdio.h>
#include <spi.h>

/* String que enviarei via SPI */

char myString[] = "S";

struct spi_buf bufs[] = {
	{
		.buf = myString,
		.len = sizeof(myString)
	}
};

struct spi_buf_set tx = {
	.buffers = bufs
};

void main(void)
{
	struct device *spi;
	struct spi_config spi_cfg;

	printk("SPI communication example \n");

	spi = device_get_binding(DT_SPI_2_NAME);
	if (!spi) {
		printk("Could not find SPI driver \n");
		return;
	}
	spi_cfg.operation = SPI_WORD_SET(8);
	spi_cfg.frequency = 256000U;
	spi_cfg.slave = 0x02;

	spi_write(spi, &spi_cfg, &tx);
}
