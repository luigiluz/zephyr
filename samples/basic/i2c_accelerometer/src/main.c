#include <zephyr.h>
#include <device.h>
#include <gpio.h>
#include <i2c.h>

#include "mpu6050.h"

/* I2C slave address */
#define MPU_ADDRESS 0X68 /* Defines MPU 6050 I2C address */

/* MPU6050 internal registers */
#define PWR_MGMT_1_REG 0X6B /* Power management register address */

#define FS_SEL 0x00 /* Set Full Scale Range to +- 2g */
#define AFS_SEL 0x00 /* Set Full Scal Range to 250deg/seg */

/* Approximation of gravity acceleration */
#define GRAVITY_ACC 10

/* Full scale range of gyroscope*/
#define GYRO_FS_RANGE 250

/* Range of signed 16 bits variable */
#define MAXS16B 32767
#define MINS16B -32768

/*
 * Offset values for sensor calibration
 * To obtain these values, put sensor in your chosen origin and realise a group
 * of measurements, then take the mean value for each variable (that is: AccelX
 * , AccelY, AccelZ, GyroX, GyroY and GyroZ)
 * Obs: Note that if you're using a high precision sensibility, the measures
 * will vary slightly everytime.
 */
s16_t calibration_offsets[6] = {236, -752, 20996, 75, -147, 23};

u8_t buf[14]; /* Buffer vector to store data from sensor 8 bit registers */
s16_t buf16[7]; /* Buffer vector to store measured data from sensor */

s16_t accelX, accelY, accelZ, gyroX, gyroY, gyroZ;

int map(int in, int in_max, int in_min, int out_max, int out_min) {
	int out;
	out = (in*(out_max - out_min))/(in_max - in_min) + out_min;
	return out;
}

void main(void)
{
	struct device *i2c_dev;
	printk("GY-521 Accelerometer and Gyroscope using I2C example \n");

	i2c_dev = device_get_binding(DT_NORDIC_NRF_I2C_0_LABEL);
	if (!i2c_dev) {
		printk("I2C: Device driver not found.\n");

		return;
	}

	/* Take sensor out of sleep mode */
	i2c_reg_update_byte(i2c_dev, MPU_ADDRESS,
				PWR_MGMT_1_REG, MPU6050_SLEEP_EN,
				0);

	/*
	 * Read values of 14 sequential registers responsible for storing data
	 * of acceleration, temperature and angular velocity and write it
	 * in buffer vector.
	 */
	i2c_burst_read(i2c_dev, MPU_ADDRESS, MPU6050_REG_DATA_START, buf, 14);

	/* Set the full scale range for gyroscope */
	i2c_reg_write_byte(i2c_dev, MPU_ADDRESS,
				 MPU6050_REG_GYRO_CFG, FS_SEL);

	/* Set the full scale range for accelerometer */
	i2c_reg_write_byte(i2c_dev, MPU_ADDRESS,
				 MPU6050_REG_ACCEL_CFG, AFS_SEL);

	int64_t last_toggle_time = 0;
	int i = 0;

	while (1) {

		buf16[i] = buf[2*i] << 8 | buf[2*i + 1];
		i > 6 ? i=0 : i++ ;

		/* Get current time */
		int64_t current_time = k_uptime_get();
		/* Change led state every 2 seconds */
		if (current_time - last_toggle_time > 2000){

			i2c_burst_read(i2c_dev, MPU_ADDRESS,
					 MPU6050_REG_DATA_START, buf, 14);

			accelX = map(buf16[0] - calibration_offsets[0],
						 MAXS16B, MINS16B,
						 2*GRAVITY_ACC, -2*GRAVITY_ACC);
			accelY = map(buf16[1] - calibration_offsets[1],
						 MAXS16B, MINS16B,
						 2*GRAVITY_ACC, -2*GRAVITY_ACC);
			accelZ = map(buf16[2] - calibration_offsets[2],
						 MAXS16B, MINS16B,
						 2*GRAVITY_ACC, -2*GRAVITY_ACC);
			gyroX = map(buf16[4] - calibration_offsets[3],
						 MAXS16B, MINS16B,
						 GYRO_FS_RANGE, -GYRO_FS_RANGE);
			gyroY = map(buf16[5] - calibration_offsets[4],
						 MAXS16B, MINS16B,
						 GYRO_FS_RANGE, -GYRO_FS_RANGE);
			gyroZ = map(buf16[6] - calibration_offsets[5],
						 MAXS16B, MINS16B,
						 GYRO_FS_RANGE, -GYRO_FS_RANGE);

			printk("Accel X: %d | ", accelX);
			printk("Accel Y: %d | ", accelY);
			printk("Accel Z: %d | ", accelZ);
			printk("Gyro X: %d | ", gyroX);
			printk("Gyro Y: %d | ", gyroY);
			printk("Gyro Z: %d \n ", gyroZ);

			last_toggle_time = current_time;
		}
	}
}
