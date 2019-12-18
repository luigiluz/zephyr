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
#include <gpio.h>

#define SS_PIN 22

/* String que enviarei via SPI */

u8_t byte_enviado = 1;
char myString[] = "S";

struct spi_buf bufs[] = {
	{
		.buf = &byte_enviado,
		.len = sizeof(byte_enviado)
	}
};

const struct spi_buf_set tx = {
	.buffers = bufs,
	.count = 1U,
};

void main(void)
{
	struct device *spi;
	struct spi_config spi_cfg;
	struct device *gpio_ss;		/* GPIO device */

	printk("SPI communication example \n");

	gpio_ss = device_get_binding(DT_NORDIC_NRF_GPIO_0_LABEL);
	if (!gpio_ss) {
		printk("Could not find GPIO driver \n");
		return;
	}
	printk("blablabla1 \n");

	gpio_pin_write(gpio_ss, SS_PIN, 0); /* Returns 0 if successfull */
	printk("blablabla2 \n");
	if (gpio_pin_write(gpio_ss, SS_PIN, 0)) {
		printk("Error in gpio pin write, errno: %d \n", gpio_pin_write(gpio_ss, SS_PIN, 0));
		return;
	}

	spi = device_get_binding(DT_SPI_2_NAME);
	if (!spi) {
		printk("Could not find SPI driver \n");
		return;
	}
	printk("blablabla3 \n");

	spi_cfg.operation = SPI_WORD_SET(8);
	spi_cfg.frequency = 2000000U; /* Posso voltar pra 2000000 depois */

	printk("blablabla4 \n");

	/* Problema provavelmente está na passagem de parâmetros pro spi_write */

	int error_spi = spi_write(spi, &spi_cfg, &tx);

	/* if (error_spi < 0) {
		printk("SPI driver error, errno:%d \n", error_spi);
		return;
	}
	*/

	printk("blablabla5 \n");
}
