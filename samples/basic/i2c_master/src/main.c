#include <zephyr.h>
#include <device.h>
#include <gpio.h>
#include <i2c.h>

#define I2C_SLV_ADDR 0X08 /* Defines i2c slave address number */

void main(void)
{
	struct device *i2c_dev;

	printk("Executing main function \n");

	char myString[] = "I2C está funcionando! \n";
	char stringReceived[20];

	i2c_dev = device_get_binding(DT_NORDIC_NRF_I2C_0_LABEL);
	if (!i2c_dev) {
		printk("I2C: Device driver not found.\n");
		return;
	}

	int64_t last_toggle_time = 0;

	while (1) {
		/* Get current time */
		int64_t current_time = k_uptime_get();

		/* Sends/receives messages every 5 seconds */
		if (current_time - last_toggle_time > 5000) {
			i2c_write(i2c_dev, myString, sizeof(myString), I2C_SLV_ADDR);
			/* i2c_read(i2c_dev, stringReceived, sizeof(stringReceived) , I2C_SLV_ADDR); */
			/* printk("Mensagem recebida foi: %s \n", stringReceived); */
			last_toggle_time = current_time;
		}

	}
}
