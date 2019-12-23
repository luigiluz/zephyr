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
#define GPIO_TESTE 23

/* String que enviarei via SPI */

bool led = false;
u8_t byte_enviado = 1;
char myString[] = "S";

void main(void)
{
	struct device *gpio_ss;		        /* GPIO device */
	struct device *spi;                     /* SPI device */
	struct spi_cs_control cs_control;
	struct spi_config spi_cfg;
	const struct spi_buf bufs[] = {
		{
			.buf = &byte_enviado,
			.len = sizeof(byte_enviado),
		},
	};
	const struct spi_buf_set tx_buffs = {
		.buffers = bufs,
		.count = ARRAY_SIZE(bufs),
	};

	printk("SPI communication example \n");

	gpio_ss = device_get_binding(DT_NORDIC_NRF_SPI_40023000_CS_GPIOS_CONTROLLER_0);
	if (!gpio_ss) {
		printk("Could not find GPIO driver \n");
		return;
	}

	printk("Device GPIO configurado \n");

	// int gpio_wr = gpio_pin_write(gpio_ss, SS_PIN, led);  Returns 0 if successfull

	// if (gpio_wr) {
	// 	printk("Error in gpio pin write, errno: %d \n", gpio_wr);
	// 	return;
	// }


	printk("Consegui escrever nivel logico baixo no GPIO \n");

	spi = device_get_binding(DT_SPI_2_NAME);
	if (!spi) {
		printk("Could not find SPI driver \n");
		return;
	}

	printk("Device SPI configurado \n");

	cs_control.gpio_dev = gpio_ss;
	cs_control.gpio_pin = DT_NORDIC_NRF_SPI_40023000_CS_GPIOS_PIN_0;
	cs_control.delay = 0;
	gpio_pin_configure(gpio_ss, DT_NORDIC_NRF_SPI_40023000_CS_GPIOS_PIN_0, GPIO_DIR_OUT);

	printk("Estabeleci as configuracoes do CS \n");

	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8);
	spi_cfg.frequency = 2000000U;
	spi_cfg.slave = 0;
	spi_cfg.cs = &cs_control;

	printk("Estabeleci os parametros do spi_cfg \n");

	// /* Problema provavelmente está na passagem de parâmetros pro spi_write */
	while(1){
		gpio_pin_write(gpio_ss, DT_NORDIC_NRF_SPI_40023000_CS_GPIOS_PIN_0, led);
		int error_spi = spi_write(spi, &spi_cfg, &tx_buffs);
		if (error_spi < 0) {
			printk("SPI driver error, errno:%d \n", error_spi);
			return;
		}

		printk("Mensagem enviada por SPI: %d \n", byte_enviado);
		k_sleep(1000);
	}

}
